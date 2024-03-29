
//********************************************************************
//**    includes
//********************************************************************

#include "navierStokesGPU.h"

#include <iostream>
#include <math.h>

//********************************************************************
//**    implementation
//********************************************************************

// -------------------------------------------------
//	constructor / destructor
// -------------------------------------------------

//============================================================================
NavierStokesGPU::NavierStokesGPU
	(
		Parameters* parameters,
		CLManager* clManager
	)
	: NavierStokesSolver ( parameters )
{
	_clManager = clManager;
	_clContext = clManager->getContext();
	_clQueue   = clManager->getQueue();

	_pitch = 0;

	// load and compile kernels
	_clManager->loadKernels();

	// define global thread range
	_clRange = cl::NDRange( _parameters->nx + 2, _parameters->ny + 2 );
	_clWorkgroupSize = _clManager->getWorkgroupSize();
}

//============================================================================
NavierStokesGPU::~NavierStokesGPU ( )
{
	// free buffer memory
	freeHostMatrix( _U_host );
	freeHostMatrix( _V_host );
	freeHostMatrix( _P_host );
	free( _FLAG_host[0] );
	free( _FLAG_host );
}

// -------------------------------------------------
//	initialization
// -------------------------------------------------

//============================================================================
void NavierStokesGPU::initialize ( )
{
	int nx2 = _parameters->nx + 2;
	int ny2 = _parameters->ny + 2;
	int size = ( _parameters->nx + 2 ) * ( _parameters->ny + 2 );

	// calculate pitch
	// todo
	_pitch = _parameters->nx + 2;

	//-----------------------
	// allocate memory for matrices U, V, P, RHS, F, G
	//-----------------------

	#if VERBOSE
		std::cout << "allocating device buffers..." << std::endl;
	#endif

	// TODO: use pitched memory
	// TODO: implement an allocate buffer method in the cl manager?
	_U_g   = cl::Buffer ( *_clContext, CL_MEM_READ_WRITE, sizeof(CL_REAL) * size );
	_V_g   = cl::Buffer ( *_clContext, CL_MEM_READ_WRITE, sizeof(CL_REAL) * size );
	_P_g   = cl::Buffer ( *_clContext, CL_MEM_READ_WRITE, sizeof(CL_REAL) * size );
	_RHS_g = cl::Buffer ( *_clContext, CL_MEM_READ_WRITE, sizeof(CL_REAL) * size );
	_F_g   = cl::Buffer ( *_clContext, CL_MEM_READ_WRITE, sizeof(CL_REAL) * size );
	_G_g   = cl::Buffer ( *_clContext, CL_MEM_READ_WRITE, sizeof(CL_REAL) * size );

	//_FLAG_g


	//-----------------------
	// initialise U, V and P with given initial values (0.0 at borders)
	//-----------------------

	#if VERBOSE
		std::cout << "initializing device buffers..." << std::endl;
	#endif

	// todo: border might not be neccessary

	// set kernel arguments for initialisation
	REAL initialBoundaryValue = 0.0;

	cl::Kernel* kernel = _clManager->getKernel( kernel::setBoundaryAndInterior );

	kernel->setArg( 0, _U_g );
	kernel->setArg( 1, sizeof(CL_REAL), &initialBoundaryValue ); // boundary value
	kernel->setArg( 2, sizeof(CL_REAL), &_parameters->ui ); // interior value
	kernel->setArg( 3, sizeof(int),     &nx2 );
	kernel->setArg( 4, sizeof(int),     &ny2 );
	kernel->setArg( 5, sizeof(int),     &_pitch );

	// call kernel
	_clManager->runRangeKernel ( kernel::setBoundaryAndInterior, cl::NullRange, _clRange, cl::NullRange );

	// update arguments for V
	kernel->setArg( 0, _V_g );
	kernel->setArg( 2, sizeof(CL_REAL), &_parameters->vi ); // interior value

	_clManager->runRangeKernel ( kernel::setBoundaryAndInterior, cl::NullRange, _clRange, cl::NullRange );


	kernel->setArg( 0, _P_g );
	kernel->setArg( 2, sizeof(CL_REAL), &_parameters->pi ); // interior value

	_clManager->runRangeKernel ( kernel::setBoundaryAndInterior, cl::NullRange, _clRange, cl::NullRange );


	//-----------------------
	// initialise RHS, F and G with 0.0
	//-----------------------

	// todo: might not be neccessary

	kernel = _clManager->getKernel( kernel::setKernel );

	kernel->setArg( 0, _RHS_g );
	kernel->setArg( 1, sizeof(CL_REAL), &initialBoundaryValue );
	kernel->setArg( 2, sizeof(int),  &nx2 );
	kernel->setArg( 3, sizeof(int),  &ny2 );
	kernel->setArg( 4, sizeof(int),  &_pitch );

	_clManager->runRangeKernel ( kernel::setKernel, cl::NullRange, _clRange, cl::NullRange );

	kernel->setArg( 0, _F_g );
	_clManager->runRangeKernel ( kernel::setKernel, cl::NullRange, _clRange, cl::NullRange );

	kernel->setArg( 0, _G_g );
	_clManager->runRangeKernel ( kernel::setKernel, cl::NullRange, _clRange, cl::NullRange );



	//-----------------------
	// allocate host memory for U, V and P buffers for communication
	//-----------------------

	#if VERBOSE
		std::cout << "allocating host buffers..." << std::endl;
	#endif

	_U_host = allocHostMatrix ( nx2, ny2 );
	_V_host = allocHostMatrix ( nx2, ny2 );
	_P_host = allocHostMatrix ( nx2, ny2 );


	// set kernel arguments for frequently called kernels
	setKernelArguments();

	// wait for completion
	_clQueue->finish();
}

