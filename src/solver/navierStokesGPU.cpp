
//********************************************************************
//**    includes
//********************************************************************

#include "navierStokesGPU.h"

#include <fstream>
#include <iostream>
#include <math.h>

//********************************************************************
//**    implementation
//********************************************************************

// -------------------------------------------------
//	constructor / destructor
// -------------------------------------------------

//============================================================================
NavierStokesGPU::NavierStokesGPU ( )
{
	_pitch = 0;
	_clWorkgroupSize = 0;

	try
	{
		// create OpenCL platform
		cl::Platform::get( &_clPlatforms );
		_clPlatforms[0].getDevices( CL_DEVICE_TYPE_GPU, &_clDevices );

		// create context
		_clContext = cl::Context( _clDevices );

		// create command queue
		_clQueue = cl::CommandQueue( _clContext, _clDevices[0] );

		// define thread range
		_clRange = cl::NDRange( BW, BH );
	}
	catch( cl::Error error )
	{
		std::cerr << "CL ERROR: " << error.what() << "(" << error.err() << ")" << std::endl;
		throw error;
	}
}

//============================================================================
NavierStokesGPU::~NavierStokesGPU ( )
{
	#ifdef VERBOSE
		std::cout << "destructing NSGPU..." << std::endl;
	#endif

	// cleanup kernel source
	for( std::vector<std::string*>::iterator it = _clSourceCode.begin(); it != _clSourceCode.end(); ++it )
	{
		SAVE_DELETE( *it );
	}

	// free buffer memory
	freeHostMatrix( _U_host );
	freeHostMatrix( _V_host );
	freeHostMatrix( _P_host );
}

// -------------------------------------------------
//	initialization
// -------------------------------------------------

//============================================================================
void NavierStokesGPU::init ( )
{
	// calculate pitch
	// todo
	_pitch = _nx;

	// load and compile kernels
	loadKernels();

	//-----------------------
	// allocate memory for matrices U, V, P, RHS, F, G
	//-----------------------

	#ifdef VERBOSE
		std::cout << "allocating device buffers..." << std::endl;
	#endif

	int size = ( _nx + 2 ) * ( _ny * 2 );

	// todo: use pitched memory
	_U_g   = cl::Buffer ( _clContext, CL_MEM_READ_WRITE, sizeof(REAL) * size );
	_V_g   = cl::Buffer ( _clContext, CL_MEM_READ_WRITE, sizeof(REAL) * size );
	_P_g   = cl::Buffer ( _clContext, CL_MEM_READ_WRITE, sizeof(REAL) * size );
	_RHS_g = cl::Buffer ( _clContext, CL_MEM_READ_WRITE, sizeof(REAL) * size );
	_F_g   = cl::Buffer ( _clContext, CL_MEM_READ_WRITE, sizeof(REAL) * size );
	_G_g   = cl::Buffer ( _clContext, CL_MEM_READ_WRITE, sizeof(REAL) * size );

	//_FLAG_g


	//-----------------------
	// initialise U, V and P with given initial values (0.0 at borders)
	//-----------------------

	#ifdef VERBOSE
		std::cout << "initializing device buffers..." << std::endl;
	#endif

	// todo: border might not be neccessary

	// set kernel arguments for initialisation
	REAL initialValue = 0.0;

	_clKernels[1].setArg( 0, _U_g );
	_clKernels[1].setArg( 1, sizeof(REAL), &initialValue ); // boundary value
	_clKernels[1].setArg( 2, sizeof(REAL), &_ui ); // interior value
	_clKernels[1].setArg( 3, sizeof(int),  &_nx );
	_clKernels[1].setArg( 4, sizeof(int),  &_ny );
	_clKernels[1].setArg( 5, sizeof(int),  &_pitch );

	// call kernel
	_clQueue.enqueueNDRangeKernel (
			_clKernels[1],
			cl::NullRange,	// offset
			_clRange,		// global
			cl::NullRange	// local
		);


	// update arguments for V
	_clKernels[1].setArg( 0, _V_g );
	_clKernels[1].setArg( 1, sizeof(REAL), &_vi ); // interior value

	_clQueue.enqueueNDRangeKernel ( _clKernels[1], cl::NullRange, _clRange, cl::NullRange );

	_clKernels[1].setArg( 0, _P_g );
	_clKernels[1].setArg( 1, sizeof(REAL), &_pi ); // interior value

	_clQueue.enqueueNDRangeKernel ( _clKernels[1], cl::NullRange, _clRange, cl::NullRange );



	// initialise RHS, F and G with 0.0
	// todo: might not be neccessary

	_clKernels[0].setArg( 0, _RHS_g );
	_clKernels[0].setArg( 1, sizeof(REAL), &initialValue ); // boundary value
	_clKernels[0].setArg( 2, sizeof(int),  &_nx );
	_clKernels[0].setArg( 3, sizeof(int),  &_ny );
	_clKernels[0].setArg( 4, sizeof(int),  &_pitch );

	_clQueue.enqueueNDRangeKernel ( _clKernels[0], cl::NullRange, _clRange, cl::NullRange );

	_clKernels[0].setArg( 0, _F_g );
	_clQueue.enqueueNDRangeKernel ( _clKernels[0], cl::NullRange, _clRange, cl::NullRange );

	_clKernels[0].setArg( 0, _G_g );
	_clQueue.enqueueNDRangeKernel ( _clKernels[0], cl::NullRange, _clRange, cl::NullRange );




	#ifdef VERBOSE
		std::cout << "allocating host buffers..." << std::endl;
	#endif

	// allocate host memory for U, V and P buffers for communication
	_U_host = allocHostMatrix ( _nx + 2, _ny + 2 );
	_V_host = allocHostMatrix ( _nx + 2, _ny + 2 );
	_P_host = allocHostMatrix ( _nx + 2, _ny + 2 );


	// set kernel arguments for frequently called kernels
	setKernelArguments();

	// wait for completion
	_clQueue.finish();
}

