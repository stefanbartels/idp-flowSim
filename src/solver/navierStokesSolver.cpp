
//********************************************************************
//**    includes
//********************************************************************

#include <stdlib.h>
#include "navierStokesCPU.h"


//********************************************************************
//**    implementation
//********************************************************************

// -------------------------------------------------
//	initialisation
// -------------------------------------------------

//============================================================================
void NavierStokesSolver::setParameters
	(
		ProblemParameters *parameters
	)
{
	_xlength = parameters->xlength;
	_ylength = parameters->ylength;

	_nx = parameters->nx;
	_ny = parameters->ny;

	_dx = _xlength / (REAL) _nx;
	_dy = _ylength / (REAL) _ny;

	_dt = parameters->dt;
	_tau = parameters->tau;

	_it_max = parameters->it_max;

	_epsilon = parameters->epsilon;
	_omega = parameters->omega;
	_gamma = parameters->gamma;

	_re = parameters->re;
	_gx = parameters->gx;
	_gy = parameters->gy;

	_ui = parameters->ui;
	_vi = parameters->vi;
	_pi = parameters->pi;

	_wN = parameters->wN;
	_wS = parameters->wS;
	_wW = parameters->wW;
	_wE = parameters->wE;

	_problem = parameters->problem;
}


// -------------------------------------------------
//	auxiliary functions
// -------------------------------------------------

//============================================================================
REAL** NavierStokesSolver::allocHostMatrix
	(
		int	width,
		int	height
	)
{
	// array of pointers to rows
	REAL** rows = (REAL**)malloc( height * sizeof( REAL* ) );

	// the actual data array. allocation for all rows at once to get continuous memory
	REAL* matrix = (REAL*)malloc( width * height * sizeof( REAL ) );

	rows[0] = matrix;
	for ( int i = 1; i < height; ++i )
	{
		rows[i] = matrix + i * width;
	}

	return rows;
}

//============================================================================
void NavierStokesSolver::setHostMatrix
	(
		REAL**	matrix,
		int		xStart,
		int		xStop,
		int		yStart,
		int		yStop,
		REAL	value
	)
{
	// faster than comparing using <=
	++xStop;
	++yStop;

	for ( int y = yStart; y < yStop; ++y )
	{
		for( int x = xStart; x < xStop; ++x )
		{
			matrix[y][x] = value;
		}
	}
}

//============================================================================
void NavierStokesSolver::freeHostMatrix
	(
		REAL **matrix
	)
{
	// free data memory
	free( matrix[0] );

	// free row information memory
	free( matrix );
}
