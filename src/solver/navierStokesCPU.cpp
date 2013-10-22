
//********************************************************************
//**    includes
//********************************************************************

#include <stdlib.h>
#include <math.h>
#include <cmath>
#include "navierStokesCPU.h"

#include <iostream>

using namespace std;

// todo: use enum instead of defines?
#define FREE_SLIP	1
#define NO_SLIP		2
#define OUTFLOW 	3
#define PERIODIC	4

//********************************************************************
//**    implementation
//********************************************************************

// -------------------------------------------------
//	constructor / destructor
// -------------------------------------------------

//============================================================================
NavierStokesCPU::NavierStokesCPU ( )
{

}

//============================================================================
NavierStokesCPU::~NavierStokesCPU()
{
	freeMatrix( _U );
	freeMatrix( _V );
	freeMatrix( _P );
	freeMatrix( _RHS );
	freeMatrix( _F );
	freeMatrix( _G );

	delete [] _FLAG[0];
	delete [] _FLAG;
}

// -------------------------------------------------
//	initialization
// -------------------------------------------------

//============================================================================
void NavierStokesCPU::init ( )
{
	// allocate memory for matrices U, V, P, RHS, F, G

	_U   = allocMatrix ( _nx + 2, _ny + 2 );
	_V   = allocMatrix ( _nx + 2, _ny + 2 );
	_P   = allocMatrix ( _nx + 2, _ny + 2 );
	_RHS = allocMatrix ( _nx + 2, _ny + 2 );
	_F   = allocMatrix ( _nx + 2, _ny + 2 );
	_G   = allocMatrix ( _nx + 2, _ny + 2 );

	// initialise matrices with 0.0
	// todo: might not be neccessary

	setMatrix ( _U, 0, _nx + 1, 0, _ny + 1, 0.0 );
	setMatrix ( _V, 0, _nx + 1, 0, _ny + 1, 0.0 );
	setMatrix ( _P, 0, _nx + 1, 0, _ny + 1, 0.0 );

	setMatrix ( _RHS, 0, _nx + 1, 0, _ny + 1, 0.0 );
	setMatrix ( _F,	  0, _nx + 1, 0, _ny + 1, 0.0 );
	setMatrix ( _G,   0, _nx + 1, 0, _ny + 1, 0.0 );

	// initialise interior cells of U, V and P with given initial values

	setMatrix ( _U, 1, _nx, 1, _ny, _ui );
	setMatrix ( _V, 1, _nx, 1, _ny, _vi );
	setMatrix ( _P, 1, _nx, 1, _ny, _pi );
}

