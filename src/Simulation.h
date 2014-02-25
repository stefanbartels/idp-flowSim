#ifndef SIMULATION_H
#define SIMULATION_H

#include "Definitions.h"
#include "solver/navierStokesSolver.h"
#include "viewer/Viewer.h"

//====================================================================
/*! \class Simulation
	\brief Class handling the fluid simulation
*/
//====================================================================
class Simulation
{
	protected:
		Parameters*			_parameters;
		NavierStokesSolver*	_solver;

	public:

		Simulation ( Parameters* parameters );

		~Simulation ( );




		void simulate ( Viewer* viewer );



		// TODO: move to separate flow field class
		REAL** getU_CPU();
		REAL** getV_CPU();
		REAL** getP_CPU();
};

#endif // SIMULATION_H
