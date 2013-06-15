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
	stringstream img_name;
	img_name << "it_" << it << ".pgm";

	ofstream fimg ( img_name.str().c_str() );

	if ( !fimg.is_open() )
	{
		cerr << "\nFailed to open image file " << img_name.str();
		return;
	}

	// copy array

	double T[nx*ny];
	double max = 0.0;

	for ( int i = 0; i < nx*ny; ++i )
	{
		T[i] = A[i];
		if ( T[i] > max )
			max = T[i];
	}

	// convert array to int array and normalize to 0 - 255
	int I[nx*ny];
	double factor = 255.0 / max;

	for ( int i = 0; i < nx*ny; ++i )
	{
		I[i] = (int)( T[i] * factor );
	}


	// pgm header
	fimg << "P5\n" << nx << " " << ny << " 255\n";

	fimg.write( (char *)I, nx * ny * sizeof( char ));

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

	while ( n < 10 )
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
