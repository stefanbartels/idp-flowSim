
//********************************************************************
//**    includes
//********************************************************************

#include "navierStokesGPU.h"

#include <fstream>
#include <iostream>
//#include <iterator>

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

		// load and compile kernels
		loadKernels();
	}
	catch( cl::Error e )
	{
		//std::cerr << "ERROR: " << e.what() << "(" << e.err() << ")" << std::endl;
	}
}

//============================================================================
NavierStokesGPU::~NavierStokesGPU ( )
{

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


	// allocate memory for matrices U, V, P, RHS, F, G

	int size = ( _nx + 2 ) * ( _ny * 2 );

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



	// wait for completion
	queue.finish();

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
}


// -------------------------------------------------
//	helper functions
// -------------------------------------------------

//============================================================================
void NavierStokesGPU::loadKernels ( )
{
	// load opencl source
	std::ifstream cl_file( "kernels/kernel.cl" );
	string cl_string(
				istreambuf_iterator<char>( cl_file ),
				(istreambuf_iterator<char>())
			);

	cl::Program::Sources source( 1, make_pair( cl_string.c_str(), cl_string.length() + 1 ) );

	// create program
	_clProgram = cl::Program( _clContext, source );

	// compile opencl source
	_clProgram.build( _clDevices );

	// load kernels
	_clKernels = vector<cl::Kernel> ( 2 );

	_clKernels[0] = cl::Kernel( _clProgram, "setKernel" );
	_clKernels[1] = cl::Kernel( _clProgram, "setBoundaryAndInteriorKernel" );
}
