#ifndef AUXILIARYKERNELSTEST_H
#define AUXILIARYKERNELSTEST_H

//********************************************************************
//**    includes
//********************************************************************

#include "CLTest.h"

//====================================================================
/*! \class AuxiliaryKernelsTest
	\brief Class for testing auxiliary kernels
*/
//====================================================================

class AuxiliaryKernelsTest : public CLTest
{
	private:

		REAL** _M_h;
		REAL** _buffer;

		bool _clean;

	public:
		AuxiliaryKernelsTest ( std::string name ) : CLTest( name )
		{
			_clean = false;
		}

		~AuxiliaryKernelsTest ( )
		{
			if( !_clean )
			{
				freeHostMatrix( _M_h );
				freeHostMatrix( _buffer );
				_clean = true;
			}
		}

		ErrorCode run ( )
		{
			int nx = 64;
			int ny = 64;
			int size = nx * ny;

			std::string kernelNames[] = { "setKernel", "setBoundaryAndInteriorKernel" };
			loadKernels( "auxiliary.cl", std::vector<std::string>(begin(kernelNames), end(kernelNames)) );


			// allocate host memory
			_M_h = allocHostMatrix( nx, ny );

			// allocate device memory
			cl::Buffer M_g( _clContext, CL_MEM_READ_WRITE, sizeof(REAL) * size );

			// allocate buffer
			_buffer = allocHostMatrix( nx, ny );



			//-----------------------
			// setKernel
			//-----------------------

			REAL value = 11.0;

			// set kernel arguments
			_clKernels["setKernel"].setArg( 0, M_g );
			_clKernels["setKernel"].setArg( 1, sizeof(REAL), &value );
			_clKernels["setKernel"].setArg( 2, sizeof(int),  &nx );
			_clKernels["setKernel"].setArg( 3, sizeof(int),  &ny );
			_clKernels["setKernel"].setArg( 4, sizeof(int),  &nx );

			// call kernel
			_clQueue.enqueueNDRangeKernel (
					_clKernels["setKernel"],
					cl::NullRange,			// offset
					cl::NDRange( nx, ny ),	// global,
					cl::NullRange			// local,
				);

			_clQueue.finish();

			// get result
			_clQueue.enqueueReadBuffer( M_g, CL_TRUE, 0, sizeof(REAL) * size, *_buffer );

			// check result
			for( int i = 0; i < size; ++i )
			{
				if( _buffer[0][i] != value )
				{
					std::cout << " Kernel \"setKernel\"";
					return Error;
				}
			}



			//-----------------------
			// setBoundaryAndInteriorKernel
			//-----------------------

			REAL boundaryValue = 88.0;

			// set kernel arguments
			_clKernels["setBoundaryAndInteriorKernel"].setArg( 0, M_g );
			_clKernels["setBoundaryAndInteriorKernel"].setArg( 1, sizeof(REAL), &boundaryValue );
			_clKernels["setBoundaryAndInteriorKernel"].setArg( 2, sizeof(REAL), &value );
			_clKernels["setBoundaryAndInteriorKernel"].setArg( 3, sizeof(int),  &nx );
			_clKernels["setBoundaryAndInteriorKernel"].setArg( 4, sizeof(int),  &ny );
			_clKernels["setBoundaryAndInteriorKernel"].setArg( 5, sizeof(int),  &nx );

			// call kernel
			_clQueue.enqueueNDRangeKernel (
					_clKernels["setBoundaryAndInteriorKernel"],
					cl::NullRange,			// offset
					cl::NDRange( nx, ny ),	// global,
					cl::NullRange			// local,
				);

			_clQueue.finish();

			// get result
			_clQueue.enqueueReadBuffer( M_g, CL_TRUE, 0, sizeof(REAL) * size, *_buffer );

			setHostMatrix ( _M_h, 0, nx-1, 0, ny-1, boundaryValue );
			setHostMatrix ( _M_h, 1, nx-2, 1, ny-2, value );

			// check result
			for( int y = 0; y < ny; ++y )
			{
				for( int x = 0; x < nx; ++x )
				{
					if( _buffer[y][x] != _M_h[y][x] )
					{
						std::cout << " Kernel \"setBoundaryAndInteriorKernel\"";
						return Error;
					}
				}
			}


			freeHostMatrix( _M_h );
			freeHostMatrix( _buffer );
			_clean = true;

			return Success;
		}
};

#endif // AUXILIARYKERNELSTEST_H
