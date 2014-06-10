#ifndef VIEWER_H
#define VIEWER_H

//********************************************************************
//**    includes
//********************************************************************

#include "../Definitions.h"
#include "../Parameters.h"

//====================================================================
/*! \class Viewer
	\brief Interface for visualization classes
*/
//====================================================================

class Viewer
{
	protected:
		// -------------------------------------------------
		//	member variables
		// -------------------------------------------------
			//! @name member variables
			//! @{

		Parameters* _parameters;	//! pointer to the set of simulation parameters

			//! @}

	public:
		// -------------------------------------------------
		//	constructor / destructor
		// -------------------------------------------------
			//! @name constructor / destructor
			//! @{

			//! \param pointer to parameters struct

		Viewer ( Parameters* parameters );

		virtual ~Viewer ( );

			//! @}

		// -------------------------------------------------
		//	initialization
		// -------------------------------------------------
			//! @name initialisation
			//! @{

			//! \brief handles praparation for the visualization, if required

		virtual void initialze ( );

			//! @}

		// -------------------------------------------------
		//	visualization
		// -------------------------------------------------
			//! @name visualization
			//! @{

			//! \brief visualizes the result of a timestep
			//! \param pointer to horizontal velocity components (host memory)
			//! \param pointer to vertical velocity components (host memory)
			//! \param pointer to pressure components (host memory)
			//! \param number of the current iteration
			// TODO: give access to a FlowField class

		virtual void renderFrame
			(
                REAL** U,
                REAL** V,
                REAL** P,
				int it
			) = 0;

			//! @}
};

#endif // VIEWER_H
