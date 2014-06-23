#ifndef NAVIERSTOKESSOLVER_H
#define NAVIERSTOKESSOLVER_H

//********************************************************************
//**    includes
//********************************************************************

#include "../Definitions.h"
#include "../Parameters.h"

//====================================================================
/*! \class NavierStokesSolver
	\brief Interface for Navier Stokes Solver implementations

	\todo Warning/Error if not properly initialised or change of
		  initialisation procedure. Until now the following methods
		  have to be called in this order:
		   - setObstacleMap
		   - initialize
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

	Parameters* _parameters;	//! Pointer to the set of simulation parameters

			//! @}

	public:
		// -------------------------------------------------
		//	constructor / destructor
		// -------------------------------------------------
			//! @name constructor / destructor
			//! @{

			//! \param pointer to parameters struct

		NavierStokesSolver ( Parameters* parameters );

		virtual ~NavierStokesSolver ( );

			//! @}

		// -------------------------------------------------
		//	initialization
		// -------------------------------------------------
			//! @name initialisation
			//! @{

			//! \brief allocates and initialises simulation memory

		virtual void initialize ( ) = 0;

			//! \brief takes the obstacle map and creates geometry information for each cell
			//! true stands for fluid cells and false for boundary cells
			//! Maps must have no obstacle cell between two fluid cells to be valid.
			//! An additional boundary will be applied.
			//! \param obstacle map (domain size)
			//! \returns true if the obstacle map is valid, false otherwise

		virtual bool setObstacleMap ( bool** map ) = 0;

			//! @}


		// -------------------------------------------------
		//	execution
		// -------------------------------------------------
			//! @name execution
			//! @{

			//! \brief simulates the next timestep
			//! \returns number of iterations used to solve the pressure equation

		virtual int doSimulationStep ( ) = 0;

			//! @}


		// -------------------------------------------------
		//	data access
		// -------------------------------------------------
			//! @name data access
			//! @{

			//! \brief gives access to the horizontal velocity component
			//! \returns pointer to horizontal velocity array

		virtual REAL** getU_CPU ( ) = 0;

			//! \brief gives access to the vertical velocity component
			//! \returns pointer to vertical velocity array

		virtual REAL** getV_CPU ( ) = 0;

			//! \brief gives access to the pressure
			//! \returns pointer to pressure array

		virtual REAL** getP_CPU ( ) = 0;

			//! @}


		// -------------------------------------------------
		//	interaction
		// -------------------------------------------------
			//! @name interaction
			//! @{

			//! \brief inserts or removes a line of obstacles
			//! four cells will be marked as obstacles to prevent
			//! obstacles from lying between two fluid cells
			//! \param first x offset of the obstacle to draw
			//! \param first y offset of the obstacle to draw
			//! \param last x offset of the obstacle to draw
			//! \param last y offset of the obstacle to draw
			//! \param drawing mode, true if a wall ist to be teared down instead of created

		virtual void drawObstacles
			(
				int x0,
				int y0,
				int x1,
				int y1,
				bool delete_flag
			) = 0;

			//! @}


		// -------------------------------------------------
		//	auxiliary functions
		// -------------------------------------------------
			//! @name auxiliary functions
			//! @{

			//! \brief allocates memory for a 2D matrix of a gives size
			//! The returned pointer points to an array of pointers,
			//! each addressing the first cell of a row. The actual memory,
			//! however, is continuous.
			//! \param width of the matrix
			//! \param height of the matrix
			//! \returns pointer to the created 2D array

		REAL**	allocHostMatrix (
				int width,
				int height
			);
	
			//! \brief assigns a value to cells in a given range in a 2D array
			//! \param first cell to set in x direction
			//! \param last cell to set in x direction
			//! \param first cell to set in y direction
			//! \param last cell to set in y direction

		void	setHostMatrix (
				REAL** matrix,
				int xStart,
				int xStop,
				int yStart,
				int yStop,
				REAL value
			);

			//! \brief frees memory of a 2D matrix correctly
			//! \param pointer to matrix to be freed

		void	freeHostMatrix (
				REAL** matrix
			);
};

#endif // NAVIERSTOKESSOLVER_H
