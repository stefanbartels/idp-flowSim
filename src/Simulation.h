#ifndef SIMULATION_H
#define SIMULATION_H

#include "Definitions.h"
#include "Parameters.h"
#include "solver/navierStokesSolver.h"
#include "viewer/Viewer.h"
#include "CLManager.h"
#include <QThread>

//====================================================================
/*! \class Simulation
	\brief Class handling the fluid simulation
*/
//====================================================================
class Simulation : public QThread
{
	Q_OBJECT

	protected:
		Parameters*			_parameters;
		NavierStokesSolver*	_solver;

		Viewer*				_viewer;

		CLManager*			_clManager;

		bool                _running;
		int					_iterations;

	public:

		Simulation ( Parameters* parameters, Viewer* viewer );

		~Simulation ( );


		// TODO: move to separate flow field class
		REAL** getU_CPU();
		REAL** getV_CPU();
		REAL** getP_CPU();

	protected:
		void run ( );

	public slots:
		void simulationTrigger ( );

		void stopSimulation ( );

	signals:

		void simulationStarted ( );
		void simulationStopped ( );

		//! \brief emitted when a time step is finished
		//! \returns returning number of iterations used to solve pressure equation
		void simulatedFrame ( int numPressureIterations );
};

#endif // SIMULATION_H
