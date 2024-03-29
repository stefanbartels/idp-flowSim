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
#include <QElapsedTimer>

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

		Parameters*			_parameters;			//! pointer to the set of simulation parameters
		NavierStokesSolver*	_solver;				//! pointer to the solver

		Viewer*				_viewer;				//! pointer to the viewer

		CLManager*			_clManager;				//! the object handling the CL setup if GPU solver is used

		bool                _running;				//! flag indicating if the simulation is currently running
		unsigned int		_iterations;			//! counter for the total number of simulated timesteps
		long unsigned int	_pressureIterations;	//! counter for total number of pressure iterations
		double				_time;					//! simulated time interval

		QElapsedTimer		_totalTimer;			//! timer for performance measurements
		QElapsedTimer		_simulationTimer;		//! timer for performance measurements
		qint64				_elapsedTotalTime;		//! time spent for simulation and visualization
		qint64				_elapsedSimulationTime;	//! time spent for simulation only

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

			//! \brief prints the results of the performance measurements to console

		void printPerformanceMeasurements ( );

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

			//! \brief triggers creation / removal of a line of obstacles
			//! \param first x offset of the obstacle to draw
			//! \param first y offset of the obstacle to draw
			//! \param last x offset of the obstacle to draw
			//! \param last y offset of the obstacle to draw
			//! \param drawing mode, true if a wall ist to be teared down instead of created

		void drawObstacles
			(
				int  x0,
				int  y0,
				int  x1,
				int  y1,
				bool delete_flag
			);

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
