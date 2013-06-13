
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

	_U   = allocMatrix ( _nx + 2, _ny + 2 );
	_V   = allocMatrix ( _nx + 2, _ny + 2 );
	_P   = allocMatrix ( _nx + 2, _ny + 2 );
	_RHS = allocMatrix ( _nx + 2, _ny + 2 );
	_F   = allocMatrix ( _nx + 2, _ny + 2 );
	_G   = allocMatrix ( _nx + 2, _ny + 2 );

	// initialise matrices U, V and P with given initial values

	setMatrix ( _U, 1, _nx + 1, 1, _ny + 1, _ui );
	setMatrix ( _V, 1, _nx + 1, 1, _ny + 1, _vi );
	setMatrix ( _P, 1, _nx + 1, 1, _ny + 1, _pi );
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
	// todo: is it correct to skip corners?

	int nx1 = _nx + 1;
	int ny1 = _ny + 1;

	//set northern boundary depending on wN
	switch( _wN )
	{
		case NO_SLIP:
			for( int x = 1; x < nx1; ++x )
			{
				_U[0][x] = -_U[1][x];
				_V[0][x] = 0.0;
			}
			break;
		case FREE_SLIP:
			for( int x = 1; x < nx1; ++x )
			{
				_U[0][x] = _U[1][x];
				_V[0][x] = 0.0;
			}
			break;
		case OUTFLOW:
			for( int x = 1; x < nx1; ++x )
			{
				_U[0][x] = _U[1][x];
				_V[0][x] = _V[1][x];
			}
			break;
	}

	//set southern boundary depending on wS
	switch( _wS )
	{
		case NO_SLIP:
			for( int x = 1; x < nx1; ++x )
			{
				_U[ny1][x] = -_U[_ny][x];
				_V[_ny][x] = 0.0;
			}
			break;
		case FREE_SLIP:
			for( int x = 1; x < nx1; ++x )
			{
				_U[ny1][x] = _U[_ny][x];
				_V[_ny][x] = 0.0;
			}
			break;
		case OUTFLOW:
			for( int x = 1; x < nx1; ++x )
			{
				_U[ny1][x] = _U[_ny][x];
				_V[_ny][x] = _V[_ny-1][x];
			}
			break;
	}

	//set western boundary depending on wW
	switch( _wW )
	{
		case NO_SLIP:
			for( int y = 1; y < ny1; ++y )
			{
				_U[y][0] = 0.0;
				_V[y][0] = -_V[y][1];
			}
			break;
		case FREE_SLIP:
			for( int y = 1; y < ny1; ++y )
			{
				_U[y][0] = 0.0;
				_V[y][0] = _V[y][1];
			}
			break;
		case OUTFLOW:
			for( int y = 1; y < ny1; ++y )
			{
				_U[y][0] = _U[y][1];
				_V[y][0] = _V[y][1];
			}
			break;
	}

	//set eastern boundary depending on wE
	switch( _wE )
	{
		case NO_SLIP:
			for( int y = 1; y < ny1; ++y )
			{
				_U[y][_nx] = 0.0;
				_V[y][nx1] = -_U[y][_nx];
			}
			break;
		case FREE_SLIP:
			for( int y = 1; y < ny1; ++y )
			{
				_U[y][_nx] = 0.0;
				_V[y][nx1] = _U[y][_nx];
			}
			break;
		case OUTFLOW:
			for( int y = 1; y < ny1; ++y )
			{
				_U[y][_nx] = _U[y][_nx-1];
				_V[y][nx1] = _V[y][_nx];
			}
			break;
	}
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
	// todo: 0 - nx+1 or 1 - nx?
	for ( int y = 0; y < _ny + 2; ++y )
	{
		for ( int x = 0; x < _nx + 2; ++x )
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
void NavierStokesCPU::setMatrix (
		double**	matrix,
		int			xStart,
		int			xStop,
		int			yStart,
		int			yStop,
		double		value
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
void NavierStokesCPU::freeMatrix( double **matrix )
{
	// delete data array
	delete [] matrix[0];

	// delete row array
	delete [] matrix;
}
