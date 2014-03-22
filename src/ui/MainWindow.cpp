#include "MainWindow.h"
#include "../Definitions.h"

#include <QApplication>
#include <QDesktopWidget>

#include <iostream>

//============================================================================
MainWindow::MainWindow
	(
		Parameters* parameters
	)
{
	_parameters = parameters;
	_viewer = new GLViewer( parameters, this );

	setWindowTitle( "Interactive Navier Stokes" );

	createUI();

	// move window to screen center
	adjustSize();
	move( QApplication::desktop()->screen()->rect().center() - rect().center() );
}

//============================================================================
MainWindow::~MainWindow ( )
{

}

//============================================================================
Viewer* MainWindow::getViewer()
{
	return _viewer;
}

//============================================================================
void MainWindow::createUI ( )
{
	//------------------------
	// create UI
	//------------------------

	_ui = new QWidget( this );
	setCentralWidget( _ui );

	// create buttons
	_button_run   = new QPushButton( "Run" );
	_button_pause = new QPushButton( "Pause" );


	// create window layout
	_layout = new QVBoxLayout();
	_layout->addWidget( _viewer );
	_layout->addWidget( _button_run );
	_layout->addWidget( _button_pause );

	_ui->setLayout( _layout );


	//--------------------------
	// connect ui to simulation
	//--------------------------

	QObject::connect(	_button_run, SIGNAL( clicked() ),
						this, SLOT( runSimulationSlot() ) );
	QObject::connect(	_button_pause, SIGNAL( clicked() ),
						this, SLOT( stopSimulationSlot() ) );
}

//============================================================================
void MainWindow::runSimulationSlot ( )
{
	_time   = QTime::currentTime();
	_frames = 0;

	emit runSimulation();
}

//============================================================================
void MainWindow::stopSimulationSlot ( )
{
	emit stopSimulation();
}

//============================================================================
void MainWindow::simulatedFrame ( int numPressureIterations )
{
	// calculate frame time and fps
	QTime current_time = QTime::currentTime();
	++_frames;
	int elapsed_time = (current_time.second() * 1000 + current_time.msec() ) - ( _time.second()*1000 + _time.msec() );

	if( elapsed_time > 1000 )
	{
		setWindowTitle(   "Interactive Navier Stokes ("
						+ QString::number( elapsed_time / _frames )
						+ "ms/frame, "
						+ QString::number( _frames )
						+ " fps, "
						+ QString::number( numPressureIterations )
						+ " iterations)");

		_frames = 0;
		_time = current_time;
	}
}
