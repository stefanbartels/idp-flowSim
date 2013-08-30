#include "SimplePGMWriter.h"

#include <iostream>
#include <fstream>

using namespace std;

SimplePGMWriter::SimplePGMWriter()
{
}


//============================================================================
void SimplePGMWriter::renderFrame (
		double** U,
		double** V,
		double** P,
		int nx,
		int ny,
		int it
	)
{
	double* T = *P;

	char img_name[32];
	sprintf( img_name, "output/it_%05d.pgm", it );

	ofstream fimg ( img_name );

	if ( !fimg.is_open() )
	{
		cerr << "\nFailed to open image file " << img_name;
		return;
	}

	// find min/max for normalization

	int size = (nx+2)*(ny+2);
	double max = 0.0, min = 0.0;

	for ( int i = 0; i < size; ++i )
	{
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
}
