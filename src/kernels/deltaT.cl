// -------------------------------------------------
//	kernel for timestap size calculation
// -------------------------------------------------

//============================================================================

/*
 * this is not a ranged kernel and is to be called for just one workgroup
 * todo: seems to be very inefficient
 *
 * only for arrays without pitch
 * uses a two step reduction algorithm
 * see http://developer.amd.com/resources/documentation-articles/articles-whitepapers/opencl-optimization-case-study-simple-reductions/
 */

__kernel void getUVMaximumKernel
	(
		__global float*	u_g,
		__global float*	v_g,
		__global float* results,		// result of max (Array with length 2: [u_max, v_max])
		__local  float* u_s,			// dynamically allocated shared memory for workgroup
		__local  float* v_s,			// dynamically allocated shared memory for workgroup
		int				nx,				// dimension in x direction (including boundaries)
		int				ny				// dimension in y direction (including boundaries)
	)
{
	const unsigned int idx_global	= get_global_id(0);
	const unsigned int idx_local	= get_local_id(0);
	const unsigned int limit 		= nx * ny;
	const unsigned int local_size 	= get_local_size(0);

	float local_max_u = -INFINITY;
	float local_max_v = -INFINITY;

	if( idx_global < limit ) // guard
	{
		// process simulation area chunkwise in parallel

		unsigned int i = idx_global;
		float temp, temp2;

		while( i < limit )
		{
			temp = u_g[i];
			local_max_u = ( temp > local_max_u ) ? temp : local_max_u;

			temp = v_g[i];
			local_max_v = ( temp > local_max_v ) ? temp : local_max_v;

			i += local_size;
		}

		// local result
		u_s[idx_local] = local_max_u;
		v_s[idx_local] = local_max_v;


		// collect results hierarchically
		barrier(CLK_LOCAL_MEM_FENCE);

		int offset = local_size / 2;
		while( offset > 0 )
		{
			if( idx_local < offset )
			{
      			temp  = u_s[idx_local];
				temp2 = u_s[idx_local + offset];
				u_s[ idx_local ] = (temp > temp2) ? temp : temp2;

      			temp  = v_s[idx_local];
				temp2 = v_s[idx_local + offset];
				v_s[ idx_local ] = (temp > temp2) ? temp : temp2;
			}

			offset = offset / 2;

			barrier(CLK_LOCAL_MEM_FENCE);
		}

		// write back results
		if( idx_local == 0 )
		{
			results[0] = u_s[0];
			results[1] = v_s[0];
		}
	}
}

