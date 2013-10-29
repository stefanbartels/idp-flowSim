#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// set to true to use OpenCL for the simulation
#define USE_GPU true

// thread block dimensions
#define BW 16
#define BH 16

// defining data type, as double is not supported by all GPUs
#if USE_GPU
	#define REAL float
#else
	#define REAL double
#endif

/*
 * obstacle map data values
 * ----------------------------------------------------
 * | 0 | 0 | 0 | center | east | west | south | north |
 * ----------------------------------------------------
 *
 * 1 = fluid cell
 * 0 = obstacle cell
 */
#define C_F		0x10	// 000 10000
#define C_B		0x00	// 000 00000

#define B_N		0x01	// 000 00001
#define B_S		0x02	// 000 00010
#define B_W		0x04	// 000 00100
#define B_E		0x08	// 000 01000

#define B_NW	0x05	// 000 00101
#define B_NE	0x09	// 000 01001
#define B_SW	0x06	// 000 00110
#define B_SE	0x0A	// 000 01010

#endif // DEFINITIONS_H
