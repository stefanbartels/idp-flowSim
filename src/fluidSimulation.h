#ifndef FLUIDSIMULATION_H
#define FLUIDSIMULATION_H
#include "viewer/SimpleQTViewer.h"
#include "solver/navierStokesSolver.h"
#include "inputParser.h"

class fluidSimulation
{
public:
    fluidSimulation();
	fluidSimulation(SimpleQTViewer* viewer);
    void runSimulation();

private:
	SimpleQTViewer* viewer;
	NavierStokesSolver* solver;
	ProblemParameters parameters;
};

#endif // FLUIDSIMULATION_H