//============================================================================
bool NavierStokesGPU::setObstacleMap
	(
		bool **map
	)
{
	int nx1 = _nx + 1;
	int ny1 = _ny + 1;
	int nx2 = _nx + 2;
	int ny2 = _ny + 2;

	//-----------------------
	// allocate memory for flag array
	//-----------------------

	// flag array is computed on CPU and copied to device memory later
	// todo: do it on GPU?
	// todo: use constant memory

	unsigned char ** flag;

	flag = (unsigned char**)malloc( ny2 * sizeof( unsigned char* ) );

	// the actual data array. allocation for all rows at once to get continuous memory
	unsigned char* data = (unsigned char*)malloc( nx2 * ny2 * sizeof( unsigned char ) );

	flag[0] = data;
	for( int i = 1; i < ny2; ++i )
	{
		flag[i] = data + i * nx2;
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
				flag[y][x] = C_F;
			}
			else
			{
				// cell is a boundary cell

				// check for invalid boundary cell (between two fluid cells)
				if( ( map[y-1][x] && map[y+1][x] ) || ( map[y][x-1] && map[y][x+1] ) )
					return false;

				// look for surrounding cells to get correct flag
				flag[y][x] = C_B
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
		flag[0][x]	= C_B
					+ B_N * map[1][x]
					+ B_S
					+ B_W
					+ B_E;

		// northern boundary
		flag[ny1][x] = C_B
					  + B_N
					  + B_S * map[_ny][x]
					  + B_W
					  + B_E;
	}

	for( int y = 1; y < ny1; ++y )
	{
		// western boundary
		flag[y][0]	= C_B
					+ B_N
					+ B_S
					+ B_W
					+ B_E * map[y][1];

		// eastern boundary
		flag[y][nx1] = C_B
					  + B_N
					  + B_S
					  + B_W * map[y][_nx]
					  + B_E;
	}

	// edge cells (not neccessary, but uninitialised cells are ugly)
	flag[0][0] = flag[0][nx1] = flag[ny1][0] = flag[ny1][nx1] = 0x0F;


	//-----------------------
	// copy to device memory
	//-----------------------

	// allocate memory and copy to device
	// todo: use pitched memory
	_FLAG_g   = cl::Buffer (
					_clContext,
					CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
					nx2 * ny2 * sizeof( unsigned char ),
					flag
				);

	delete [] flag[0];
	delete [] flag;

	return true;
}


