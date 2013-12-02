#ifndef UPDATEUVKERNELTEST_H
#define UPDATEUVKERNELTEST_H

//********************************************************************
//**    includes
//********************************************************************

#include "CLTest.h"
#include <math.h>

//====================================================================
/*! \class UpdateUVKernelTest
	\brief Class for testing UV update kernel
*/
//====================================================================

class UpdateUVKernelTest : public CLTest
{
	private:

		REAL** _P_h;
		REAL** _F_h;
		REAL** _G_h;
		REAL** _U_h;
		REAL** _V_h;
		REAL** _U_buffer;
		REAL** _V_buffer;

		bool _clean;

	public:
		UpdateUVKernelTest ( std::string name ) : CLTest( name )
		{
			_clean = false;
		}

		~UpdateUVKernelTest ( )
		{
			cleanup();
		}

		void cleanup ( )
		{
			if( !_clean )
			{
				freeHostMatrix( _P_h );
				freeHostMatrix( _F_h );
				freeHostMatrix( _G_h );
				freeHostMatrix( _U_h );
				freeHostMatrix( _V_h );
				freeHostMatrix( _U_buffer );
				freeHostMatrix( _V_buffer );
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

			loadKernels( "updateUV.cl", "updateUVKernel" );

			// allocate host memory
			_P_h = allocHostMatrix( nx, ny );
			_F_h = allocHostMatrix( nx, ny );
			_G_h = allocHostMatrix( nx, ny );
			_U_h = allocHostMatrix( nx, ny );
			_V_h = allocHostMatrix( nx, ny );
			_U_buffer = allocHostMatrix( nx, ny );
			_V_buffer = allocHostMatrix( nx, ny );

			// create flag array
			unsigned char* FLAG_h[ny];
			unsigned char flag_data[size];
			FLAG_h[0] = flag_data;
			for( int i = 1; i < ny; ++i )
			{
				FLAG_h[i] = flag_data + i * nx;
			}

			// init host memory with random values between -10 and 10
			srand ( time( NULL ) );
			for( int y = 0; y < ny; ++y )
			{
				for( int x = 0; x < nx; ++x )
				{
					_P_h[y][x] = (REAL(rand()) / REAL(RAND_MAX)) * 20.0 -10.0;
					_F_h[y][x] = (REAL(rand()) / REAL(RAND_MAX)) * 20.0 -10.0;
					_G_h[y][x] = (REAL(rand()) / REAL(RAND_MAX)) * 20.0 -10.0;
					_U_h[y][x] = 99;
					_V_h[y][x] = 99;
					FLAG_h[y][x] = C_F;
					// todo: test with geometry
				}
			}

			// allocate device memory
			cl::Buffer P_g( _clContext, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_P_h );
			cl::Buffer F_g( _clContext, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_F_h );
			cl::Buffer G_g( _clContext, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_G_h );
			cl::Buffer U_g( _clContext, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_U_h );
			cl::Buffer V_g( _clContext, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_V_h );
			cl::Buffer FLAG_g( _clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_char) * size, *FLAG_h );

			// set kernel arguments
			_clKernels["updateUVKernel"].setArg( 0,  P_g );
			_clKernels["updateUVKernel"].setArg( 1,  F_g );
			_clKernels["updateUVKernel"].setArg( 2,  G_g );
			_clKernels["updateUVKernel"].setArg( 3,  FLAG_g );
			_clKernels["updateUVKernel"].setArg( 4,  U_g );
			_clKernels["updateUVKernel"].setArg( 5,  V_g );
			_clKernels["updateUVKernel"].setArg( 6,  sizeof(cl_float), &dt );
			_clKernels["updateUVKernel"].setArg( 7,  sizeof(cl_float), &dx );
			_clKernels["updateUVKernel"].setArg( 8,  sizeof(cl_float), &dy );
			_clKernels["updateUVKernel"].setArg( 9,  sizeof(int), &nx );
			_clKernels["updateUVKernel"].setArg( 10, sizeof(int), &ny );

			// call kernel
			_clQueue.enqueueNDRangeKernel (
					_clKernels["updateUVKernel"],
					cl::NullRange,			// offset
					cl::NDRange( nx, ny ),	// global,
					cl::NullRange			// local,
				);

			_clQueue.finish();

			// get results
			_clQueue.enqueueReadBuffer( U_g, CL_TRUE, 0, sizeof(cl_float) * size, *_U_buffer );
			_clQueue.enqueueReadBuffer( V_g, CL_TRUE, 0, sizeof(cl_float) * size, *_V_buffer );

			// set U and V on CPU
			updateUV( _P_h, _F_h, _G_h, FLAG_h, _U_h, _V_h, dt, dx, dy, nx, ny );

			// compare results
			bool equal = true;
			for( int y = 0; y < ny && equal; ++y )
			{
				for( int x = 0; x < nx && equal; ++x )
				{
					if(
							_U_h[y][x] != _U_buffer[y][x]
							||
							_V_h[y][x] != _V_buffer[y][x]
					   )
					{
						std::cout << " Kernel \"updateUVKernel\" suffers from different float precision" << std::endl;
						// debug output:
						//printHostMatrix( "CPU U:", _U_h, nx, ny );
						//printHostMatrix( "GPU U:", _U_buffer, nx, ny );
						//printHostMatrix( "CPU V:", _V_h, nx, ny );
						//printHostMatrix( "GPU V:", _V_buffer, nx, ny );
						//printHostMatrixDifference( "U differences:", _U_h, _U_buffer, nx, ny );
						//printHostMatrixDifference( "V differences:", _V_h, _V_buffer, nx, ny );
						//cleanup();
						//return Error;
						equal = false;
					}
				}
			}

			cleanup();

			return Success;
		}

		//============================================================================
		void updateUV
			(
				REAL** P,
				REAL** F,
				REAL** G,
				unsigned char** FLAG,
				REAL** U,
				REAL** V,
				REAL dt,
				REAL dx,
				REAL dy,
				int nx,
				int ny
			)
		{
			nx -= 2;	// for mirroring CPU implementation
			ny -= 2;

			int nx1 = nx + 1;
			int ny1 = ny + 1;

			REAL dt_dx = dt / dx;
			REAL dt_dy = dt / dy;

			// update u
			for ( int y = 1; y < ny1; ++y )
			{
				for ( int x = 1; x < nx; ++x )
				{
					if ( FLAG[y][x] == C_F && FLAG[y][x+1] == C_F )
					{
						U[y][x] = F[y][x] - dt_dx * ( P[y][x+1] - P[y][x] );
					}
				}
			}

			// update v
			for ( int y = 1; y < ny; ++y )
			{
				for ( int x = 1; x < nx1; ++x )
				{
					if ( FLAG[y][x] == C_F && FLAG[y+1][x] == C_F )	//todo evtl y+1?
					{
						V[y][x] = G[y][x] - dt_dy * ( P[y+1][x] - P[y][x] );
					}
				}
			}
		}
};

#endif // UPDATEUVKERNELTEST_H
