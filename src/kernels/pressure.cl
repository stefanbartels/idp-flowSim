// -------------------------------------------------
//	kernels for solving the pressure equation
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
// uses red/black pattern for parallelisation
// todo: constant/texture memory for FLAG and RHS
// todo: shared memory for P
__kernel void gaussSeidelRedBlackKernel
	(
		__global float*			p_g,			// pressure array
		__global unsigned char*	flag_g,			// boundary cell flags
		__global float*			rhs_g,			// storage array for righ hand side
		float					dx2,			// sqare of length delta x of on cell in x-direction
		float					dy2,			// sqare of length delta y of on cell in y-direction
		int						red,			// 1 for red, 0 for black
		float					constant_expr,	// constant expression 1.0 / ( 2.0 / dx2 + 2.0 / dy2 )
		int						nx,				// dimension in x direction (including boundaries)
		int						ny				// dimension in y direction (including boundaries)
	)
{
	const unsigned int x   = get_global_id( 0 );
	const unsigned int y   = get_global_id( 1 );
	const unsigned int idx = y * nx + x;

	if( ((x + y) & 1) == red &&			// = ( (x + y) % 2 ) == red, but faster
		x > 0 &&
		y > 0 &&
		x < nx - 1 &&
		y < ny - 1 )
	{
		// calculate pressure in fluid cells
		if( flag_g[idx] == C_F )
		{
			p_g[idx] =
				constant_expr * (
					( p_g[idx - 1] + p_g[idx + 1] ) / dx2
					+
					( p_g[idx - nx] + p_g[idx + nx] ) / dy2
					-
					rhs_g[idx]
				);
		}
		else
		{
			// set boundary pressure value for obstacle cells
			switch ( flag_g[idx] )
			{
				case B_N:
					p_g[idx] = p_g[idx + nx];
					break;
				case B_S:
					p_g[idx] = p_g[idx - nx];
					break;
				case B_W:
					p_g[idx] = p_g[idx - 1];
					break;
				case B_E:
					p_g[idx] = p_g[idx + 1];
					break;
				case B_NW:
					p_g[idx] = (p_g[idx - nx] + p_g[idx + 1]) / 2;
					break;
				case B_NE:
					p_g[idx] = (p_g[idx + nx] + p_g[idx + 1]) / 2;
					break;
				case B_SW:
					p_g[idx] = (p_g[idx - nx] + p_g[idx - 1]) / 2;
					break;
				case B_SE:
					p_g[idx] = (p_g[idx + nx] + p_g[idx - 1]) / 2;
					break;
			}
		}
	}
}


//============================================================================
// todo: use 1D kernel and proper range (terribly inefficient right now)
// todo: only neumann, implement dirichlet and periodic boundary conditions

__kernel void pressureBoundaryConditionsKernel
	(
		__global float* p_g,			// pressure array
		//int				problemId,		// id of the problem
		int				nx,				// dimension in x direction (including boundaries)
		int				ny				// dimension in y direction (including boundaries)
	)
{
	const unsigned int x   = get_global_id( 0 );
	const unsigned int y   = get_global_id( 1 );
	const unsigned int idx = y * nx + x;

	if(	x == 0 && y > 0 && y < ny-1 )
	{
		p_g[y*nx]			= p_g[1 + y*nx];
		p_g[nx-1 + y*nx]	= p_g[nx-2 + y*nx];
	}
	if(	y == 0 && x > 0 && x < nx-1 )
	{
		p_g[x]				= p_g[x + nx];
		p_g[x + (ny-1)*nx]	= p_g[x + (ny-2)*nx];
	}
}


//============================================================================
/*
 * this is an 1D kernel and is to be called with a range of the size of one workgroup
 * todo: seems to be inefficient
 * todo: constant/texture memory for P, FLAG and RHS
 * todo: determine optimal work size N: N = x^2, N<=max_work_size, SIZE<=max_work_size ? N>=SIZE
 *
 * returns the sum of all squared elements of the pressure array
 *
 * uses a two step reduction algorithm
 * see http://developer.amd.com/resources/documentation-articles/articles-whitepapers/opencl-optimization-case-study-simple-reductions/
 */

__kernel void pressureResidualReductionKernel
	(
		__global float*	p_g,			// pressure array
		__global float*	rhs_g,			// storage array for right hand side
		__global float*	result,			// result buffer for residual
		__local  float*	residual_s,		// dynamically allocated shared memory for workgroup
		float			dx2,			// sqare of length delta x of on cell in x-direction
		float			dy2,			// sqare of length delta y of on cell in y-direction
		int				nx,				// dimension in x direction (including boundaries)
		int				ny				// dimension in y direction (including boundaries)
	)
{
	const unsigned int idx_global	= get_global_id(0);
	const unsigned int idx_local	= get_local_id(0);
	const unsigned int limit 		= nx * ny;
	const unsigned int local_size 	= get_local_size(0);

	float local_sum = 0.0;

	unsigned int i = idx_global;
	float temp, temp2;
	int x, y;

	if( idx_global < limit ) // guard
	{
		// process simulation area chunkwise in parallel

		while( i < limit )
		{
			x = i % nx;
			y = i / nx;

			if( x > 0 &&
				y > 0 &&
				x < nx-1 &&
				y < ny-1 ) // guards
			{
				temp =
					  ( p_g[i +  1] - 2.0 * p_g[i] + p_g[i -  1] ) / dx2
					+ ( p_g[i + nx] - 2.0 * p_g[i] + p_g[i - nx] ) / dy2
					- rhs_g[i];

				local_sum += temp * temp;
			}

			i += local_size;
		}
	}

	// local result
	residual_s[idx_local] = local_sum;

	// all threads must reach barrier
	barrier( CLK_LOCAL_MEM_FENCE );

	// collect results hierarchically

	int offset = local_size / 2;
	while( offset > 0 )
	{
		if( idx_local < offset )
		{
			temp  = residual_s[idx_local];
			temp2 = residual_s[idx_local + offset];
			residual_s[ idx_local ] = temp + temp2;
		}

		offset = offset / 2;

		// all threads must reach barrier
		barrier( CLK_LOCAL_MEM_FENCE );
	}

	// write back results
	if( idx_local == 0 )
	{
		result[0] = residual_s[0];
	}
}
