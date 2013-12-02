#ifndef MAIN_CPP
#define MAIN_CPP

//********************************************************************
//**    includes
//********************************************************************

#include <vector>
#include <iostream>

#include "cltests/AuxiliaryKernelsTest.h"
#include "cltests/TimestepKernelTest.h"
#include "cltests/BoundaryKernelsTest.h"
#include "cltests/FGKernelsTest.h"

//********************************************************************
//**    implementation
//********************************************************************

//============================================================================
int main ( int argc, char *argv[] )
{
	Test::ErrorCode error = Test::Success;
	std::vector<Test*> tests;

	std::cout << std::endl;

	// add all tests
	tests.push_back( new AuxiliaryKernelsTest("Auxiliary kernels test") );
	tests.push_back( new TimestepKernelTest("Timestep kernel test") );
	tests.push_back( new BoundaryKernelsTest("Boundary kernels test") );
	tests.push_back( new FGKernelsTest("FG computation kernels test") );

	unsigned int size = tests.size();

	for( unsigned int i = 0; i < size; ++i )
	{
		if( tests[i] == NULL )
		{
			std::cout << "ERROR: Could not allocate test case " << i << std::endl;
			break;
		}

		// run test
		std::cout << "Running test " << tests[i] << "...";
		error = tests[i]->run();

		if( error == Test::Error )
		{
			std::cout << " failed!" << std::endl;
			break;
		}
		else
		{
			std::cout << " successful!" << std::endl;
		}
	}

	// remove all tests -> valgrind test
	for( unsigned int i = 0; i < size; ++i )
	{
		if( tests[i] != NULL )
		{
			delete tests[i];
			tests[i] = NULL;
		}
	}
	tests.clear();

	return 0;
}

#endif // MAIN_CPP