//============================================================================
bool NavierStokesGPU::setObstacleMap
	(
		bool **map
	)
{
	int nx1 = _parameters->nx + 1;
	int ny1 = _parameters->ny + 1;
	int nx2 = _parameters->nx + 2;
	int ny2 = _parameters->ny + 2;

	// flag array is computed on CPU and then copied to device memory
	// the flag array is kept to simplify obstacle drawing at least a bit

	// todo: do it on GPU?
	// todo: use constant memory


	//-----------------------
	// allocate memory for flag array
	//-----------------------

	_FLAG_host    = (unsigned char**)malloc( ny2 * sizeof( unsigned char* ) );
	_FLAG_host[0] = (unsigned char*)malloc( nx2 * ny2 * sizeof( unsigned char ) );

	for( int i = 1; i < ny2; ++i )
	{
		_FLAG_host[i] = *_FLAG_host + i * nx2;
	}


	//-----------------------
	// create geometry map
	//-----------------------

	// Domain boundary cells are treated like interior boundary cells.

	// compute interior cells
	for ( int y = 1; y < ny1; ++y )
	{
		for ( int x = 1; x < nx1; ++x )
		{
			if( map[y][x] )
			{
				// cell is a fluid cell
				// neighbour cells do not matter
				_FLAG_host[y][x] = C_F;
			}
			else
			{
				// cell is a boundary cell

				// check for invalid boundary cell (between two fluid cells)
				// TODO: do that in the parameter parser
				if( ( map[y-1][x] && map[y+1][x] ) || ( map[y][x-1] && map[y][x+1] ) )
					return false;

				// look for surrounding cells to get correct flag
				_FLAG_host[y][x] = C_B
						+ B_N * map[y+1][x]
						+ B_S * map[y-1][x]
						+ B_W * map[y][x-1]
						+ B_E * map[y][x+1];
			}
		}
	}

	// compute boundary cells
	for( int x = 1; x < nx1; ++x )
	{
		// southern boundary
		_FLAG_host[0][x] = C_B
					+ B_N * map[1][x]
					+ B_S
					+ B_W
					+ B_E;

		// northern boundary
		_FLAG_host[ny1][x] = C_B
					  + B_N
					  + B_S * map[_parameters->ny][x]
					  + B_W
					  + B_E;
	}

	for( int y = 1; y < ny1; ++y )
	{
		// western boundary
		_FLAG_host[y][0] = C_B
					+ B_N
					+ B_S
					+ B_W
					+ B_E * map[y][1];

		// eastern boundary
		_FLAG_host[y][nx1] = C_B
					  + B_N
					  + B_S
					  + B_W * map[y][_parameters->nx]
					  + B_E;
	}

	// edge cells (not neccessary, but uninitialised memory is ugly)
	_FLAG_host[0][0] = _FLAG_host[0][nx1] = _FLAG_host[ny1][0] = _FLAG_host[ny1][nx1] = 0x0F;


	//-----------------------
	// copy to device memory
	//-----------------------

	// allocate memory and copy to device
	// todo: use pitched memory
	_FLAG_g   = cl::Buffer (
					*_clContext,
					CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
					nx2 * ny2 * sizeof( unsigned char ),
					*_FLAG_host
				);

	return true;
}


// -------------------------------------------------
//	execution
// -------------------------------------------------

