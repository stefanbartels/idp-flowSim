#ifndef CLMANAGER_H
#define CLMANAGER_H

#define __CL_ENABLE_EXCEPTIONS

#include "Definitions.h"
#include "Parameters.h"
#include <CL/cl.hpp>
#include <CL/opencl.h>

// uncommented, as all kernels use float at the moment
//#if REAL // not working yet!
//	#define CL_REAL cl_double
//#else
	#define CL_REAL cl_float
//#endif

//====================================================================
//! Kernel Identifiers

enum KernelIDs
{
	setKernel                            = 0,
	setBoundaryAndInteriorKernel         = 1,
	setBoundaryConditionsKernel          = 2,
	setArbitraryBoundaryConditionsKernel = 3,
	problemSpecificKernel                = 4,
	getUVMaximumKernel					 = 5,
	computeFKernel                       = 6,
	computeGKernel                       = 7,
	rightHandSideKernel                  = 8,
	gaussSeidelRedBlackKernel            = 9,
	pressureBoundaryConditionsKernel     = 10,
	pressureResidualReductionKernel      = 11,
	updateUVKernel                       = 12
};

//====================================================================
/*! \class CLManager
	\brief Class handling the CL methods
*/
//====================================================================
class CLManager
{
	protected:
		// -------------------------------------------------
		//	member variables
		// -------------------------------------------------
			//! @name member variables
			//! @{

		Parameters*					_parameters;

		// OpenCL data
		std::vector<cl::Platform>	_clPlatforms;
		std::vector<cl::Device>		_clDevices;

		cl::Context					_clContext;
		cl::CommandQueue			_clQueue;

		// kernels
		std::vector<std::string*>	_clSourceCode;
		std::vector<cl::Kernel>		_clKernels;
		cl::Program					_clProgram;

		int							_clWorkgroupSize;	//! maximum size of a work group

			//! @}

	public:
		// -------------------------------------------------
		//	constructor / destructor
		// -------------------------------------------------
			//! @name constructor / destructor
			//! @{

		CLManager ( Parameters* parameters );

		~CLManager ( );

			//! @}

		// -------------------------------------------------
		//	kernel loading functions
		// -------------------------------------------------
			//! @name kernel loading functions
			//! @{

			//! \brief loads and compiles all required kernels

		void	loadKernels ( );

			//! \brief loads the content of a cl source file to the source vector

		void	loadSource (
						cl::Program::Sources&	sources,
						std::string				fileName
					);

			//! @}


		// -------------------------------------------------
		//	get functions
		// -------------------------------------------------
			//! @name kernel execution functions
			//! @{

			//! \brief Returns a reference to the queue object
			//! Allows use of cl functions without implementing
			//! a manager method for each of them

		cl::CommandQueue* getQueue ( );

			//! \brief Returns a reference to the context object

		cl::Context* getContext ( );

			//! \brief Returns the size of a workgroup on the device

		int getWorkgroupSize ( );

			//! @}

		// -------------------------------------------------
		//	kernel execution functions
		// -------------------------------------------------
			//! @name kernel execution functions
			//! @{

			//! \brief Function to get a specific kernel
			//! usefull to set kernel arguments

		cl::Kernel* getKernel( int kernelID );

			//! \brief Enqueues a range kernel on the device

		cl_int runRangeKernel
			(
				int kernelID,
				const cl::NDRange& offset,
				const cl::NDRange& global,
				const cl::NDRange& local
			);

			//! \brief Blocks until all queued kernels have finished

		cl_int finish ( );


			//! @}
};

#endif // CLMANAGER_H
