#include <string.h>
#include "Motion.h"
#include <cmath>
#include <stdlib.h>
#include <stdio.h>


#define Clip3(a, b, c) ((c < a) ? a : ((c > b) ? b : c))
#define Clip1(x)       Clip3(0, 255, x)


__global__  void Motion_Estimation_tool_cuda(int* d_lastFrameY, int* d_currentFrameY, int* d_resultFrameY, int* d_interpolation_block, int* d_temp_interpolation_block, int block_size, int search_region_R, int im_height, int im_width)
{
	int SAD_min = 1000000;
	int current_SAD;
	int SAD_min_x = 0, SAD_min_y = 0;


	// the top-left corner of this block on current frame and search block on last frame
	int search_block_x, search_block_y;
	int search_x, search_y, refer_x, refer_y;
	int delta_x, delta_y;

	const int block_x = threadIdx.x * block_size;
	const int block_y = threadIdx.y * block_size;

	bool inter_better_flag = false;



#pragma region Homework4
	for (delta_x = -search_region_R; delta_x <= search_region_R; delta_x++)
	{
		for (delta_y = -search_region_R; delta_y <= search_region_R; delta_y++)
		{
			search_block_x = block_x + delta_x;
			search_block_y = block_y + delta_y;
			if ((search_block_x >= 0) && (search_block_x <= im_height - block_size) && (search_block_y >= 0) && (search_block_y <= im_width - block_size))
			{
				// block position on lastFrameY
				current_SAD = 0;
				for (int m = 0; m < block_size; m++)
				{
					for (int n = 0; n < block_size; n++)
					{
						search_x = search_block_x + m;
						search_y = search_block_y + n;
						refer_x = block_x + m;
						refer_y = block_y + n;
						current_SAD += fabsf(d_lastFrameY[search_x * im_width + search_y] - d_currentFrameY[refer_x * im_width + refer_y]);
					}
				}
				if (current_SAD < SAD_min)
				{
					SAD_min_x = delta_x;
					SAD_min_y = delta_y;
					SAD_min = current_SAD;
				}
			}
		}
	}
#pragma endregion




#pragma region Homework5

	int inter_SAD_min = 1000000, inter_SAD;
	
	for (int flag = 1; flag <= 16; flag++)
	{

		//motion_vector of 1/2 pixel motion estimation
		switch (flag) {
		case 1:
			delta_x = -1;
			delta_y = -1;
			break;
		case 2:
			delta_x = -1;
			delta_y = 0;
			break;
		case 3:
			delta_x = 0;
			delta_y = -1;
			break;
		case 4:
			delta_x = 0;
			delta_y = 0;
			break;
		case 5:
			delta_x = 1;
			delta_y = -1;
			break;
		case 6:
			delta_x = 1;
			delta_y = 0;
			break;
		case 7:
			delta_x = -1;
			delta_y = -1;
			break;
		case 8:
			delta_x = -1;
			delta_y = 0;
			break;
		case 9:
			delta_x = -1;
			delta_y = 1;
			break;
		case 10:
			delta_x = 0;
			delta_y = -1;
			break;
		case 11:
			delta_x = 0;
			delta_y = 0;
			break;
		case 12:
			delta_x = 0;
			delta_y = 1;
			break;
		case 13:
			delta_x = -1;
			delta_y = -1;
			break;
		case 14:
			delta_x = -1;
			delta_y = 0;
			break;
		case 15:
			delta_x = 0;
			delta_y = -1;
			break;
		case 16:
			delta_x = 0;
			delta_y = 0;
			break;
		}

		search_block_x = block_x + SAD_min_x + delta_x;
		search_block_y = block_y + SAD_min_y + delta_y;

		if ((search_block_x >= 0) && (search_block_x <= im_height - block_size) && (search_block_y >= 0) && (search_block_y <= im_width - block_size))
		{
			inter_SAD = 0;

			for (int m = 0; m < block_size; m++)
			{
				for (int n = 0; n < block_size; n++)
				{
					if (flag >= 1 && flag <= 6)  //the first kind of interpolation
					{
						d_temp_interpolation_block[m * im_width + n] = (d_lastFrameY[(search_block_x + m) * im_width + search_block_y + n] + d_lastFrameY[(search_block_x + m) * im_width + search_block_y + n + 1]) / 2;
					}
					else if (flag >= 7 && flag <= 12)
					{
						d_temp_interpolation_block[m * im_width + n] = (d_lastFrameY[(search_block_x + m) * im_width + search_block_y + n] + d_lastFrameY[(search_block_x + m + 1) * im_width + search_block_y + n]) / 2;
					}
					else if (flag >= 13 && flag <= 16)
					{
						d_temp_interpolation_block[m * im_width + n] = (d_lastFrameY[(search_block_x + m) * block_size + search_block_y + n] + d_lastFrameY[(search_block_x + m + 1) * block_size + search_block_y + n]) / 2;
					}
					refer_x = block_x + m;
					refer_y = block_y + n;
					inter_SAD += fabsf(d_temp_interpolation_block[m * im_width + n] - d_currentFrameY[refer_x * im_width + refer_y]);
				}
			}

			if (inter_SAD < inter_SAD_min)
			{
				for (int m = 1; m <= block_size; m++)
				{
					for (int n = 1; n <= block_size; n++)
					{
						d_interpolation_block[m * im_width + n] = d_temp_interpolation_block[m * im_width + n];
					}
				}
			}

		}

		if (inter_SAD_min < SAD_min)
		{
			SAD_min = inter_SAD_min;
			inter_better_flag = true;
		}
	}
#pragma endregion
	

#pragma region Update Result

	if (inter_better_flag)
	{
		for (int m = 0; m < block_size; m++)
		{
			for (int n = 0; n < block_size; n++)
			{
				d_resultFrameY[(block_x + m) * im_width + (block_y + n)] = d_interpolation_block[m * im_width + n];
			}
		}
	}
	else
	{
		for (int m = 0; m < block_size; m++)
		{
			for (int n = 0; n < block_size; n++)
			{
				d_resultFrameY[(block_x + m) * im_width + (block_y + n)] = d_lastFrameY[(block_x + SAD_min_x + m) * im_width + (block_y + SAD_min_y + n)];
			}
		}
	}

#pragma endregion

}



