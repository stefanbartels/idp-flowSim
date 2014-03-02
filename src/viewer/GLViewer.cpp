#include "GLViewer.h"
#include <iostream>

//============================================================================
GLViewer::GLViewer
(
	Parameters* parameters,
	QWidget *parent
) :
	QGLWidget( parent ),
	Viewer( parameters )
{
}

//============================================================================
void GLViewer::renderFrame (
		double** U,
		double** V,
		double** P,
		int it
	)
{
	std::cout << "rendering " << it << std::endl;

	//makeCurrent();
	//updateGL();
}