//============================================================================
bool NavierStokesCPU::setObstacleMap
	(
		bool** map
	)
{
	int nx1 = _nx + 1;
	int ny1 = _ny + 1;
	int nx2 = _nx + 2;
	int ny2 = _ny + 2;

	//-----------------------
	// allocate memory for flag array
	//-----------------------

	_FLAG = (unsigned char**)malloc( ny2 * sizeof( unsigned char* ) );

	// the actual data array. allocation for all rows at once to get continuous memory
	unsigned char* data = (unsigned char*)malloc( nx2 * ny2 * sizeof( unsigned char ) );

	_FLAG[0] = data;
	for( int i = 1; i < ny2; ++i )
	{
		_FLAG[i] = data + i * nx2;
	}


	//-----------------------
	// create geometry map
	//-----------------------

	/*
	 * obstacle map data values
	 * ----------------------------------------------------
	 * | 0 | 0 | 0 | center | east | west | south | north |
	 * ----------------------------------------------------
	 *
	 * 1 = fluid cell
	 * 0 = obstacle cell
	 *
	 * C_F		0x10	000 10000
	 * C_B		0x00	000 00000

	 * B_N		0x01	000 00001
	 * B_S		0x02	000 00010
	 * B_W		0x04	000 00100
	 * B_E		0x08	000 01000

	 * B_NW		0x05	000 00101
	 * B_NE		0x09	000 01001
	 * B_SW		0x06	000 00110
	 * B_SE		0x0A	000 01010
	 *
	 */

	// Domain boundary cells are treated like interior boundary cells.


	// compute interior cells
	for ( int y = 1; y < ny1; ++y )
	{
		for ( int x = 1; x < nx1; ++x )
		{
			if( map[y][x] )
			{
				// cell is a fluid cell
				// neighbour cells do not matter
				_FLAG[y][x] = C_F;
			}
			else
			{
				// cell is a boundary cell

				// check for invalid boundary cell (between two fluid cells)
				if( ( map[y-1][x] && map[y+1][x] ) || ( map[y][x-1] && map[y][x+1] ) )
					return false;

				// look for surrounding cells to get correct flag
				_FLAG[y][x] = C_B
						+ B_N * map[y+1][x]
						+ B_S * map[y-1][x]
						+ B_W * map[y][x-1]
						+ B_E * map[y][x+1];
			}
		}
	}

	// compute boundary cells
	for( int x = 1; x < nx1; ++x )
	{
		// southern boundary
		_FLAG[0][x]	= C_B
					+ B_N * map[1][x]
					+ B_S
					+ B_W
					+ B_E;

		// northern boundary
		_FLAG[ny1][x] = C_B
					  + B_N
					  + B_S * map[_ny][x]
					  + B_W
					  + B_E;
	}

	for( int y = 1; y < ny1; ++y )
	{
		// western boundary
		_FLAG[y][0]	= C_B
					+ B_N
					+ B_S
					+ B_W
					+ B_E * map[y][1];

		// eastern boundary
		_FLAG[y][nx1] = C_B
					  + B_N
					  + B_S
					  + B_W * map[y][_nx]
					  + B_E;
	}

	// edge cells (not neccessary, but uninitialised cells are ugly)
	_FLAG[0][0] = _FLAG[0][nx1] = _FLAG[ny1][0] = _FLAG[ny1][nx1] = 0x0F;

	return true;
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

	setSpecificBoundaryConditions();

	// compute F(n) and G(n)
	computeFG();

	// compute right hand side of pressure equation
	computeRightHandSide();

	// poisson overrelaxation loop
	double residual = INFINITY;

	for ( int it = 0; it < _it_max && abs(residual) > _epsilon; ++it )
	{
		// do SOR step (includes residual computation)
		residual =  SORPoisson();
	}

	// compute U(n+1) and V(n+1)
	adaptUV();
}


// -------------------------------------------------
//	data access
// -------------------------------------------------

//============================================================================
double** NavierStokesCPU::getU_CPU()
{
	return _U;
}

//============================================================================
double** NavierStokesCPU::getV_CPU()
{
	return _V;
}

//============================================================================
double** NavierStokesCPU::getP_CPU()
{
	return _P;
}


// -------------------------------------------------
//	boundaries
// -------------------------------------------------

