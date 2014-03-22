// -------------------------------------------------
//	kernels for velocity updating after SOR loop
// -------------------------------------------------

// boundary flag for fluid cells
#define C_F		0x10

//============================================================================

// update u and v according to 3.34 and 3.35
// todo: try local shared memory for P and FLAG

__kernel void updateUVKernel
	(
		__global float*			p_g,			// pressure array
		__global float*			f_g,			// F
		__global float*			g_g,			// G
		__global unsigned char*	flag_g,			// array with fluid/boundary cell flags
		__global float*			u_g,			// horizontal velocity
		__global float*			v_g,			// vertical velocity
		float					dt,				// time step size
		float					dx,				// length delta x of on cell in x-direction
		float					dy,				// length delta y of on cell in y-direction
		int						nx,				// dimension in x direction (including boundaries)
		int						ny,				// dimension in y direction (including boundaries)
		int						pitch
	)
{
	const unsigned int x   = get_global_id( 0 );
	const unsigned int y   = get_global_id( 1 );
	const unsigned int idx = y * pitch + x;

	float dt_dx = dt / dx;
	float dt_dy = dt / dy;

	// guards
	if( x > 0 &&
		y > 0 &&
		x < nx - 1 &&
		y < ny - 1 )
	{
		// update horizontal velocity U
		if( x < nx - 2 && flag_g[idx] == C_F && flag_g[idx + 1] == C_F )
		{
			u_g[idx] = f_g[idx] - dt_dx * ( p_g[idx + 1] - p_g[idx] );
		}

		// update vertical velocity V
		if ( y < ny - 2 && flag_g[idx] == C_F && flag_g[idx + pitch] == C_F )
		{
			v_g[idx] = g_g[idx] - dt_dy * ( p_g[idx + pitch] - p_g[idx] );
		}
	}
}

