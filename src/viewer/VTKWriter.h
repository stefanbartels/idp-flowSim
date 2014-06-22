#ifndef VTKWRITER_H
#define VTKWRITER_H

//********************************************************************
//**    includes
//********************************************************************

#include "Viewer.h"
#include <QObject>

//====================================================================
/*! \class VTKWriter
	\brief Class for writing the simulation results to file
	in VTK format. Currently only VTK ASCII format ist supported
*/
//====================================================================

class VTKWriter : public Viewer
{
	protected:
		// -------------------------------------------------
		//	member variables
		// -------------------------------------------------
			//! @name member variables
			//! @{

		double _nextOutput;	//! next point in simulated time to write a vtk file at

			//! @}

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
				double time,
				int    iteration
			);

			//! @}
};

#endif // VTKWRITER_H