//============================================================================
int NavierStokesGPU::doSimulationStep()
{
	//-----------------------
	// get delta_t
	//-----------------------

	#if VERBOSE
		std::cout << "computing Δt..." << std::endl;
	#endif

	computeDeltaT();


	//-----------------------
	// set boundary values for u and v
	//-----------------------

	#if VERBOSE
		std::cout << "applying boundary conditions..." << std::endl;
	#endif

	setBoundaryConditions();

	#if VERBOSE
		std::cout << "applying problem specific boundary conditions..." << std::endl;
	#endif

	setSpecificBoundaryConditions();


	//-----------------------
	// compute F(n) and G(n)
	//-----------------------

	#if VERBOSE
		std::cout << "computing F and G..." << std::endl;
	#endif

	computeFG();


	//-----------------------
	// compute right hand side of pressure equation
	//-----------------------

	#if VERBOSE
		std::cout << "computing right hand side of pressure equation..." << std::endl;
	#endif

	computeRightHandSide();


	//-----------------------
	// poisson overrelaxation loop
	//-----------------------

	#if VERBOSE
		std::cout << "poisson overrelaxation loop..." << std::endl;
	#endif

	REAL residual = INFINITY;

	int sor_iterations = 0;
	for ( ; sor_iterations < _parameters->it_max && fabs( residual ) > _parameters->epsilon; ++sor_iterations )
	{
		// do SOR step (includes residual computation)
		residual =  SORPoisson();
	}

	#if VERBOSE
		std::cout << "SOR iterations: " << sor_it << " / " << _parameters->it_max << std::endl;
	#endif


	//-----------------------
	// compute U(n+1) and V(n+1)
	//-----------------------

	#if VERBOSE
		std::cout << "computing U and V" << std::endl;
	#endif

	adaptUV();

	return sor_iterations;
}


// -------------------------------------------------
//	interaction
// -------------------------------------------------