//============================================================================
void NavierStokesCPU::setBoundaryConditions ( )
{
	int nx1 = _nx + 1;
	int ny1 = _ny + 1;

	//-----------------------
	// southern boundary
	//-----------------------

	switch( _wS )
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


	//-----------------------
	// northern boundary
	//-----------------------

	switch( _wN )
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


	//-----------------------
	// western boundary
	//-----------------------

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


	//-----------------------
	// eastern boundary
	//-----------------------

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


	//-----------------------
	// boundary of arbitrary geometries
	//-----------------------

	// according to 3.51, 3.52 and 3.53

	/*
	 * obstacle map data values
	 * ----------------------------------------------------
	 * | 0 | 0 | 0 | center | east | west | south | north |
	 * ----------------------------------------------------
	 *
	 * 1 = fluid cell
	 * 0 = obstacle cell
	 *
	 * C_F		0x10	000 10000
	 * C_B		0x00	000 00000

	 * B_N		0x01	000 00001
	 * B_S		0x02	000 00010
	 * B_W		0x04	000 00100
	 * B_E		0x08	000 01000

	 * B_NW		0x05	000 00101
	 * B_NE		0x09	000 01001
	 * B_SW		0x06	000 00110
	 * B_SE		0x0A	000 01010
	 *
	 */

	// loop over interior cells

	for( int y = 1; y < ny1; ++y )
	{
		for( int x = 1; x < nx1; ++x )
		{
			switch( _FLAG[y][x] )
			{
				case C_F:
					continue;
					break;

				case B_N: // northern obstacle boundary => fluid in the north
					_U[y][x-1] = -_U[y+1][x-1];
					_U[y][x]   = -_U[y+1][x];
					_V[y][x]   = 0.0;
					break;

				case B_S: // fluid in the south
					_U[y][x-1] = -_U[y-1][x-1];
					_U[y][x]   = -_U[y-1][x];
					_V[y][x]   = 0.0;
					break;

				case B_W: // fluid in the west
					_U[y][x-1] = 0.0;
					_V[y-1][x] = -_V[y-1][x-1];
					_V[y][x]   = -_V[y][x-1];
					break;

				case B_E: // fluid in the east
					_U[y][x] = 0.0;
					_V[y-1][x] = -_V[y-1][x+1];
					_V[y][x]   = -_V[y][x+1];
					break;


				case B_NW: // fluid in the north and west
					_U[y][x]   = -_U[y+1][x];
					_U[y][x-1] = 0.0;

					_V[y][x]   = 0.0;
					_V[y-1][x] = -_V[y-1][x+1];
					break;

				case B_NE: // fluid in the north and east
					_U[y][x]   = 0.0;
					_U[y][x-1] = -_U[y+1][x-1];

					_V[y][x]   = 0.0;
					_V[y-1][x] = -_V[y-1][x+1];
					break;

				case B_SW: // fluid	in the south and west
					_U[y][x]   = -_U[y-1][x];
					_U[y][x-1] = 0.0;

					_V[y][x]   = -_V[y][x-1];
					_V[y-1][x] = 0.0;
					break;

				case B_SE: // fluid	in the south and east
					_U[y][x]   = 0.0;
					_U[y][x-1] = -_U[y-1][x-1];

					_V[y][x]   = -_V[y][x+1];
					_V[y-1][x] = 0.0;
					break;
			}
		}
	}

}

//============================================================================
void NavierStokesCPU::setSpecificBoundaryConditions ( )
{
	// todo: find sophisticated way to specifiy this in the input file

	if ( _problem == "moving_lid" )
	{
		//const double lid_velocity = 1.0;
		for ( int x = 1; x < _nx + 1; ++x )
		{
			_U[0][x] = 2.0 - _U[1][x];
		}
	}
	else if ( _problem == "left_inflow" )
	{
		for ( int y = 1; y < _ny + 1; ++y )
		{
			_U[y][0] = 1.0;
		}
	}
}


// -------------------------------------------------
//	simulation
// -------------------------------------------------

//============================================================================
void NavierStokesCPU::computeDeltaT ( )
{
	// compute delta t according to formula 3.50

	double u_max = 0.0, v_max = 0.0;
	double opt_a, opt_x, opt_y, min;

	// get u_max and v_max: iterate over arrays U and V (same size => one loop)

	// faster than comparing using <=
	int nx1 = _nx + 1;
	int ny1 = _ny + 1;

	for ( int y = 1; y < ny1; ++y )
	{
		for ( int x = 1; x < nx1; ++x )
		{
			if( abs( _U[y][x] ) > u_max )
				u_max = abs( _U[y][x] );
			if( abs( _V[y][x] ) > v_max )
				v_max = abs( _V[y][x] );
		}
	}

	// compute the three options for the min-function
	opt_a = ( _re / 2.0 ) * ( 1.0 / (_dx * _dx) + 1.0 / (_dy * _dy) );
	opt_x = _dx / abs( u_max );
	opt_y = _dy / abs( v_max );

	// get smallest value
	min = opt_a < opt_x ? opt_a : opt_x;
	min = min   < opt_y ? min   : opt_y;

	// compute delta t
	_dt = _tau * min;
}

