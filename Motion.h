#pragma once
#ifndef __MOTION_H__
#define __MOTION_H__

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "iostream"

__global__  void Motion_Estimation_tool_cuda(int* d_lastFrameY, int* d_currentFrameY, int* d_resultFrameY, int block_size, int search_region_R, int im_height, int im_width);
void Motion_Estimation_cuda(int* lastFrameY, int* currentFrameY, int* resultFrameY, int block_size, int search_region_R, int im_height, int im_width);

#endif// __MOTION_H__