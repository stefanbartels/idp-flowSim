#ifndef SIMPLEPGMWRITER_H
#define SIMPLEPGMWRITER_H

#include "Viewer.h"

class SimplePGMWriter : public Viewer
{
	public:
		SimplePGMWriter
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

#endif // SIMPLEPGMWRITER_H
