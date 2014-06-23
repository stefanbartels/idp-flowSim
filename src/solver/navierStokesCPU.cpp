
//********************************************************************
//**    includes
//********************************************************************

#include <stdlib.h>
#include <math.h>
#include <cmath>
#include "navierStokesCPU.h"

#include <iostream>

//********************************************************************
//**    implementation
//********************************************************************

// -------------------------------------------------
//	constructor / destructor
// -------------------------------------------------

//============================================================================
NavierStokesCPU::NavierStokesCPU ( Parameters* parameters )
	: NavierStokesSolver( parameters )
{

}

//============================================================================
NavierStokesCPU::~NavierStokesCPU()
{
	freeHostMatrix( _U );
	freeHostMatrix( _V );
	freeHostMatrix( _P );
	freeHostMatrix( _RHS );
	freeHostMatrix( _F );
	freeHostMatrix( _G );

	free( _FLAG[0] );
	free( _FLAG );
}

// -------------------------------------------------
//	initialization
// -------------------------------------------------

//============================================================================
void NavierStokesCPU::initialize ( )
{
	// allocate memory for matrices U, V, P, RHS, F, G

	_U   = allocHostMatrix ( _parameters->nx + 2, _parameters->ny + 2 );
	_V   = allocHostMatrix ( _parameters->nx + 2, _parameters->ny + 2 );
	_P   = allocHostMatrix ( _parameters->nx + 2, _parameters->ny + 2 );
	_RHS = allocHostMatrix ( _parameters->nx + 2, _parameters->ny + 2 );
	_F   = allocHostMatrix ( _parameters->nx + 2, _parameters->ny + 2 );
	_G   = allocHostMatrix ( _parameters->nx + 2, _parameters->ny + 2 );

	// initialise matrices with 0.0
	// todo: might not be neccessary

	setHostMatrix ( _U, 0, _parameters->nx + 1, 0, _parameters->ny + 1, 0.0 );
	setHostMatrix ( _V, 0, _parameters->nx + 1, 0, _parameters->ny + 1, 0.0 );
	setHostMatrix ( _P, 0, _parameters->nx + 1, 0, _parameters->ny + 1, 0.0 );

	setHostMatrix ( _RHS, 0, _parameters->nx + 1, 0, _parameters->ny + 1, 0.0 );
	setHostMatrix ( _F,	  0, _parameters->nx + 1, 0, _parameters->ny + 1, 0.0 );
	setHostMatrix ( _G,   0, _parameters->nx + 1, 0, _parameters->ny + 1, 0.0 );

	// initialise interior cells of U, V and P with given initial values

	setHostMatrix ( _U, 1, _parameters->nx, 1, _parameters->ny, _parameters->ui );
	setHostMatrix ( _V, 1, _parameters->nx, 1, _parameters->ny, _parameters->vi );
	setHostMatrix ( _P, 1, _parameters->nx, 1, _parameters->ny, _parameters->pi );
}

//============================================================================
bool NavierStokesCPU::setObstacleMap
	(
		bool** map
	)
{
	int nx1 = _parameters->nx + 1;
	int ny1 = _parameters->ny + 1;
	int nx2 = _parameters->nx + 2;
	int ny2 = _parameters->ny + 2;

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
					  + B_S * map[_parameters->ny][x]
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
					  + B_W * map[y][_parameters->nx]
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
int NavierStokesCPU::doSimulationStep ( )
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
	REAL residual = INFINITY;

	int sor_iterations = 0;
	for ( ; sor_iterations < _parameters->it_max && fabs( residual ) > _parameters->epsilon; ++sor_iterations )
	{
		// do SOR step (includes residual computation)
		residual = SORPoisson();
	}

	// compute U(n+1) and V(n+1)
	adaptUV();

	return sor_iterations;
}


// -------------------------------------------------
//	interaction
// -------------------------------------------------

