
//********************************************************************
//**    includes
//********************************************************************

#include "navierStokesGPU.h"

#include <fstream>
#include <iostream>
#include <math.h>

// thread block dimensions
#define BW 16
#define BH 16

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
	catch( cl::Error e )
	{
		//std::cerr << "ERROR: " << e.what() << "(" << e.err() << ")" << std::endl;
	}
}

//============================================================================
NavierStokesGPU::~NavierStokesGPU ( )
{
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

	// allocate memory for matrices U, V, P, RHS, F, G

	int size = ( _nx + 2 ) * ( _ny * 2 );

	// todo: use pitched memory
	_U_g   = cl::Buffer ( _clContext, CL_MEM_READ_WRITE, sizeof(int) * size );
	_V_g   = cl::Buffer ( _clContext, CL_MEM_READ_WRITE, sizeof(int) * size );
	_P_g   = cl::Buffer ( _clContext, CL_MEM_READ_WRITE, sizeof(int) * size );
	_RHS_g = cl::Buffer ( _clContext, CL_MEM_READ_WRITE, sizeof(int) * size );
	_F_g   = cl::Buffer ( _clContext, CL_MEM_READ_WRITE, sizeof(int) * size );
	_G_g   = cl::Buffer ( _clContext, CL_MEM_READ_WRITE, sizeof(int) * size );

	//_FLAG_g


	// initialise U, V and P with given initial values (0.0 at borders)
	// todo: border might not be neccessary

	// set kernel arguments for initialisation
	float initialValue = 0.0;

	_clKernels[1].setArg( 0, _U_g );
	_clKernels[1].setArg( 1, sizeof(float), &initialValue ); // boundary value
	_clKernels[1].setArg( 2, sizeof(float), &_ui ); // interior value
	_clKernels[1].setArg( 3, sizeof(int),   &_nx );
	_clKernels[1].setArg( 4, sizeof(int),   &_ny );
	_clKernels[1].setArg( 5, sizeof(int),   &_pitch );

	// call kernel
	_clQueue.enqueueNDRangeKernel (
			_clKernels[1],
			cl::NullRange,	// offset
			_clRange,		// global
			cl::NullRange	// local
		);


	// update arguments for V
	_clKernels[1].setArg( 0, _V_g );
	_clKernels[1].setArg( 1, sizeof(float), &_vi ); // interior value

	_clQueue.enqueueNDRangeKernel ( _clKernels[1], cl::NullRange, _clRange, cl::NullRange );

	_clKernels[1].setArg( 0, _P_g );
	_clKernels[1].setArg( 1, sizeof(float), &_pi ); // interior value

	_clQueue.enqueueNDRangeKernel ( _clKernels[1], cl::NullRange, _clRange, cl::NullRange );


	// initialise RHS, F and G with 0.0
	// todo: might not be neccessary

	_clKernels[0].setArg( 0, _RHS_g );
	_clKernels[0].setArg( 1, sizeof(float), &initialValue ); // boundary value
	_clKernels[0].setArg( 2, sizeof(int),   &_nx );
	_clKernels[0].setArg( 3, sizeof(int),   &_ny );
	_clKernels[0].setArg( 4, sizeof(int),   &_pitch );

	_clQueue.enqueueNDRangeKernel ( _clKernels[0], cl::NullRange, _clRange, cl::NullRange );

	_clKernels[0].setArg( 0, _F_g );
	_clQueue.enqueueNDRangeKernel ( _clKernels[0], cl::NullRange, _clRange, cl::NullRange );

	_clKernels[0].setArg( 0, _G_g );
	_clQueue.enqueueNDRangeKernel ( _clKernels[0], cl::NullRange, _clRange, cl::NullRange );






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
	// get delta_t
	computeDeltaT();

	// set boundary values for u and v
	setBoundaryConditions();

	setSpecificBoundaryConditions();

	// compute F(n) and G(n)
	//computeFG();

	// compute right hand side of pressure equation
	//computeRightHandSide();

	// poisson overrelaxation loop
	double residual = INFINITY;

	for ( int it = 0; it < _it_max && abs(residual) > _epsilon; ++it )
	{
		// do SOR step (includes residual computation)
		//residual =  SORPoisson();
	}

	// compute U(n+1) and V(n+1)
	//adaptUV();
}


// -------------------------------------------------
//	data access
// -------------------------------------------------

//============================================================================
double **NavierStokesGPU::getU_CPU ( )
{
	// copy data from device to host
	_clQueue.enqueueReadBuffer (
				_U_g,		// device buffer
				CL_TRUE,	// blocking
				0,			// offset
				sizeof(float) * (_nx + 2) * (_ny + 2), // size
				_U_host		// host buffer
			);

	return _U_host;
}

//============================================================================
double **NavierStokesGPU::getV_CPU ( )
{
	// copy data from device to host
	_clQueue.enqueueReadBuffer (
				_V_g,
				CL_TRUE,
				0,
				sizeof(float) * (_nx + 2) * (_ny + 2),
				_V_host
			);

	return _V_host;
}

//============================================================================
double **NavierStokesGPU::getP_CPU ( )
{
	// copy data from device to host
	_clQueue.enqueueReadBuffer (
				_P_g,
				CL_TRUE,
				0,
				sizeof(float) * (_nx + 2) * (_ny + 2),
				_P_host
			);

	return _P_host;
}


