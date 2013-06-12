
//********************************************************************
//**    includes
//********************************************************************

#include <stdlib.h>
#include "NavierStokesCPU.h"

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
	// create matrices U, V, P, RHS, F, G
	// init matrices
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
	for ( it = 0; it < it_max; ++it ) // TODO: complete exit condition
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
}

void NavierStokesCPU::setSpecificBoundaryConditions ( )
{
}


// -------------------------------------------------
//	simulation
// -------------------------------------------------

//============================================================================
void NavierStokesCPU::computeDeltaT ( )
{
}

void NavierStokesCPU::computeFG ( )
{
}

void NavierStokesCPU::computeRightHandSide ( )
{
}

int NavierStokesCPU::SORPoisson ( )
{
	return 0;
}

void NavierStokesCPU::adaptUV ( )
{
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
