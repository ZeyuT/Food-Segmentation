#include "stdafx.h"
#include "globals.h"

int is_in_input(int x, int y, int height, int width)
{
	if (x >= 0 && x < width && y >= 0 && y < height)
		return 1;
	else
		return 0;
}

void bilinear_interpolation(unsigned char* input, int height, int width,
	unsigned char* output, int out_height, int out_width)
{
	float h_times = (float)out_height / (float)height,
		w_times = (float)out_width / (float)width;
	int  x1, y1, x2, y2, f11, f12, f21, f22;
	float x, y;

	for (int i = 0; i < out_height; i++) {
		for (int j = 0; j < out_width; j++) {
			x = j / w_times;
			y = i / h_times;
			x1 = (int)(x - 1);
			x2 = (int)(x + 1);
			y1 = (int)(y + 1);
			y2 = (int)(y - 1);
			f11 = is_in_input(x1, y1, height, width) ? input[y1*width + x1] : 0;
			f12 = is_in_input(x1, y2, height, width) ? input[y1*width + x1] : 0;
			f21 = is_in_input(x2, y1, height, width) ? input[y1*width + x1] : 0;
			f22 = is_in_input(x2, y2, height, width) ? input[y1*width + x1] : 0;
			output[i*out_width + j] = (int)(((f11 * (x2 - x) * (y2 - y)) +
				(f21 * (x - x1) * (y2 - y)) +
				(f12 * (x2 - x) * (y - y1)) +
				(f22 * (x - x1) * (y - y1))) / ((x2 - x1) * (y2 - y1)));
		}
	}
}

void Normalization(float* list, int length, int scale) {
	float max;
	float min;
	int i;
	max = 0.0;
	min = 100000.0;
	for (i = 0; i < length; i++) {
		if (list[i] < min)
			min = list[i];
		if (list[i] > max)
			max = list[i];
	}
	if (max > min)
		for (i = 0; i < length; i++)
			list[i] = scale * (list[i] - min) / (max - min);
	else
		memset(list, 0, length);
}

float CalculateDistance(int x1, int y1, int x2, int y2) {
	return sqrt(SQR(x1 - x2) + SQR(y1 - y2));
}

void ActiveContour(HWND AnimationWindowHandle)

