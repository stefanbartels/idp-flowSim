#ifndef NAVIERSTOKESSOLVER_H
#define NAVIERSTOKESSOLVER_H

#include <string>
#include "../Definitions.h"
#include "../Parameters.h"

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

	Parameters* _parameters;

		//! @}

	public:

	// -------------------------------------------------
	//	constructor / destructor
	// -------------------------------------------------
		//! @name constructor / destructor
		//! @{

	NavierStokesSolver ( Parameters* parameters );

	virtual ~NavierStokesSolver ( );

		//! @}

	// -------------------------------------------------
	//	initialisation
	// -------------------------------------------------
		//! @name initialisation
		//! @{

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
