
//********************************************************************
//**    includes
//********************************************************************

#include "inputParser.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdio.h>

//********************************************************************
//**    implementation
//********************************************************************

//============================================================================
void InputParser::setStandardParameters(ProblemParameters *parameters)
{
	parameters->xlength			= 1.0;
	parameters->ylength			= 1.0;
	parameters->nx				= 128;
	parameters->ny				= 128;
	parameters->dt				= 0.02;
	parameters->tau				= 0.5;
	parameters->it_max			= 100;
	parameters->epsilon			= 0.001;
	parameters->omega			= 1.7;
	parameters->gamma			= 0.9;
	parameters->re				= 1000;
	parameters->gx				= 0.0;
	parameters->gy				= 0.0;
	parameters->ui				= 0.0;
	parameters->vi				= 0.0;
	parameters->pi				= 0.0;
	parameters->wN				= 2;
	parameters->wS				= 2;
	parameters->wW				= 2;
	parameters->wE				= 2;
	parameters->problem			= "moving_lid";
	parameters->obstacleFile	= "";
}

//============================================================================
bool InputParser::readParameters
	(
		ProblemParameters*	parameters,
		char*				fileName
	)
{
	// open input file
	std::ifstream file( fileName );

	if ( file.fail() || !file.is_open() )
	{
		std::cerr << "\nCould not open parameter file \"" << fileName << "\"";
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
			std::cerr << "\nUnknown parameter \"" << buffer << "\".\nPlease check yout input file!";
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
		std::cerr << "\nAborted parameter parsing after line " << line << ".";
		// return false;
	}

	if ( file.bad() )
	{
		std::cerr << "\nParameter file corrupted.";
		return false;
	}

	file.close();

	// no values found? => no valid parameter file
	if ( numReadValues == 0 )
	{
		std::cerr << "\nNo valid parameter file.";
		return false;
	}
	// not all values given? =>	using standard values for missing parameters
	else if ( numReadValues < 22 )
	{
		std::cerr << "\nUsing standard values for missing parameters.\nPlease check yout input file!";
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
		std::cerr << "\nCreating empty map...";

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
		std::cerr << "\nCould not open obstacle map file \"" << fileName << "\"";
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
		std::cerr << "\nObstacle map is no compatible PGM image! Supported types: P2 and P5";
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
		std::cerr << "\nObstacle map does not fit domain size!\nDomain size: " << width << "x" << height <<
				"\nPGM size: " << pgm_width << "x" << pgm_height;
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
		ProblemParameters *parameters
	)
{
	std::cout << "\n====================\nParameter set:";

	std::cout << "\nDomain size:\t" << parameters->xlength << " x " << parameters->ylength;
	std::cout << "\nGrid size:\t" << parameters->nx << " x " << parameters->ny;

	std::cout << "\nTime step Δt:\t" << parameters->dt;
	std::cout << "\nSafety factor τ:\t" << parameters->tau;

	std::cout << "\nMax. SOR iterations:\t" << parameters->it_max;

	std::cout << "\nε:\t" << parameters->epsilon;
	std::cout << "\nω:\t" << parameters->omega;
	std::cout << "\nγ:\t" << parameters->gamma;

	std::cout << "\nReynolds number:\t" << parameters->re;
	std::cout << "\nGravity X:\t" << parameters->gx;
	std::cout << "\nGravity Y:\t" << parameters->gy;

	std::cout << "\nInitial horizontal velocity:\t" << parameters->ui;
	std::cout << "\nInitial vertical velocity:\t" << parameters->vi;
	std::cout << "\nInitial pressure:\t" << parameters->pi;

	std::cout << "\nNorthern boundary:\t" << parameters->wN;
	std::cout << "\nSouthern boundary:\t" << parameters->wS;
	std::cout << "\nWestern boundary:\t" << parameters->wW;
	std::cout << "\nEastern boundary:\t" << parameters->wE;
	std::cout << "\n(1: free slip, 2: no slip, 3: outflow, 4: periodic)";

	std::cout << "\nProblem:\t" << parameters->problem;
	std::cout << "\nObstacle map:\t" << parameters->obstacleFile;
	std::cout << "\n====================";
}
