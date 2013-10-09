#include "VTKWriter.h"

#include <iostream>
#include <fstream>
#include <iomanip>

VTKWriter::VTKWriter()
{
}

//============================================================================
void VTKWriter::renderFrame
	(
		double **U,
		double **V,
		double **P,
		int nx,
		int ny,
		int it
	)
{
	// www.vtk.org/VTK/img/file-formats.pdf‎

	//-----------------------
	// open file
	//-----------------------

	char img_name[32];
	sprintf( img_name, "output/it_%05d.vtk", it );

	std::ofstream vtk ( img_name );

	if ( !vtk.is_open() )
	{
		std::cerr << "\nFailed to open vtk file \"" << img_name << "\" for writing!";
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

	int numPoints = (nx + 1) * (ny + 1);

	int nx1 = nx + 1,
		ny1 = ny + 1;

	vtk << "DATASET STRUCTURED_GRID\n"
		<< "DIMENSIONS " << ( nx + 1 ) << " " << ( ny + 1 ) << " 1\n"
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

	int numCells = nx * ny;

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
