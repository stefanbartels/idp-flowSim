
//********************************************************************
//**    includes
//********************************************************************

#include "inputParser.h"
#include <iostream>
//#include <fstream>
//#include <stdio.h>

using namespace std;


#define FREE_SLIP	1
#define NO_SLIP		2
#define OUTFLOW 	3
#define PERIODIC	4

//********************************************************************
//**    implementation
//********************************************************************

//============================================================================
bool InputParser::readParameters
	(
		ProblemParameters*	parameters,
		std::string			fileName
	)
{
	// open input file
	//ifstream file ( fileName );

	//if ( file.fail() | !file.is_open() )
	//{
	//	return false;
	//};

	// example values: lid-driven cavity (p. 67 ff)

	parameters->xlength = 1.0;
	parameters->ylength = 1.0;

	parameters->nx = 128;
	parameters->ny = 128;

	parameters->dt = 0.02;
	parameters->tau = 0.5;

	parameters->it_max = 100;

	parameters->epsilon = 0.001;
	parameters->omega = 1.7;
	parameters->gamma = 0.9;

	parameters->re = 1000;
	parameters->gx = 0.0;
	parameters->gy = 0.0;

	parameters->ui = 0.0;
	parameters->vi = 0.0;
	parameters->pi = 0.0;

	parameters->wN = NO_SLIP;
	parameters->wS = NO_SLIP;
	parameters->wW = NO_SLIP;
	parameters->wE = NO_SLIP;

	parameters->problem = "moving_lid";

	return true;
}

//============================================================================
bool InputParser::printParameters
	(
		ProblemParameters *parameters
	)
{
	cout << "====================\nParameter set:\n";

	cout << "Domain size:\t" << parameters->xlength << " x " << parameters->ylength << "\n";
	cout << "Grid size:\t" << parameters->nx << " x " << parameters->ny << "\n";

	cout << "Time step Δt:\t" << parameters->dt << "\n";
	cout << "Safety factor τ:\t" << parameters->tau << "\n";

	cout << "Max. SOR iterations:\t" << parameters->it_max << "\n";

	cout << "ε:\t" << parameters->epsilon << "\n";
	cout << "ω:\t" << parameters->omega << "\n";
	cout << "γ:\t" << parameters->gamma << "\n";

	cout << "Reynolds number:\t" << parameters->re << "\n";
	cout << "Gravity X:\t" << parameters->gx << "\n";
	cout << "Gravity Y:\t" << parameters->gy << "\n";

	cout << "Initial horizontal velocity:\t" << parameters->ui << "\n";
	cout << "Initial vertical velocity:\t" << parameters->vi << "\n";
	cout << "Initial pressure:\t" << parameters->pi << "\n";

	cout << "Northern boundary:\t" << parameters->wN << "\n";
	cout << "Southern boundary:\t" << parameters->wS << "\n";
	cout << "Western boundary:\t" << parameters->wW << "\n";
	cout << "Eastern boundary:\t" << parameters->wE << "\n";
	cout << "(1: free slip, 2: no slip, 3: outflow, 4: periodic)\n";

	cout << "Problem:\t" << parameters->problem << "\n";
	cout << "====================\n";
}
