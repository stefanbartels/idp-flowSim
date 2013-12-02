#ifndef BOUNDARYKERNELSTEST_H
#define BOUNDARYKERNELSTEST_H

//********************************************************************
//**    includes
//********************************************************************

#include "CLTest.h"
#include <math.h>

//====================================================================
/*! \class BoundaryKernelsTest
	\brief Class for testing boundary condition kernels
*/
//====================================================================

class BoundaryKernelsTest : public CLTest
{
	private:

		REAL** _U_h;
		REAL** _V_h;
		REAL** _U_buffer;
		REAL** _V_buffer;

		bool _clean;

	public:
		BoundaryKernelsTest ( std::string name ) : CLTest( name )
		{
			_clean = false;
		}

		~BoundaryKernelsTest ( )
		{
			cleanup();
		}

		void cleanup ( )
		{
			if( !_clean )
			{
				freeHostMatrix( _U_h );
				freeHostMatrix( _V_h );
				freeHostMatrix( _U_buffer );
				freeHostMatrix( _V_buffer );
				_clean = true;
			}
		}


		ErrorCode run ( )
		{
			int nx = 5;
			int ny = 5;
			int size = nx * ny;

			std::string kernelNames[] = {
				"setBoundaryConditionsKernel",
				"setArbitraryBoundaryConditionsKernel",
				// Problem specific boundary conditions
				"setMovingLidBoundaryConditionsKernel",
				"setLeftInflowBoundaryConditionsKernel"
			};
			loadKernels( "boundaryConditions.cl", std::vector<std::string>(begin(kernelNames), end(kernelNames)) );

			// allocate host memory for velocity
			_U_h = allocHostMatrix( nx, ny );
			_V_h = allocHostMatrix( nx, ny );

			_U_buffer = allocHostMatrix( nx, ny );
			_V_buffer = allocHostMatrix( nx, ny );

			// init host memory with random values between -10 and 10
			srand ( time( NULL ) );
			for( int y = 0; y < ny; ++y )
			{
				for( int x = 0; x < nx; ++x )
				{
					_U_h[y][x] = (REAL(rand()) / REAL(RAND_MAX)) * 20.0 -10.0;
					_V_h[y][x] = (REAL(rand()) / REAL(RAND_MAX)) * 20.0 -10.0;
					_U_buffer[y][x] = 0.0;
					_V_buffer[y][x] = 0.0;
				}
			}

			// allocate device memory
			cl::Buffer U_g( _clContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_U_h );
			cl::Buffer V_g( _clContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_V_h );


			//-----------------------
			// setBoundaryConditionsKernel
			//-----------------------

			int	boundaryConditionNorth = NO_SLIP;
			int	boundaryConditionEast  = NO_SLIP;
			int	boundaryConditionSouth = NO_SLIP;
			int	boundaryConditionWest  = NO_SLIP;

			// set kernel arguments
			_clKernels["setBoundaryConditionsKernel"].setArg( 0, U_g );
			_clKernels["setBoundaryConditionsKernel"].setArg( 1, V_g );
			_clKernels["setBoundaryConditionsKernel"].setArg( 2, sizeof(int), &boundaryConditionNorth );
			_clKernels["setBoundaryConditionsKernel"].setArg( 3, sizeof(int), &boundaryConditionEast );
			_clKernels["setBoundaryConditionsKernel"].setArg( 4, sizeof(int), &boundaryConditionSouth );
			_clKernels["setBoundaryConditionsKernel"].setArg( 5, sizeof(int), &boundaryConditionWest );
			_clKernels["setBoundaryConditionsKernel"].setArg( 6, sizeof(int), &nx );
			_clKernels["setBoundaryConditionsKernel"].setArg( 7, sizeof(int), &ny );

			// call kernel
			_clQueue.enqueueNDRangeKernel (
					_clKernels["setBoundaryConditionsKernel"],
					cl::NullRange,			// offset
					cl::NDRange( nx, ny ),	// global,
					cl::NullRange			// local,
				);

			_clQueue.finish();

			// get result
			_clQueue.enqueueReadBuffer( U_g, CL_TRUE, 0, sizeof(cl_float) * size, *_U_buffer );
			_clQueue.enqueueReadBuffer( V_g, CL_TRUE, 0, sizeof(cl_float) * size, *_V_buffer );

			// set boundaries on CPU
			setBoundaryConditionsHost( _U_h, _V_h, boundaryConditionNorth, boundaryConditionEast, boundaryConditionSouth, boundaryConditionWest, nx, ny );

			// compare results
			for( int y = 0; y < ny; ++y )
			{
				for( int x = 0; x < nx; ++x )
				{
					if(
							_U_h[y][x] != _U_buffer[y][x]
							||
							_V_h[y][x] != _V_buffer[y][x]
						)
					{
						std::cout << " Kernel \"setBoundaryConditionsKernel\"" << std::endl;
						// debug output:
						//printHostMatrix( "CPU U:", _U_h, nx, ny );
						//printHostMatrix( "GPU U:", _U_buffer, nx, ny );
						//printHostMatrix( "CPU V:", _V_h, nx, ny );
						//printHostMatrix( "GPU V:", _V_buffer, nx, ny );
						//printHostMatrixDifference( "U differences:", _U_h, _U_buffer, nx, ny );
						//printHostMatrixDifference( "V differences:", _V_h, _V_buffer, nx, ny );
						cleanup();
						return Error;
					}
				}
			}


			cleanup();

			return Success;
		}



