#include "SimpleQTViewer.h"
#include <iostream>

SimpleQTViewer::SimpleQTViewer(QWidget *parent) :
    QGLWidget(parent)
{

}
    void SimpleQTViewer::initializeGL()
    {

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    }

    void SimpleQTViewer::resizeGL(int nwidth, int nheight)
    {


    }

    void SimpleQTViewer::paintGL()
    {
        unsigned char* Test = (unsigned char*) malloc(width*height*3 * sizeof(unsigned char));
        for(int i = 0; i < width*height*3; i++)
        {
            Test[i] = 255;
        }
        //glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, Cp);

        glDrawPixels(250, 250, GL_RGB, GL_UNSIGNED_BYTE, Test);

    }

    void SimpleQTViewer::renderFrame(double **U, double **V, double **P, int nx, int ny, int it)
    {
        width = nx+2;
        height = ny+2;

		makeCurrent();
		updateGL();

        /*

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

        //std::cerr << "\nmax value is " << max;

        // convert array to int array and normalize to 0 - 255
        unsigned char C[size];
        double factor = 0.0;

        if ( max - min != 0.0 )
            factor = 255 / ( max - min );

        //std::cerr << "\nfactor is " << factor;

        for ( int i = 0; i < size; ++i )
        {
            C[i] = (char)( (T[i] - min ) * factor );
        }

        Cp = (unsigned char*)malloc(size*3 * sizeof(unsigned char));
        for(int i = 0; i < size; ++i)
        {
            Cp[3*i] = C[i];
            Cp[3*i+1] = C[i+1];
            Cp[3*i+2] = C[i+2];

        }



        */

    }
