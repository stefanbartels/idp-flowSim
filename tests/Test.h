#ifndef TEST_H
#define TEST_H

//********************************************************************
//**    includes
//********************************************************************

#include "../src/Definitions.h"
#include <string>
#include <iostream>

#warning "========== COMPILING TEST =========="

//====================================================================
/*! \class Test
    \brief Superclass for testing
*/
//====================================================================

class Test
{
	public:
	
		// -------------------------------------------------
		// type declarations
		// -------------------------------------------------

		enum ErrorCode
		{
			Success = 0,
			Error = 1
		};

	protected:

		// -------------------------------------------------
		// member variables
		// -------------------------------------------------

		const std::string _name;

//		ErrorCode fail( std::string errorMessage )
//		{
//			std::cerr << errorMessage << std::endl;
//			return Error;
//		}

		friend std::ostream& operator<< ( std::ostream&, const Test* );
		friend std::ostream& operator<< ( std::ostream&, const Test& );

	public:

		// -------------------------------------------------
		//	constructor / destructor
		// -------------------------------------------------

		Test ( std::string name ) : _name( name ) { }

		virtual ~Test ( ) { }

		// -------------------------------------------------
		//	test execution
		// -------------------------------------------------
	
		virtual ErrorCode run ( ) = 0;


		std::string getName ( )
		{
			return _name;
		}



	protected:
		// -------------------------------------------------
		//	auxiliary functions
		// -------------------------------------------------
			//! @name auxiliary functions
			//! @{

		REAL** allocHostMatrix (
				int width,
				int height
			);

		void setHostMatrix (
				REAL** matrix,
				int xStart,
				int xStop,
				int yStart,
				int yStop,
				REAL value
			);

		void freeHostMatrix (
				REAL** matrix
			);
};


#endif // TEST_H
