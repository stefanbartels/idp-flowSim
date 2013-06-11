#ifndef NAVIERSTOKESCPU_H
#define NAVIERSTOKESCPU_H

//********************************************************************
//**    includes
//********************************************************************

#include "NavierStokesSolver.h"


//====================================================================
/*! \class NavierStokesCpu
    \brief Class for solving of Navier Stokes on CPU
*/
//====================================================================

class NavierStokesCPU : public NavierStokesSolver
{
    public:
		NavierStokesCPU();
};

#endif // NAVIERSTOKESCPU_H
