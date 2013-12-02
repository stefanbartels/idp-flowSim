// -------------------------------------------------
//	auxiliary functions
// -------------------------------------------------

// are automatically inlined by OpenCL

//============================================================================
float d2m_dx2 (
		__global float* m_g,
		float dx,
		int idx
	)
{
	return ( m_g[idx - 1] - 2.0 * m_g[idx] + m_g[idx + 1] ) / ( dx * dx );
}

//============================================================================
float d2m_dy2 (
		__global float* m_g,
		float dy,
		int idx,
		int nx
	)
{
	return ( m_g[idx - nx] - 2.0 * m_g[idx] + m_g[idx + nx] ) / ( dy * dy );
}

//============================================================================
float du2_dx (
		__global float* u_g,
		float dx,
		float alpha,
		int idx
	)
{
	return
		(
			(
				( u_g[idx] + u_g[idx + 1] ) *
				( u_g[idx] + u_g[idx + 1] )
				-
				( u_g[idx - 1] + u_g[idx] ) *
				( u_g[idx - 1] + u_g[idx] )
			)
			+
			alpha *
			(
				fabs( u_g[idx] + u_g[idx + 1] ) *
				    ( u_g[idx] - u_g[idx + 1] )
				-
				fabs( u_g[idx - 1] + u_g[idx] ) *
				    ( u_g[idx - 1] - u_g[idx] )
			)
		) / ( 4.0 * dx);
}

//============================================================================
float dv2_dy (
		__global float* v_g,
		float dy,
		float alpha,
		int idx,
		int nx
	)
{
	return
		(
			(
				( v_g[idx] + v_g[idx + nx] ) *
				( v_g[idx] + v_g[idx + nx] )
				-
				( v_g[idx - nx] + v_g[idx] ) *
				( v_g[idx - nx] + v_g[idx] )
			)
			+
			alpha *
			(
				fabs( v_g[idx] + v_g[idx + nx] ) *
				    ( v_g[idx] - v_g[idx + nx] )
				-
				fabs( v_g[idx - nx] + v_g[idx] ) *
				    ( v_g[idx - nx] - v_g[idx] )
			)
		) / ( 4.0 * dy );
}

//============================================================================
float duv_dx (
		__global float* u_g,
		__global float* v_g,
		float dx,
		float alpha,
		int idx,
		int nx
	)
{
	return
		(
			(
				( u_g[idx] + u_g[idx + nx] ) *
				( v_g[idx] + v_g[idx + 1] )
				-
				( u_g[idx - 1] + u_g[idx + nx - 1] ) *
				( v_g[idx - 1] + v_g[idx] )
			)
			+
			alpha *
			(
					fabs( u_g[idx] + u_g[idx + nx] ) *
					    ( v_g[idx] - v_g[idx + 1] )
					-
					fabs( u_g[idx - 1] + u_g[idx + nx - 1] ) *
					    ( v_g[idx - 1] - v_g[idx] )
			)
		) / ( 4.0 * dx );
}

//============================================================================
float duv_dy (
		__global float* u_g,
		__global float* v_g,
		float dy,
		float alpha,
		int idx,
		int nx
	)
{
	return
		(
			(
				( v_g[idx] + v_g[idx + 1] ) *
				( u_g[idx] + u_g[idx + nx] )
				-
				( v_g[idx - nx] + v_g[idx - nx + 1] ) *
				( u_g[idx - nx] + u_g[idx] )
			)
			+
			alpha *
			(
				fabs( v_g[idx] + v_g[idx + 1] ) *
				    ( u_g[idx] - u_g[idx + nx] )
				-
				fabs( v_g[idx - nx] + v_g[idx - nx + 1] ) *
				    ( u_g[idx - nx] - u_g[idx] )
			)
		) / ( 4.0 * dy );
}


// -------------------------------------------------
//	kernels for F and G computation
// -------------------------------------------------

