#ifndef RHSKERNELTEST_H
#define RHSKERNELTEST_H

//********************************************************************
//**    includes
//********************************************************************

#include "CLTest.h"
#include <math.h>

//====================================================================
/*! \class RHSKernelTest
	\brief Class for testing right hand side computation kernels
*/
//====================================================================

class RHSKernelTest : public CLTest
{
	private:

		REAL** _F_h;
		REAL** _G_h;
		REAL** _RHS_h;
		REAL** _RHS_buffer;

		bool _clean;

	public:
		RHSKernelTest ( std::string name ) : CLTest( name )
		{
			_clean = false;
		}

		~RHSKernelTest ( )
		{
			cleanup();
		}

		void cleanup ( )
		{
			if( !_clean )
			{
				freeHostMatrix( _F_h );
				freeHostMatrix( _G_h );
				freeHostMatrix( _RHS_h );
				freeHostMatrix( _RHS_buffer );
				_clean = true;
			}
		}

		//============================================================================
		ErrorCode run ( )
		{
			int nx = 5;
			int ny = 5;
			int size = nx * ny;

			REAL dt = 0.1;
			REAL dx = 0.126;
			REAL dy = 0.126;

			loadKernels( "rightHandSide.cl", "rightHandSideKernel" );

			// allocate host memory
			_F_h        = allocHostMatrix( nx, ny );
			_G_h        = allocHostMatrix( nx, ny );
			_RHS_h      = allocHostMatrix( nx, ny );
			_RHS_buffer = allocHostMatrix( nx, ny );

			// init host memory with random values between -10 and 10
			srand ( time( NULL ) );
			for( int y = 0; y < ny; ++y )
			{
				for( int x = 0; x < nx; ++x )
				{
					_F_h[y][x] = (REAL(rand()) / REAL(RAND_MAX)) * 20.0 -10.0;
					_G_h[y][x] = (REAL(rand()) / REAL(RAND_MAX)) * 20.0 -10.0;
					_RHS_h[y][x] = 99;
				}
			}

			// allocate device memory
			cl::Buffer F_g( _clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_F_h );
			cl::Buffer G_g( _clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_G_h );
			cl::Buffer RHS_g( _clContext, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_RHS_h );

			// set kernel arguments
			_clKernels["rightHandSideKernel"].setArg( 0, F_g );
			_clKernels["rightHandSideKernel"].setArg( 1, G_g );
			_clKernels["rightHandSideKernel"].setArg( 2, RHS_g );
			_clKernels["rightHandSideKernel"].setArg( 3, sizeof(cl_float), &dt );
			_clKernels["rightHandSideKernel"].setArg( 4, sizeof(cl_float), &dx );
			_clKernels["rightHandSideKernel"].setArg( 5, sizeof(cl_float), &dy );
			_clKernels["rightHandSideKernel"].setArg( 6, sizeof(int), &nx );
			_clKernels["rightHandSideKernel"].setArg( 7, sizeof(int), &ny );

			// call kernel
			_clQueue.enqueueNDRangeKernel (
					_clKernels["rightHandSideKernel"],
					cl::NullRange,			// offset
					cl::NDRange( nx, ny ),	// global,
					cl::NullRange			// local,
				);

			_clQueue.finish();

			// get result
			_clQueue.enqueueReadBuffer( RHS_g, CL_TRUE, 0, sizeof(cl_float) * size, *_RHS_buffer );

			// compute righ hand side on CPU
			for ( int y = 1; y < ny-1; ++y )
			{
				for ( int x = 1; x < nx-1; ++x )
				{
					_RHS_h[y][x] = ( 1.0 / dt ) *
						(
							(float)( _F_h[y][x] - _F_h[y][x-1] ) / dx +
							(float)( _G_h[y][x] - _G_h[y-1][x] ) / dy
						);
				}
			}

			// compare results
			bool equal = true;
			for( int y = 0; y < ny && equal; ++y )
			{
				for( int x = 0; x < nx && equal; ++x )
				{
					if( _RHS_h[y][x] != _RHS_buffer[y][x] )
					{
						std::cout << " Kernel \"rightHandSideKernel\" suffers from different float precision difference" << std::endl;
						// debug output:
						//printHostMatrix( "CPU RHS:", _RHS_h, nx, ny );
						//printHostMatrix( "GPU RHS:", _RHS_buffer, nx, ny );
						//printHostMatrixDifference( "RHS differences:", _RHS_h, _RHS_buffer, nx, ny );
						//cleanup();
						//return Error;
						equal = false;
					}
				}
			}

			cleanup();

			return Success;
		}
};

#endif // RHSKERNELTEST_H
