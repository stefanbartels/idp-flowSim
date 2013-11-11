#ifndef NAVIERSTOKESGPU_H
#define NAVIERSTOKESGPU_H

//********************************************************************
//**    includes
//********************************************************************

#define __CL_ENABLE_EXCEPTIONS

#include "navierStokesSolver.h"

#include <CL/cl.hpp>
#include <CL/opencl.h>

// uncommented, as all kernels use float at the moment
//#if REAL // not working yet!
//	#define CL_REAL cl_double
//#else
	#define CL_REAL cl_float
//#endif

//====================================================================
/*! \class NavierStokesCpu
    \brief Class for solving of Navier Stokes on GPU
	\todo optimize device memory, for example u,v and flag as constant memory
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

		// host arrays for data exchange
		REAL	**_U_host,
				**_V_host,
				**_P_host;

		// OpenCL data
		std::vector<cl::Platform>	_clPlatforms;
		std::vector<cl::Device>		_clDevices;

		cl::Context					_clContext;
		cl::CommandQueue			_clQueue;

		cl::NDRange					_clRange;

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

			//! \brief takes the obstacle map and creates geometry information for each cell
			//! \param obstacle map (domain size)
			//! an additional boundary will be applied

		bool	setObstacleMap ( bool** map );

			//! @}


		// -------------------------------------------------
		//	execution
		// -------------------------------------------------
			//! @name execution
			//! @{

		void	doSimulationStep ( );

			//! @}


		// -------------------------------------------------
		//	data access
		// -------------------------------------------------
			//! @name data access
			//! @{

		REAL** getU_CPU ( );

		REAL** getV_CPU ( );

		REAL** getP_CPU ( );

			//! @}


	protected:
		// -------------------------------------------------
		//	boundaries
		// -------------------------------------------------
			//! @name boundaries
			//! @{

			//!  \brief sets the boundary values for U and V depending on wN, wS, wW and wE

		void	setBoundaryConditions ( );

			//! \brief  TODO

		void	setSpecificBoundaryConditions ( );

			//! @}

		// -------------------------------------------------
		//	simulation
		// -------------------------------------------------
			//! @name simulation
			//! @{

			//! \brief calculates the stepsize for next time step
			//! According to formula 3.50

		void	computeDeltaT ( );

			//! \brief computes F and G

		void	computeFG ( );

			//! \brief computes the right-hand side of the pressure equation

		void	computeRightHandSide ( );

			//! \brief SOR iteration step for pressure Poisson equation
			//! \returns residual

		REAL		SORPoisson ( );

			//! \brief calculates new velocities

		void	adaptUV ( );

			//! @}


		// -------------------------------------------------
		//	auxiliary functions
		// -------------------------------------------------
			//! @name auxiliary functions
			//! @{

			//! \brief loads and compiles all required kernels

		void	loadKernels ( );

			//! \brief loads the content of a cl source file to the source vector

		void	loadSource (
						cl::Program::Sources&	sources,
						std::string				fileName
					);

			//! \brief sets kernel arguments for frequently called kernels

		void	setKernelArguments ( );

			//! @}
};

#endif // NAVIERSTOKESGPU_H
