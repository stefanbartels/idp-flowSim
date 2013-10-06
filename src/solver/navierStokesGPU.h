#ifndef NAVIERSTOKESGPU_H
#define NAVIERSTOKESGPU_H

//********************************************************************
//**    includes
//********************************************************************

#define __CL_ENABLE_EXCEPTIONS;

#include "navierStokesSolver.h"

#include <CL/cl.hpp>
#include <CL/opencl.h>

//====================================================================
/*! \class NavierStokesCpu
    \brief Class for solving of Navier Stokes on GPU
*/
//====================================================================

class NavierStokesGPU : public NavierStokesSolver
{
	protected:
		// -------------------------------------------------
		//	member variables
		// -------------------------------------------------
			//! @name member variables
			//! @{

		// GPU arrays
		cl::Buffer	_U_g,		//! velocity in x-direction
					_V_g,		//! velocity in y-direction
					_P_g,		//! pressure
					_RHS_g,		//! right-hand side for pressure iteration
					_F_g,
					_G_g,
					_FLAG_g;	//! obstacle map


		int			_pitch;		//! pitch for GPU arrays

		// OpenCL data
		vector<cl::Platform>	_clPlatforms;
		vector<cl::Device>		_clDevices;

		cl::Context				_clContext;
		cl::CommandQueue		_clQueue;

		cl::NDRange				_clRange;

		// kernels
		vector<cl::Kernel>		_clKernels;
		cl::Program				_clProgram;

			//! @}

	public:
		// -------------------------------------------------
		//	constructor / destructor
		// -------------------------------------------------
			//! @name constructor / destructor
			//! @{

		NavierStokesGPU ( );

		~NavierStokesGPU ( );

			//! @}

		// -------------------------------------------------
		//	initialization
		// -------------------------------------------------
			//! @name initialisation
			//! @{

			//! \brief allocates and initialises simulation memory in GPU memory

		void	init ( );

			//! @}



	protected:


		// -------------------------------------------------
		//	helper functions
		// -------------------------------------------------
			//! @name helper functions
			//! @{

			//! \brief loads and compiles all required kernels

		void loadKernels ( );

			//! @}
};

#endif // NAVIERSTOKESGPU_H
