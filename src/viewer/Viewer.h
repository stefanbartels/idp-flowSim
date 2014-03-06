#ifndef VIEWER_H
#define VIEWER_H

#include "../Definitions.h"
#include "../Parameters.h"

class Viewer
{
	protected:
		Parameters* _parameters;

	public:
		Viewer
		(
			Parameters* parameters
		);

		virtual void initialze ( );

		// TODO: give access to FlowField class
		virtual void renderFrame  (
                REAL** U,
                REAL** V,
                REAL** P,
				int it
			) = 0;
};

#endif // VIEWER_H
