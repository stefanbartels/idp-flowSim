#ifndef CLMANAGER_H
#define CLMANAGER_H

//********************************************************************
//**    includes
//********************************************************************

#define __CL_ENABLE_EXCEPTIONS

#include "Definitions.h"
#include "Parameters.h"
#include <CL/cl.hpp>
#include <CL/opencl.h>

//********************************************************************
//**    additional types
//********************************************************************

// all kernels use float at the moment
typedef cl_float CL_REAL;


//====================================================================
//! Kernel Identifiers

namespace kernel
{
	enum KernelIDs
	{
		setKernel                      = 0,
		setBoundaryAndInterior         = 1,
		setBoundaryConditions          = 2,
		setArbitraryBoundaryConditions = 3,
		problemSpecific                = 4,
		getUVMaximum                   = 5,
		computeF                       = 6,
		computeG                       = 7,
		rightHandSide                  = 8,
		gaussSeidelRedBlack            = 9,
		pressureBoundaryConditions     = 10,
		pressureResidualReduction      = 11,
		updateUV                       = 12
	};
}

//====================================================================
/*! \class CLManager
	\brief Class handling the CL setup
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

		Parameters*					_parameters;		//! pointer to the set of simulation parameters

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

			//! \param pointer to parameters struct

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
			//! \param set of sources to add the loaded file to
			//! \param source code file to read

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