// boundary flags for arbitrary geometries
// for explanation, see Definitions.h
#define C_F		0x10
#define C_B		0x00

#define B_N		0x01
#define B_S		0x02
#define B_W		0x04
#define B_E		0x08

#define B_NW	0x05
#define B_NE	0x09
#define B_SW	0x06
#define B_SE	0x0A

//============================================================================

// todo: change u, v and flag to constant memory
// todo: try local shared memory for u and v
// todo: reduce kernel range according to guards

__kernel void computeF
	(
		__global float*			u_g,			// horizontal velocity
		__global float*			v_g,			// horizontal velocity
		__global unsigned char*	flag_g,			// array with fluid/boundary cell flags
		__global float*			f_g,			// storage array for F
		float					gx,				// body force in x direction (gravity)
		float					dt,				// time step size
		float					re,				// Reynolds number
		float					alpha,
		float					dx,				// length delta x of on cell in x-direction
		float					dy,				// length delta y of on cell in y-direction
		int						nx,				// dimension in x direction (including boundaries)
		int						ny				// dimension in y direction (including boundaries)
	)
{
	const unsigned int x   = get_global_id( 0 );
	const unsigned int y   = get_global_id( 1 );
	const unsigned int idx = y * nx + x;

	// todo: check guards
	if( y > 0 &&
		y < ny - 1 )
	{
		 if( x > 0 &&
			x < nx - 1 )
		{
			// compute F between fluid cells only
			if( flag_g[idx] == C_F && flag_g[idx + 1] == C_F ) // second cell test for not to overwrite boundary values
			{
				f_g[idx] =
					u_g[idx] + dt *
					(
						(
							d2m_dx2 ( u_g, dx, idx ) +
							d2m_dy2 ( u_g, dy, idx, nx )
						) / re
						- du2_dx ( u_g, dx, alpha, idx )
						- duv_dy ( u_g, v_g, dy, alpha, idx, nx )
						+ gx
					);
			}
			else
			{
				// according to formula 3.42
				f_g[idx]   = u_g[idx];
			}
		}
		else
		{
			f_g[idx]   = u_g[idx];
		}
	}
}


//============================================================================

// todo: change u, v and flag to constant memory
// todo: try local shared memory for u and v
__kernel void computeG
	(
		__global float*			u_g,			// horizontal velocity
		__global float*			v_g,			// horizontal velocity
		__global unsigned char*	flag_g,			// array with fluid/boundary cell flags
		__global float*			g_g,			// storage array for G
		float					gy,				// body force in x direction (gravity)
		float					dt,				// time step size
		float					re,				// Reynolds number
		float					alpha,
		float					dx,				// length delta x of on cell in x-direction
		float					dy,				// length delta y of on cell in y-direction
		int						nx,				// dimension in x direction (including boundaries)
		int						ny				// dimension in y direction (including boundaries)
	)
{
	const unsigned int x   = get_global_id( 0 );
	const unsigned int y   = get_global_id( 1 );
	const unsigned int idx = y * nx + x;

	// todo: check guards
	if( x > 0 &&
		x < nx - 1 )
	{
		if( y > 0 &&
			y < ny - 1 )
		{
			// compute G between fluid cells only
			if( flag_g[idx] == C_F && flag_g[idx + nx] == C_F ) // second cell test for not to overwrite boundary values
			{
				g_g[idx] =
					v_g[idx] + dt *
					(
						(
							d2m_dx2 ( v_g, dx, idx ) +
							d2m_dy2 ( v_g, dy, idx, nx )
						) / re
						- dv2_dy ( v_g, dx, alpha, idx, nx )
						- duv_dx ( u_g, v_g, dy, alpha, idx, nx )
						+ gy
					);
			}
			else
			{
				// according to formula 3.42
				g_g[idx]   = v_g[idx];
			}
		}
		else
		{
			g_g[idx]   = v_g[idx];
		}
	}

}