//============================================================================
void NavierStokesGPU::drawObstacles
	(
		int x0,
		int y0,
		int x1,
		int y1,
		bool delete_flag
	)
{
	if( delete_flag )
	{
		std::cout << "obstacle removing not implemented yet!" << std::endl;
		//_parameters->obstacleMap[y][x] = true;
	}
	else
	{
		//-----------------------
		// draw line
		//-----------------------

		// using bresenham's algorithm

		int dx    = abs( x1 - x0 );		// difference in x direction
		int dy    = abs( y1 - y0 );		// difference in y direction
		int sx    = x0 < x1 ? 1 : -1;	// define step direction
		int sy    = y0 < y1 ? 1 : -1;	// define step direction
		int error = dx - dy;			// initial error value
		int e2;

		while( true )
		{
			//-----------------------
			// update obstacle flags
			//-----------------------

			// the host memory arrays can be used, as this drawing takes place
			// between visualization and simulation and host and device memory
			// contents are currently identical

			// south west corner of painted square
			_parameters->obstacleMap[y0][x0] = false;
			_FLAG_host[y0][x0] = C_B
					+ B_N
					+ B_S * ( y0 > 1 ? _parameters->obstacleMap[y0-1][x0] : 1 )
					+ B_W * ( x0 > 1 ? _parameters->obstacleMap[y0][x0-1] : 1 )
					+ B_E;

			// south east corner
			_parameters->obstacleMap[y0][x0+1] = false;
			_FLAG_host[y0][x0+1] = C_B
					+ B_N
					+ B_S * ( y0 > 1 ? _parameters->obstacleMap[y0-1][x0+1] : 1 )
					+ B_W
					+ B_E * _parameters->obstacleMap[y0][x0+2];

			// north west corner
			_parameters->obstacleMap[y0+1][x0] = false;
			_FLAG_host[y0+1][x0] = C_B
					+ B_N * _parameters->obstacleMap[y0+2][x0]
					+ B_S
					+ B_W * ( x0 > 1 ? _parameters->obstacleMap[y0+1][x0-1] : 1 )
					+ B_E;

			// north east corner
			_parameters->obstacleMap[y0+1][x0+1] = false;
			_FLAG_host[y0+1][x0+1] = C_B
					+ B_N
					+ B_S * _parameters->obstacleMap[y0+2][x0+1]
					+ B_W
					+ B_E * _parameters->obstacleMap[y0+1][x0+2];


			//-----------------------
			// reset velocities
			//-----------------------

			_U_host[y0][x0]     = _V_host[y0][x0]     = 0.0;
			_U_host[y0][x0+1]   = _V_host[y0][x0+1]   = 0.0;
			_U_host[y0+1][x0]   = _V_host[y0+1][x0]   = 0.0;
			_U_host[y0+1][x0+1] = _V_host[y0+1][x0+1] = 0.0;

			// without reseting the results of the surrounding cells
			// the results are quite unphysical
			if( y0 > 1 )
			{
				_U_host[y0-1][x0]   = _V_host[y0-1][x0]   = 0.0;
				_U_host[y0-1][x0+1] = _V_host[y0-1][x0+1] = 0.0;
			}
			_U_host[y0+2][x0]   = _V_host[y0+2][x0]   = 0.0;
			_U_host[y0+2][x0+1] = _V_host[y0+2][x0+1] = 0.0;

			if( x0 > 1 )
			{
				_U_host[y0][x0-1]   = _V_host[y0][x0-1]   = 0.0;
				_U_host[y0+1][x0-1] = _V_host[y0+1][x0-1] = 0.0;
			}

			_U_host[y0][x0+2]   = _V_host[y0][x0+2]   = 0.0;
			_U_host[y0+1][x0+2] = _V_host[y0+1][x0+2] = 0.0;


			//-----------------------
			// reset pressure
			//-----------------------

			_P_host[y0][x0]     = 0.0;
			_P_host[y0][x0+1]   = 0.0;
			_P_host[y0+1][x0]   = 0.0;
			_P_host[y0+1][x0+1] = 0.0;


			//-----------------------
			// next line step
			//-----------------------

			if( x0 == x1 && y0 == y1 )
			{
				break;
			}

			e2 = error * 2;

			if( e2 > -dy )
			{
				error = error - dy;
				x0 = x0 + sx;
			}

			if( e2 < dx )
			{
				error = error + dx;
				y0 = y0 + sy;
			}
		}


		//-----------------------
		// copy to device
		//-----------------------

		// copy flags, velocities and pressure to device memory

		/*cl::size_t<3> origin, region;

		origin[0] = x;
		origin[1] = y;
		origin[2] = 0;

		region[0] = 2;
		region[1] = 2;
		region[2] = 0;*/

		cl::Event event;

		/*_clQueue->enqueueWriteBufferRect(
				_FLAG_g,				// target buffer
				CL_TRUE,					// blocking write
				origin,					// offset in target buffer
				origin,					// offset in source buffer
				region,					// dimensions of region to copy
				_parameters->nx + 2,	// buffer row pitch
				0,						// buffer slice pitch (3D)
				_parameters->nx + 2,	// host row pitch
				0,						// host slice pitch (3D)
				*_FLAG_host,				// pointer to host source memory
				NULL,
				&event
			);*/

		// TODO: use writeBufferRect instead of copy whole buffers
		_clQueue->enqueueWriteBuffer(
				_FLAG_g,
				CL_TRUE,
				0,
				(_parameters->nx + 2) * (_parameters->ny + 2) * sizeof( unsigned char ),
				*_FLAG_host,
				NULL,
				&event
			);
		_clQueue->enqueueWriteBuffer(
				_U_g,
				CL_TRUE,
				0,
				(_parameters->nx + 2) * (_parameters->ny + 2) * sizeof( CL_REAL ),
				*_U_host,
				NULL,
				&event
			);
		_clQueue->enqueueWriteBuffer(
				_V_g,
				CL_TRUE,
				0,
				(_parameters->nx + 2) * (_parameters->ny + 2) * sizeof( CL_REAL ),
				*_V_host,
				NULL,
				&event
			);
		_clQueue->enqueueWriteBuffer(
				_P_g,
				CL_TRUE,
				0,
				(_parameters->nx + 2) * (_parameters->ny + 2) * sizeof( CL_REAL ),
				*_P_host,
				NULL,
				&event
			);

		event.wait();
	}
}


// -------------------------------------------------
//	data access
// -------------------------------------------------

//============================================================================
REAL **NavierStokesGPU::getU_CPU ( )
{
	// copy data from device to host
	// beware: the host array has type REAL**
	_clQueue->enqueueReadBuffer(
				_U_g,		// device buffer
				CL_TRUE,	// blocking
				0,			// offset
				sizeof(CL_REAL) * (_parameters->nx + 2) * (_parameters->ny + 2), // size
				*_U_host	// host buffer
			);

	_clQueue->finish();

	return _U_host;
}

//============================================================================
REAL **NavierStokesGPU::getV_CPU ( )
{
	// copy data from device to host
	// beware: the host array has type REAL**
	_clQueue->enqueueReadBuffer (
				_V_g,
				CL_TRUE,
				0,
				sizeof(CL_REAL) * (_parameters->nx + 2) * (_parameters->ny + 2),
				*_V_host
			);

	_clQueue->finish();

	return _V_host;
}

