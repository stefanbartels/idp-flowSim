#ifndef SIMPLEQTVIEWER_H
#define SIMPLEQTVIEWER_H

#include <QGLWidget>
#include "Viewer.h"
#include <QMutex>
#include "../inputParser.h"


class SimpleQTViewer : public QGLWidget, public Viewer
{
    Q_OBJECT

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

public:
	explicit SimpleQTViewer(QWidget *parent = 0);

	void initializeParameters(ProblemParameters* parameters);

    void renderFrame (
            double** U,
            double** V,
            double** P,
            int nx,
            int ny,
            int it
        );

signals:
    
public slots:

private:
    unsigned char* Cp;
	ProblemParameters parameters;
	QMutex mutex;

    
};

#endif // SIMPLEQTVIEWER_H
