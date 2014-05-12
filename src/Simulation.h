#ifndef SIMULATION_H
#define SIMULATION_H

//********************************************************************
//**    includes
//********************************************************************

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
		// -------------------------------------------------
		//	member variables
		// -------------------------------------------------
			//! @name member variables
			//! @{

		Parameters*			_parameters;	//! pointer to the set of simulation parameters
		NavierStokesSolver*	_solver;		//! pointer to the solver

		Viewer*				_viewer;		//! pointer to the viewer

		CLManager*			_clManager;		//! the object handling the CL setup if GPU solver is used

		bool                _running;		//! flag indicating if the simulation is currently running
		int					_iterations;	//! counter for the total number of simulated timesteps

			//! @}

	public:
		// -------------------------------------------------
		//	constructor / destructor
		// -------------------------------------------------
			//! @name constructor / destructor
			//! @{

			//! \param pointer to parameters struct
			//! \param pointer to viewer object

		Simulation ( Parameters* parameters, Viewer* viewer );

		~Simulation ( );

			//! @}

		// -------------------------------------------------
		//	data access
		// -------------------------------------------------
			//! @name data access
			//! @{

			// TODO: move to separate flow field class

			//! \brief gives access to the horizontal velocity component
			//! \returns pointer to horizontal velocity array

		REAL** getU_CPU ( );

			//! \brief gives access to the vertical velocity component
			//! \returns pointer to vertical velocity array

		REAL** getV_CPU ( );

			//! \brief gives access to the pressure
			//! \returns pointer to pressure array

		REAL** getP_CPU ( );

			//! @}

	protected:
		// -------------------------------------------------
		//	execution
		// -------------------------------------------------
			//! @name execution
			//! @{

			//! \brief contains the timestep loop

		void run ( );

			//! @}

	public slots:
		// -------------------------------------------------
		//	slots
		// -------------------------------------------------
			//! @name slots
			//! @{

			//! \brief starts / pauses the simulation
			//! Depending on the current state.

		void simulationTrigger ( );

			//! \brief stops the simulation

		void stopSimulation ( );

			//! \brief triggers creation / removal of obstacles
			//! \param x offset of the obstacle to draw
			//! \param y offset of the obstacle to draw
			//! \param drawing mode, true if a wall ist to be teared down instead of created

		void drawObstacle ( int x, int y, bool mode );

			//! @}

	signals:
		// -------------------------------------------------
		//	signals
		// -------------------------------------------------
			//! @name signals
			//! @{

			//! \brief signal emitted when the simulation is starting

		void simulationStarted ( );

			//! \brief signal emitted when the simulation is stopping

		void simulationStopped ( );

			//! \brief emitted when a time step is finished
			//! \returns returning number of iterations used to solve pressure equation

		void simulatedFrame ( int numPressureIterations );

			//! @}
};

#endif // SIMULATION_H
