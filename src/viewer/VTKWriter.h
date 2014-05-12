#ifndef VTKWRITER_H
#define VTKWRITER_H

//********************************************************************
//**    includes
//********************************************************************

#include "Viewer.h"

//====================================================================
/*! \class VTKWriter
	\brief Class for writing the simulation results to file
	in VTK format. Currently only VTK ASCII format ist supported
*/
//====================================================================

class VTKWriter : public Viewer
{
	public:
		// -------------------------------------------------
		//	constructor / destructor
		// -------------------------------------------------
			//! @name constructor / destructor
			//! @{

			//! \param pointer to parameters struct

		VTKWriter ( Parameters* parameters );

			//! @}

		// -------------------------------------------------
		//	visualization
		// -------------------------------------------------
			//! @name visualization
			//! @{

			//! \brief visualizes the result of a timestep
			//! Writes a ASCII file in VTK legacy format
			//! \param pointer to horizontal velocity components (host memory)
			//! \param pointer to vertical velocity components (host memory)
			//! \param pointer to pressure components (host memory)
			//! \param number of the current iteration

		void renderFrame
			(
                REAL** U,
                REAL** V,
                REAL** P,
				int it
			);

			//! @}
};

#endif // VTKWRITER_H
