#ifndef NAVIERSTOKESSOLVER_H
#define NAVIERSTOKESSOLVER_H

#include <string>
#include "../inputParser.h"
#include "../Definitions.h"

//====================================================================
/*! \class NavierStokesSolver
	\brief Interface for Navier Stokes Solver implementations

	\todo Warning/Error if not properly initialised or change of
		  initialisation procedure. Until now the following methods
		  have to be called in this order:
		   - setParameters
		   - setObstacleMap
		   - init
		  => ugly
*/
//====================================================================

class NavierStokesSolver
{
	protected:
	// -------------------------------------------------
	//	member variables
	// -------------------------------------------------
		//! @name member variables
		//! @{

	// geometry data
	REAL		_xlength,	//! domain size in x-direction
				_ylength;	//! domain size in y-direction

	int			_nx,		//! number of interior cells in x-direction
				_ny;		//! number of interior cells in y-direction

	REAL		_dx,		//! length delta x of on cell in x-direction
				_dy;		//! length delta y of on cell in y-direction

	// time stepping data
	REAL		_t0,		//! start time
				_t,			//! current time value
				_dt,		//! time step size
				_tau;		//! safety factor for time step size control

	// pressure-iteration data
	int			_it_max;	//! maximal number of pressure iterations per time step
				//_it;		//! SOR iteration counter (-> local variable)

	REAL		_epsilon,	//! stopping tolerance eps for pressure iteration
				_omega,		//! relaxation parameter for SOR iteration
				_gamma;		//! upwind differencing factor

	// problem dependent quantities
	REAL		_re,		//! Reynolds number Re
				_gx,		//! body force gx (e.g. gravity)
				_gy;		//! body force gy (e.g. gravity)

	REAL		_ui,		//! initial velocity in x-direction
				_vi,		//! initial velocity in y-direction
				_pi;		//! initial pressure

	int			_wN,		//! boundary condition along northern boundary
				_wS,		//! boundary condition along southern boundary
				_wW,		//! boundary condition along western boundary
				_wE;		//! boundary condition along eastern boundary

						/*
						 * boundary conditions:
						 * 1 = free-slip
						 * 2 = no-slip
						 * 3 = outflow
						 * 4 = periodic
						 */

	std::string	_problem;	//! problem type

		//! @}

	public:

	// -------------------------------------------------
	//	initialisation
	// -------------------------------------------------
		//! @name initialisation
		//! @{

		//! \brief defines the problem parameters
		//! \param problem parameter set

	void setParameters
		(
			ProblemParameters*	parameters
		);

		//! \brief allocates and initialises simulation memory

	virtual void init ( ) = 0;


	/*
		//! \brief defines the arbitrary geometry
		//! \param obstacle map
		//!
		//! the map must contain 1 for fluid cells and 0 for boundary cells
		//! using the following pattern for each cell:
		//!	----------------------------------------------------
		//! | 0 | 0 | 0 | center | east | west | south | north |
		//! ----------------------------------------------------

		// todo: take bool array and create obstacle map inside?
		//		 less efficient but maybe cleaner
		//		 bool array may also be faster for visualisation
		//		 -> done: setObstacleMap

	virtual void setGeometryMap ( unsigned char** map ) = 0;
	*/

		//! \brief defines the arbitrary geometry
		//! \param obstacle map (null means no map is given)
		//! \returns true, if the obstacle map was valid, false else
		//! true stands for fluid cells and false for boundary cells
		//! valid maps have no obstacle cell between two fluid cells

	virtual bool setObstacleMap ( bool** map ) = 0;

		//! @}
	// -------------------------------------------------
	//	execution
	// -------------------------------------------------
		//! @name execution
		//! @{

	virtual void doSimulationStep ( ) = 0;

		//! @}

	// -------------------------------------------------
	//	data access
	// -------------------------------------------------
		//! @name data access
		//! @{

	virtual REAL** getU_CPU ( ) = 0;

	virtual REAL** getV_CPU ( ) = 0;

	virtual REAL** getP_CPU ( ) = 0;

		//! @}






	// -------------------------------------------------
	//	auxiliary functions
	// -------------------------------------------------
		//! @name auxiliary functions
		//! @{

	REAL**	allocHostMatrix (
			int width,
			int height
		);

	void	setHostMatrix (
			REAL** matrix,
			int xStart,
			int xStop,
			int yStart,
			int yStop,
			REAL value
		);

	void	freeHostMatrix (
			REAL** matrix
		);
};

#endif // NAVIERSTOKESSOLVER_H
