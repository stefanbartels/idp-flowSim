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
				double** U,
				double** V,
				double** P,
				int it
			);
};

#endif // VTKWRITER_H
