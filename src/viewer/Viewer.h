#ifndef VIEWER_H
#define VIEWER_H

#include "../Definitions.h"

class Viewer
{
	public:
		Viewer();

		virtual void renderFrame (
				REAL** U,
				REAL** V,
				REAL** P,
				int nx,
				int ny,
				int it
			) = 0;
};

#endif // VIEWER_H
