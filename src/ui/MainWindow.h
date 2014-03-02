#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../Parameters.h"
#include "../viewer/GLViewer.h"
#include "../viewer/Viewer.h"
#include <QMainWindow>

#include <QPushButton>
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
		void runSimulationSlot ( );
		void stopSimulationSlot ( );

	signals:
		void runSimulation ( );

		void stopSimulation ( );
};

#endif // MAINWINDOW_H
