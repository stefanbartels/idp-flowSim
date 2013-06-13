
//********************************************************************
//**    includes
//********************************************************************

#include <stdlib.h>
#include "navierStokesCPU.h"


// todo: use enum instead of defines?
#define NO_SLIP		1
#define FREE_SLIP	2
#define OUTFLOW 	3
#define PERIODIC	4

//********************************************************************
//**    implementation
//********************************************************************

// -------------------------------------------------
//	constructor / destructor
// -------------------------------------------------

//============================================================================
NavierStokesCPU::NavierStokesCPU()
{

}

// -------------------------------------------------
//	initialisation
// -------------------------------------------------

//============================================================================
void NavierStokesCPU::init(  )
{
	// allocate memory for matrices U, V, P, RHS, F, G

	_U   = allocMatrix ( _nx + 1, _ny + 1 );
	_V   = allocMatrix ( _nx + 1, _ny + 1 );
	_P   = allocMatrix ( _nx + 1, _ny + 1 );
	_RHS = allocMatrix ( _nx + 1, _ny + 1 );
	_F   = allocMatrix ( _nx + 1, _ny + 1 );
	_G   = allocMatrix ( _nx + 1, _ny + 1 );

	// initialise matrices U, V and P with given initial values

	initMatrix ( _U, _nx + 1, _ny + 1, _ui );
	initMatrix ( _V, _nx + 1, _ny + 1, _vi );
	initMatrix ( _P, _nx + 1, _ny + 1, _pi );
}


// -------------------------------------------------
//	execution
// -------------------------------------------------

//============================================================================
void NavierStokesCPU::doSimulationStep ( )
{
	// get delta_t
	computeDeltaT();

	// set boundary values for u and v
	setBoundaryConditions();
	// setSpecificBoundaryConditions();

	// compute F(n) and G(n)
	computeFG();

	// compute right hand side of pressure equation
	computeRightHandSide();

	// do SOR loop
	for ( int it = 0; it < _it_max; ++it ) // TODO: complete exit condition
	{
		// do SOR step (includes residual computation)
		SORPoisson();

	}

	// compute U(n+1) and V(n+1)
	adaptUV();
}


// -------------------------------------------------
//	boundaries
// -------------------------------------------------

//============================================================================
void NavierStokesCPU::setBoundaryConditions ( )
{
	// todo
}

void NavierStokesCPU::setSpecificBoundaryConditions ( )
{
	// todo
}


// -------------------------------------------------
//	simulation
// -------------------------------------------------

//============================================================================
void NavierStokesCPU::computeDeltaT ( )
{
	/*
	 * Formula 3.50:
	 *
	 * delta_t = tau * min(
	 *   Re/2 * ( 1/delta_x² + 1/delta_y² ),
	 *   delta_x / |u_max|,
	 *   delta_y / |v_max|
	 * )
	 *
	 * u_max and v_max are the maximum velocities in U and V
	 */

	double u_max = 0, v_max = 0;
	double opt_a, opt_x, opt_y, min;

	// get u_max and v_max: iterate over arrays U and V (same size => one loop)
	for ( int y = 0; y < _ny; ++y )
	{
		for ( int x = 0; x < _nx; ++x )
		{
			if( abs( _U[y][x] ) > u_max )
				u_max = _U[y][x];
			if( abs( _V[y][x] ) > v_max )
				v_max = _V[y][x];
		}
	}

	// compute the three options for the min-function
	opt_a = ( _re/2 ) * ( 1/(_dx*_dx) + 1/(_dy*_dy) );
	opt_x = _dx / abs( u_max );
	opt_y = _dx / abs( u_max );

	// get smallest value
	min = opt_a < opt_x ? opt_a : opt_x;
	min = min   < opt_y ? min   : opt_y;

	// compute delta t
	_dt = _tau * min;
}

void NavierStokesCPU::computeFG ( )
{
	// todo
}

void NavierStokesCPU::computeRightHandSide ( )
{
	// todo
}

int NavierStokesCPU::SORPoisson ( )
{
	// todo

	return 0;
}

void NavierStokesCPU::adaptUV ( )
{
	// todo
}


// -------------------------------------------------
//	helper functions
// -------------------------------------------------

//============================================================================
double** NavierStokesCPU::allocMatrix (
		int	width,
		int	height
	)
{
	// array of pointers to rows
	double** rows = (double**)malloc( height * sizeof( double* ) );

	if ( height )
	{
		// the actual data array. allocation for all rows at once to get continuous memory
		double* matrix = (double*)malloc( width * height * sizeof( double ) );

		rows[0] = matrix;
		for ( int i = 1; i < height; ++i )
		{
			rows[i] = matrix + i * width;
		}
	}

	return rows;
}

//============================================================================
void NavierStokesCPU::initMatrix (
		double**	matrix,
		int			width,
		int			height,
		double		value
	)
{
	for ( int y = 0; y <= height; ++y )
	{
		for( int x = 0; x <= width; ++x )
		{
			matrix[y][x] = value;
		}
	}
}

//============================================================================
void NavierStokesCPU::freeMatrix( double **matrix )
{
	// delete data array
	delete [] matrix[0];

	// delete row array
	delete [] matrix;
}
