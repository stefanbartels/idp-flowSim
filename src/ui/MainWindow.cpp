#include "MainWindow.h"
#include "../Definitions.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QCheckBox>

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
GLViewer* MainWindow::getViewer()
{
	return _viewer;
}

//============================================================================
void MainWindow::createUI ( )
{
	_ui = new QWidget( this );
	setCentralWidget( _ui );

	//------------------------
	// create layout items required later
	//------------------------
	_button_run   = new QPushButton( "Start" );
	_label_info   = new QLabel( "<table><tr><td width=\"60\"></td><td>ms / frame</td></tr>" \
								"<tr><td></td><td>FPS</td></tr>" \
								"<tr><td></td><td>Iterations per timestep</td></tr></table>" );
	_label_info->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );


	//------------------------
	// create window layout
	//------------------------

	_layout = new QVBoxLayout();
	_layout->addWidget( _viewer );

	// buttons
	QHBoxLayout* buttonLayout = new QHBoxLayout();
		// add start/pause button
		buttonLayout->addWidget( _button_run, 2 );
		// create and add checkbox to toggle rescaling
		QCheckBox* autoScaleCheckbox = new QCheckBox( "Rescale", this );
		autoScaleCheckbox->setChecked( true );
		buttonLayout->addWidget( autoScaleCheckbox );
	_layout->addLayout( buttonLayout );

	_layout->addWidget( _label_info );

	_ui->setLayout( _layout );


	//--------------------------
	// connect ui to simulation
	//--------------------------

	QObject::connect(	_button_run, SIGNAL( clicked() ),
						this, SLOT( simulationTriggerSlot() ) );

	QObject::connect(	autoScaleCheckbox, SIGNAL( clicked() ),
						_viewer, SLOT( toggleRescaling() ) );
}

//============================================================================
void MainWindow::simulationTriggerSlot ( )
{
	_time   = QTime::currentTime();
	_frames = 0;

	emit simulationTrigger();
}

//============================================================================
void MainWindow::simulationStartedSlot ( )
{
	_button_run->setText( "Pause" );
}

//============================================================================
void MainWindow::simulationStoppedSlot ( )
{
	_button_run->setText( "Start" );
}

//============================================================================
void MainWindow::simulatedFrame ( int numPressureIterations )
{
	// calculate frame time and fps
	QTime current_time = QTime::currentTime();
	++_frames;
	int elapsed_time = _time.msecsTo( current_time );

	if( elapsed_time > 1000 )
	{
		_label_info->setText( "<table><tr><td width=\"60\">" + QString::number( (float)elapsed_time / _frames ) + "</td><td>ms / frame</td></tr>"\
							  "<tr><td>" + QString::number( _frames ) + "</td><td>FPS</td></tr>"\
							  "<tr><td>" + QString::number( numPressureIterations ) + "</td><td>Iterations per timestep</td></tr></table>" );

		_frames = 0;
		_time = current_time;
	}
}
