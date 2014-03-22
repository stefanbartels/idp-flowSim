//********************************************************************
//**    includes
//********************************************************************

#include "CLManager.h"
#include <iostream>
#include <fstream>

//********************************************************************
//**    implementation
//********************************************************************

// -------------------------------------------------
//	constructor / destructor
// -------------------------------------------------

//============================================================================
CLManager::CLManager ( Parameters* parameters )
{
	_parameters = parameters;

	_clWorkgroupSize = 0;
	_clPreferredWorkgroupSize = 0;

	try
	{
		// create OpenCL platform
		cl::Platform::get( &_clPlatforms );
		_clPlatforms[0].getDevices( CL_DEVICE_TYPE_GPU, &_clDevices );

		// create context
		_clContext = cl::Context( _clDevices );

		// create command queue
		_clQueue = cl::CommandQueue( _clContext, _clDevices[0] );
	}
	catch( cl::Error error )
	{
		std::cerr << "CL ERROR: " << error.what() << "(" << error.err() << ")" << std::endl;
		throw error;
	}
}

//============================================================================
CLManager::~CLManager ( )
{
	// cleanup kernel source
	for( std::vector<std::string*>::iterator it = _clSourceCode.begin(); it != _clSourceCode.end(); ++it )
	{
		SAFE_DELETE( *it );
	}
}


// -------------------------------------------------
//	kernel loading functions
// -------------------------------------------------

//============================================================================
void CLManager::loadKernels ( )
{
	// TODO: cross platform way to find all files in a directory?

	#if VERBOSE
		std::cout << "Compiling kernels..." << std::endl;
	#endif

	// cl source codes
	cl::Program::Sources source;

	// load simulation kernels from files
	loadSource ( source, "kernels/auxiliary.cl" );
	loadSource ( source, "kernels/boundaryConditions.cl" );
	loadSource ( source, "kernels/deltaT.cl" );
	loadSource ( source, "kernels/computeFG.cl" );
	loadSource ( source, "kernels/rightHandSide.cl" );
	loadSource ( source, "kernels/pressure.cl" );
	loadSource ( source, "kernels/updateUV.cl" );

	// load visualization kernels from files


	// create program
	_clProgram = cl::Program( _clContext, source );

	// compile opencl source
	try
	{
		_clProgram.build( _clDevices );
	}
	catch( cl::Error error )
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

	#if VERBOSE
		std::cout << "Kernels compiled" << std::endl;
	#endif

	//-----------------------
	// load kernels
	//-----------------------

		_clKernels = std::vector<cl::Kernel>( 12 );

	#if VERBOSE
		std::cout << "Binding kernels..." << std::endl;
	#endif

	try
	{
		// auxiliary kernels [0],[1]
		_clKernels[kernel::setKernel] =
				cl::Kernel( _clProgram, "setKernel" );
		_clKernels[kernel::setBoundaryAndInterior] =
				cl::Kernel( _clProgram, "setBoundaryAndInteriorKernel" );

		// boundary condition kernels [2],[3]
		_clKernels[kernel::setBoundaryConditions] =
				cl::Kernel( _clProgram, "setBoundaryConditionsKernel"	);
		_clKernels[kernel::setArbitraryBoundaryConditions] =
				cl::Kernel( _clProgram, "setArbitraryBoundaryConditionsKernel" );

		// problem specific kernel [4]
		if ( _parameters->problem == "moving_lid" )
		{
			_clKernels[kernel::problemSpecific] =
					cl::Kernel( _clProgram, "setMovingLidBoundaryConditionsKernel" );
		}
		else if ( _parameters->problem == "left_inflow" )
		{
			_clKernels[kernel::problemSpecific] =
					cl::Kernel( _clProgram, "setLeftInflowBoundaryConditionsKernel" );
		}
		/*else
		{
			// add empty dummy kernel to prevent kernel list from being shifted
			_clKernels[kernel::problemSpecific] = cl::Kernel();
		}*/

		// kernel to find maximum UV value for delta t computation [5]
		_clKernels[kernel::getUVMaximum] =
				cl::Kernel( _clProgram, "getUVMaximumKernel" );

		// kernels for F and G computation [6],[7]
		_clKernels[kernel::computeF] = cl::Kernel( _clProgram, "computeF" );
		_clKernels[kernel::computeG] = cl::Kernel( _clProgram, "computeG" );

		// kernel for the right hand side of the pressure equation [8]
		_clKernels[kernel::rightHandSide] =
				cl::Kernel( _clProgram, "rightHandSideKernel" );

		// kernel for pressure equation step [9],[10],[11]
		_clKernels[kernel::gaussSeidelRedBlack] =
				cl::Kernel( _clProgram, "gaussSeidelRedBlackKernel" );
		_clKernels[kernel::pressureBoundaryConditions] =
				cl::Kernel( _clProgram, "pressureBoundaryConditionsKernel" );
		_clKernels[kernel::pressureResidualReduction] =
				cl::Kernel( _clProgram, "pressureResidualReductionKernel" );

		// kernel for velocity update [12]
		_clKernels[kernel::updateUV] = cl::Kernel( _clProgram, "updateUVKernel" );
	}
	catch( cl::Error error )
	{
		std::cerr << "CL ERROR while kernel binding: " << error.what() << "(" << error.err() << ")" << std::endl;
		throw error;
	}


	// get work group size
	_clWorkgroupSize = _clKernels[0].getWorkGroupInfo< CL_KERNEL_WORK_GROUP_SIZE >( _clDevices[0] );

	// get preferred work group size multiple
	_clPreferredWorkgroupSize = _clKernels[0].getWorkGroupInfo< CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE >( _clDevices[0] );

	std::cout << "Preferred WG size: " << _clPreferredWorkgroupSize << std::endl;
}

//============================================================================
void CLManager::loadSource
	(
		cl::Program::Sources &sources,
		std::string fileName
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
cl::CommandQueue *CLManager::getQueue ( )
{
	return &_clQueue;
}

//============================================================================
cl::Context *CLManager::getContext()
{
	return &_clContext;
}


// -------------------------------------------------
//	kernel execution functions
// -------------------------------------------------

//============================================================================
cl::Kernel *CLManager::getKernel ( int kernelID )
{
	return &(_clKernels[kernelID]);
}

//============================================================================
cl_int CLManager::runRangeKernel
	(
		int kernelID,
		const cl::NDRange &offset,
		const cl::NDRange &global,
		const cl::NDRange &local
	)
{
	return _clQueue.enqueueNDRangeKernel ( _clKernels[kernelID], offset, global, local );
}

//============================================================================
cl_int CLManager::finish ( )
{
	return _clQueue.finish();
}

//============================================================================
int CLManager::getWorkgroupSize ( )
{
	return _clWorkgroupSize;
}

//============================================================================
int CLManager::getPreferredWorkgroupSize ( )
{
	return _clPreferredWorkgroupSize;
}
