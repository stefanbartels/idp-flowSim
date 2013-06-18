#ifndef SIMPLEPGMWRITER_H
#define SIMPLEPGMWRITER_H

#include "Viewer.h"

class SimplePGMWriter : public Viewer
{
	public:
		SimplePGMWriter();

		void renderFrame (
				double** U,
				double** V,
				double** P,
				int nx,
				int ny,
				int it
			);
};

#endif // SIMPLEPGMWRITER_H