//============================================================================
void NavierStokesCPU::drawObstacles
	(
		int x0,
		int y0,
		int x1,
		int y1,
		bool delete_flag
	)
{
	if( delete_flag )
	{
		std::cout << "obstacle removing not implemented yet!" << std::endl;
		//_parameters->obstacleMap[y][x] = true;

		// TODO: test cells adjacent to removed cells recursively
	}
	else
	{
		//-----------------------
		// draw line
		//-----------------------

		// using bresenham's algorithm

		int dx    = abs( x1 - x0 );		// difference in x direction
		int dy    = abs( y1 - y0 );		// difference in y direction
		int sx    = x0 < x1 ? 1 : -1;	// define step direction
		int sy    = y0 < y1 ? 1 : -1;	// define step direction
		int error = dx - dy;			// initial error value
		int e2;

		while( true )
		{
			//-----------------------
			// update obstacle flags
			//-----------------------

			// south west corner of painted square
			_parameters->obstacleMap[y0][x0] = false;
			_FLAG[y0][x0] = C_B
					+ B_N
					+ B_S * ( y0 > 1 ? _parameters->obstacleMap[y0-1][x0] : 1 )
					+ B_W * ( x0 > 1 ? _parameters->obstacleMap[y0][x0-1] : 1 )
					+ B_E;

			// south east corner
			_parameters->obstacleMap[y0][x0+1] = false;
			_FLAG[y0][x0+1] = C_B
					+ B_N
					+ B_S * ( y0 > 1 ? _parameters->obstacleMap[y0-1][x0+1] : 1 )
					+ B_W
					+ B_E * _parameters->obstacleMap[y0][x0+2];

			// north west corner
			_parameters->obstacleMap[y0+1][x0] = false;
			_FLAG[y0+1][x0] = C_B
					+ B_N * _parameters->obstacleMap[y0+2][x0]
					+ B_S
					+ B_W * ( x0 > 1 ? _parameters->obstacleMap[y0+1][x0-1] : 1 )
					+ B_E;

			// north east corner
			_parameters->obstacleMap[y0+1][x0+1] = false;
			_FLAG[y0+1][x0+1] = C_B
					+ B_N
					+ B_S * _parameters->obstacleMap[y0+2][x0+1]
					+ B_W
					+ B_E * _parameters->obstacleMap[y0+1][x0+2];

			//-----------------------
			// reset velocities
			//-----------------------

			_U[y0][x0]     = _V[y0][x0]     = 0.0;
			_U[y0][x0+1]   = _V[y0][x0+1]   = 0.0;
			_U[y0+1][x0]   = _V[y0+1][x0]   = 0.0;
			_U[y0+1][x0+1] = _V[y0+1][x0+1] = 0.0;

			// without reseting the results of the surrounding cells
			// the results are quite unphysical
			if( y0 > 1 )
			{
				_U[y0-1][x0]   = _V[y0-1][x0]   = 0.0;
				_U[y0-1][x0+1] = _V[y0-1][x0+1] = 0.0;
			}
			_U[y0+2][x0]   = _V[y0+2][x0]   = 0.0;
			_U[y0+2][x0+1] = _V[y0+2][x0+1] = 0.0;

			if( x0 > 1 )
			{
				_U[y0][x0-1]   = _V[y0][x0-1]   = 0.0;
				_U[y0+1][x0-1] = _V[y0+1][x0-1] = 0.0;
			}

			_U[y0][x0+2]   = _V[y0][x0+2]   = 0.0;
			_U[y0+1][x0+2] = _V[y0+1][x0+2] = 0.0;

			//-----------------------
			// reset pressure
			//-----------------------

			_P[y0][x0]     = 0.0;
			_P[y0][x0+1]   = 0.0;
			_P[y0+1][x0]   = 0.0;
			_P[y0+1][x0+1] = 0.0;



			//-----------------------
			// next line step
			//-----------------------

			if( x0 == x1 && y0 == y1 )
			{
				break;
			}

			e2 = error * 2;

			if( e2 > -dy )
			{
				error = error - dy;
				x0 = x0 + sx;
			}

			if( e2 < dx )
			{
				error = error + dx;
				y0 = y0 + sy;
			}
		}
	}
}


// -------------------------------------------------
//	data access
// -------------------------------------------------

//============================================================================
REAL** NavierStokesCPU::getU_CPU ( )
{
	return _U;
}

//============================================================================
REAL** NavierStokesCPU::getV_CPU ( )
{
	return _V;
}

//============================================================================
REAL** NavierStokesCPU::getP_CPU ( )
{
	return _P;
}


// -------------------------------------------------
//	boundaries
// -------------------------------------------------

