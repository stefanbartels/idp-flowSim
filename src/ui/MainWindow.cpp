#include "MainWindow.h"
#include "../Definitions.h"

#include <QApplication>
#include <QDesktopWidget>

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
	// TODO: find better solution

	QObject::connect(	_button_run, SIGNAL( clicked() ),
						this, SLOT( runSimulationSlot() ) );
	QObject::connect(	_button_pause, SIGNAL( clicked() ),
						this, SLOT( stopSimulationSlot() ) );
}

//============================================================================
void MainWindow::runSimulationSlot()
{
	emit runSimulation();
}

//============================================================================
void MainWindow::stopSimulationSlot()
{
	emit stopSimulation();
}