//============================================================================
REAL **NavierStokesGPU::getP_CPU ( )
{
	// copy data from device to host
	// beware: the host array has type REAL**
	_clQueue->enqueueReadBuffer (
				_P_g,
				CL_TRUE,
				0,
				sizeof(CL_REAL) * (_parameters->nx + 2) * (_parameters->ny + 2),
				*_P_host
			);

	_clQueue->finish();

	return _P_host;
}


// -------------------------------------------------
//	boundaries
// -------------------------------------------------

//============================================================================
void NavierStokesGPU::setBoundaryConditions ( )
{
	try
	{
		// kernel arguments are set in setKernelArguments()

		// call kernel setBoundaryConditionsKernel
		_clManager->runRangeKernel ( kernel::setBoundaryConditions, cl::NullRange, _clRange, cl::NullRange );

		// wait for completion
		_clQueue->finish();

		// call kernel setArbitraryBoundaryConditionsKernel
		_clManager->runRangeKernel ( kernel::setArbitraryBoundaryConditions, cl::NullRange, _clRange, cl::NullRange );

		// wait for completion
		_clQueue->finish();
	}
	catch( cl::Error error )
	{
		std::cerr << "CL ERROR while applying boundary conditions: " << error.what() << "(" << error.err() << ")" << std::endl;
		throw error;
	}
}

//============================================================================
void NavierStokesGPU::setSpecificBoundaryConditions ( )
{
	if ( _parameters->problem == "moving_lid" || _parameters->problem == "channel" )
	{
		try
		{
			// the problem specific kernel is determined during kernel compilation
			_clManager->runRangeKernel ( kernel::problemSpecific, cl::NullRange, _clRange, cl::NullRange );

			// wait for completion
			_clQueue->finish();
		}
		catch( cl::Error error )
		{
			std::cerr << "CL ERROR while applying problem specific boundary conditions: " << error.what() << "(" << error.err() << ")" << std::endl;
			throw error;
		}
	}
}


// -------------------------------------------------
//	simulation
// -------------------------------------------------

//============================================================================
void NavierStokesGPU::computeDeltaT ( )
{
	// results of UV reduction kernel: { max_u, max_v }
	REAL results[2] = { 0.0, 0.0 };

	try
	{
		// allocate memory for UV maximum result
		// todo: move to constructor?
		cl::Buffer results_g ( *_clContext, CL_MEM_WRITE_ONLY, sizeof(CL_REAL) * 2 );

		// set result buffer as kernel argument
		_clManager->getKernel( kernel::getUVMaximum )->setArg( 2, results_g );

		// call min/max reduction kernel
		// todo: determine optimal work size N: N = x^2, N<=max_work_size, SIZE<=max_work_size ? N>=SIZE
		_clManager->runRangeKernel (
					kernel::getUVMaximum,
					cl::NullRange,
					cl::NDRange( _clWorkgroupSize ),
					cl::NDRange( _clWorkgroupSize )		// make sure that all GPU cores are in one workgroup for optimal reduction speed
				);

		// wait for completion
		_clQueue->finish();

		// retrieve reduction result
		_clQueue->enqueueReadBuffer( results_g, CL_TRUE, 0, sizeof(CL_REAL) * 2, results );
	}
	catch( cl::Error error )
	{
		std::cerr << "CL ERROR while computing Δt: " << error.what() << "(" << error.err() << ")" << std::endl;
		throw error;
	}

	// compute the three options for the min-function
	REAL opt_a, opt_x, opt_y, min;

	opt_a =   ( _parameters->re / 2.0 )
			* 1.0 / (
				  1.0 / (_parameters->dx * _parameters->dx)
				+ 1.0 / (_parameters->dy * _parameters->dy)
			);
	opt_x = _parameters->dx / fabs( results[0] ); // results[0] = u_max
	opt_y = _parameters->dy / fabs( results[1] ); // results[1] = v_max

	// get smallest value
	min = opt_a < opt_x ? opt_a : opt_x;
	min = min   < opt_y ? min   : opt_y;

	// compute delta t
	_parameters->dt = _parameters->tau * min;
}

