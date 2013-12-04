#ifndef CLTEST_H
#define CLTEST_H

//********************************************************************
//**    includes
//********************************************************************

#define __CL_ENABLE_EXCEPTIONS

#include "Test.h"
#include <CL/cl.hpp>
#include <CL/opencl.h>
#include <map>

//====================================================================
/*! \class CLTest
	\brief Superclass for OpenCL testing
*/
//====================================================================

template <typename T, size_t N>
T* begin(T(&arr)[N]) { return &arr[0]; }
template <typename T, size_t N>
T* end(T(&arr)[N]) { return &arr[0]+N; }

class CLTest : public Test
{
	protected:

		// OpenCL data
		std::vector<cl::Platform>			_clPlatforms;
		std::vector<cl::Device>				_clDevices;

		cl::Context							_clContext;
		cl::CommandQueue					_clQueue;

		// kernels
		std::vector<std::string*>			_clSourceCode;
		std::map<std::string, cl::Kernel>	_clKernels;
		cl::Program							_clProgram;

		int									_clWorkgroupSize;

	public:
		CLTest ( std::string name );

		~CLTest ( );

	protected:
		//! prints a matrix to console
		void printHostMatrix ( std::string title, REAL** M, int width, int height );

		//! marks differences between two matrices
		void printHostMatrixDifference
			(
				std::string title,
				REAL **M_1,
				REAL **M_2,
				int width,
				int height
			);

		//! load one kernel from one file
		//! compiled kernels can be accessed with _clKernels["kernel name"]
		void loadKernels ( std::string clfile, std::string kernelName );

		//! load multiple kernels from one file
		//! compiled kernels can be accessed with _clKernels["kernel name"]
		void loadKernels ( std::string clfiles, std::vector<std::string> kernelNames );

		//! load multiple kernels from multiple files
		//! compiled kernels can be accessed with _clKernels["kernel name"]
		void loadKernels ( std::vector<std::string> clfiles, std::vector<std::string> kernelNames );

	private:
		void loadSource ( cl::Program::Sources& sources, std::string fileName );
};



#endif // CLTEST_H
