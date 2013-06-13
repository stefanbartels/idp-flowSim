#ifndef NAVIERSTOKESGPU_H
#define NAVIERSTOKESGPU_H

//********************************************************************
//**    includes
//********************************************************************

#include "navierStokesSolver.h"


//====================================================================
/*! \class NavierStokesCpu
    \brief Class for solving of Navier Stokes on GPU
*/
//====================================================================

class NavierStokesGPU : public NavierStokesSolver
{
    public:
		NavierStokesGPU();
};

#endif // NAVIERSTOKESGPU_H
