#ifndef TIMESTEPKERNELTEST_H
#define TIMESTEPKERNELTEST_H

//********************************************************************
//**    includes
//********************************************************************

#include "CLTest.h"
#include <math.h>

//====================================================================
/*! \class TimestepKernelTest
	\brief Class for testing timestep kernels
*/
//====================================================================

class TimestepKernelTest : public CLTest
{
	private:

		REAL** _U_h;
		REAL** _V_h;

		bool _clean;

	public:
		TimestepKernelTest ( std::string name ) : CLTest( name )
		{
			_clean = false;
		}

		~TimestepKernelTest ( )
		{
			if( !_clean )
			{
				freeHostMatrix( _U_h );
				freeHostMatrix( _V_h );
				_clean = true;
			}
		}

		ErrorCode run ( )
		{
			int nx = 512;
			int ny = 512;
			int size = nx * ny;

			loadKernels( "deltaT.cl", "getUVMaximumKernel" );

			// allocate host memory
			_U_h = allocHostMatrix( nx, ny );
			_V_h = allocHostMatrix( nx, ny );

			// init host memory with random values between -10 and 10
			srand ( time( NULL ) );
			for( int y = 0; y < ny; ++y )
			{
				for( int x = 0; x < nx; ++x )
				{
					_U_h[y][x] = (REAL(rand()) / REAL(RAND_MAX)) * 20.0 -10.0;
					_V_h[y][x] = (REAL(rand()) / REAL(RAND_MAX)) * 20.0 -10.0;
				}
			}

			// allocate device memory
			cl::Buffer U_g( _clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_U_h );
			cl::Buffer V_g( _clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * size, *_V_h );
			cl::Buffer results_g( _clContext, CL_MEM_WRITE_ONLY, sizeof(REAL) * 2 );


			//-----------------------
			// getUVMaximumKernel
			//-----------------------

			// set kernel arguments
			_clKernels["getUVMaximumKernel"].setArg( 0, U_g );
			_clKernels["getUVMaximumKernel"].setArg( 1, V_g );
			_clKernels["getUVMaximumKernel"].setArg( 2, results_g );
			_clKernels["getUVMaximumKernel"].setArg( 3, sizeof(cl_float) * _clWorkgroupSize, NULL);
			_clKernels["getUVMaximumKernel"].setArg( 4, sizeof(cl_float) * _clWorkgroupSize, NULL);
			_clKernels["getUVMaximumKernel"].setArg( 5, sizeof(int), &nx );
			_clKernels["getUVMaximumKernel"].setArg( 6, sizeof(int), &ny );


			// call kernel
			_clQueue.enqueueNDRangeKernel (
					_clKernels["getUVMaximumKernel"],
					cl::NullRange,						// offset
					cl::NDRange( _clWorkgroupSize ),	// global,
					cl::NDRange( _clWorkgroupSize )		// local,
				);

			_clQueue.finish();

			// get result
			float results[2] = { 0.0, 0.0 };

			_clQueue.enqueueReadBuffer( results_g, CL_TRUE, 0, sizeof(cl_float) * 2, results );

			// find maximum values on CPU
			float max_u = 0.0;
			float max_v = 0.0;

			for( int y = 0; y < ny; ++y )
			{
				for( int x = 0; x < nx; ++x )
				{
					max_u = fabs( _U_h[y][x] ) > max_u ? fabs( _U_h[y][x] ) : max_u;
					max_v = fabs( _V_h[y][x] ) > max_v ? fabs( _V_h[y][x] ) : max_v;
				}
			}

			// check result
			if( results[0] != max_u || results[1] != max_v )
			{
				std::cout << " Kernel \"getUVMaximumKernel\"" << std::endl;
				std::cout << "CPU: " << max_u << "\t" << max_v << std::endl;
				std::cout << "CP_: " << _U_h[0][0] << "\t" << _U_h[0][1] << std::endl;
				std::cout << "GPU: " << results[0] << "\t" << results[1] << std::endl;
				return Error;
			}

			freeHostMatrix( _U_h );
			freeHostMatrix( _V_h );
			_clean = true;

			return Success;
		}
};

#endif // TIMESTEPKERNELTEST_H
