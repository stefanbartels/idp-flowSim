#ifndef INPUTPARSER_H
#define INPUTPARSER_H

//********************************************************************
//**    includes
//********************************************************************

#include "Definitions.h"
#include "Parameters.h"
#include <string>
#include <stdlib.h>

//====================================================================
/*! \class InputParser
	\brief Class for input file parsing
*/
//====================================================================

class InputParser
{
	public:
		// -------------------------------------------------
		//	static parameter handling functions
		// -------------------------------------------------
			//! @name static parameter handling functions
			//! @{

			//! \brief parses the command line parameters and reads
			//! the parameters from a given config file
			//! \param number of command line arguments
			//! \param array of command line arguments
			//! \param pointer to parameter structure to fill with the imported values

		static bool readParameters
			(
				int			argc,
				char*		argv[],
				Parameters*	parameters
			);

			//! \brief creates an obstacle map from a PGM image
			//! \param obstacle map pointer. The memory will be allocated within the function
			//! \param width of the simulation domain
			//! \param height of the simulation domain
			//! \param file name of the PGM file to read

			//! \todo move check for valid map from solver to here!
		static bool readObstacleMap
			(
				bool***		obstacleMap,
				int			width,
				int			height,
				std::string	fileName
			);

			//! \brief prints the parameters to console
			//! \param pointer to parameter structure

		static void printParameters
			(
				Parameters*	parameters
			);

			//! \brief prints the parameter usage to console

		static void printUsage
			(
				char* programName
			);

			//! @}
};

#endif // INPUTPARSER_H
