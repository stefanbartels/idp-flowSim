#ifndef INPUTPARSER_H
#define INPUTPARSER_H

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

		static void setDefaultParameters (
				Parameters*	parameters
			);

		static bool readParameters (
				int			argc,
				char*		argv[],
				Parameters*	parameters
			);

		// TODO: move check for valid map here!
		static bool readObstacleMap (
				bool***		obstacleMap,
				int			width,
				int			height,
				std::string	fileName
			);

		static void printParameters (
				Parameters*	parameters
			);
};

#endif // INPUTPARSER_H
