#ifndef SIMPLEPGMWRITER_H
#define SIMPLEPGMWRITER_H

//********************************************************************
//**    includes
//********************************************************************

#include "Viewer.h"

//====================================================================
/*! \class VTKWriter
	\brief Class for writing the pressure as a greyscale image
	to file
*/
//====================================================================

class SimplePGMWriter : public Viewer
{
	public:
		// -------------------------------------------------
		//	constructor / destructor
		// -------------------------------------------------
			//! @name constructor / destructor
			//! @{

			//! \param pointer to parameters struct

		SimplePGMWriter ( Parameters* parameters );

			//! @}

		// -------------------------------------------------
		//	visualization
		// -------------------------------------------------
			//! @name visualization
			//! @{

			//! \brief visualizes the result of a timestep
			//! Writes a greyscale PGM file
			//! \param pointer to horizontal velocity components (host memory)
			//! \param pointer to vertical velocity components (host memory)
			//! \param pointer to pressure components (host memory)
			//! \param number of the current iteration

		void renderFrame
			(
                REAL** U,
                REAL** V,
                REAL** P,
				double time,
				unsigned int iteration
			);

			//! @}
};

#endif // SIMPLEPGMWRITER_H