// -------------------------------------------------
//	execution
// -------------------------------------------------

//============================================================================
void NavierStokesGPU::doSimulationStep()
{
	//-----------------------
	// get delta_t
	//-----------------------

	#ifdef VERBOSE
		std::cout << "computing Δt..." << std::endl;
	#endif

	computeDeltaT();


	//-----------------------
	// set boundary values for u and v
	//-----------------------

	#ifdef VERBOSE
		std::cout << "applying boundary conditions..." << std::endl;
	#endif

	setBoundaryConditions();

	#ifdef VERBOSE
		std::cout << "applying problem specific boundary conditions..." << std::endl;
	#endif

	setSpecificBoundaryConditions();


	//-----------------------
	// compute F(n) and G(n)
	//-----------------------

	#ifdef VERBOSE
		std::cout << "computing F and G..." << std::endl;
	#endif

	computeFG();


	//-----------------------
	// compute right hand side of pressure equation
	//-----------------------

	#ifdef VERBOSE
		std::cout << "computing right hand side of pressure equation..." << std::endl;
	#endif

	//computeRightHandSide();


	//-----------------------
	// poisson overrelaxation loop
	//-----------------------

	#ifdef VERBOSE
		std::cout << "poisson overrelaxation loop..." << std::endl;
	#endif

	REAL residual = INFINITY;

	int sor_it = 0;
	// for ( ; sor_it < _it_max && abs(residual) > _epsilon; ++sor_it )
	{
		// do SOR step (includes residual computation)
		//residual =  SORPoisson();
	}

	#ifdef VERBOSE
		std::cout << "SOR iterations: " << sor_it << " / " << _it_max << std::endl;
	#endif


	//-----------------------
	// compute U(n+1) and V(n+1)
	//-----------------------

	#ifdef VERBOSE
		std::cout << "computing U and V" << std::endl;
	#endif

	//adaptUV();
}


// -------------------------------------------------
//	data access
// -------------------------------------------------

//============================================================================
REAL **NavierStokesGPU::getU_CPU ( )
{
	// copy data from device to host
	// beware: the host array has type REAL**
	_clQueue.enqueueReadBuffer (
				_U_g,		// device buffer
				CL_TRUE,	// blocking
				0,			// offset
				sizeof(REAL) * (_nx + 2) * (_ny + 2), // size
				*_U_host	// host buffer
			);

	return _U_host;
}

//============================================================================
REAL **NavierStokesGPU::getV_CPU ( )
{
	// copy data from device to host
	// beware: the host array has type REAL**
	_clQueue.enqueueReadBuffer (
				_V_g,
				CL_TRUE,
				0,
				sizeof(REAL) * (_nx + 2) * (_ny + 2),
				*_V_host
			);

	return _V_host;
}

