#ifndef VTKWRITER_H
#define VTKWRITER_H

#include "Viewer.h"

class VTKWriter : public Viewer
{
	public:
		VTKWriter();

		void renderFrame (
				REAL** U,
				REAL** V,
				REAL** P,
				int nx,
				int ny,
				int it
			);
};

#endif // VTKWRITER_H
