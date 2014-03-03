#ifndef VTKWRITER_H
#define VTKWRITER_H

#include "Viewer.h"

class VTKWriter : public Viewer
{
	public:
		VTKWriter
			(
				Parameters* parameters
			);

		void renderFrame  (
                REAL** U,
                REAL** V,
                REAL** P,
				int it
			);
};

#endif // VTKWRITER_H
