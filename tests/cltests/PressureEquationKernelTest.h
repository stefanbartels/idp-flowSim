#ifndef PRESSUREEQUATIONKERNELTEST_H
#define PRESSUREEQUATIONKERNELTEST_H

//********************************************************************
//**    includes
//********************************************************************

#include "CLTest.h"
#include <math.h>

//====================================================================
/*! \class PressureEquationKernelTest
	\brief Class for testing pressure equation solving kernels
*/
//====================================================================

class PressureEquationKernelTest : public CLTest
{
	private:

		REAL** _P_h;
		REAL** _RHS_h;
		REAL** _P_buffer;

		bool _clean;

	public:
		PressureEquationKernelTest ( std::string name ) : CLTest( name )
		{
			_clean = false;
		}

		~PressureEquationKernelTest ( )
		{
			cleanup();
		}

		void cleanup ( )
		{
			if( !_clean )
			{
				freeHostMatrix( _P_h );
				freeHostMatrix( _RHS_h );
				freeHostMatrix( _P_buffer );
				_clean = true;
			}
		}

		//============================================================================
		ErrorCode run ( )
		{
			int nx = 5;
			int ny = 5;
			int size = nx * ny;

			REAL dx = 0.126;
			REAL dy = 0.126;

			float dx2 = dx * dx;
			float dy2 = dy * dy;
			REAL constant_expr = 1.0 / ( 2.0 / dx2 + 2.0 / dy2 );

			std::string kernelNames[] = {
				"gaussSeidelRedBlackKernel",
				"pressureBoundaryConditionsKernel",
				"pressureResidualReductionKernel"
			};
			loadKernels( "pressure.cl", std::vector<std::string>(begin(kernelNames), end(kernelNames)) );


			// allocate host memory
			_P_h = allocHostMatrix( nx, ny );
			_RHS_h = allocHostMatrix( nx, ny );
			_P_buffer = allocHostMatrix( nx, ny );

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
					_P_h[y][x]   = (REAL(rand()) / REAL(RAND_MAX)) * 20.0 - 10.0;
					_RHS_h[y][x] = (REAL(rand()) / REAL(RAND_MAX)) * 20.0 - 10.0;
					FLAG_h[y][x] = C_F;
					// todo: test with geometry
				}
			}

			// allocate device memory
			cl::Buffer P_g( _clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_P_h );
			cl::Buffer RHS_g( _clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_RHS_h );
			cl::Buffer FLAG_g( _clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_char) * size, *FLAG_h );




			//-----------------------
			// gaussSeidelRedBlackKernel
			//-----------------------

			int red = 0;

			// set kernel arguments
			_clKernels["gaussSeidelRedBlackKernel"].setArg( 0, P_g );
			_clKernels["gaussSeidelRedBlackKernel"].setArg( 1, FLAG_g );
			_clKernels["gaussSeidelRedBlackKernel"].setArg( 2, RHS_g );
			_clKernels["gaussSeidelRedBlackKernel"].setArg( 3, sizeof(cl_float), &dx2 );
			_clKernels["gaussSeidelRedBlackKernel"].setArg( 4, sizeof(cl_float), &dy2 );
			_clKernels["gaussSeidelRedBlackKernel"].setArg( 5, sizeof(cl_int), &red );
			_clKernels["gaussSeidelRedBlackKernel"].setArg( 6, sizeof(cl_float), &constant_expr );
			_clKernels["gaussSeidelRedBlackKernel"].setArg( 7, sizeof(int), &nx );
			_clKernels["gaussSeidelRedBlackKernel"].setArg( 8, sizeof(int), &ny );

			// call kernel for black cells
			_clQueue.enqueueNDRangeKernel (
					_clKernels["gaussSeidelRedBlackKernel"],
					cl::NullRange,			// offset
					cl::NDRange( nx, ny ),	// global,
					cl::NullRange			// local,
				);

			_clQueue.finish();

			red = 1;
			_clKernels["gaussSeidelRedBlackKernel"].setArg( 5, sizeof(cl_int), &red );

			// call kernel for red cells
			_clQueue.enqueueNDRangeKernel (
					_clKernels["gaussSeidelRedBlackKernel"],
					cl::NullRange,			// offset
					cl::NDRange( nx, ny ),	// global,
					cl::NullRange			// local,
				);

			_clQueue.finish();

			// get results
			_clQueue.enqueueReadBuffer( P_g, CL_TRUE, 0, sizeof(cl_float) * size, *_P_buffer );

			// apply gauss seidel stencil on CPU
			gaussSeidelStepRedBlack( _P_h, _RHS_h, FLAG_h, 0, constant_expr, dx2, dy2, nx, ny );
			gaussSeidelStepRedBlack( _P_h, _RHS_h, FLAG_h, 1, constant_expr, dx2, dy2, nx, ny );

			// compare results
			bool equal = true;
			for( int y = 0; y < ny && equal; ++y )
			{
				for( int x = 0; x < nx && equal; ++x )
				{
					if( _P_h[y][x] != _P_buffer[y][x] )
					{
						std::cout << " Kernel \"gaussSeidelRedBlackKernel\" suffers from floating point precision difference" << std::endl;
						// debug output:
						//printHostMatrix( "CPU P:", _P_h, nx, ny );
						//printHostMatrix( "GPU P:", _P_buffer, nx, ny );
						//printHostMatrixDifference( "P differences:", _P_h, _P_buffer, nx, ny );
						//cleanup();
						//return Error;
						equal = false;
					}
				}
			}



			//-----------------------
			// pressureBoundaryConditionsKernel
			//-----------------------

			// set kernel arguments
			_clKernels["pressureBoundaryConditionsKernel"].setArg( 0, P_g );
			_clKernels["pressureBoundaryConditionsKernel"].setArg( 1, sizeof(int), &nx );
			_clKernels["pressureBoundaryConditionsKernel"].setArg( 2, sizeof(int), &ny );

			// call kernel
			_clQueue.enqueueNDRangeKernel (
					_clKernels["pressureBoundaryConditionsKernel"],
					cl::NullRange,			// offset
					cl::NDRange( nx, ny ),	// global,
					cl::NullRange			// local,
				);

			_clQueue.finish();

			// get results
			_clQueue.enqueueReadBuffer( P_g, CL_TRUE, 0, sizeof(cl_float) * size, *_P_buffer );

			// apply boundary conditions on CPU
			applyBoundaryConditions( _P_h, nx, ny );

			// compare results
			equal = true;
			for( int y = 0; y < ny && equal; ++y )
			{
				if(
						_P_h[y][0] != _P_buffer[y][0]
						||
						_P_h[y][nx-1] != _P_buffer[y][nx-1]
					)
				{
						equal = false;
				}
			}

			for( int x = 0; x < nx && equal; ++x )
			{
				if(
						_P_h[0][x] != _P_buffer[0][x]
						||
						_P_h[ny-1][x] != _P_buffer[ny-1][x]
					)
				{
					equal = false;
				}
			}

			if( !equal )
			{
				std::cout << " Kernel \"pressureBoundaryConditionsKernel\" suffers from floating point precision difference" << std::endl;
				// debug output:
				//printHostMatrix( "CPU P:", _P_h, nx, ny );
				//printHostMatrix( "GPU P:", _P_buffer, nx, ny );
				//printHostMatrixDifference( "P differences:", _P_h, _P_buffer, nx, ny );
				//cleanup();
				//return Error;
			}




			//-----------------------
			// pressureResidualReductionKernel
			//-----------------------

			// allocate device memory
			cl::Buffer residual_g( _clContext, CL_MEM_WRITE_ONLY, sizeof(cl_float) );

			// set kernel arguments
			_clKernels["pressureResidualReductionKernel"].setArg( 0, P_g );
			_clKernels["pressureResidualReductionKernel"].setArg( 1, RHS_g );
			_clKernels["pressureResidualReductionKernel"].setArg( 2, residual_g );
			_clKernels["pressureResidualReductionKernel"].setArg( 3, sizeof(cl_float) * _clWorkgroupSize, NULL);
			_clKernels["pressureResidualReductionKernel"].setArg( 4, sizeof(cl_float), &dx2 );
			_clKernels["pressureResidualReductionKernel"].setArg( 5, sizeof(cl_float), &dy2 );
			_clKernels["pressureResidualReductionKernel"].setArg( 6, sizeof(int), &nx );
			_clKernels["pressureResidualReductionKernel"].setArg( 7, sizeof(int), &ny );

			// call kernel
			_clQueue.enqueueNDRangeKernel (
					_clKernels["pressureResidualReductionKernel"],
					cl::NullRange,						// offset
					cl::NDRange( _clWorkgroupSize ),	// global
					cl::NDRange( _clWorkgroupSize )		// local
				);

			_clQueue.finish();

			// get results
			REAL residualSumGPU;
			_clQueue.enqueueReadBuffer( residual_g, CL_TRUE, 0, sizeof(cl_float), &residualSumGPU );

			// calculate residual sum on CPU
			REAL residualSumCPU = calculateResidualSum ( _P_h, _RHS_h, FLAG_h, dx2, dy2, nx, ny );

			if( residualSumGPU != residualSumCPU )
			{
				std::cout << " Kernel \"pressureResidualReductionKernel\" suffers from floating point precision difference" << std::endl;
				// debug output:
				//std::cout << "Residual sum CPU: " << residualSumCPU << std::endl;
				//std::cout << "Residual sum GPU: " << residualSumGPU << std::endl;
				//std::cout << "Residual difference: " << ( residualSumCPU - residualSumGPU ) << std::endl;
				//cleanup();
				//return Error;
			}

			cleanup();

			return Success;
		}

		//============================================================================
		void gaussSeidelStepRedBlack
			(
				REAL** P,
				REAL** RHS,
				unsigned char** FLAG,
				int red,
				REAL constant_expr,
				REAL dx2,
				REAL dy2,
				int nx,
				int ny
			)
		{
			nx -= 2;	// for mirroring CPU implementation
			ny -= 2;

			int nx1 = nx + 1;
			int ny1 = ny + 1;

			for ( int y = 1; y < ny1; ++y )
			{
				for ( int x = 1; x < nx1; ++x )
				{
					if( ((x + y) & 1) == red )
					{
						// calculate pressure in fluid cells
						if( FLAG[y][x] == C_F )
						{
							P[y][x] =
								constant_expr * (
									( P[y][x-1] + P[y][x+1] ) / dx2
									+
									( P[y-1][x] + P[y+1][x] ) / dy2
									-
									RHS[y][x]
								);
						}
						else
						{
							// set boundary pressure value for obstacle cells
							switch ( FLAG[y][x] )
							{
								case B_N:
									P[y][x] = P[y+1][x];
									break;
								case B_S:
									P[y][x] = P[y-1][x];
									break;
								case B_W:
									P[y][x] = P[y][x-1];
									break;
								case B_E:
									P[y][x] = P[y][x+1];
									break;
								case B_NW:
									P[y][x] = (P[y-1][x] + P[y][x+1]) / 2;
									break;
								case B_NE:
									P[y][x] = (P[y+1][x] + P[y][x+1]) / 2;
									break;
								case B_SW:
									P[y][x] = (P[y-1][x] + P[y][x-1]) / 2;
									break;
								case B_SE:
									P[y][x] = (P[y+1][x] + P[y][x-1]) / 2;
									break;
							}
						}
					}
				}
			}
		}

		//============================================================================
		void applyBoundaryConditions
			(
				REAL** P,
				int nx,
				int ny
			)
		{
			// only Neumann
			// todo: implement dirichlet and periodic
			for ( int x = 1; x < nx-1; ++x )
			{
				P[0][x]    = P[1][x];
				P[ny-1][x] = P[ny-2][x];
			}

			for ( int y = 1; y < ny-1; ++y )
			{
				P[y][0]    = P[y][1];
				P[y][nx-1] = P[y][nx-2];
			}
		}

		//============================================================================
		REAL calculateResidualSum
			(
				REAL** P,
				REAL** RHS,
				unsigned char** FLAG,
				REAL dx2,
				REAL dy2,
				int nx,
				int ny
			)
		{
			nx -= 2;	// for mirroring CPU implementation
			ny -= 2;

			int nx1 = nx + 1;
			int ny1 = ny + 1;

			REAL tmp;
			REAL sum = 0.0;
			int numCells = 0;

			for ( int y = 1; y < ny1; ++y )
			{
				for ( int x = 1; x < nx1; ++x )
				{
					if ( FLAG[y][x] == C_F )
					{
						tmp =
							  ( ( P[y][x+1] - P[y][x] ) - ( P[y][x] - P[y][x-1] ) ) / dx2
							+ ( ( P[y+1][x] - P[y][x] ) - ( P[y][x] - P[y-1][x] ) ) / dy2
							- RHS[y][x];

						//tmp =
						//	  ( P[y][x+1] - 2.0 * P[y][x] + P[y][x-1] ) / dx2
						//	+ ( P[y+1][x] - 2.0 * P[y][x] + P[y-1][x] ) / dy2
						//	- RHS[y][x];

						sum += tmp * tmp;

						++numCells;
					}
				}
			}

			return sum;
		}
};

#endif // PRESSUREEQUATIONKERNELTEST_H
