
//********************************************************************
//**    includes
//********************************************************************

#include "inputParser.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include <string.h>

//********************************************************************
//**    implementation
//********************************************************************

//============================================================================
bool InputParser::readParameters
	(
		int			argc,
		char*		argv[],
		Parameters*	parameters
	)
{
	//-------------------------------
	// parse command line parameters
	//-------------------------------

	char* parameterFileName;
	bool  parameterFileNameSet = false;

	int arg = 1;


	arg = 0;
	while( arg < argc )
	{
		std::cout << argv[arg] << " ";
		arg++;
	}
	std::cout << std::endl;
	arg= 1;


	while( arg < argc )
	{
		if( strcmp( argv[arg], "-vtk" ) == 0 )
		{
			if( arg + 2 < argc )
			{
				parameters->VTKWriteFiles = true;
				parameters->VTKInterval   = atoi( argv[++arg] );
				parameters->VTKIterations = atoi( argv[++arg] );
				++arg;
			}
			else
			{
				printUsage( argv[0] );
				return false;
			}
		}
		else if( !parameterFileNameSet )
		{
			parameterFileName = argv[arg];
			++arg;
		}
		else
		{
			printUsage( argv[0] );
			return false;
		}
	}











	if ( argc > 1 )
	{
		std::cout << "Problem parameter file: " << parameterFileName << std::endl;

		//-------------------------------
		// read parameter file
		//-------------------------------

		std::ifstream file( parameterFileName );

		if ( file.fail() || !file.is_open() )
		{
			std::cerr << "Could not open parameter file \"" << parameterFileName << "\"" << std::endl;
			return false;
		}

		std::string	buffer;
		int			i_buffer;
		REAL		d_buffer;

		int line = 0;
		int numReadValues = 0;

		// parse file

		// read word from file
		file >> buffer;

		while ( file.good() )
		{
			if ( buffer.substr( 0, 1 ) == "#" ) // comment
			{
			}

			//=================================
			// geometry data
			//=================================
			else if ( buffer == "xlength" )
			{
				file >> d_buffer;
				parameters->xlength = d_buffer;
				++numReadValues;
			}
			else if ( buffer == "ylength" )
			{
				file >> d_buffer;
				parameters->ylength = d_buffer;
				++numReadValues;
			}
			else if ( buffer == "grid_x" )
			{
				file >> i_buffer;
				parameters->nx = i_buffer;
				++numReadValues;
			}
			else if ( buffer == "grid_y" )
			{
				file >> i_buffer;
				parameters->ny = i_buffer;
				++numReadValues;
			}

			//=================================
			// time stepping data
			//=================================
			else if ( buffer == "delta_t" )
			{
				file >> d_buffer;
				parameters->dt = d_buffer;
				++numReadValues;
			}
			else if ( buffer == "tau" )
			{
				file >> d_buffer;
				parameters->tau = d_buffer;
				++numReadValues;
			}

			//=================================
			// pressure-iteration data
			//=================================
			else if ( buffer == "it_max" )
			{
				file >> i_buffer;
				parameters->it_max = i_buffer;
				++numReadValues;
			}
			else if ( buffer == "epsilon" )
			{
				file >> d_buffer;
				parameters->epsilon = d_buffer;
				++numReadValues;
			}
			else if ( buffer == "omega" )
			{
				file >> d_buffer;
				parameters->omega = d_buffer;
				++numReadValues;
			}
			else if ( buffer == "gamma" )
			{
				file >> d_buffer;
				parameters->gamma = d_buffer;
				++numReadValues;
			}

			//=================================
			// problem dependent quantities
			//=================================
			else if ( buffer == "re" )
			{
				file >> d_buffer;
				parameters->re = d_buffer;
				++numReadValues;
			}
			else if ( buffer == "gravity_x" )
			{
				file >> d_buffer;
				parameters->gx = d_buffer;
				++numReadValues;
			}
			else if ( buffer == "gravity_y" )
			{
				file >> d_buffer;
				parameters->gy = d_buffer;
				++numReadValues;
			}
			else if ( buffer == "ui" )
			{
				file >> d_buffer;
				parameters->ui = d_buffer;
				++numReadValues;
			}
			else if ( buffer == "vi" )
			{
				file >> d_buffer;
				parameters->vi = d_buffer;
				++numReadValues;
			}
			else if ( buffer == "pi" )
			{
				file >> d_buffer;
				parameters->pi = d_buffer;
				++numReadValues;
			}

			//=================================
			// boundary conditions
			//=================================
			else if ( buffer == "boundary_N" )
			{
				file >> i_buffer;
				parameters->wN = i_buffer;
				++numReadValues;
			}
			else if ( buffer == "boundary_S" )
			{
				file >> i_buffer;
				parameters->wS = i_buffer;
				++numReadValues;
			}
			else if ( buffer == "boundary_W" )
			{
				file >> i_buffer;
				parameters->wW = i_buffer;
				++numReadValues;
			}
			else if ( buffer == "boundary_E" )
			{
				file >> i_buffer;
				parameters->wE = i_buffer;
				++numReadValues;
			}
			else if ( buffer == "problem" )
			{
				std::string s_buffer;
				file >> s_buffer;
				parameters->problem = s_buffer;
				++numReadValues;
			}
			else if ( buffer == "map" )
			{
				std::string s_buffer;
				file >> s_buffer;
				parameters->obstacleFile = s_buffer;
				++numReadValues;
			}
			else // unknown parameter
			{
				std::cerr << "Unknown parameter \"" << buffer << "\". Please check your input file!" << std::endl;
				file.close();
				return false;
			}

			++line;

			// ignore rest of line
			file.ignore(1000, '\n');

			// read word from file
			file >> buffer;
		}

		if ( !file.eof() )
		{
			std::cerr << "Aborted parameter parsing after line " << line << "." << std::endl;
			// return false;
		}

		if ( file.bad() )
		{
			std::cerr << "Parameter file corrupted." << std::endl;
			return false;
		}

		file.close();

		// no values found? => no valid parameter file
		if ( numReadValues == 0 )
		{
			std::cerr << "No valid parameter file." << std::endl;
			return false;
		}
		// not all values given? =>	using standard values for missing parameters
		else if ( numReadValues < 22 )
		{
			std::cerr << "Using standard values for missing parameters. Please check yout input file!" << std::endl;
		}

	}
	else
	{
		std::cerr << "No parameter file specified. Using default parameters." << std::endl;
	}

	// calculate cell dimensions
	parameters->dx = parameters->xlength / (REAL)parameters->nx;
	parameters->dy = parameters->ylength / (REAL)parameters->ny;

	//-------------------------------
	// read obstacle map
	//-------------------------------

	if ( !readObstacleMap(
			 &(parameters->obstacleMap),
			 parameters->nx,
			 parameters->ny,
			 parameters->obstacleFile )
		 )
	{
		std::cerr << "Error reading obstacle map." << std::endl;
		return false;
	}

	// done
	return true;
}







