#include "SimplePGMWriter.h"

#include <iostream>
#include <fstream>

SimplePGMWriter::SimplePGMWriter
	(
		Parameters* parameters
	) :
	Viewer( parameters )
{

}


//============================================================================
void SimplePGMWriter::renderFrame (
		double** U,
		double** V,
		double** P,
		int it
	)
{
	int nx = _parameters->nx;
	int ny = _parameters->ny;

	char img_name[32];
	sprintf( img_name, "output/it_%05d.pgm", it );

	std::ofstream fimg ( img_name );

	if ( !fimg.is_open() )
	{
		std::cerr << "Failed to open image file \"" << img_name << "\"" << std::endl;
		return;
	}

	// find min/max for normalization

	int size = (nx+2)*(ny+2);
	REAL max = 0.0, min = 0.0;

	for ( int i = 0; i < size; ++i )
	{
		if ( (*P)[i] > max )
			max = (*P)[i];
		if ( (*P)[i] < min )
			min = (*P)[i];
	}

	#if VERBOSE
		std::cout << "max value is " << max << std::endl;
	#endif

	// convert array to int array and normalize to 0 - 255
	unsigned char C[size];
	REAL factor = 0.0;

	if ( max - min != 0.0 )
		factor = 255 / ( max - min );

	#if VERBOSE
		std::cout << "factor is " << factor << std::endl;
	#endif

	for ( int i = 0; i < size; ++i )
	{
		C[i] = (char)( ((*P)[i] - min ) * factor );
	}


	// pgm header
	fimg << "P5\n" << ( nx + 2 ) << " " << ( ny + 2 ) << " 255\n";

	fimg.write( (char *)C, size * sizeof( unsigned char ));

	fimg.close();
}
