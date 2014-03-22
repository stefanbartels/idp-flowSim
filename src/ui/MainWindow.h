#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../Parameters.h"
#include "../viewer/GLViewer.h"
#include "../viewer/Viewer.h"
#include <QMainWindow>
#include <QTime>

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class MainWindow : public QMainWindow
{
	Q_OBJECT

	protected:
		Parameters* _parameters;
		GLViewer*   _viewer;

		// UI

		QWidget*     _ui;
		QVBoxLayout* _layout;

		QPushButton* _button_run;
		QPushButton* _button_pause;
		QLabel*      _label_info;


		// frames / second
		QTime _time;
		int _frames;


	public:
		// TODO: change Simulation to FlowField class
		MainWindow
			(
				Parameters* parameters
			);

		~MainWindow ( );

		Viewer* getViewer ( );

	protected:
		void createUI ( );

	public slots:
		// TODO: find better solution
		void simulationTriggerSlot ( );
		void simulationStoppedSlot ( );
		void simulationStartedSlot ( );

		//! \brief updates the UI after a timestep is simulated
		//! \param number of iterations used to solve the pressure equation
		void simulatedFrame ( int numPressureIterations );

	signals:
		void simulationTrigger ( );

		void stopSimulation ( );
};

#endif // MAINWINDOW_H