// -------------------------------------------------
//	boundaries
// -------------------------------------------------

//============================================================================
void NavierStokesGPU::setBoundaryConditions ( )
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

//============================================================================
void NavierStokesGPU::setSpecificBoundaryConditions ( )
{
	// the problem specific kernel is determined during kernel compilation
	_clQueue.enqueueNDRangeKernel ( _clKernels[4], cl::NullRange, _clRange, cl::NullRange );

	// wait for completion
	_clQueue.finish();
}


// -------------------------------------------------
//	simulation
// -------------------------------------------------

//============================================================================
void NavierStokesGPU::computeDeltaT ( )
{
	// allocate memory for UV maximum result
	// todo: move to constructor?
	cl::Buffer results_g ( _clContext, CL_MEM_WRITE_ONLY, sizeof(float) * 2 );

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
	float results[2];
	_clQueue.enqueueReadBuffer( results_g, CL_TRUE, 0, sizeof(float) * 2, results );

	// compute the three options for the min-function
	double opt_a, opt_x, opt_y, min;

	opt_a = ( _re / 2.0 ) * ( 1.0 / (_dx * _dx) + 1.0 / (_dy * _dy) );
	opt_x = _dx / abs( results[0] ); // results[0] = u_max
	opt_y = _dy / abs( results[1] );// results[1] = v_max

	// get smallest value
	min = opt_a < opt_x ? opt_a : opt_x;
	min = min   < opt_y ? min   : opt_y;

	// compute delta t
	_dt = _tau * min;
}


// -------------------------------------------------
//	helper functions
// -------------------------------------------------

//============================================================================
void NavierStokesGPU::loadKernels ( )
{
	// cl source codes
	cl::Program::Sources source;

	// load kernels from files
	loadSource ( source, "kernels/auxiliary.cl" );
	loadSource ( source, "kernels/boundaryConditions.cl" );
	loadSource ( source, "kernels/deltaT.cl" );

	// create program
	_clProgram = cl::Program( _clContext, source );

	// compile opencl source
	_clProgram.build( _clDevices );


	//-----------------------
	// load kernels
	//-----------------------

	_clKernels = std::vector<cl::Kernel> ( 5 );

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
	std::string cl_string( std::istreambuf_iterator<char>( cl_file ), (std::istreambuf_iterator<char>()) );

	// add it to the source list
	sources.push_back( std::make_pair( cl_string.c_str(), cl_string.length() + 1 ) );
}

//============================================================================
void NavierStokesGPU::setKernelArguments ( )
{
	// domain size including boundaries
	int nx = _nx+2;
	int ny = _ny+2;

	// set kernel arguments for setBoundaryConditionsKernel
	_clKernels[2].setArg( 0, _U_g );
	_clKernels[2].setArg( 1, _V_g );
	_clKernels[2].setArg( 2, sizeof(int), &_wN ); // northern boundary condition
	_clKernels[2].setArg( 3, sizeof(int), &_wE ); // eastern boundary condition
	_clKernels[2].setArg( 4, sizeof(int), &_wS ); // southern boundary condition
	_clKernels[2].setArg( 5, sizeof(int), &_wW ); // western boundary condition
	_clKernels[2].setArg( 6, sizeof(int), &nx );
	_clKernels[2].setArg( 7, sizeof(int), &ny );
	//_clKernels[2].setArg( 8, sizeof(int),   &_pitch );

	// set kernel arguments for setArbitraryBoundaryConditionsKernel
	_clKernels[3].setArg( 0, _U_g );
	_clKernels[3].setArg( 1, _V_g );
	_clKernels[3].setArg( 2, sizeof(int), &_wN ); // northern boundary condition
	_clKernels[3].setArg( 3, sizeof(int), &_wE ); // eastern boundary condition
	_clKernels[3].setArg( 4, sizeof(int), &_wS ); // southern boundary condition
	_clKernels[3].setArg( 5, sizeof(int), &_wW ); // western boundary condition
	_clKernels[3].setArg( 6, sizeof(int), &nx );
	_clKernels[3].setArg( 7, sizeof(int), &ny );
	//_clKernels[3].setArg( 8, sizeof(int),   &_pitch );

	// set kernel arguments for the problem specific boundary condition kernel
	_clKernels[4].setArg( 0, _U_g );
	_clKernels[4].setArg( 1, sizeof(int), &nx );
	_clKernels[4].setArg( 2, sizeof(int), &ny );
	//_clKernels[4].setArg( 3, sizeof(int),   &_pitch );

	// kernel arguments for delta t computation (UV maximum)
	_clKernels[5].setArg( 0, _U_g );
	_clKernels[5].setArg( 1, _V_g );
	// argument 2: result buffer, { float u_max, float v_max }
	_clKernels[5].setArg( 3, sizeof(cl_float) * _clWorkgroupSize, NULL); // dynamically allocated local shared memory for U
	_clKernels[5].setArg( 4, sizeof(cl_float) * _clWorkgroupSize, NULL); // dynamically allocated local shared memory for V
	_clKernels[5].setArg( 5, sizeof(int), &nx );
	_clKernels[5].setArg( 6, sizeof(int), &ny );
}