//============================================================================
void NavierStokesCPU::computeFG ( )
{
	// y coordinates in book are counted from lower left edge, in array from upper left edge
	// => y-1 in the book becomes y+1 here

	double alpha = 0.9; // todo: select alpha

	// faster than comparing using <=
	int nx1 = _nx + 1;
	int ny1 = _ny + 1;

	// todo: combine loops

	// compute F according to formula 3.36
	for( int y = 1; y < ny1; ++y )
	{
		for( int x = 1; x < _nx; ++x )
		{
			// compute F between fluid cells only
			if( _FLAG[y][x] == C_F && _FLAG[y][x+1] == C_F ) // second cell test for not to overwrite boundary values
			{
				_F[y][x] =
					_U[y][x] + _dt *
					(
						(
							d2m_dx2 ( _U, x, y ) +
							d2m_dy2 ( _U, x, y )
						) / _re
						- du2_dx ( x, y, alpha )
						- duv_dy ( x, y, alpha )
						+ _gx
					);
			}
			else
			{
				// according to formula 3.42
				_F[y][x]   = _U[y][x];
			}
		}
	}

	// compute G according to formula 3.37
	for( int y = 1; y < _ny; ++y )
	{
		for( int x = 1; x < nx1; ++x )
		{
			// compute G between fluid cells only
			if( _FLAG[y][x] == C_F && _FLAG[y+1][x] == C_F ) // y+1 in the book
			{
				_G[y][x] =
					_V[y][x] + _dt *
					(
						(
							d2m_dx2 ( _V, x, y ) +
							d2m_dy2 ( _V, x, y )
						) / _re
						- dv2_dy ( x, y, alpha )
						- duv_dx ( x, y, alpha )
						+ _gy
					);
			}
			else
			{
				// according to formula 3.42
				_G[y][x]   = _V[y][x];
			}
		}
	}


	// boundary values for arbitrary geometries
	// todo: neccessary?
	for ( int y = 1; y < ny1; ++y )
	{
		for ( int x = 1; x < nx1; ++x )
		{
			switch ( _FLAG[y][x] )
			{
				case B_N:
					_G[y][x]   = _V[y][x];
					break;
				case B_S:
					_G[y-1][x] = _V[y-1][x];
					break;
				case B_W:
					_F[y][x-1] = _U[y][x-1];
					break;
				case B_E:
					_F[y][x]   = _U[y][x];
					break;
				case B_NW:
					_F[y][x-1] = _U[y][x-1];
					_G[y][x]   = _V[y][x];
					break;
				case B_NE:
					_F[y][x]   = _U[y][x];
					_G[y][x]   = _V[y][x];
					break;
				case B_SW:
					_F[y][x-1] = _U[y][x-1];
					_G[y-1][x] = _V[y-1][x];
					break;
				case B_SE:
					_F[y][x]   = _U[y][x];
					_G[y-1][x] = _V[y-1][x];
					break;
			}
		}
	}



	// setting boundary values for f according to formula 3.42
	for ( int y = 1; y < ny1; ++y )
	{
		_F[y][0]   = _U[y][0];
		_F[y][_nx] = _U[y][_nx];
	}

	// setting boundary values for g according to formula 3.42
	for ( int x = 1; x < nx1; ++x )
	{
		_G[0][x]   = _V[0][x];
		_G[_ny][x] = _V[_ny][x];
	}

}

//============================================================================
void NavierStokesCPU::computeRightHandSide ( )
{
	// compute right-hand side of poisson equation according to formula 3.38

	// faster than comparing using <=
	int nx1 = _nx + 1;
	int ny1 = _ny + 1;

	for ( int y = 1; y < ny1; ++y )
	{
		for ( int x = 1; x < nx1; ++x )
		{
			// todo: only for fluid cells?
			_RHS[y][x] = ( 1 / _dt ) *
				(
					( _F[y][x] - _F[y][x-1] ) / _dx +
					( _G[y][x] - _G[y-1][x] ) / _dy
				);
		}
	}
}

