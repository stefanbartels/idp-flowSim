#ifndef NAVIERSTOKESGPU_H
#define NAVIERSTOKESGPU_H

//********************************************************************
//**    includes
//********************************************************************

#include "navierStokesSolver.h"
#include "../CLManager.h"


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
		CLManager*			_clManager;			//! Pointer to the CL Manager
		cl::NDRange			_clRange;			//! Range to use for kernels, size of the domain incl. boundaries
		cl::NDRange			_clWorkgroupRange;	//! Range of a workgroup
		int					_clWorkgroupSize;	//! maximum size of a work group

		cl::Context*		_clContext; // context and queue allow use of cl functions without extra methods in the manager
		cl::CommandQueue*	_clQueue;


/*		std::vector<cl::Platform>	_clPlatforms;
		std::vector<cl::Device>		_clDevices;




		// kernels
		std::vector<std::string*>	_clSourceCode;
		std::vector<cl::Kernel>		_clKernels;
		cl::Program					_clProgram;

		int							_clWorkgroupSize;	//! maximum size of a work group
*/
			//! @}

	public:
		// -------------------------------------------------
		//	constructor / destructor
		// -------------------------------------------------
			//! @name constructor / destructor
			//! @{

		NavierStokesGPU
			(
				Parameters* parameters,
				CLManager*  clManager
			);

		~NavierStokesGPU ( );

			//! @}

		// -------------------------------------------------
		//	initialization
		// -------------------------------------------------
			//! @name initialisation
			//! @{

			//! \brief allocates and initialises simulation memory in GPU memory

		void	initialize ( );

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

			//! \brief simulates the next timestep
			//! \returns number of iterations used to solve the pressure equation

		int		doSimulationStep ( );

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


			//! \brief sets kernel arguments for frequently called kernels

		void	setKernelArguments ( );

			//! @}
};

#endif // NAVIERSTOKESGPU_H