//============================================================================
bool** allocateObstacleMap
	(
		int width,
		int height
	)
{
	// the domain boundaries are treated as obstacles as well,
	// so the map must be slightly larger than the image

	// array of pointers to rows
	bool** map = (bool**)malloc( (height+2) * sizeof( bool* ) );

	// the actual data array. allocation for all rows at once to get continuous memory
	bool* matrix = (bool*)malloc( (width+2) * (height+2) * sizeof( bool ) );

	map[0] = matrix;
	for ( int i = 1; i < (height+2); ++i )
	{
		map[i] = matrix + i * (width+2);
	}

	return map;
}





//============================================================================
bool InputParser::readObstacleMap
	(
		bool***		obstacleMap,
		int			width,
		int			height,
		std::string	fileName
	)
{
	std::string buffer;
	int i_buffer;

	int pgm_width, pgm_height;

	bool binary, moreThanOneByte;

	bool** map;

	int nx1 = width  + 1,
		ny1 = height + 1;

	//-----------------------
	// create empty map if no image is given
	//-----------------------

	if( fileName.compare("") == 0 )
	{
		#if VERBOSE
			std::cout << "Creating empty map..." << std::endl;
		#endif

		map = allocateObstacleMap( width, height );

		// initialise empty obstacle map
		for( int y = 1; y < ny1; ++y )
		for( int x = 1; x < nx1; ++x )
		{
			map[y][x] = true;
		}

		// set boundaries to obstacles

		for( int x = 0; x < width+2; ++x )
		{
			map[0][x]   = false;
			map[ny1][x] = false;
		}

		// 1 - height, because edges have already been set by previous loop
		for( int y = 1; y < height+1; ++y )
		{
			map[y][0]   = false;
			map[y][nx1] = false;
		}

		// pass array back
		*obstacleMap = map;

		return true;
	}


	//-----------------------
	// open obstacle map file
	//-----------------------

	std::ifstream file( fileName.c_str() );

	if ( file.fail() || !file.is_open() )
	{
		std::cerr << "Could not open obstacle map file \"" << fileName << "\"" << std::endl;
		return false;
	}

	//-----------------------
	// check PGM header
	//-----------------------

	getline( file, buffer );

	if( buffer == "P2" )
	{
		// PGM is ASCII file
		binary = false;
	}
	else if( buffer == "P5" )
	{
		// PGM is binary file
		binary = true;
	}
	else
	{
		std::cerr << "Obstacle map is no compatible PGM image! Supported types: P2 and P5" << std::endl;
		file.close();
		return false;
	}

	// skip comments

	file >> buffer;
	while( file.good() && buffer.substr( 0, 1 ) == "#" ) // comment
	{
		// ignore rest of line
		file.ignore(1000, '\n');

		file >> buffer;
	}

	//-----------------------
	// read image dimensions (width, height)
	//-----------------------

	pgm_width  = atoi( buffer.c_str() );
	file >> i_buffer;
	pgm_height = i_buffer;

	if( pgm_width != width || pgm_height != height )
	{
		std::cerr << "Obstacle map does not fit domain size!" << std::endl
				  << "Domain size: " << width << "*" << height << std::endl
				  << "PGM size: " << pgm_width << "x" << pgm_height << std::endl;
		file.close();
		return false;
	}

	// read maximum color value (important for binary images)
	file >> i_buffer;

	if( i_buffer > 255 )
		moreThanOneByte = true;
	else
		moreThanOneByte = false;


	//-----------------------
	// create obstacle array
	//-----------------------

	map = allocateObstacleMap( width, height );

	//-----------------------
	// parse image data
	//-----------------------

	// interior cells
	if( binary )
	{
		// binary PGM file with two bytes per pixel
		if( moreThanOneByte )
		{
			unsigned char byte1,byte2;

			// two bytes are enough as the max. number
			// of grey values allowed for PGM is 65535

			for( int y = 1; y < ny1; ++y )
			for( int x = 1; x < nx1; ++x )
			{
				byte2 = file.get();
				byte1 = file.get();

				map[y][x] = ( (byte2 << 8) + byte1 ) != 0;
			}
		}
		else // binary PGM file with one byte per pixel
		{
			unsigned char byte;

			for( int y = 1; y < ny1; ++y )
			for( int x = 1; x < nx1; ++x )
			{
				byte = file.get();
				map[y][x] = byte != 0;
			}
		}
	}
	else // ASCII PGM file
	{
		for( int y = 1; y < ny1; ++y )
		for( int x = 1; x < nx1; ++x )
		{
			file >> i_buffer;
			map[y][x] = i_buffer != 0;
		}
	}

	file.close();

	//-----------------------
	// set boundaries to obstacles
	//-----------------------

	for( int x = 0; x < width+2; ++x )
	{
		map[0][x]   = false;
		map[ny1][x] = false;
	}

	// 1 - height, because edges have already been set by previous loop
	for( int y = 1; y < height+1; ++y )
	{
		map[y][0]   = false;
		map[y][nx1] = false;
	}

	// pass array back
	*obstacleMap = map;

	return true;
}

