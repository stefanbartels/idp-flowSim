#ifndef NAVERSTOKESCPU_H
#define NAVERSTOKESCPU_H

//********************************************************************
//**    includes
//********************************************************************

#include "NavierStokesSolver.h"


//====================================================================
/*! \class NavierStokesCpu
    \brief Class for solving of Navier Stokes on CPU
*/
//====================================================================

class NaverStokesCPU : public NavierStokesSolver
{
    public:
        NaverStokesCPU();
};

#endif // NAVERSTOKESCPU_H
