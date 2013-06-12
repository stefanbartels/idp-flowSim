#ifndef NAVIERSTOKESSOLVER_H
#define NAVIERSTOKESSOLVER_H


//====================================================================
/*! \class NavierStokesSolver
    \brief Interface for Navier Stokes Solver implementations
*/
//====================================================================

class NavierStokesSolver
{
	public:
		virtual void doSimulationStep ( ) = 0;
};

#endif // NAVIERSTOKESSOLVER_H