//============================================================================
void NavierStokesGPU::computeFG ( )
{
	try
	{
		// set missing kernel arguments
		_clManager->getKernel( kernel::computeF )->setArg( 5, sizeof(CL_REAL), &_parameters->dt );
		_clManager->getKernel( kernel::computeG )->setArg( 5, sizeof(CL_REAL), &_parameters->dt );

		// todo: try combined kernel for F and G

		// call kernel for F computation
		_clManager->runRangeKernel ( kernel::computeF, cl::NullRange, _clRange, cl::NullRange );

		// call kernel for G computation
		_clManager->runRangeKernel ( kernel::computeG, cl::NullRange, _clRange, cl::NullRange );

		// wait for completion
		_clQueue->finish();
	}
	catch( cl::Error error )
	{
		std::cerr << "CL ERROR while computing Δt: " << error.what() << "(" << error.err() << ")" << std::endl;
		throw error;
	}
}

//============================================================================
void NavierStokesGPU::computeRightHandSide ( )
{
	try
	{
		// set missing kernel arguments
		_clManager->getKernel( kernel::rightHandSide )->setArg( 3, sizeof(CL_REAL), &_parameters->dt );

		// call kernel for RHS computation
		_clManager->runRangeKernel ( kernel::rightHandSide, cl::NullRange, _clRange, cl::NullRange );

		// wait for completion
		_clQueue->finish();
	}
	catch( cl::Error error )
	{
		std::cerr << "CL ERROR while computing RHS: " << error.what() << "(" << error.err() << ")" << std::endl;
		throw error;
	}
}

//============================================================================
REAL NavierStokesGPU::SORPoisson()
{
	// this solver does not use overrelaxation for gauß-seidel,
	// as it may not converge with the symmetrical red/black parallelisation
	// this is the same as setting omega to 1.0

	// the epsilon-parameters in formula 3.44 are set to 1.0 according to page 38

	REAL residual = 0.0;

	try
	{
		//-----------------------
		// gauß seidel step
		//-----------------------

		int red = 0;

		// call gaussSeidelRedBlackKernel
		// todo: use correct range and offset for kernel call to exclude boundaries

		// set red flag as kernel argument
		_clManager->getKernel( kernel::gaussSeidelRedBlack )->setArg( 5, sizeof(int), &red );

		// call kernel for black cells
		_clManager->runRangeKernel ( kernel::gaussSeidelRedBlack, cl::NullRange, _clRange, cl::NullRange );

		// wait for completion
		_clQueue->finish();

		// set flag for red cells
		red = 1;
		_clManager->getKernel( kernel::gaussSeidelRedBlack )->setArg( 5, sizeof(int), &red );

		// call kernel for red cells
		_clManager->runRangeKernel ( kernel::gaussSeidelRedBlack, cl::NullRange, _clRange, cl::NullRange );

		// wait for completion
		_clQueue->finish();


		//-----------------------
		// boundary values
		//-----------------------

		// call pressureBoundaryConditionsKernel
		// todo: use better range (1D wit max(nx,ny))

		_clManager->runRangeKernel ( kernel::pressureBoundaryConditions, cl::NullRange, _clRange, cl::NullRange );

		// wait for completion
		_clQueue->finish();


		//-----------------------
		// residual
		//-----------------------

		// allocate output buffer
		// todo: move to constructor?
		cl::Buffer result_g ( *_clContext, CL_MEM_WRITE_ONLY, sizeof(CL_REAL) );
		REAL result = 0.0;

		// set output buffer as kernel argument
		_clManager->getKernel( kernel::pressureResidualReduction )->setArg( 3, result_g );

		// call pressureResidualReductionKernel
		// todo: determine optimal work size N: N = x^2, N<=max_work_size, SIZE<=max_work_size ? N>=SIZE
		_clManager->runRangeKernel (
						kernel::pressureResidualReduction,
						cl::NullRange,
						cl::NDRange( _clWorkgroupSize ),
						cl::NDRange( _clWorkgroupSize )		// make sure that all GPU cores are in one workgroup for optimal reduction speed
					);

		// wait for completion
		_clQueue->finish();

		// get result
		_clQueue->enqueueReadBuffer( result_g, CL_TRUE, 0, sizeof(CL_REAL) , &result );

		// compute residual
		residual = sqrt( result / (_parameters->nx * _parameters->ny) );
	}
	catch( cl::Error error )
	{
		std::cerr << "CL ERROR during pressure iteration: " << error.what() << "(" << error.err() << ")" << std::endl;
		throw error;
	}

	return residual;
}

