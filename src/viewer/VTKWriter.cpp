#include "VTKWriter.h"

#include <iostream>
#include <fstream>
#include <iomanip>

VTKWriter::VTKWriter
	(
		Parameters* parameters
	) :
	Viewer( parameters )
{
	_nextOutput = 0.0;
}

//============================================================================
void VTKWriter::renderFrame (
        REAL** U,
        REAL** V,
        REAL** P,
		double time,
		unsigned int iteration
	)
{
	// www.vtk.org/VTK/img/file-formats.pdf‎

	if( _nextOutput > time )
		return;

	// determine next output time
	_nextOutput = time + _parameters->VTKInterval;

	std::cout << "Writing vtk file for iteration " << iteration << std::endl;


	// TODO: if a flow field object was used, maintaining the data arrays,
	// pressure and velocities only had to be copied from the GPU memory if required

	int nx = _parameters->nx;
	int ny = _parameters->ny;

	//-----------------------
	// open file
	//-----------------------

	char filename[32];
	sprintf( filename, "output/it_%05d.vtk", iteration );

	std::ofstream vtk ( filename );

	if ( !vtk.is_open() )
	{
		std::cerr << "Failed to open vtk file \"" << filename << "\" for writing! (Does the directory exist?)" << std::endl;
		return;
	}

	// set output flags
	vtk << std::setprecision( 16 );

	//-----------------------
	// write vtk header
	//-----------------------

	vtk << "# vtk DataFile Version 2.0\n"
		<< "Navier Stokes\n"
		<< "ASCII\n\n";

	//-----------------------
	// write point positions
	//-----------------------

	int nx1 = nx + 1,
		ny1 = ny + 1;

	int numPoints = nx1 * ny1;

	vtk << "DATASET STRUCTURED_GRID\n"
		<< "DIMENSIONS " << nx1 << " " << ny1 << " 1\n"
		<< "POINTS " << numPoints << " float\n";

	for( int y = 0; y < ny1; ++y )
	{
		for( int x = 0; x < nx1; ++x )
		{
			vtk << x << " " << y << " 0\n";
		}
	}

	//-----------------------
	// write velocity
	//-----------------------

	int numCells = nx * ny;

	vtk << "\nPOINT_DATA " << numPoints << "\n"
		<< "VECTORS velocity float\n";

	// U and V are located on cell borders
	// => interpolate velocity on grid points

	for( int y = 0; y < ny1; ++y )
	{
		for( int x = 0; x < nx1; ++x )
		{
			vtk << std::fixed <<
				   ( U[y][x] + U[y+1][x] ) * 0.5 // todo: check conversion to float
				<< " " <<
				   ( V[y][x] + V[y][x+1] ) * 0.5
				<< " 0\n";
		}
	}

	//-----------------------
	// write pressure
	//-----------------------

	vtk << "\nCELL_DATA " << numCells << "\n"
		<< "SCALARS pressure float\n"
		<< "LOOKUP_TABLE default\n";

	for( int y = 1; y < ny1; ++y )
	{
		for( int x = 1; x < nx1; ++x )
		{
			vtk << std::fixed << P[y][x] << "\n";
		}
	}

	vtk.close();
}
