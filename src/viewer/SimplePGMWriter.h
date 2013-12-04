#ifndef SIMPLEPGMWRITER_H
#define SIMPLEPGMWRITER_H

#include "Viewer.h"

class SimplePGMWriter : public Viewer
{
	public:
		SimplePGMWriter();

		void renderFrame (
				REAL** U,
				REAL** V,
				REAL** P,
				int nx,
				int ny,
				int it
			);
};

#endif // SIMPLEPGMWRITER_H
