#include "SimpleQTViewer.h"
#include <iostream>
#include <QMutex>

using namespace std;

SimpleQTViewer::SimpleQTViewer(QWidget *parent) :
    QGLWidget(parent)
{

}

void SimpleQTViewer::initializeParameters(ProblemParameters *para)
{
	parameters = *para;
	cerr<<"\nparameters initialized";
}

void SimpleQTViewer::initializeGL()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	int size = (parameters.nx+2) * (parameters.ny+2);
	Cp = new unsigned char[size * 3];
	for(int i = 0; i < size; i++)
	{
		Cp[3*i] = 0;
		Cp[3*i+1] = 0;
		Cp[3*i+2] = 0;
	}
	//cerr<<"\ntestnx5\n";
	InputParser::printParameters(&parameters);

}

void SimpleQTViewer::resizeGL(int nwidth, int nheight)
{
	glViewport(0, 0, nwidth, nheight);

}

void SimpleQTViewer::paintGL()
{

	cerr<<"\ntest1";

	//TODO: change glDrawPixels to texture quads#
	//http://stackoverflow.com/questions/8774521/how-to-scale-gldrawpixels/8774580#8774580
	glDrawPixels(parameters.nx+2, parameters.ny+2, GL_RGB, GL_UNSIGNED_BYTE, Cp);
	glPixelZoom(2, 2);
}

void SimpleQTViewer::renderFrame(double **U, double **V, double **P, int nx, int ny, int it)
{
	//cerr<<"\ntest11";

	double* T = *P;

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

	//cerr << "\nmax value is " << max;

	// convert array to int array and normalize to 0 - 255
	unsigned char C[size*3];
	double factor = 0.0;

	if ( max - min != 0.0 )
		factor = 255 / ( max - min );

	//cerr << "\nfactor is " << factor;

	for ( int i = 0; i < size; ++i )
	{
		char val = (char)( (T[i] - min ) * factor );
		C[3*i] = val;
		C[3*i+1] = val;
		C[3*i+2] = val;
	}

	Cp = C;

	//cerr<<"\ntest55";


	makeCurrent();
	updateGL();
}
