#ifndef NAVERSTOKESGPU_H
#define NAVERSTOKESGPU_H

//********************************************************************
//**    includes
//********************************************************************

#include "NavierStokesSolver.h"


//====================================================================
/*! \class NavierStokesCpu
    \brief Class for solving of Navier Stokes on GPU
*/
//====================================================================

class NaverStokesGPU : public NavierStokesSolver
{
    public:
        NaverStokesGPU();
};

#endif // NAVERSTOKESGPU_H