		// auxiliary functions
		void setBoundaryConditionsHost
			(
				REAL** U,
				REAL** V,
				int wN,
				int wE,
				int wS,
				int wW,
				int nx,
				int ny
			)
		{
			nx -= 2;	// for mirroring CPU implementation
			ny -= 2;

			int nx1 = nx + 1;
			int ny1 = ny + 1;

			//-----------------------
			// southern boundary
			//-----------------------

			switch( wS )
			{
				case NO_SLIP:
					for( int x = 1; x < nx1; ++x )
					{
						U[0][x] = -U[1][x];
						V[0][x] = 0.0;
					}
					break;
				case FREE_SLIP:
					for( int x = 1; x < nx1; ++x )
					{
						U[0][x] = U[1][x];
						V[0][x] = 0.0;
					}
					break;
				case OUTFLOW:
					for( int x = 1; x < nx1; ++x )
					{
						U[0][x] = U[1][x];
						V[0][x] = V[1][x];
					}
					break;
			}


			//-----------------------
			// northern boundary
			//-----------------------

			switch( wN )
			{
				case NO_SLIP:
					for( int x = 1; x < nx1; ++x )
					{
						U[ny1][x] = -U[ny][x];
						V[ny][x] = 0.0;
					}
					break;
				case FREE_SLIP:
					for( int x = 1; x < nx1; ++x )
					{
						U[ny1][x] = U[ny][x];
						V[ny][x] = 0.0;
					}
					break;
				case OUTFLOW:
					for( int x = 1; x < nx1; ++x )
					{
						U[ny1][x] = U[ny][x];
						V[ny][x] = V[ny-1][x];
					}
					break;
			}


			//-----------------------
			// western boundary
			//-----------------------

			switch( wW )
			{
				case NO_SLIP:
					for( int y = 1; y < ny1; ++y )
					{
						U[y][0] = 0.0;
						V[y][0] = -V[y][1];
					}
					break;
				case FREE_SLIP:
					for( int y = 1; y < ny1; ++y )
					{
						U[y][0] = 0.0;
						V[y][0] = V[y][1];
					}
					break;
				case OUTFLOW:
					for( int y = 1; y < ny1; ++y )
					{
						U[y][0] = U[y][1];
						V[y][0] = V[y][1];
					}
					break;
			}


			//-----------------------
			// eastern boundary
			//-----------------------

			switch( wE )
			{
				case NO_SLIP:
					for( int y = 1; y < ny1; ++y )
					{
						U[y][nx] = 0.0;
						V[y][nx1] = -V[y][nx];
					}
					break;
				case FREE_SLIP:
					for( int y = 1; y < ny1; ++y )
					{
						U[y][nx] = 0.0;
						V[y][nx1] = V[y][nx];
					}
					break;
				case OUTFLOW:
					for( int y = 1; y < ny1; ++y )
					{
						U[y][nx] = U[y][nx-1];
						V[y][nx1] = V[y][nx];
					}
					break;
			}
		}
};

#endif // BOUNDARYKERNELSTEST_H