//============================================================================
void InputParser::printParameters
	(
		Parameters *parameters
	)
{
	std::cout << "====================" << std::endl << "Parameter set:" << std::endl;

	std::cout << "Domain size:\t" << parameters->xlength << " x " << parameters->ylength << std::endl;
	std::cout << "Grid size:\t" << parameters->nx << " x " << parameters->ny << std::endl;

	std::cout << "Time step Δt:\t" << parameters->dt << std::endl;
	std::cout << "Safety factor τ:\t" << parameters->tau << std::endl;

	std::cout << "Max. SOR iterations:\t" << parameters->it_max << std::endl;

	std::cout << "ε:\t" << parameters->epsilon << std::endl;
	std::cout << "ω:\t" << parameters->omega << std::endl;
	std::cout << "γ:\t" << parameters->gamma << std::endl;

	std::cout << "Reynolds number:\t" << parameters->re << std::endl;
	std::cout << "Gravity X:\t" << parameters->gx << std::endl;
	std::cout << "Gravity Y:\t" << parameters->gy << std::endl;

	std::cout << "Initial horizontal velocity:\t" << parameters->ui << std::endl;
	std::cout << "Initial vertical velocity:\t" << parameters->vi << std::endl;
	std::cout << "Initial pressure:\t" << parameters->pi << std::endl;

	std::cout << "Northern boundary:\t" << parameters->wN << std::endl;
	std::cout << "Southern boundary:\t" << parameters->wS << std::endl;
	std::cout << "Western boundary:\t" << parameters->wW << std::endl;
	std::cout << "Eastern boundary:\t" << parameters->wE << std::endl;
	std::cout << "(1: free slip, 2: no slip, 3: outflow, 4: periodic)";

	std::cout << "Problem:\t" << parameters->problem << std::endl;
	std::cout << "Obstacle map:\t" << parameters->obstacleFile << std::endl;
	std::cout << "====================" << std::endl;
}

//============================================================================
void InputParser::printUsage
	(
		char* programName
	)
{
	std::cerr << "Error reading parameters!\n"
			  << "Usage: " << programName << " [-vtk INTERVAL ITERATIONS] [PARAMETER_FILE]" << std::endl;
}