{
	FILE	*fpt;
	unsigned char	*output_img, *gradient_output;
	uint8_t		*copy_img, *CannyEdge;
	int			i, m, j, r, c, dc, dr, indice, iteration;
	int			sum_x, sum_y, min_indice, ini_indice;
	float		average;
	int			*X_temp, *Y_temp, *loc_x, *loc_y;
	float		*magnitude, *distance, *difference, *gradient, *intensity_diff, *radius, *gradient_canny;
	float		min_energy;
	/* Sobel operator */
	int			fx[3][3] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
	int			fy[3][3] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };
	int			window_size = 17;
	float		centroid_y, centroid_x; /* centroid location of all selected pixels */
	int			closest_y, closest_x, count, closest_distance;
	float		aver_intensity;
	float T[6] = { -2, 3, 6, 1,1, -1 };		/* for egg-pancakes-milk.pnm */
	//float T[6] = { -2, 3, 6, 1,1,0 };		/* for others */
	//float T[6] = { -2, 3, 6, 1,1,0 };		/* for macaroni-kale.pnm */
	char		Gradient_T = 0;
	if (balloon_flag == 1) {
		/* {-2 2 6 6 -3 2} for egg-pancakes-milk.pnm */
		/* {-2 2 6 6 -3 -3 } for egg-pancakes-milk.pnm */
		T[0] = -2;	/* Magnitude */
		T[1] = 2;	/* Distance */
		T[2] = 6;	/* Difference */
		T[3] = 6;	/* Intensity difference */
		T[4] = -3;	/* Radius */
		T[5] = 2;	/* Canny edge */
	}
	/* read in initial contour points*/
	copy_img = (uint8_t *)calloc(ROWS*COLS, sizeof(uint8_t));
	CannyEdge = (uint8_t *)calloc(ROWS*COLS, sizeof(uint8_t));
	output_img = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));

	for (i = 0; i < COLS * ROWS; i++) {
		copy_img[i] = OriginalImage[i];
		OriginalImage[i] = copy_img[i];
	}
	CannyEdge = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
	canny(copy_img, COLS, ROWS, &CannyEdge);

	//fpt = fopen("result.txt", "w");

	/* Convolution with sobel template */
	gradient = (float *)calloc(ROWS*COLS, sizeof(float));
	gradient_output = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
	for (r = 1; r < ROWS - 1; r++)
		for (c = 1; c < COLS - 1; c++) {
			sum_x = sum_y = 0;
			for (dr = -1; dr <= 1; dr++)
				for (dc = -1; dc <= 1; dc++) {
					sum_x += OriginalImage[(r + dr) * COLS + dc + c] * fx[dr + 1][dc + 1];
					sum_y += OriginalImage[(r + dr) * COLS + dc + c] * fy[dr + 1][dc + 1];
				}
			gradient[r * COLS + c] = sqrt(sum_x * sum_x + sum_y * sum_y);
		}
	Normalization(gradient, COLS*ROWS, 255);
	for (i = 0; i < COLS * ROWS; i++) {
		if (gradient[i] < Gradient_T)
			gradient[i] = 0;
		gradient_output[i] = (char)CannyEdge[i];
		//OriginalImage[i] = (char)CannyEdge[i];

	}


	X_temp = (int *)calloc(PointNum, sizeof(int));
	Y_temp = (int *)calloc(PointNum, sizeof(int));
	magnitude = (float *)calloc(SQR(window_size), sizeof(double));
	distance = (double *)calloc(SQR(window_size), sizeof(double));
	difference = (double *)calloc(SQR(window_size), sizeof(double));
	intensity_diff = (double *)calloc(SQR(window_size), sizeof(double));
	radius = (double *)calloc(SQR(window_size), sizeof(double));
	gradient_canny = (double *)calloc(SQR(window_size), sizeof(double));
	loc_x = (int *)calloc(SQR(window_size), sizeof(int));
	loc_y = (int *)calloc(SQR(window_size), sizeof(int));

	for (iteration = 0; iteration < iterations; iteration++) {
		/* Average distance between all contour points */
		if (ThreadRunning_AC == 0)
			return;
		if (balloon_flag == 1 && iteration == 30) {
			/* {-2 1 10 1 0 -1} for egg-pancakes-milk.pnm */
			/* {-3 1 6 1 1 -2} for bacon-eggs-toast.pnm */
			T[0] = -2;
			T[1] = 1;
			T[2] = 10;
			T[3] = 1;
			T[4] = 0;
			T[5] = -1;			
		}
		/* for pancake */
		if (balloon_flag == 0 && iteration == 30) {
			T[0] = -3;
			T[1] = -2;
			T[2] = 12;
			T[3] = 1;
			T[4] = 0;
			T[5] = -2;
		}

		average = 0;
		aver_intensity = 0;
		centroid_y = 0;
		centroid_x = 0;
		for (m = 0; m < PointNum; m++) {
			average += CalculateDistance(Xcoordinates[m], Ycoordinates[m], Xcoordinates[(m + 1) % PointNum], Ycoordinates[(m + 1) % PointNum]);
			centroid_y += Ycoordinates[m];
			centroid_x += Xcoordinates[m];
		}

		average = average / PointNum;
		centroid_x = centroid_x / PointNum;
		centroid_y = centroid_y / PointNum;
		closest_y = 0;
		closest_x = 0;
		for (m = 0; m < PointNum; m++)
			if (sqrt(SQR(centroid_x - Xcoordinates[m]) + SQR(centroid_y - Ycoordinates[m]))) {
				closest_y = Ycoordinates[m];
				closest_x = Xcoordinates[m];
			}
		closest_distance = (int)CalculateDistance(closest_x, closest_y, centroid_x, centroid_y);
		aver_intensity = 0;
		count = 0;
		for (r = max(0, centroid_y - closest_distance); r < min(ROWS, centroid_y + closest_distance); r++)
			for (c = max(0, centroid_x - closest_distance); c < min(COLS, centroid_x + closest_distance); c++)
				if (CalculateDistance(c, r, centroid_x, centroid_y) <= closest_distance) {
					count += 1;
					aver_intensity += OriginalImage[r*COLS + c];
				}
		aver_intensity = aver_intensity / count;
		for (i = 0; i < PointNum; i++) {
			indice = 0;
			memset(magnitude, 0, SQR(window_size));
			memset(distance, 0, SQR(window_size));
			memset(difference, 0, SQR(window_size));
			memset(intensity_diff, 0, SQR(window_size));
			memset(radius, 0, SQR(window_size));
			memset(gradient_canny, 0, SQR(window_size));
			memset(loc_x, 0, SQR(window_size));
			memset(loc_y, 0, SQR(window_size));
			ini_indice = 0;
			/* Calculate energy items in windows around each contour points */
			for (r = max(0, Ycoordinates[i] - window_size / 2); r <= min(ROWS - 1, Ycoordinates[i] + window_size / 2); r++)
				for (c = max(0, Xcoordinates[i] - window_size / 2); c <= min(COLS - 1, Xcoordinates[i] + window_size / 2); c++)
				{	
					magnitude[indice] = SQR(gradient[r * COLS + c]);
					if (grab_idx > 0)
						distance[indice] = SQR(c - Xcoordinates[(i + 1) % PointNum]) + SQR(r - Ycoordinates[(i + 1) % PointNum]) +
						3*SQR(c - Xcoordinates[(i - 1 + PointNum) % PointNum]) + SQR(r - Ycoordinates[(i - 1 + PointNum) % PointNum]);
					else
						distance[indice] = SQR(c - Xcoordinates[(i + 1) % PointNum]) + SQR(r - Ycoordinates[(i + 1) % PointNum]);
					difference[indice] = SQR(CalculateDistance(c, r, Xcoordinates[(i + 1) % PointNum], Ycoordinates[(i + 1) % PointNum]) - average);
					intensity_diff[indice] = SQR(OriginalImage[r*COLS + c] - aver_intensity);
					radius[indice] = SQR(CalculateDistance(c, r, centroid_x, centroid_y));
					gradient_canny[indice] = SQR(CannyEdge[r * COLS + c]);
					loc_x[indice] = c;
					loc_y[indice] = r;
					if (c == Xcoordinates[i] && r == Ycoordinates[i])
						ini_indice = indice;
					//if (iteration == 40) {
					//	fprintf(fpt, "%f %d \r\n", distance[indice],indice);
					//}
					indice++;
				}
			Normalization(magnitude, indice, 1);
			Normalization(distance, indice, 1);
			Normalization(difference, indice, 1);
			Normalization(intensity_diff, indice, 1);
			Normalization(radius, indice, 1);
			Normalization(gradient_canny, indice, 1);
			/* Initialize min energy to the energy at current pixel */
			//if (grab_idx > 0 && iteration == 0)
			//	T[2] = 20;
			//if (grab_idx > 0 && iteration == 10)
			//	T[2] = 6;
			min_indice = ini_indice;
			min_energy = T[0] * magnitude[min_indice] + T[1] * distance[min_indice] + T[2] * difference[min_indice] + 
							T[3] * intensity_diff[min_indice] + T[4] * radius[min_indice] + T[5] * gradient_canny[min_indice];
			for (j = 0; j < indice; j++) {
				if (T[0] * magnitude[j] + T[1] * distance[j] + T[2] * difference[j] + T[3] * intensity_diff[j] + 
					T[4] * radius[j] + T[5] * gradient_canny[min_indice] < min_energy) {
					min_energy = T[0] * magnitude[j] + T[1] * distance[j] + T[2] * difference[j] + 
						T[3] * intensity_diff[j] + T[4] * radius[j] + T[5] * gradient_canny[min_indice];
					min_indice = j;
				}
			}
			//fprintf(fpt, "%d %f\n", min_indice,min_energy);

			X_temp[i] = loc_x[min_indice];
			Y_temp[i] = loc_y[min_indice];
		}
		for (m = 0; m < PointNum; m++) {
			if (m != grab_idx) {
				Xcoordinates[m] = X_temp[m];
				Ycoordinates[m] = Y_temp[m];
			}
		}
		PaintImage(OriginalImage, ROWS, COLS);
		Sleep(50);
	}
	/* write out results */
	fpt = fopen("test.ppm", "w");
	fprintf(fpt, "P5 %d %d 255\n", COLS, ROWS);
	fwrite(gradient_output, COLS*ROWS, 1, fpt);
	fclose(fpt);
}