//============================================================================
int NavierStokesCPU::SORPoisson ( )
{
	int nx1 = _nx + 1;
	int ny1 = _ny + 1;

	// gauss seidel is writing back the results back to the original array immediately
	// so a mixture of values from timestep n and n+1 is used

	// the epsilon-parameters in formula 3.44 are set to 1.0 according to page 38
	double constant_expr = _omega / ( 2.0 / (_dx * _dx) + 2.0 / (_dy * _dy) );

	double dx2 = _dx * _dx;
	double dy2 = _dy * _dy;

	//-----------------------
	// SOR step
	//-----------------------

	// according to formula 3.44

	for ( int y = 1; y < ny1; ++y )
	{
		for ( int x = 1; x < nx1; ++x )
		{
			// calculate pressure in fluid cells
			if( _FLAG[y][x] == C_F )
			{
				_P[y][x] =
					( 1 - _omega ) * _P[y][x] + constant_expr *
					(
						( _P[y][x-1] + _P[y][x+1] ) / dx2
						+
						( _P[y-1][x] + _P[y+1][x] ) / dy2
						-
						_RHS[y][x]
					);
			}
			else
			{
				// set boundary pressure value for obstacle cells
				switch ( _FLAG[y][x] )
				{
					case B_N:
						_P[y][x] = _P[y+1][x];
						break;
					case B_S:
						_P[y][x] = _P[y-1][x];
						break;
					case B_W:
						_P[y][x] = _P[y][x-1];
						break;
					case B_E:
						_P[y][x] = _P[y][x+1];
						break;
					case B_NW:
						_P[y][x] = (_P[y-1][x] + _P[y][x+1]) / 2;
						break;
					case B_NE:
						_P[y][x] = (_P[y+1][x] + _P[y][x+1]) / 2;
						break;
					case B_SW:
						_P[y][x] = (_P[y-1][x] + _P[y][x-1]) / 2;
						break;
					case B_SE:
						_P[y][x] = (_P[y+1][x] + _P[y][x-1]) / 2;
						break;
				}
			}
		}
	}

	//-----------------------
	// boundary values
	//-----------------------

	// according to formula 3.41
	// 3.48 instead? (=> before SOR step)
	/* // todo
	for ( int x = 1; x < nx1; ++x )
	{
		_P[0][x]   = _P[1][x];
		_P[ny1][x] = _P[_ny][x];
	}

	for ( int y = 1; y < ny1; ++y )
	{
		_P[y][0]   = _P[y][1];
		_P[y][nx1] = _P[y][_nx];
	}
	*/

	//-----------------------
	// residual
	//-----------------------

	// compute residual using L²-Norm (according to formula 3.45 and 3.46)

	double tmp;
	double sum = 0.0;
	int numCells = 0;

	for ( int y = 1; y < ny1; ++y )
	{
		for ( int x = 1; x < nx1; ++x )
		{
			if ( _FLAG[y][x] == C_F )
			{
				tmp =
					  ( ( _P[y][x+1] - _P[y][x] ) - ( _P[y][x] - _P[y][x-1] ) ) / dx2
					+ ( ( _P[y+1][x] - _P[y][x] ) - ( _P[y][x] - _P[y-1][x] ) ) / dy2
					- _RHS[y][x];

				sum += tmp * tmp;

				++numCells;
			}
		}
	}

	// compute L²-Norm and return residual

	return sqrt( sum / numCells );
}

//============================================================================
void NavierStokesCPU::adaptUV ( )
{
	// update u and v according to 3.34 and 3.35

	int nx1 = _nx + 1;
	int ny1 = _ny + 1;

	double dt_dx = _dt / _dx;
	double dt_dy = _dt / _dy;

	// update u. two nested loops because of different limits
	for ( int y = 1; y < ny1; ++y )
	{
		for ( int x = 1; x < _nx; ++x )
		{
			if ( _FLAG[y][x] == C_F && _FLAG[y][x+1] == C_F )
			{
				_U[y][x] = _F[y][x] - dt_dx * ( _P[y][x+1] - _P[y][x] );
			}
		}
	}

	// update v
	for ( int y = 1; y < _ny; ++y )
	{
		for ( int x = 1; x < nx1; ++x )
		{
			if ( _FLAG[y][x] == C_F && _FLAG[y+1][x] == C_F )	//todo evtl y+1?
			{
				_V[y][x] = _G[y][x] - dt_dy * ( _P[y+1][x] - _P[y][x] );
			}
		}
	}
}