//============================================================================
REAL **NavierStokesGPU::getP_CPU ( )
{
	// copy data from device to host
	// beware: the host array has type REAL**
	_clQueue.enqueueReadBuffer (
				_P_g,
				CL_TRUE,
				0,
				sizeof(REAL) * (_nx + 2) * (_ny + 2),
				*_P_host
			);

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
		_clQueue.enqueueNDRangeKernel ( _clKernels[2], cl::NullRange, _clRange, cl::NullRange );

		// wait for completion
		_clQueue.finish();

		// call kernel setArbitraryBoundaryConditionsKernel
		_clQueue.enqueueNDRangeKernel ( _clKernels[3], cl::NullRange, _clRange, cl::NullRange );

		// wait for completion
		_clQueue.finish();
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
	try
	{
		// the problem specific kernel is determined during kernel compilation
		_clQueue.enqueueNDRangeKernel ( _clKernels[4], cl::NullRange, _clRange, cl::NullRange );

		// wait for completion
		_clQueue.finish();
	}
	catch( cl::Error error )
	{
		std::cerr << "CL ERROR while applying problem specific boundary conditions: " << error.what() << "(" << error.err() << ")" << std::endl;
		throw error;
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
		cl::Buffer results_g ( _clContext, CL_MEM_WRITE_ONLY, sizeof(REAL) * 2 );

		// set result buffer as kernel argument
		_clKernels[5].setArg( 2, results_g );

		// call min/max reduction kernel
		_clQueue.enqueueNDRangeKernel (
					_clKernels[5],
					cl::NullRange,
					cl::NDRange( _clWorkgroupSize ),
					cl::NDRange( _clWorkgroupSize )		// make sure that all GPU cores are in one workgroup for optimal reduction speed
				);

		// wait for completion
		_clQueue.finish();

		// retrieve reduction result
		_clQueue.enqueueReadBuffer( results_g, CL_TRUE, 0, sizeof(REAL) * 2, results );
	}
	catch( cl::Error error )
	{
		std::cerr << "CL ERROR while computing Δt: " << error.what() << "(" << error.err() << ")" << std::endl;
		throw error;
	}

	// compute the three options for the min-function
	REAL opt_a, opt_x, opt_y, min;

	opt_a = ( _re / 2.0 ) * ( 1.0 / (_dx * _dx) + 1.0 / (_dy * _dy) );
	opt_x = _dx / abs( results[0] ); // results[0] = u_max
	opt_y = _dy / abs( results[1] );// results[1] = v_max

	// get smallest value
	min = opt_a < opt_x ? opt_a : opt_x;
	min = min   < opt_y ? min   : opt_y;

	// compute delta t
	_dt = _tau * min;
}

//============================================================================
void NavierStokesGPU::computeFG ( )
{
	REAL alpha = 0.9; // todo: select alpha

	try
	{
		// set missing kernel arguments
		_clKernels[6].setArg( 5, sizeof(CL_REAL), &_dt );
		_clKernels[6].setArg( 7, sizeof(CL_REAL), &alpha );
		_clKernels[7].setArg( 5, sizeof(CL_REAL), &_dt );
		_clKernels[7].setArg( 7, sizeof(CL_REAL), &alpha );

		// todo: try combined kernel for F and G

		// call kernel for F computation
		_clQueue.enqueueNDRangeKernel ( _clKernels[6], cl::NullRange, _clRange, cl::NullRange );

		// call kernel for G computation
		_clQueue.enqueueNDRangeKernel ( _clKernels[7], cl::NullRange, _clRange, cl::NullRange );

		// wait for completion
		_clQueue.finish();


		// boundary values for arbitrary geometries
		//! TODO

	}
	catch( cl::Error error )
	{
		std::cerr << "CL ERROR while computing Δt: " << error.what() << "(" << error.err() << ")" << std::endl;
		throw error;
	}
}


// -------------------------------------------------
//	auxiliary functions
// -------------------------------------------------

