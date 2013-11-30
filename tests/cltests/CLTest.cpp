
//********************************************************************
//**    includes
//********************************************************************

#include "CLTest.h"
#include <fstream>

//********************************************************************
//**    implementation
//********************************************************************

// -------------------------------------------------
//	constructor / destructor
// -------------------------------------------------

//============================================================================
CLTest::CLTest ( std::string name ) : Test ( name )
{
	_clWorkgroupSize = 0;

	// set up opencl environment
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
CLTest::~CLTest ( )
{
	// cleanup kernel source
	for( std::vector<std::string*>::iterator it = _clSourceCode.begin(); it != _clSourceCode.end(); ++it )
	{
		delete( *it );
	}
}

//============================================================================
void CLTest::printHostMatrix
	(
		std::string title,
		REAL **M,
		int width,
		int height
	)
{
	std::cout << title << std::endl;

	for( int y = 0; y < height; ++y )
	{
		for( int x = 0; x < width; ++x )
		{
			std::cout << M[y][x] << " ";
		}

		std::cout << std::endl;
	}
}

//============================================================================
void  CLTest::loadKernels
	(
		std::string clfile,
		std::string kernelName
	)
{
	std::vector<std::string> clfiles;
	std::vector<std::string> kernelNames;

	clfiles.reserve(1);
	kernelNames.reserve(1);

	clfiles.push_back( clfile );
	kernelNames.push_back( kernelName );

	loadKernels( clfiles, kernelNames );
}

//============================================================================
void  CLTest::loadKernels
	(
		std::string clfile,
		std::vector<std::string> kernelNames
	)
{
	std::vector<std::string> clfiles;
	clfiles.reserve(1);
	clfiles.push_back( clfile );

	loadKernels( clfiles, kernelNames );
}

//============================================================================
void CLTest::loadKernels
	(
		std::vector<std::string> clfiles,
		std::vector<std::string> kernelNames
	)
{
	// cl source codes
	cl::Program::Sources source;

	// load kernels from files
	for( std::vector<std::string>::iterator it = clfiles.begin(); it != clfiles.end(); ++it )
	{
		loadSource ( source, std::string( "../kernels/").append(*it) );
	}

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

	//-----------------------
	// bind kernels
	//-----------------------

	try
	{
		for( std::vector<std::string>::iterator it = kernelNames.begin(); it != kernelNames.end(); ++it )
		{
			_clKernels[*it] = cl::Kernel( _clProgram, (*it).c_str() );
		}
	}
	catch( cl::Error error )
	{
		std::cerr << "CL ERROR while kernel binding: " << error.what() << "(" << error.err() << ")" << std::endl;
		throw error;
	}

	// get work group size
	_clWorkgroupSize = (*(_clKernels.begin())).second.getWorkGroupInfo< CL_KERNEL_WORK_GROUP_SIZE >( _clDevices[0] );
}

//============================================================================
void CLTest::loadSource
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
