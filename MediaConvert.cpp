/*
	Copyright (C) 2015, Liang Fan
	All rights reverved.
*/

/** \file		MediaConvert.cpp
	\brief		Implements convert functions.
*/

#include "MediaConvert.h"
#include "Motion.h"

#include <string.h>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "cuda_runtime.h"
#include "device_launch_parameters.h"


#define Clip3(a, b, c) ((c < a) ? a : ((c > b) ? b : c))
#define Clip1(x)       Clip3(0, 255, x)


// Global parameters from CTestFilter.cpp
extern int frame_num;
extern int lastFrameY[99840];
extern int currentFrameY[99840];
extern int resultFrameY[99840];
//extern clock_t start, end;
extern double cpu_time_used;


int Motion_Estimation(unsigned char* in, int width, int height)
{

	// timing 2.04 fps
	//if (frame_num == 1)
	//{
	//	start = clock();
	//}
	//if (frame_num == 26)
	//{
	//	end = clock();
	//	cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
	//}


	const int im_height = height;
	const int im_width = width;
	const int size = im_height * im_width;
	const int block_size = 16;
	const int search_region_R = 16;


	// input -> currentFrameY
	for (int x = 0; x < size; x++)
	{
		currentFrameY[x] = int(in[x * 2]);
	}


	// Motion Estimation-----------------------------------------

	Motion_Estimation_cuda(lastFrameY, currentFrameY, resultFrameY, block_size, search_region_R, im_height, im_width);

	//-----------------------------------------------------------


	// currentFrameY -> lastFrameY
	for (int x = 0; x < size; x++)
	{
		lastFrameY[x] = currentFrameY[x];
	}

	if (frame_num == 1)
	{
		return 0;
	}

	//  restore the Y array in its original form
	for (int x = 0; x < size; x++)
	{
		in[x * 2] = (unsigned char)(Clip1(resultFrameY[x]));
	}

	return 0;

}



int yuv422_to_rgb24(unsigned char* out, unsigned char* in, int width, int height)
{
	int  i, j;
	int  r0, r1,
		g0, g1,
		b0, b1;
	int  y0, y1,
		u0, u1,
		v0, v1;
	unsigned char* ipy, * ipu, * ipv, * op;

	// initialize buf pointer
	// note: output RGB24 is bottom line first.
	op = out + width * (height - 1) * 3;
	ipy = in;
	ipu = in + 1;
	ipv = in + 3;

	for (j = 0; j < height; j += 1)
	{
		for (i = 0; i < width; i += 2)
		{
			// get YUV data
			y0 = (int)* ipy - 16;
			y1 = (int) * (ipy + 2) - 16;

			u0 = (int)* ipu - 128;
			u1 = (int)* ipu - 128;

			v0 = (int)* ipv - 128;
			v1 = (int)* ipv - 128;

			// convert YUV to RGB
			// ITU-R BT.601
			//     R = Y           + 1.403*V
			//     G = Y - 0.344*U - 0.714*V
			//     B = Y + 1.770*U
			// 32bit calculation:
			//     R = (65536*Y            + 91947*V + 32768) >> 16
			//     G = (65536*Y -  22544*U - 46793*V + 32768) >> 16
			//     B = (65536*Y + 115999*U           + 32768) >> 16
			r0 = (65536 * y0 + 91947 * v0 + 32768) >> 16;
			r1 = (65536 * y1 + 91947 * v1 + 32768) >> 16;

			g0 = (65536 * y0 - 22544 * u0 - 46793 * v0 + 32768) >> 16;
			g1 = (65536 * y1 - 22544 * u1 - 46793 * v1 + 32768) >> 16;

			b0 = (65536 * y0 + 115999 * u0 + 32768) >> 16;
			b1 = (65536 * y1 + 115999 * u1 + 32768) >> 16;

			// RGB24
			// unsigned char: B0, G0, R0, B1, G1, R1, ...
			// note: RGB24 is bottom line first.
			*op = (unsigned char)Clip1(b0);
			*(op + 1) = (unsigned char)Clip1(g0);
			*(op + 2) = (unsigned char)Clip1(r0);
			*(op + 3) = (unsigned char)Clip1(b1);
			*(op + 4) = (unsigned char)Clip1(g1);
			*(op + 5) = (unsigned char)Clip1(r1);

			//
			op += 6;
			ipy += 4;
			ipu += 4;
			ipv += 4;
		}
		op -= width * 3 * 2;
	}

	return 0;
}



int yuv422_to_rgb24_grey(unsigned char* out, unsigned char* in, int width, int height)
{
	int  i, j;
	int  r0, r1,
		g0, g1,
		b0, b1;
	int  y0, y1,
		u0, u1,
		v0, v1;
	unsigned char* ipy, * ipu, * ipv, * op;

	// initialize buf pointer
	// note: output RGB24 is bottom line first.
	op = out + width * (height - 1) * 3;
	ipy = in;
	ipu = in + 1;
	ipv = in + 3;

	for (j = 0; j < height; j += 1)
	{
		for (i = 0; i < width; i += 2)
		{
			// get Y data
			y0 = (int)* ipy;
			y1 = (int) * (ipy + 2);

			// reverse color for better vision
	/*		y0 = 255 - y0;
			y1 = 255 - y1;*/

			// convert Y to RGB
			r0 = y0;
			r1 = y1;

			g0 = y0;
			g1 = y1;

			b0 = y0;
			b1 = y1;

			// RGB24
			// unsigned char: B0, G0, R0, B1, G1, R1, ...
			// note: RGB24 is bottom line first.
			*op = (unsigned char)Clip1(b0);
			*(op + 1) = (unsigned char)Clip1(g0);
			*(op + 2) = (unsigned char)Clip1(r0);
			*(op + 3) = (unsigned char)Clip1(b1);
			*(op + 4) = (unsigned char)Clip1(g1);
			*(op + 5) = (unsigned char)Clip1(r1);

			//
			op += 6;
			ipy += 4;
			ipu += 4;
			ipv += 4;
		}
		op -= width * 3 * 2;
	}

	return 0;
}