void Motion_Estimation_cuda(int* lastFrameY, int* currentFrameY, int* resultFrameY, int block_size, int search_region_R, int im_height, int im_width)
{
	size_t size = im_height * im_width * sizeof(int);


	// Allocate memory
	int* d_currentFrameY;
	cudaMalloc(&d_currentFrameY, size);
	int* d_lastFrameY;
	cudaMalloc(&d_lastFrameY, size);
	int* d_resultFrameY;
	cudaMalloc(&d_resultFrameY, size);

	int* d_interpolation_block;
	cudaMalloc(&d_interpolation_block, size);
	int* d_temp_interpolation_block;
	cudaMalloc(&d_temp_interpolation_block, size);


	// Copy vectors from host memory to device memory
	cudaMemcpy(d_currentFrameY, currentFrameY, size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_lastFrameY, lastFrameY, size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_resultFrameY, resultFrameY, size, cudaMemcpyHostToDevice);


	// Invoke kernel
	dim3 threadsPerBlock(im_height / block_size, im_width / block_size);
	Motion_Estimation_tool_cuda << <1, threadsPerBlock >> > (d_lastFrameY, d_currentFrameY, d_resultFrameY, d_interpolation_block, d_temp_interpolation_block,  block_size, search_region_R, im_height, im_width);


	//// Copy result from device memory to host memory
	cudaMemcpy(resultFrameY, d_resultFrameY, size, cudaMemcpyDeviceToHost);


	// Free device memory
	cudaFree(d_currentFrameY);
	cudaFree(d_lastFrameY);
	cudaFree(d_resultFrameY);

	cudaFree(d_interpolation_block);
	cudaFree(d_temp_interpolation_block);

}
