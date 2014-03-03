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
                REAL** U,
                REAL** V,
                REAL** P,
				int it
			);
};

#endif // SIMPLEPGMWRITER_H
