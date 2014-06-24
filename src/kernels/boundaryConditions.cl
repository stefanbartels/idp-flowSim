// -------------------------------------------------
//	kernel for bounday conditions
// -------------------------------------------------

// boundary condition ids
#define FREE_SLIP	1
#define NO_SLIP		2
#define OUTFLOW 	3
#define PERIODIC	4

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
// todo: terrible inefficient => optimize
// todo: consider calling as 1D kernel or with range nx-2, ny-2 and offset 1,1
__kernel void setBoundaryConditionsKernel
	(
		__global float*	u_g,
		__global float*	v_g,
		int				wN,		// boundary condition for northern boundaries
		int				wE,		// boundary condition for eastern boundaries
		int				wS,		// boundary condition for southern boundaries
		int				wW,		// boundary condition for western boundaries
		int				nx,		// dimension in x direction (including boundaries)
		int				ny		// dimension in y direction (including boundaries)
//		int				pitch
	)
{
	const unsigned int x   = get_global_id( 0 );
	const unsigned int y   = get_global_id( 1 );
	const unsigned int idx = y * nx + x;

	const unsigned nx_1 = nx - 1;
	const unsigned nx_2 = nx - 2;
	const unsigned ny_1 = ny - 1;
	const unsigned ny_2 = ny - 2;

	if	(
			x > 0		&&
			x < nx-1	&&
			y > 0		&&
			y < ny-1
		)
	{
		//-----------------------
		// southern boundary
		//-----------------------

		if( y == 1 )
		{
			if( wS == NO_SLIP )
			{
				u_g[ x ] = -u_g[ nx + x ];
				v_g[ x ] = 0.0;
			}
			else if( wS == FREE_SLIP )
			{
				u_g[ x ] = u_g[ nx + x ];
				v_g[ x ] = 0.0;
			}
			else if( wS == OUTFLOW )
			{
				u_g[ x ] = u_g[ nx + x ];
				v_g[ x ] = v_g[ nx + x ];
			}
		}

		//-----------------------
		// northern boundary
		//-----------------------

		if( y == ny_2 )
		{
			if( wN == NO_SLIP )
			{
				u_g[ idx + nx  ] = -u_g[ idx ];
				v_g[ idx ]       = 0.0;
			}
			else if( wN == FREE_SLIP )
			{
				u_g[ idx + nx ] = u_g[ idx ];
				v_g[ idx ]      = 0.0;
			}
			else if( wN == OUTFLOW )
			{
				u_g[ idx + nx ] = u_g[ idx ];
				v_g[ idx ]      = v_g[ idx - nx ];
			}
		}

		//-----------------------
		// western boundary
		//-----------------------

		if( x == 1 )
		{
			if( wW == NO_SLIP )
			{
				u_g[ y*nx ] = 0.0;
				v_g[ y*nx ] = -v_g[ y*nx + 1 ];
			}
			else if( wW == FREE_SLIP )
			{
				u_g[ y*nx ] = 0.0;
				v_g[ y*nx ] = v_g[ y*nx + 1 ];
			}
			else if( wW == OUTFLOW )
			{
				u_g[ y*nx ] = u_g[ y*nx + 1 ];
				v_g[ y*nx ] = v_g[ y*nx + 1 ];
			}
		}


		//-----------------------
		// eastern boundary
		//-----------------------

		if( x == nx_2 )
		{
			if( wE == NO_SLIP )
			{
				u_g[ idx ]     = 0.0;
				v_g[ idx + 1 ] = -v_g[ idx ];
			}
			else if( wE == FREE_SLIP )
			{
				u_g[ idx ]    = 0.0;
				v_g[ idx + 1] = v_g[ idx ];
			}
			else if( wE == OUTFLOW )
			{
				u_g[ idx ]    = u_g[ idx - 1 ];
				v_g[ idx + 1] = v_g[ idx ];
			}
		}
	}
}