//============================================================================
void NavierStokesCPU::setBoundaryConditions ( )
{
	int nx1 = _parameters->nx + 1;
	int ny1 = _parameters->ny + 1;

	//-----------------------
	// southern boundary
	//-----------------------

	switch( _parameters->wS )
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

	switch( _parameters->wN )
	{
		case NO_SLIP:
			for( int x = 1; x < nx1; ++x )
			{
				_U[ny1][x] = -_U[_parameters->ny][x];
				_V[_parameters->ny][x] = 0.0;
			}
			break;
		case FREE_SLIP:
			for( int x = 1; x < nx1; ++x )
			{
				_U[ny1][x] = _U[_parameters->ny][x];
				_V[_parameters->ny][x] = 0.0;
			}
			break;
		case OUTFLOW:
			for( int x = 1; x < nx1; ++x )
			{
				_U[ny1][x] = _U[_parameters->ny][x];
				_V[_parameters->ny][x] = _V[_parameters->ny-1][x];
			}
			break;
	}


	//-----------------------
	// western boundary
	//-----------------------

	switch( _parameters->wW )
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

	switch( _parameters->wE )
	{
		case NO_SLIP:
			for( int y = 1; y < ny1; ++y )
			{
				_U[y][_parameters->nx] = 0.0;
				_V[y][nx1] = -_V[y][_parameters->nx];
			}
			break;
		case FREE_SLIP:
			for( int y = 1; y < ny1; ++y )
			{
				_U[y][_parameters->nx] = 0.0;
				_V[y][nx1] = _V[y][_parameters->nx];
			}
			break;
		case OUTFLOW:
			for( int y = 1; y < ny1; ++y )
			{
				_U[y][_parameters->nx] = _U[y][_parameters->nx-1];
				_V[y][nx1] = _V[y][_parameters->nx];
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
	// todo: at corners (fluid cell with walls at two adjecent walls) the velocity value
	//       inside the corner is dependent on the order the cells are processed.
	//       As no wall is allowed to be between two fluid cells, this should not matter,
	//       but leads to different values on the GPU.
	//        => check if it really doesn't matter

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
					_V[y-1][x] = 0.0;
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
					_V[y-1][x] = -_V[y-1][x-1];
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

	if ( _parameters->problem == "moving_lid" )
	{
		//const REAL lid_velocity = 1.0;
		for ( int x = 1; x < _parameters->nx + 1; ++x )
		{
			_U[0][x] = 2.0 - _U[1][x];
		}
	}
	else if ( _parameters->problem == "left_inflow" )
	{
		for ( int y = 1; y < _parameters->ny + 1; ++y )
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

	REAL u_max = 0.0, v_max = 0.0;
	REAL opt_a, opt_x, opt_y, min;

	// get u_max and v_max: iterate over arrays U and V (same size => one loop)

	// faster than comparing using <=
	int nx1 = _parameters->nx + 1;
	int ny1 = _parameters->ny + 1;

	for ( int y = 1; y < ny1; ++y )
	{
		for ( int x = 1; x < nx1; ++x )
		{
			if( fabs( _U[y][x] ) > u_max )
				u_max = fabs( _U[y][x] );
			if( fabs( _V[y][x] ) > v_max )
				v_max = fabs( _V[y][x] );
		}
	}

	// compute the three options for the min-function
	opt_a =   ( _parameters->re / 2.0 )
			* 1.0 / (
				  1.0 / (_parameters->dx * _parameters->dx)
				+ 1.0 / (_parameters->dy * _parameters->dy)
			);
	opt_x = _parameters->dx / fabs( u_max );
	opt_y = _parameters->dy / fabs( v_max );

	// get smallest value
	min = opt_a < opt_x ? opt_a : opt_x;
	min = min   < opt_y ? min   : opt_y;

	// compute delta t
	_parameters->dt = _parameters->tau * min;
}

//============================================================================
void NavierStokesCPU::computeFG ( )
{
	// y coordinates are counted from lower left edge

	REAL alpha = 0.9; // todo: select alpha

	// faster than comparing using <=
	int nx1 = _parameters->nx + 1;
	int ny1 = _parameters->ny + 1;

	for( int y = 1; y < ny1; ++y )
	{
		for( int x = 1; x < nx1; ++x )
		{
			//-----------------------
			// compute F
			//-----------------------

			// according to formula 3.36

			// compute F between fluid cells only
			if( _FLAG[y][x] == C_F && _FLAG[y][x+1] == C_F ) // second cell test for not to overwrite boundary values
			{
				_F[y][x] =
					_U[y][x] + _parameters->dt *
					(
						(
							d2m_dx2 ( _U, x, y ) +
							d2m_dy2 ( _U, x, y )
						) / _parameters->re
						- du2_dx ( x, y, alpha )
						- duv_dy ( x, y, alpha )
						+ _parameters->gx
					);
			}
			else
			{
				// according to formula 3.42
				_F[y][x]   = _U[y][x];
			}


			//-----------------------
			// compute G
			//-----------------------

			// according to formula 3.37

			// compute G between fluid cells only
			if( _FLAG[y][x] == C_F && _FLAG[y+1][x] == C_F )
			{
				_G[y][x] =
					_V[y][x] + _parameters->dt *
					(
						(
							d2m_dx2 ( _V, x, y ) +
							d2m_dy2 ( _V, x, y )
						) / _parameters->re
						- dv2_dy ( x, y, alpha )
						- duv_dx ( x, y, alpha )
						+ _parameters->gy
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
	/*
	// todo: really not necessary?
	for ( int y = 1; y < ny1; ++y )
	{
		for ( int x = 1; x < nx1; ++x )
		{
			switch ( _FLAG[y][x] )
			{
			//	case B_N:
			//		_G[y][x]   = _V[y][x];
			//		break;
				case B_S:
					_G[y-1][x] = _V[y-1][x];
					break;
				case B_W:
					_F[y][x-1] = _U[y][x-1];
					break;
			//	case B_E:
			//		_F[y][x]   = _U[y][x];
			//		break;
				case B_NW:
					_F[y][x-1] = _U[y][x-1];
			//		_G[y][x]   = _V[y][x];
					break;
			//	case B_NE:
			//		_F[y][x]   = _U[y][x];
			//		_G[y][x]   = _V[y][x];
			//		break;
				case B_SW:
					_F[y][x-1] = _U[y][x-1];
					_G[y-1][x] = _V[y-1][x];
					break;
				case B_SE:
			//		_F[y][x]   = _U[y][x];
					_G[y-1][x] = _V[y-1][x];
					break;
			}
		}
	}
	*/


	// setting boundary values for f according to formula 3.42
	for ( int y = 1; y < ny1; ++y )
	{
		_F[y][0]   = _U[y][0];
		_F[y][_parameters->nx] = _U[y][_parameters->nx];
	}

	// setting boundary values for g according to formula 3.42
	for ( int x = 1; x < nx1; ++x )
	{
		_G[0][x]   = _V[0][x];
		_G[_parameters->ny][x] = _V[_parameters->ny][x];
	}
}

//============================================================================
void NavierStokesCPU::computeRightHandSide ( )
{
	// compute right-hand side of poisson equation according to formula 3.38

	// faster than comparing using <=
	int nx1 = _parameters->nx + 1;
	int ny1 = _parameters->ny + 1;

	for ( int y = 1; y < ny1; ++y )
	{
		for ( int x = 1; x < nx1; ++x )
		{
			// todo: only for fluid cells?
			_RHS[y][x] = ( 1 / _parameters->dt ) *
				(
					( _F[y][x] - _F[y][x-1] ) / _parameters->dx +
					( _G[y][x] - _G[y-1][x] ) / _parameters->dy
				);
		}
	}
}

//============================================================================
REAL NavierStokesCPU::SORPoisson ( )
{
	int nx1 = _parameters->nx + 1;
	int ny1 = _parameters->ny + 1;

	REAL dx2 = _parameters->dx * _parameters->dx;
	REAL dy2 = _parameters->dy * _parameters->dy;

	// gauss seidel is writing back the results back to the original array immediately
	// so a mixture of values from timestep n and n+1 is used

	// the epsilon-parameters in formula 3.44 are set to 1.0 according to page 38
	REAL constant_expr = _parameters->omega / ( 2.0 / dx2 + 2.0 / dy2 );
	//REAL constant_expr = _omega / ( 2.0 * (1.0 / (_dx * _dx) + 1.0 / (_dy * _dy)) );

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
					( 1.0 - _parameters->omega ) * _P[y][x] +
					constant_expr * (
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
						//_P[y][x] = (_P[y+1][x] + _P[y][x-1]) / 2;	// todo is this more correct?
						break;
					case B_NE:
						_P[y][x] = (_P[y+1][x] + _P[y][x+1]) / 2;
						break;
					case B_SW:
						_P[y][x] = (_P[y-1][x] + _P[y][x-1]) / 2;
						break;
					case B_SE:
						_P[y][x] = (_P[y+1][x] + _P[y][x-1]) / 2;
						//_P[y][x] = (_P[y-1][x] + _P[y][x+1]) / 2;	// todo is this more correct? Karman not working any longer with this line!
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
	// only Neumann
	// todo: implement dirichlet and periodic

	for ( int x = 1; x < nx1; ++x )
	{
		_P[0][x]   = _P[1][x];
		_P[ny1][x] = _P[_parameters->ny][x];
	}

	for ( int y = 1; y < ny1; ++y )
	{
		_P[y][0]   = _P[y][1];
		_P[y][nx1] = _P[y][_parameters->nx];
	}

	//-----------------------
	// residual
	//-----------------------

	// compute residual using L²-Norm (according to formula 3.45 and 3.46)

	REAL tmp;
	REAL sum = 0.0;
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

				//tmp =
				//	  ( _P[y][x+1] - 2.0 * _P[y][x] + _P[y][x-1] ) / dx2
				//	+ ( _P[y+1][x] - 2.0 * _P[y][x] + _P[y-1][x] ) / dy2
				//	- _RHS[y][x];

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

	int nx1 = _parameters->nx + 1;
	int ny1 = _parameters->ny + 1;

	REAL dt_dx = _parameters->dt / _parameters->dx;
	REAL dt_dy = _parameters->dt / _parameters->dy;

	// update u. two nested loops because of different limits
	for ( int y = 1; y < ny1; ++y )
	{
		for ( int x = 1; x < _parameters->nx; ++x )
		{
			if ( _FLAG[y][x] == C_F && _FLAG[y][x+1] == C_F )
			{
				_U[y][x] = _F[y][x] - dt_dx * ( _P[y][x+1] - _P[y][x] );
			}
		}
	}

	// update v
	for ( int y = 1; y < _parameters->ny; ++y )
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
//	auxiliary functions for F & G
// -------------------------------------------------

//============================================================================
inline REAL NavierStokesCPU::d2m_dx2 ( REAL** M, int x, int y )
{
	return ( M[y][x-1] - 2.0 * M[y][x] + M[y][x+1] ) / ( _parameters->dx * _parameters->dx );
}

//============================================================================
inline REAL NavierStokesCPU::d2m_dy2 ( REAL** M, int x, int y )
{
	return ( M[y-1][x] - 2.0 * M[y][x] + M[y+1][x] ) / ( _parameters->dy * _parameters->dy );
}

//============================================================================
inline REAL NavierStokesCPU::du2_dx  ( int x, int y, REAL alpha )
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
				fabs( _U[y][x] + _U[y][x+1] ) *
					( _U[y][x] - _U[y][x+1] )
				-
				fabs( _U[y][x-1] + _U[y][x] ) *
					( _U[y][x-1] - _U[y][x] )
			)
		) / ( 4.0 * _parameters->dx);
}

//============================================================================
inline REAL NavierStokesCPU::dv2_dy  ( int x, int y, REAL alpha )
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
				fabs( _V[y][x] + _V[y+1][x] ) *
					( _V[y][x] - _V[y+1][x] )
				-
				fabs( _V[y-1][x] + _V[y][x] ) *
					( _V[y-1][x] - _V[y][x] )
			)
		) / ( 4.0 * _parameters->dy );
}

//============================================================================
inline REAL NavierStokesCPU::duv_dx  ( int x, int y, REAL alpha )
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
					fabs( _U[y][x] + _U[y+1][x] ) *
						( _V[y][x] - _V[y][x+1] )
					-
					fabs( _U[y][x-1] + _U[y+1][x-1] ) *
						( _V[y][x-1] - _V[y][x] )
			)
		) / ( 4.0 * _parameters->dx );
}

//============================================================================
inline REAL NavierStokesCPU::duv_dy  ( int x, int y, REAL alpha )
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
				fabs( _V[y][x] + _V[y][x+1] ) *
				   ( _U[y][x] - _U[y+1][x] )
				-
				fabs( _V[y-1][x] + _V[y-1][x+1] ) *
				   ( _U[y-1][x] - _U[y][x] )
			)
		) / ( 4.0 * _parameters->dy );
}