//============================================================================
void NavierStokesGPU::loadKernels ( )
{
	#ifdef VERBOSE
		std::cout << "Compiling kernels..." << std::endl;
	#endif

	// cl source codes
	cl::Program::Sources source;

	// load kernels from files
	loadSource ( source, "kernels/auxiliary.cl" );
	loadSource ( source, "kernels/boundaryConditions.cl" );
	loadSource ( source, "kernels/deltaT.cl" );
	loadSource ( source, "kernels/computeFG.cl" );

	// create program
	_clProgram = cl::Program( _clContext, source );

	// compile opencl source
	try
	{
		_clProgram.build( _clDevices );
	}
	catch( cl::Error error )	//-----------------------
			// load kernels
			//-----------------------
	{
		// display kernel compile errors
		if( error.err() == CL_BUILD_PROGRAM_FAILURE )
		{
			std::cerr << "CL kernel build error:" << std::endl <<
						 _clProgram.getBuildInfo<CL_PROGRAM_BUILD_LOG>( _clDevices[0] ) << std::endl;
		}
		else
		{
			std::cerr << "CL ERROR while building kernels: " << error.err() << std::endl;
		}
		throw error;
	}

	#ifdef VERBOSE
		std::cout << "Kernels compiled" << std::endl;
	#endif

	//-----------------------
	// load kernels
	//-----------------------

	_clKernels = std::vector<cl::Kernel> ( 8 );

	#ifdef VERBOSE
		std::cout << "Binding kernels..." << std::endl;
	#endif

	try
	{
		// auxiliary kernels
		_clKernels[0] = cl::Kernel( _clProgram, "setKernel" );
		_clKernels[1] = cl::Kernel( _clProgram, "setBoundaryAndInteriorKernel" );

		// boundary condition kernels
		_clKernels[2] = cl::Kernel( _clProgram, "setBoundaryConditionsKernel" );
		_clKernels[3] = cl::Kernel( _clProgram, "setArbitraryBoundaryConditionsKernel" );

		if ( _problem == "moving_lid" )
		{
			_clKernels[4] = cl::Kernel( _clProgram, "setMovingLidBoundaryConditionsKernel" );
		}
		else if ( _problem == "left_inflow" )
		{
			_clKernels[4] = cl::Kernel( _clProgram, "setLeftInflowBoundaryConditionsKernel" );
		}

		// kernel to find maximum UV value for delta t computation
		_clKernels[5] = cl::Kernel( _clProgram, "getUVMaximumKernel" );

		// kernels for F and G computation
		_clKernels[6] = cl::Kernel( _clProgram, "computeF" );
		_clKernels[7] = cl::Kernel( _clProgram, "computeG" );
	}
	catch( cl::Error error )
	{
		std::cerr << "CL ERROR while kernel binding: " << error.what() << "(" << error.err() << ")" << std::endl;
		throw error;
	}


	// get work group size
	_clWorkgroupSize = _clKernels[0].getWorkGroupInfo< CL_KERNEL_WORK_GROUP_SIZE >( _clDevices[0] );
}

//============================================================================
void NavierStokesGPU::loadSource
	(
		cl::Program::Sources&	sources,
		std::string				fileName
	)
{
	// read file
	std::ifstream cl_file( fileName.c_str() );

	// using a pointer and new, because the strings have been deleted to soon and where damaged until compilation
	// remember to delete them after compilation or in destructor!
	std::string *cl_sourcecode = new std::string ( std::istreambuf_iterator<char>( cl_file ), (std::istreambuf_iterator<char>()) );
	_clSourceCode.push_back( cl_sourcecode );

	// add it to the source list
	sources.push_back( std::make_pair( cl_sourcecode->c_str(), cl_sourcecode->length() ) );
}