//============================================================================
// todo: terrible inefficient => optimize
// todo: call with range nx-2, ny-2 and offset 1,1
__kernel void setArbitraryBoundaryConditionsKernel
	(
		__global float*	u_g,
		__global float*	v_g,
		__global unsigned char* flag_g,
		int				nx,
		int				ny
//		int				pitch
	)
{
	const unsigned int x   = get_global_id( 0 );
	const unsigned int y   = get_global_id( 1 );
	const unsigned int idx = y * nx + x;

	if	(
			x > 0		&&
			x < nx-1	&&
			y > 0		&&
			y < ny-1
		)
	{
		char flag = flag_g[ idx ];

		// todo: at corners (fluid cell with walls at two adjecent walls) the velocity value
		//       inside the corner is dependent on the order the cells are processed.
		//       As no wall is allowed to be between two fluid cells, this should not matter,
		//       but leads to different values on the GPU.
		//        => check if it really doesn't matter

		if( flag == B_N ) { // northern obstacle boundary => fluid in the north
			u_g[ idx-1 ]  = -u_g[ idx+nx-1 ];
			u_g[ idx ]    = -u_g[ idx+nx ];
			v_g[ idx ]    = 0.0;
		}
		else if( flag == B_S ) { // fluid in the south
			u_g[ idx-1 ]  = -u_g[ idx-nx-1 ];
			u_g[ idx ]    = -u_g[ idx-nx ];
			v_g[ idx-nx ] = 0.0;
		}
		else if( flag == B_W ) { // fluid in the west
			u_g[ idx-1 ]  = 0.0;
			v_g[ idx-nx ] = -v_g[ idx-nx-1 ];
			v_g[ idx ]    = -v_g[ idx-1 ];
		}
		else if( flag == B_E ) { // fluid in the east
			u_g[ idx ]    = 0.0;
			v_g[ idx-nx ] = -v_g[ idx-nx+1 ];
			v_g[ idx ]    = -v_g[ idx+1 ];
		}

		else if( flag == B_NW ) { // fluid in the north and west
			u_g[ idx ]    = -u_g[ idx+nx ];
			u_g[ idx-1 ]  = 0.0;

			v_g[ idx ]    = 0.0;
			v_g[ idx-nx ] = -v_g[ idx-nx-1 ];
		}
		else if( flag == B_NE ) { // fluid in the north and east
			u_g[ idx ]    = 0.0;
			u_g[ idx-1 ]  = -u_g[ idx+nx-1 ];

			v_g[ idx ]    = 0.0;
			v_g[ idx-nx ] = -v_g[ idx-nx+1 ];
		}
		else if( flag == B_SW ) { // fluid in the south and west
			u_g[ idx ]    = -u_g[ idx-nx ];
			u_g[ idx-1 ]  = 0.0;

			v_g[ idx ]    = -v_g[ idx-1 ];
			v_g[ idx-nx ] = 0.0;
		}
		else if( flag == B_SE ) { // fluid in the south and east
			u_g[ idx ]    = 0.0;
			u_g[ idx-1 ]  = -u_g[ idx-nx-1 ];

			v_g[ idx ]    = -v_g[ idx+1 ];
			v_g[ idx-nx ] = 0.0;
		}
	}
}

// -------------------------------------------------
//	auxiliary functions
// -------------------------------------------------





// -------------------------------------------------
//	problem specific boundary conditions
// -------------------------------------------------

//============================================================================
// todo: call only for one row

__kernel void setMovingLidBoundaryConditionsKernel
	(
		__global float*	u_g,
		int				nx,
		int				ny
//		int				pitch
	)
{
	const unsigned int x   = get_global_id( 0 );
	const unsigned int y   = get_global_id( 1 );
	//const unsigned int idx = y * nx + x;

	if	(
			y == 0	&&
			x >	0	&&
			x < nx
		)
	{
		u_g[ x ] = 2.0 - u_g[ x + nx ];
	}
}

//============================================================================
// todo: call only for one column

__kernel void setChannelBoundaryConditionsKernel
	(
		__global float*	u_g,
		int				nx,
		int				ny
//		int				pitch
	)
{
	const unsigned int x   = get_global_id( 0 );
	const unsigned int y   = get_global_id( 1 );
	//const unsigned int idx = y * nx + x;

	if	(
			x == 0	&&
			y > 0	&&
			y < ny
		)
	{
		u_g[ y * nx ] = 1.0;
	}
}
