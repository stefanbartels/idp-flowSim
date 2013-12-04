// -------------------------------------------------
//	auxiliary kernels
// -------------------------------------------------

//============================================================================
__kernel void setKernel
	(
		__global float*	field_g,
		float			value,
		int				nx,
		int				ny,
		int				pitch
	)
{
	const unsigned int x   = get_global_id( 0 );
	const unsigned int y   = get_global_id( 1 );
	const unsigned int idx = y * pitch + x;

	if( x < nx && y < ny )
	{
		field_g[ idx ] = value;
	}
}


//============================================================================
__kernel void setBoundaryAndInteriorKernel
	(
		__global float*	field_g,
		float			boundaryValue,
		float			interiorValue,
		int				nx,
		int				ny,
		int				pitch
	)
{
	const unsigned int x   = get_global_id( 0 );
	const unsigned int y   = get_global_id( 1 );
	const unsigned int idx = y * pitch + x;

	if(	x > 0 &&
		y > 0 &&
		x < (nx - 1) &&
		y < (ny - 1) )
	{
		// set interior values
		field_g[ idx ] = interiorValue;
	}
	else if( x < nx && y < ny )
	{
		// set boundary values
		field_g[ idx ] = boundaryValue;
	}
}