//============================================================================
void NavierStokesGPU::setKernelArguments ( )
{
	// domain size including boundaries
	int nx = _nx+2;
	int ny = _ny+2;

	#ifdef VERBOSE
		std::cout << "Setting kernel arguments..." << std::endl;
	#endif

	try
	{
		// set kernel arguments for setBoundaryConditionsKernel
		_clKernels[2].setArg( 0, _U_g );
		_clKernels[2].setArg( 1, _V_g );
		_clKernels[2].setArg( 2, sizeof(int), &_wN ); // northern boundary condition
		_clKernels[2].setArg( 3, sizeof(int), &_wE ); // eastern boundary condition
		_clKernels[2].setArg( 4, sizeof(int), &_wS ); // southern boundary condition
		_clKernels[2].setArg( 5, sizeof(int), &_wW ); // western boundary condition
		_clKernels[2].setArg( 6, sizeof(int), &nx );
		_clKernels[2].setArg( 7, sizeof(int), &ny );

		// set kernel arguments for setArbitraryBoundaryConditionsKernel
		_clKernels[3].setArg( 0, _U_g );
		_clKernels[3].setArg( 1, _V_g );
		_clKernels[3].setArg( 2, _FLAG_g ); // northern boundary condition
		_clKernels[3].setArg( 3, sizeof(int), &nx );
		_clKernels[3].setArg( 4, sizeof(int), &ny );

		// set kernel arguments for the problem specific boundary condition kernel
		_clKernels[4].setArg( 0, _U_g );
		_clKernels[4].setArg( 1, sizeof(int), &nx );
		_clKernels[4].setArg( 2, sizeof(int), &ny );

		// kernel arguments for delta t computation (UV maximum)
		_clKernels[5].setArg( 0, _U_g );
		_clKernels[5].setArg( 1, _V_g );
		// argument 2: result buffer: { REAL u_max, REAL v_max }
		_clKernels[5].setArg( 3, sizeof(CL_REAL) * _clWorkgroupSize, NULL); // dynamically allocated local shared memory for U
		_clKernels[5].setArg( 4, sizeof(CL_REAL) * _clWorkgroupSize, NULL); // dynamically allocated local shared memory for V
		_clKernels[5].setArg( 5, sizeof(int), &nx );
		_clKernels[5].setArg( 6, sizeof(int), &ny );

		// kernel arguments for F and G computation
		_clKernels[6].setArg( 0,  _U_g );
		_clKernels[6].setArg( 1,  _V_g );
		_clKernels[6].setArg( 2,  _FLAG_g );
		_clKernels[6].setArg( 3,  _F_g );
		_clKernels[6].setArg( 4,  sizeof(CL_REAL), &_gx );
		// _clKernels[6].setArg( 5, sizeof(CL_REAL), &_dt ); // set before kernel call
		_clKernels[6].setArg( 6,  sizeof(CL_REAL), &_re );
		// _clKernels[6].setArg( 7, sizeof(CL_REAL), &alpha ); // set before kernel call
		_clKernels[6].setArg( 8,  sizeof(CL_REAL), &_dx );
		_clKernels[6].setArg( 9,  sizeof(CL_REAL), &_dy );
		_clKernels[6].setArg( 10, sizeof(int), &_nx );
		_clKernels[6].setArg( 11, sizeof(int), &_ny );

		_clKernels[7].setArg( 0,  _U_g );
		_clKernels[7].setArg( 1,  _V_g );
		_clKernels[7].setArg( 2,  _FLAG_g );
		_clKernels[7].setArg( 3,  _G_g );
		_clKernels[7].setArg( 4,  sizeof(CL_REAL), &_gy );
		// _clKernels[7].setArg( 5, sizeof(CL_REAL), &_dt ); // set before kernel call
		_clKernels[7].setArg( 6,  sizeof(CL_REAL), &_re );
		// _clKernels[7].setArg( 7, sizeof(CL_REAL), &alpha ); // set before kernel call
		_clKernels[7].setArg( 8,  sizeof(CL_REAL), &_dx );
		_clKernels[7].setArg( 9,  sizeof(CL_REAL), &_dy );
		_clKernels[7].setArg( 10, sizeof(int), &_nx );
		_clKernels[7].setArg( 11, sizeof(int), &_ny );

	}
	catch( cl::Error error )
	{
		std::cerr << "CL ERROR while setting kernel arguments: " << error.what() << "(" << error.err() << ")" << std::endl;
		throw error;
	}
}