// -------------------------------------------------
//	F & G helper functions
// -------------------------------------------------

//============================================================================
inline double NavierStokesCPU::d2m_dx2 ( double** M, int x, int y )
{
	return ( M[y][x-1] - 2.0 * M[y][x] + M[y][x+1] ) / ( _dx * _dx );
}

//============================================================================
inline double NavierStokesCPU::d2m_dy2 ( double** M, int x, int y )
{
	return ( M[y-1][x] - 2.0 * M[y][x] + M[y+1][x] ) / ( _dy * _dy );
}

//============================================================================
inline double NavierStokesCPU::du2_dx  ( int x, int y, double alpha )
{
	return
		(
			(
				( _U[y][x] + _U[y][x+1] ) *
				( _U[y][x] + _U[y][x+1] )
				-
				( _U[y][x-1] + _U[y][x] ) *
				( _U[y][x-1] + _U[y][x] )
			)
			+
			alpha *
			(
				abs( _U[y][x] + _U[y][x+1] ) *
				   ( _U[y][x] - _U[y][x+1] )
				-
				abs( _U[y][x-1] + _U[y][x] ) *
				   ( _U[y][x-1] - _U[y][x] )
			)
		) / ( 4.0 * _dx);
}

//============================================================================
inline double NavierStokesCPU::dv2_dy  ( int x, int y, double alpha )
{
	return
		(
			(
				( _V[y][x] + _V[y+1][x] ) *
				( _V[y][x] + _V[y+1][x] )
				-
				( _V[y-1][x] + _V[y][x] ) *
				( _V[y-1][x] + _V[y][x] )
			)
			+
			alpha *
			(
				abs( _V[y][x] + _V[y+1][x] ) *
				   ( _V[y][x] - _V[y+1][x] )
				-
				abs( _V[y-1][x] + _V[y][x] ) *
				   ( _V[y-1][x] - _V[y][x] )
			)
		) / ( 4.0 * _dy );
}

//============================================================================
inline double NavierStokesCPU::duv_dx  ( int x, int y, double alpha )
{
	return
		(
			(
				( _U[y][x] + _U[y+1][x] ) *
				( _V[y][x] + _V[y][x+1] )
				-
				( _U[y][x-1] + _U[y+1][x-1] ) *
				( _V[y][x-1] + _V[y][x] )
			)
			+
			alpha *
			(
					abs( _U[y][x] + _U[y+1][x] ) *
					   ( _V[y][x] - _V[y][x+1] )
					-
					abs( _U[y][x-1] + _U[y+1][x-1] ) *
					   ( _V[y][x-1] - _V[y][x] )
			)
		) / ( 4.0 * _dx );
}

//============================================================================
inline double NavierStokesCPU::duv_dy  ( int x, int y, double alpha )
{
	return
		(
			(
				( _V[y][x] + _V[y][x+1] ) *
				( _U[y][x] + _U[y+1][x] )
				-
				( _V[y-1][x] + _V[y-1][x+1] ) *
				( _U[y-1][x] + _U[y][x] )
			)
			+
			alpha *
			(
				abs( _V[y][x] + _V[y][x+1] ) *
				   ( _U[y][x] - _U[y+1][x] )
				-
				abs( _V[y-1][x] + _V[y-1][x+1] ) *
				   ( _U[y-1][x] - _U[y][x] )
			)
		) / ( 4.0 * _dy );
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

	// the actual data array. allocation for all rows at once to get continuous memory
	double* matrix = (double*)malloc( width * height * sizeof( double ) );

	rows[0] = matrix;
	for ( int i = 1; i < height; ++i )
	{
		rows[i] = matrix + i * width;
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
