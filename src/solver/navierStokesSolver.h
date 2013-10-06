#ifndef NAVIERSTOKESSOLVER_H
#define NAVIERSTOKESSOLVER_H

#include <string>
#include "../inputParser.h"

/*
 * obstacle map data values
 * ----------------------------------------------------
 * | 0 | 0 | 0 | center | east | west | south | north |
 * ----------------------------------------------------
 *
 * 1 = fluid cell
 * 0 = obstacle cell
 */
#define C_F		0x10	// 000 10000
#define C_B		0x00	// 000 00000

#define B_N		0x01	// 000 00001
#define B_S		0x02	// 000 00010
#define B_W		0x04	// 000 00100
#define B_E		0x08	// 000 01000

#define B_NW	0x05	// 000 00101
#define B_NE	0x09	// 000 01001
#define B_SW	0x06	// 000 00110
#define B_SE	0x0A	// 000 01010

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
	double		_xlength,	//! domain size in x-direction
				_ylength;	//! domain size in y-direction

	int			_nx,		//! number of interior cells in x-direction
				_ny;		//! number of interior cells in y-direction

	double		_dx,		//! length delta x of on cell in x-direction
				_dy;		//! length delta y of on cell in y-direction

	// time stepping data
	double		_t0,		//! start time
				_t,			//! current time value
				_dt,		//! time step size
				_tau;		//! safety factor for time step size control

	// pressure-iteration data
	int			_it_max;	//! maximal number of pressure iterations per time step
				//_it;		//! SOR iteration counter (-> local variable)

	double		_epsilon,	//! stopping tolerance eps for pressure iteration
				_omega,		//! relaxation parameter for SOR iteration
				_gamma;		//! upwind differencing factor

	// problem dependent quantities
	double		_re,		//! Reynolds number Re
				_gx,		//! body force gx (e.g. gravity)
				_gy;		//! body force gy (e.g. gravity)

	double		_ui,		//! initial velocity in x-direction
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
		)
	{
		_xlength = parameters->xlength;
		_ylength = parameters->ylength;

		_nx = parameters->nx;
		_ny = parameters->ny;

		_dx = _xlength / (double) _nx;
		_dy = _ylength / (double) _ny;

		_dt = parameters->dt;
		_tau = parameters->tau;

		_it_max = parameters->it_max;

		_epsilon = parameters->epsilon;
		_omega = parameters->omega;
		_gamma = parameters->gamma;

		_re = parameters->re;
		_gx = parameters->gx;
		_gy = parameters->gy;

		_ui = parameters->ui;
		_vi = parameters->vi;
		_pi = parameters->pi;

		_wN = parameters->wN;
		_wS = parameters->wS;
		_wW = parameters->wW;
		_wE = parameters->wE;

		_problem = parameters->problem;
	};

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

	virtual double** getU_CPU ( ) = 0;

	virtual double** getV_CPU ( ) = 0;

	virtual double** getP_CPU ( ) = 0;

		//! @}
};

#endif // NAVIERSTOKESSOLVER_H
