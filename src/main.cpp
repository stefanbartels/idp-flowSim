//********************************************************************
//**    includes
//********************************************************************

#include <iostream>
#include <time.h>
#include <string>

#include "solver/navierStokesCPU.h"
#include "inputParser.h"

// temp
#include <sstream>
#include <fstream>


//********************************************************************
//**    implementation
//********************************************************************

using namespace std;



// temporary code for image writing
void writePGM ( double* A, int nx, int ny, int it )
{
	char img_name[32];
	sprintf( img_name, "output/it_%05d.pgm", it );

	ofstream fimg ( img_name );

	if ( !fimg.is_open() )
	{
		cerr << "\nFailed to open image file " << img_name;
		return;
	}

	// copy array

	int size = (nx+2)*(ny+2);

	double T[size];
	double max = 0.0, min = 0.0;

	for ( int i = 0; i < size; ++i )
	{
		T[i] = A[i];
		if ( T[i] > max )
			max = T[i];
		if ( T[i] < min )
			min = T[i];
	}

	cerr << "\nmax value is " << max;

	// convert array to int array and normalize to 0 - 255
	unsigned char C[size];
	double factor = 0.0;

	if ( max - min != 0.0 )
		factor = 255 / ( max - min );

	cerr << "\nfactor is " << factor;

	for ( int i = 0; i < size; ++i )
	{
		C[i] = (char)( (T[i] - min ) * factor );
	}


	// pgm header
	fimg << "P5\n" << ( nx + 2 ) << " " << ( ny + 2 ) << " 255\n";

	fimg.write( (char *)C, size * sizeof( unsigned char ));

	fimg.close();

	//delete[] T;
	//delete[] I;
}



int main ( int argc, char* argv[] )
{
	//-----------------------
	// read parameters
	//-----------------------

	// set default problem parameters
	ProblemParameters parameters;
	InputParser::setStandardParameters ( &parameters );

	char* parameterFileName;

	// parse command line arguments
	// only one until now: parameter file name

	// parameter file
	if ( argc > 1 )
	{
		parameterFileName = argv[1];

		cout << "\nProblem parameter file: " << parameterFileName;

		if ( !InputParser::readParameters ( &parameters, parameterFileName ) )
		{
			cerr << "\nError reading parameter file. Exiting...";
			return 1;
		}
	}
	else
	{
		cerr << "\nNo parameter file specified. Using default parameters.";
	}

	// todo: read obstacle map

	// print parameter set to console
	InputParser::printParameters ( &parameters );


	//-----------------------
	// create gui, solver and viewer objects and pass parameters
	//-----------------------

	NavierStokesSolver* solver = new NavierStokesCPU ( );
	solver->setParameters ( &parameters );

	// link gui, solver, viewer

	// start gui in thread


	//-----------------------
	// simulation/visualisation loop
	//-----------------------

	solver->init();

	int n = 0;

	while ( n < 1000 )
	{
		cout << "\ndoing frame " << n;
		// do simulation step
		solver->doSimulationStep( );

		// update visualisation
			// do fancy stuff with opengl


		writePGM ( *solver->getP_CPU(), parameters.nx, parameters.ny, n );

		cout << "\ndone with frame " << n;
		++n;
	}

	//-----------------------
	// cleanup
	//-----------------------

	// todo

	delete solver;

    return 0;
}
