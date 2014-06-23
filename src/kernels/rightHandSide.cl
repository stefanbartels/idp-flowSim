// -------------------------------------------------
//	kernels for the right hand side of the pressure equation
// -------------------------------------------------

//============================================================================

// computes the right-hand side of poisson equation according to formula 3.38
// this kernel produces different results, probably due to different
//   floating point precision of GPU and CPU

// todo: try shared local memory for F and G

__kernel void rightHandSideKernel
	(
		__global float* f_g,			// F
		__global float* g_g,			// G
		__global float* rhs_g,			// storage array for righ hand side
		float			dt,				// time step size
		float			dx,				// length delta x of on cell in x-direction
		float			dy,				// length delta y of on cell in y-direction
		int				nx,				// dimension in x direction (including boundaries)
		int				ny				// dimension in y direction (including boundaries)
	)
{
	const unsigned int x   = get_global_id( 0 );
	const unsigned int y   = get_global_id( 1 );
	const unsigned int idx = y * nx + x;

	if( x > 0 &&
		y > 0 &&
		x < nx - 1 &&
		y < ny - 1 )
	{
		// todo: only for fluid cells? => should not make huge differences on GPU
		rhs_g[idx] = ( 1.0 / dt ) *
			(
				( f_g[idx] - f_g[idx - 1 ] ) / dx +
				( g_g[idx] - g_g[idx - nx] ) / dy
			);
	}
}

