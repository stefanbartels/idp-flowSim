#ifndef FGKERNELSTEST_H
#define FGKERNELSTEST_H

//********************************************************************
//**    includes
//********************************************************************

#include "CLTest.h"
#include <math.h>

//====================================================================
/*! \class FGKernelsTest
	\brief Class for testing F and G computation kernels
*/
//====================================================================

class FGKernelsTest : public CLTest
{
	private:

		REAL** _U_h;
		REAL** _V_h;
		REAL** _F_h;
		REAL** _G_h;
		REAL** _F_buffer;
		REAL** _G_buffer;

		bool _clean;

	public:
		FGKernelsTest ( std::string name ) : CLTest( name )
		{
			_clean = false;
		}

		~FGKernelsTest ( )
		{
			cleanup();
		}

		void cleanup ( )
		{
			if( !_clean )
			{
				freeHostMatrix( _U_h );
				freeHostMatrix( _V_h );
				freeHostMatrix( _F_h );
				freeHostMatrix( _G_h );
				freeHostMatrix( _F_buffer );
				freeHostMatrix( _G_buffer );
				_clean = true;
			}
		}

		//============================================================================
		ErrorCode run ( )
		{

			int nx = 5;
			int ny = 5;
			int size = nx * ny;

			REAL gx = 0.0;
			REAL gy = 0.0;

			REAL dt = 0.1;
			REAL re = 1000;
			REAL alpha = 0.9;
			REAL dx = 0.126;
			REAL dy = 0.126;

			std::string kernelNames[] = { "computeF", "computeG" };
			loadKernels( "computeFG.cl", std::vector<std::string>(begin(kernelNames), end(kernelNames)) );

			// allocate host memory
			_U_h = allocHostMatrix( nx, ny );
			_V_h = allocHostMatrix( nx, ny );
			_F_h = allocHostMatrix( nx, ny );
			_G_h = allocHostMatrix( nx, ny );
			_F_buffer = allocHostMatrix( nx, ny );
			_G_buffer = allocHostMatrix( nx, ny );

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
					_U_h[y][x] = (REAL(rand()) / REAL(RAND_MAX)) * 20.0 - 10.0;
					_V_h[y][x] = (REAL(rand()) / REAL(RAND_MAX)) * 20.0 - 10.0;
					_F_h[y][x] = 99;
					_G_h[y][x] = 99;
					FLAG_h[y][x] = C_F;
					// todo: test with geometry
				}
			}

			// allocate device memory
			cl::Buffer U_g( _clContext, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_U_h );
			cl::Buffer V_g( _clContext, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_V_h );
			cl::Buffer F_g( _clContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_F_h );
			cl::Buffer G_g( _clContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_G_h );
			cl::Buffer FLAG_g( _clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *FLAG_h );


			//-----------------------
			// computeF
			//-----------------------

			// set kernel arguments
			_clKernels["computeF"].setArg( 0,  U_g );
			_clKernels["computeF"].setArg( 1,  V_g );
			_clKernels["computeF"].setArg( 2,  FLAG_g );
			_clKernels["computeF"].setArg( 3,  F_g );
			_clKernels["computeF"].setArg( 4,  sizeof(cl_float), &gx );
			_clKernels["computeF"].setArg( 5,  sizeof(cl_float), &dt );
			_clKernels["computeF"].setArg( 6,  sizeof(cl_float), &re );
			_clKernels["computeF"].setArg( 7,  sizeof(cl_float), &alpha );
			_clKernels["computeF"].setArg( 8,  sizeof(cl_float), &dx );
			_clKernels["computeF"].setArg( 9,  sizeof(cl_float), &dy );
			_clKernels["computeF"].setArg( 10, sizeof(int), &nx );
			_clKernels["computeF"].setArg( 11, sizeof(int), &ny );

			// call kernel
			_clQueue.enqueueNDRangeKernel (
					_clKernels["computeF"],
					cl::NullRange,			// offset
					cl::NDRange( nx, ny ),	// global,
					cl::NullRange			// local,
				);


			//-----------------------
			// computeG
			//-----------------------

			// set kernel arguments
			_clKernels["computeG"].setArg( 0,  U_g );
			_clKernels["computeG"].setArg( 1,  V_g );
			_clKernels["computeG"].setArg( 2,  FLAG_g );
			_clKernels["computeG"].setArg( 3,  G_g );
			_clKernels["computeG"].setArg( 4,  sizeof(cl_float), &gx );
			_clKernels["computeG"].setArg( 5,  sizeof(cl_float), &dt );
			_clKernels["computeG"].setArg( 6,  sizeof(cl_float), &re );
			_clKernels["computeG"].setArg( 7,  sizeof(cl_float), &alpha );
			_clKernels["computeG"].setArg( 8,  sizeof(cl_float), &dx );
			_clKernels["computeG"].setArg( 9,  sizeof(cl_float), &dy );
			_clKernels["computeG"].setArg( 10, sizeof(int), &nx );
			_clKernels["computeG"].setArg( 11, sizeof(int), &ny );

			// call kernel
			_clQueue.enqueueNDRangeKernel (
					_clKernels["computeG"],
					cl::NullRange,			// offset
					cl::NDRange( nx, ny ),	// global,
					cl::NullRange			// local,
				);

			_clQueue.finish();

			// get results
			_clQueue.enqueueReadBuffer( F_g, CL_TRUE, 0, sizeof(cl_float) * size, *_F_buffer );
			_clQueue.enqueueReadBuffer( G_g, CL_TRUE, 0, sizeof(cl_float) * size, *_G_buffer );

			// compute F and G on CPU
			computeFG( _U_h, _V_h, _F_h, _G_h, FLAG_h, nx, ny, gx, gy, dt, re, alpha, dx, dy );

			// compare results
			bool equal = true;
			for( int y = 0; y < ny && equal; ++y )
			{
				for( int x = 0; x < nx && equal; ++x )
				{
					if(
							_F_h[y][x] != _F_buffer[y][x]
							||
							_G_h[y][x] != _G_buffer[y][x]
					   )
					{
						std::cout << " Kernel \"computeF\" suffers from different float precision difference" << std::endl;
						// debug output:
						//printHostMatrix( "CPU F:", _F_h, nx, ny );
						//printHostMatrix( "GPU F:", _F_buffer, nx, ny );
						//printHostMatrix( "CPU G:", _G_h, nx, ny );
						//printHostMatrix( "GPU G:", _G_buffer, nx, ny );
						//printHostMatrixDifference( "F differences:", _F_h, _F_buffer, nx, ny );
						//printHostMatrixDifference( "G differences:", _G_h, _G_buffer, nx, ny );
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
		void computeFG
			(
				REAL** U,
				REAL** V,
				REAL** F,
				REAL** G,
				unsigned char** FLAG,
				int nx,
				int ny,
				REAL gx,
				REAL gy,
				REAL dt,
				REAL re,
				REAL alpha,
				REAL dx,
				REAL dy
			)
		{
			nx -= 2;	// for mirroring CPU implementation
			ny -= 2;

			int nx1 = nx + 1;
			int ny1 = ny + 1;

			for( int y = 1; y < ny1; ++y )
			{
				for( int x = 1; x < nx1; ++x )
				{
					//-----------------------
					// compute F
					//-----------------------

					// according to formula 3.36

					// compute F between fluid cells only
					if( FLAG[y][x] == C_F && FLAG[y][x+1] == C_F ) // second cell test for not to overwrite boundary values
					{
						F[y][x] =
							U[y][x] + dt *
							(
								(
									d2m_dx2 ( U, x, y, dx ) +
									d2m_dy2 ( U, x, y, dy )
								) / re
								- du2_dx ( U, x, y, alpha, dx )
								- duv_dy ( U, V, x, y, alpha, dy )
								+ gx
							);
					}
					else
					{
						// according to formula 3.42
						F[y][x] = U[y][x];
					}


					//-----------------------
					// compute G
					//-----------------------

					// according to formula 3.37

					// compute G between fluid cells only
					if( FLAG[y][x] == C_F && FLAG[y+1][x] == C_F )
					{
						G[y][x] =
							V[y][x] + dt *
							(
								(
									d2m_dx2 ( V, x, y, dx ) +
									d2m_dy2 ( V, x, y, dy )
								) / re
								- dv2_dy ( V, x, y, alpha, dy )
								- duv_dx ( U, V, x, y, alpha, dx )
								+ gy
							);
					}
					else
					{
						// according to formula 3.42
						G[y][x]   = V[y][x];
					}
				}
			}

			// setting boundary values for f according to formula 3.42
			for ( int y = 1; y < ny1; ++y )
			{
				F[y][0]   = U[y][0];
				F[y][nx1] = U[y][nx1];
			}

			// setting boundary values for g according to formula 3.42
			for ( int x = 1; x < nx1; ++x )
			{
				G[0][x]   = V[0][x];
				G[ny1][x] = V[ny1][x];
			}
		}



		// -------------------------------------------------
		//	auxiliary functions for F & G
		// -------------------------------------------------

		//============================================================================
		inline REAL d2m_dx2 ( REAL** M, int x, int y, REAL dx )
		{
			return ( M[y][x-1] - 2.0 * M[y][x] + M[y][x+1] ) / ( dx * dx );
		}

		//============================================================================
		inline REAL d2m_dy2 ( REAL** M, int x, int y, REAL dy )
		{
			return ( M[y-1][x] - 2.0 * M[y][x] + M[y+1][x] ) / ( dy * dy );
		}

		//============================================================================
		inline REAL du2_dx  ( REAL** U, int x, int y, REAL alpha, REAL dx )
		{
			return
				(
					(
						( U[y][x] + U[y][x+1] ) *
						( U[y][x] + U[y][x+1] )
						-
						( U[y][x-1] + U[y][x] ) *
						( U[y][x-1] + U[y][x] )
					)
					+
					alpha *
					(
						fabs( U[y][x] + U[y][x+1] ) *
						   ( U[y][x] - U[y][x+1] )
						-
						fabs( U[y][x-1] + U[y][x] ) *
						   ( U[y][x-1] - U[y][x] )
					)
				) / ( 4.0 * dx);
		}

		//============================================================================
		inline REAL dv2_dy  ( REAL** V, int x, int y, REAL alpha, REAL dy )
		{
			return
				(
					(
						( V[y][x] + V[y+1][x] ) *
						( V[y][x] + V[y+1][x] )
						-
						( V[y-1][x] + V[y][x] ) *
						( V[y-1][x] + V[y][x] )
					)
					+
					alpha *
					(
						fabs( V[y][x] + V[y+1][x] ) *
						   ( V[y][x] - V[y+1][x] )
						-
						fabs( V[y-1][x] + V[y][x] ) *
						   ( V[y-1][x] - V[y][x] )
					)
				) / ( 4.0 * dy );
		}

		//============================================================================
		inline REAL duv_dx  ( REAL** U, REAL** V, int x, int y, REAL alpha, REAL dx )
		{
			return
				(
					(
						( U[y][x] + U[y+1][x] ) *
						( V[y][x] + V[y][x+1] )
						-
						( U[y][x-1] + U[y+1][x-1] ) *
						( V[y][x-1] + V[y][x] )
					)
					+
					alpha *
					(
							fabs( U[y][x] + U[y+1][x] ) *
							   ( V[y][x] - V[y][x+1] )
							-
							fabs( U[y][x-1] + U[y+1][x-1] ) *
							   ( V[y][x-1] - V[y][x] )
					)
				) / ( 4.0 * dx );
		}

		//============================================================================
		inline REAL duv_dy  ( REAL** U, REAL** V, int x, int y, REAL alpha, REAL dy )
		{
			return
				(
					(
						( V[y][x] + V[y][x+1] ) *
						( U[y][x] + U[y+1][x] )
						-
						( V[y-1][x] + V[y-1][x+1] ) *
						( U[y-1][x] + U[y][x] )
					)
					+
					alpha *
					(
						fabs( V[y][x] + V[y][x+1] ) *
						   ( U[y][x] - U[y+1][x] )
						-
						fabs( V[y-1][x] + V[y-1][x+1] ) *
						   ( U[y-1][x] - U[y][x] )
					)
				) / ( 4.0 * dy );
		}

};

#endif // FGKERNELSTEST_H
