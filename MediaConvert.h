/*
	Copyright (C) 2015, Liang Fan
	All rights reverved.
*/

/** \file		MediaConvert.h
    \brief		Header file of media convert functions.
*/

#ifndef __MEDIACONVERT_H__
#define __MEDIACONVERT_H__

int yuv422_to_rgb24(unsigned char *out, unsigned char *in, int width, int height);
int yuv422_to_rgb24_grey(unsigned char* out, unsigned char* in, int width, int height);
int Motion_Estimation(unsigned char* in, int width, int height);
//int* Motion_Estimation_tool(int** lastFrameY, int** currentFrameY, int block_x, int block_y, int block_size, int search_region_R, int im_height, int im_width);

#endif // __MEDIACONVERT_H__
