
//********************************************************************
//**    includes
//********************************************************************

#include <stdlib.h>
#include "navierStokesSolver.h"


//********************************************************************
//**    implementation
//********************************************************************

// -------------------------------------------------
//	initialisation
// -------------------------------------------------

//============================================================================
NavierStokesSolver::NavierStokesSolver ( Parameters *parameters )
{
	_parameters = parameters;
}

//============================================================================
NavierStokesSolver::~NavierStokesSolver ( )
{

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