//============================================================================
void NavierStokesGPU::adaptUV ( )
{
	try
	{
		// set missing kernel arguments
		_clManager->getKernel( kernel::updateUV )->setArg( 6, sizeof(CL_REAL), &_parameters->dt );

		// call kernel for RHS computation
		_clManager->runRangeKernel ( kernel::updateUV, cl::NullRange, _clRange, cl::NullRange );

		// wait for completion
		_clQueue->finish();
	}
	catch( cl::Error error )
	{
		std::cerr << "CL ERROR while updating UV: " << error.what() << "(" << error.err() << ")" << std::endl;
		throw error;
	}
}


// -------------------------------------------------
//	auxiliary functions
// -------------------------------------------------

//============================================================================
void NavierStokesGPU::setKernelArguments ( )
{
	// domain size including boundaries
	int nx = _parameters->nx + 2;
	int ny = _parameters->ny + 2;

	REAL alphaFG = 0.9; // TODO: select

	// constant values for pressure equation
	float dx2 = 1.0 / (_parameters->dx * _parameters->dx);
	float dy2 =	1.0 / (_parameters->dy * _parameters->dy);

	//REAL constant_expr = 1.0 / ( 2.0 * dx2 + 2.0 * dy2 );
	REAL constant_expr = _parameters->omega / ( 2.0 * dx2 + 2.0 * dy2 );
	REAL omega = 1.0 - _parameters->omega;

	#if VERBOSE
		std::cout << "Setting kernel arguments..." << std::endl;
	#endif

	try
	{
		cl::Kernel* kernel;

		// set kernel arguments for setBoundaryConditionsKernel
		kernel = _clManager->getKernel( kernel::setBoundaryConditions );
		kernel->setArg( 0, _U_g );
		kernel->setArg( 1, _V_g );
		kernel->setArg( 2, sizeof(int), &(_parameters->wN) ); // northern boundary condition
		kernel->setArg( 3, sizeof(int), &(_parameters->wE) ); // eastern boundary condition
		kernel->setArg( 4, sizeof(int), &(_parameters->wS) ); // southern boundary condition
		kernel->setArg( 5, sizeof(int), &(_parameters->wW) ); // western boundary condition
		kernel->setArg( 6, sizeof(int), &nx );
		kernel->setArg( 7, sizeof(int), &ny );

		// set kernel arguments for setArbitraryBoundaryConditionsKernel
		kernel = _clManager->getKernel( kernel::setArbitraryBoundaryConditions );
		kernel->setArg( 0, _U_g );
		kernel->setArg( 1, _V_g );
		kernel->setArg( 2, _FLAG_g ); // northern boundary condition
		kernel->setArg( 3, sizeof(int), &nx );
		kernel->setArg( 4, sizeof(int), &ny );

		// set kernel arguments for the problem specific boundary condition kernel
		if ( _parameters->problem == "moving_lid" || _parameters->problem == "channel" )
		{
			kernel = _clManager->getKernel( kernel::problemSpecific );
			kernel->setArg( 0, _U_g );
			kernel->setArg( 1, sizeof(int), &nx );
			kernel->setArg( 2, sizeof(int), &ny );
		}

		// kernel arguments for delta t computation (UV maximum)
		kernel = _clManager->getKernel( kernel::getUVMaximum );
		kernel->setArg( 0, _U_g );
		kernel->setArg( 1, _V_g );
		// argument 2: result buffer: { REAL u_max, REAL v_max }
		kernel->setArg( 3, sizeof(CL_REAL) * _clWorkgroupSize, NULL); // dynamically allocated local shared memory for U
		kernel->setArg( 4, sizeof(CL_REAL) * _clWorkgroupSize, NULL); // dynamically allocated local shared memory for V
		kernel->setArg( 5, sizeof(int), &nx );
		kernel->setArg( 6, sizeof(int), &ny );

		// kernel arguments for F and G computation
		kernel = _clManager->getKernel( kernel::computeF );
		kernel->setArg( 0,  _U_g );
		kernel->setArg( 1,  _V_g );
		kernel->setArg( 2,  _FLAG_g );
		kernel->setArg( 3,  _F_g );
		kernel->setArg( 4,  sizeof(CL_REAL), &(_parameters->gx) );
		// kernel->setArg( 5, sizeof(CL_REAL), &_dt ); // set before kernel call
		kernel->setArg( 6,  sizeof(CL_REAL), &(_parameters->re) );
		kernel->setArg( 7,  sizeof(CL_REAL), &alphaFG );
		kernel->setArg( 8,  sizeof(CL_REAL), &(_parameters->dx) );
		kernel->setArg( 9,  sizeof(CL_REAL), &(_parameters->dy) );
		kernel->setArg( 10, sizeof(int), &nx );
		kernel->setArg( 11, sizeof(int), &ny );

		kernel = _clManager->getKernel( kernel::computeG );
		kernel->setArg( 0,  _U_g );
		kernel->setArg( 1,  _V_g );
		kernel->setArg( 2,  _FLAG_g );
		kernel->setArg( 3,  _G_g );
		kernel->setArg( 4,  sizeof(CL_REAL), &(_parameters->gy) );
		// kernel->setArg( 5, sizeof(CL_REAL), &_dt ); // set before kernel call
		kernel->setArg( 6,  sizeof(CL_REAL), &(_parameters->re) );
		kernel->setArg( 7,  sizeof(CL_REAL), &alphaFG );
		kernel->setArg( 8,  sizeof(CL_REAL), &(_parameters->dx) );
		kernel->setArg( 9,  sizeof(CL_REAL), &(_parameters->dy) );
		kernel->setArg( 10, sizeof(int), &nx );
		kernel->setArg( 11, sizeof(int), &ny );

		// kernel arguments for RHS computation
		kernel = _clManager->getKernel( kernel::rightHandSide );
		kernel->setArg( 0, _F_g );
		kernel->setArg( 1, _G_g );
		kernel->setArg( 2, _RHS_g );
		// kernel->setArg( 3, sizeof(CL_REAL), &_dt ); // set before kernel call
		kernel->setArg( 4, sizeof(CL_REAL), &(_parameters->dx) );
		kernel->setArg( 5, sizeof(CL_REAL), &(_parameters->dy) );
		kernel->setArg( 6, sizeof(int), &nx );
		kernel->setArg( 7, sizeof(int), &ny );

		// kernel arguments for gauß seidel step in pressure solving
		kernel = _clManager->getKernel( kernel::gaussSeidelRedBlack );
		kernel->setArg( 0, _P_g );
		kernel->setArg( 1, _FLAG_g );
		kernel->setArg( 2, _RHS_g );
		kernel->setArg( 3, sizeof(CL_REAL), &dx2 );
		kernel->setArg( 4, sizeof(CL_REAL), &dy2 );
		// kernel->setArg( 5, sizeof(int), &red ); // red/black flag, set before kernel call
		kernel->setArg( 6, sizeof(CL_REAL), &constant_expr );
		kernel->setArg( 7, sizeof(CL_REAL), &omega );
		kernel->setArg( 8, sizeof(int), &nx );
		kernel->setArg( 9, sizeof(int), &ny );

		// kernel arguments for updating pressure boundary conditions
		kernel = _clManager->getKernel( kernel::pressureBoundaryConditions );
		kernel->setArg( 0, _P_g );
		// kernel->setArg( 1, sizeof(int), &problemId ); // todo: id of the problem
		kernel->setArg( 1, sizeof(int), &nx );
		kernel->setArg( 2, sizeof(int), &ny );

		// kernel arguments for pressure iteration residual computation
		kernel = _clManager->getKernel( kernel::pressureResidualReduction );
		kernel->setArg( 0, _P_g );
		kernel->setArg( 1, _RHS_g );
		kernel->setArg( 2, _FLAG_g );
		// argument 3: result buffer: REAL sum
		kernel->setArg( 4, sizeof(CL_REAL) * _clWorkgroupSize, NULL); // dynamically allocated local shared memory for reduction
		kernel->setArg( 5, sizeof(CL_REAL), &dx2 );
		kernel->setArg( 6, sizeof(CL_REAL), &dy2 );
		kernel->setArg( 7, sizeof(int), &nx );
		kernel->setArg( 8, sizeof(int), &ny );

		// kernel arguments for UV update
		kernel = _clManager->getKernel( kernel::updateUV );
		kernel->setArg( 0,  _P_g );
		kernel->setArg( 1,  _F_g );
		kernel->setArg( 2,  _G_g );
		kernel->setArg( 3,  _FLAG_g );
		kernel->setArg( 4,  _U_g );
		kernel->setArg( 5,  _V_g );
		// kernel->setArg( 6, sizeof(CL_REAL), &_dt ); // set before kernel call
		kernel->setArg( 7,  sizeof(CL_REAL), &(_parameters->dx) );
		kernel->setArg( 8,  sizeof(CL_REAL), &(_parameters->dy) );
		kernel->setArg( 9,  sizeof(int), &nx );
		kernel->setArg( 10, sizeof(int), &ny );
	}
	catch( cl::Error error )
	{
		std::cerr << "CL ERROR while setting kernel arguments: " << error.what() << "(" << error.err() << ")" << std::endl;
		throw error;
	}
}
