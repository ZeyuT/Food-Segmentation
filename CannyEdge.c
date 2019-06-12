#include "stdafx.h"
#include "CannyEdge.h"

real_2d*  create_real_2d(int rows, int cols)
{
	real_2d*   rd = NULL;
	rd = (real_2d*)calloc(1, sizeof(real_2d));
	if (!rd)
	{
		return NULL;
	}
	rd->data = (float*)calloc(1, rows * cols * sizeof(float));
	if (!rd->data)
	{
		destroy_real_2d(&rd);
		return NULL;
	}
	rd->arr = (float**)calloc(1, rows * sizeof(float*));
	if (!rd->arr)
	{
		destroy_real_2d(&rd);
		return NULL;
	}
	int i;
	for (i = 0; i < rows; i++)
	{
		rd->arr[i] = &rd->data[i*cols];
	}
	rd->cols = cols;
	rd->rows = rows;
	return rd;
}

void destroy_real_2d(real_2d** _rd)
{
	if (!_rd) return;
	real_2d* rd = *_rd;
	if (!rd)
	{
		return;
	}
	if (rd->data)
	{
		free(rd->data);
		rd->data = NULL;
	}
	if (rd->arr)
	{
		free(rd->arr);
		rd->arr = NULL;
	}
	free(rd);
	*_rd = NULL;
}

uint8_2d*  create_uint8_2d(int rows, int cols)
{
	uint8_2d*   ud = NULL;
	ud = (uint8_2d*)calloc(1, sizeof(uint8_2d));
	if (!ud)
	{
		return NULL;
	}
	ud->data = (uint8_t*)calloc(1, rows * cols * sizeof(uint8_t));
	if (!ud->data)
	{
		destroy_real_2d(&ud);
		return NULL;
	}
	ud->arr = (uint8_t**)calloc(1, rows * sizeof(uint8_t*));
	if (!ud->arr)
	{
		destroy_real_2d(&ud);
		return NULL;
	}
	int i;
	for (i = 0; i < rows; i++)
	{
		ud->arr[i] = &ud->data[i*cols];
	}
	ud->cols = cols;
	ud->rows = rows;
	return ud;
}

void  destroy_uint8_2d(uint8_2d** _ud)
{
	if (!_ud) return;
	uint8_2d* ud = *_ud;
	if (!ud)
	{
		return;
	}
	if (ud->data)
	{
		free(ud->data);
		ud->data = NULL;
	}
	if (ud->arr)
	{
		free(ud->arr);
		ud->arr = NULL;
	}
	free(ud);
	*_ud = NULL;
}

int_vec*  create_int_vec(int size)
{
	if (size < 0)return NULL;
	int_vec* iv = (int_vec*)calloc(1, sizeof(int_vec));
	if (!iv) return iv;
	iv->data = (int*)calloc(1, size * sizeof(int));
	if (!iv->data)
	{
		destroy_int_vec(&iv);
		return NULL;
	}
	iv->size = size;
	return iv;
}

void  destroy_int_vec(int_vec** _iv)
{
	if (!_iv)return;
	int_vec* iv = *_iv;
	if (!iv) return;
	if (iv->data)
	{
		free(iv->data);
		iv->data = NULL;
	}
	free(iv);
	*_iv = NULL;
}

real_2d* uint8_to_real(uint8_t* img, int w, int h)
{
	real_2d*  res = create_real_2d(h, w);
	if (!res)return res;
	int i, j;
	float** arr = res->arr;
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			arr[i][j] = img[i*w + j];
		}
	}
	res->cols = w;
	res->rows = h;
	return res;
}

uint8_2d* real_to_uint8(real_2d* img)
{
	uint8_2d*  res = NULL;
	if (!img)return NULL;
	int i, j;
	int rows, cols;
	int val;
	rows = img->rows;
	cols = img->cols;
	res = create_uint8_2d(rows, cols);
	if (!res)return NULL;
	float** img_arr = img->arr;
	uint8_t** res_arr = res->arr;
	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
			val = round(img_arr[i][j]);
			if (val > 255)val = 255;
			if (val < 0) val = 0;
			res_arr[i][j] = val;
		}
	}
	return res;
}

float get_real_2d_max(real_2d* rd)
{
	if (!rd)return FLT_MAX;
	int i, j;
	float max_val = FLT_MIN;
	float** rd_arr = rd->arr;
	int rows, cols;
	rows = rd->rows;
	cols = rd->cols;
	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
			if (rd_arr[i][j] > max_val)
				max_val = rd_arr[i][j];
		}
	}
	return max_val;
}

void normalize_real_2d(real_2d* rd, float max_val)
{
	if (!rd || max_val == 0.0)return;
	int i, j;
	float** rd_arr = rd->arr;
	int rows, cols;
	rows = rd->rows;
	cols = rd->cols;
	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
			rd_arr[i][j] /= max_val;
		}
	}
}

void normalized_to_unnormalize(real_2d* rd)
{
	if (!rd)return;
	int i, j;
	float** rd_arr = rd->arr;
	int rows, cols;
	rows = rd->rows;
	cols = rd->cols;
	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
			rd_arr[i][j] *= 255.0;
		}
	}
}

real_2d* create_gaussian_kernel(float sigma)
{
	int kernel_size;
	kernel_size = 2 * ceil(2 * sigma) + 1;
	real_2d* rd = create_real_2d(kernel_size, kernel_size);
	if (!rd)return rd;
	int i, j;
	int half_size = kernel_size / 2;
	float** rd_arr = rd->arr;
	float  sum = 0.0;
	float  sigma2 = sigma * sigma * 2;
	float  val = 0.0;
	for (i = -half_size; i <= half_size; i++)
	{
		for (j = -half_size; j <= half_size; j++)
		{
			val = -1 * (i*i + j * j) / sigma2;
			val = exp(val);
			sum += val;
			rd_arr[i + half_size][j + half_size] = val;
		}
	}
	assert(sum != 0.0);
	for (i = 0; i < kernel_size; i++)
	{
		for (j = 0; j < kernel_size; j++)
		{
			rd_arr[i][j] /= sum;
			printf("%.4f ", rd_arr[i][j]);
		}
		printf("\n");
	}
	return rd;
}

void gaussian_blur(real_2d* img, real_2d* kernel, int r)
{
	if (!img || !kernel)return;
	int i, j;
	int m, n;
	int bound;
	float sum = 0.0f;
	int w = img->cols;
	int h = img->rows;
	real_2d* rd = create_real_2d(h, w);
	if (!rd)return;
	float**  dst = rd->arr;
	float**  src = img->arr;
	float**  gc = kernel->arr;
	int col, row;
	bound = kernel->rows / 2;
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			sum = 0.0;
			for (m = -bound; m <= bound; m++)
			{
				for (n = -bound; n <= bound; n++)
				{
					//if (i + m < 0)sum += 0;
					//else if (i + m >= h)sum += 0;
					//else if (j + n < 0)sum += 0;
					//else if (j + n >= w)sum += 0;
					//else sum += gc[m + bound][n + bound] * src[i+m][j+n];
					if (i + m < 0)row = 0;
					else if (i + m >= h)row = h - 1;
					else row = m + i;
					if (j + n < 0)col = 0;
					else if (j + n >= w)col = w - 1;
					else col = j + n;

					sum += gc[m + bound][n + bound] * src[row][col];
				}
			}
			if (r)dst[i][j] = round(sum);
			else dst[i][j] = sum;

		}
	}
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			src[i][j] = dst[i][j];
		}
	}
	destroy_real_2d(&rd);
}

real_2d* sobel_grad(real_2d* img, float sobel[SOL_COE_SIZE][SOL_COE_SIZE])
{
	if (!img)return NULL;
	int i, j;
	int m, n;
	int bound = SOL_COE_SIZE / 2;
	float sum = 0.0f;
	int w = img->cols;
	int h = img->rows;
	real_2d* rd = create_real_2d(h, w);
	if (!rd)return NULL;
	float**  dst = rd->arr;
	float**  src = img->arr;
	int row, col;

	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			sum = 0.0;
			for (m = -bound; m <= bound; m++)
			{
				for (n = -bound; n <= bound; n++)
				{

					if (i + m < 0)sum += 0;
					else if (i + m >= h)sum += 0;
					else if (j + n < 0)sum += 0;
					else if (j + n >= w)sum += 0;
					else sum += sobel[m + bound][n + bound] * src[i + m][j + n];
				}
			}
			dst[i][j] = sum;
		}
	}
	return rd;
}

void calculate_angle_and_mag(real_2d* sgx, real_2d* sgy, real_2d** dir, real_2d** mag)
{
	if (!sgx || !sgy)return;
	int rows;
	int cols;
	int i, j;
	assert(sgx->rows == sgy->rows && sgx->cols == sgy->cols);
	rows = sgx->rows;
	cols = sgx->cols;
	real_2d* direct = create_real_2d(rows, cols);
	real_2d* magnitude = create_real_2d(rows, cols);
	if (!direct || !magnitude)
	{
		destroy_real_2d(&direct);
		destroy_real_2d(&magnitude);
		return;
	}
	float** dir_arr = direct->arr;
	float y;
	float x;
	float angle;
	float** mag_arr = magnitude->arr;
	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
			y = sgy->arr[i][j];
			x = sgx->arr[i][j];
			angle = atan2(y, x) * 180.0 / M_PI;
			dir_arr[i][j] = angle;
			mag_arr[i][j] = sqrt(x*x + y * y);
		}
	}
	*dir = direct;
	*mag = magnitude;
}

void  non_maximum_supression(real_2d* dir, real_2d* mag, real_2d** nms, float* mag_max)
{
	if (!dir || !mag)return;
	int rows;
	int cols;
	int i, j;
	assert(dir->rows == mag->rows && dir->cols == mag->cols);
	rows = mag->rows;
	cols = mag->cols;
	real_2d* res = create_real_2d(rows, cols);
	if (!res)return NULL;
	float** res_arr = res->arr;
	float** dir_arr = dir->arr;
	float** mag_arr = mag->arr;
	float   angle;
	float   mag_cur;
	float   mag1_max = -1.0;
	int     cur_max;
	rows--;
	cols--;

	for (i = 1; i < rows; i++)
	{
		for (j = 1; j < cols; j++)
		{
			angle = dir_arr[i][j];
			mag_cur = mag_arr[i][j];
			cur_max = 0;
			if ((angle > -22.5 && angle <= 22.5) || (angle < -157.5 && angle >= -180) || (angle > 157.5 && angle <= 180))
			{
				if (mag_cur >= mag_arr[i][j + 1] && mag_cur >= mag_arr[i][j - 1])
				{
					res_arr[i][j] = mag_arr[i][j];
				}
				else
				{
					res_arr[i][j] = 0;
				}
			}
			else if ((angle > 22.5 && angle <= 67.5) || (angle < -112.5 && angle >= -157.5))
			{
				if (mag_cur >= mag_arr[i + 1][j + 1] && mag_cur >= mag_arr[i - 1][j - 1])
				{
					res_arr[i][j] = mag_arr[i][j];
				}
				else
				{
					res_arr[i][j] = 0;
				}
			}
			else if ((angle > 67.5 && angle <= 112.5) || (angle >= -112.5 && angle <= -67.5))
			{
				if (mag_cur >= mag_arr[i + 1][j] && mag_cur >= mag_arr[i - 1][j])
				{
					res_arr[i][j] = mag_arr[i][j];
				}
				else
				{
					res_arr[i][j] = 0;
				}
			}
			else if ((angle > 112.5 && angle <= 157.5) || (angle <= -22.5 && angle > -67.5))
			{
				if (mag_cur >= mag_arr[i + 1][j - 1] && mag_cur >= mag_arr[i - 1][j + 1])
				{
					res_arr[i][j] = mag_arr[i][j];
				}
				else
				{
					res_arr[i][j] = 0;
				}
			}

			if (res_arr[i][j] > mag1_max)
				mag1_max = res_arr[i][j];
		}
	}
	*nms = res;
	*mag_max = mag1_max;
}

void double_thresholding(real_2d* mag, float low_thres, float high_thres,
	int_vec** _srv, int_vec** _scv, int* _scount,
	int_vec** _wrv, int_vec** _wcv, int* _wcount)
{
	if (!mag)return;
	int rows, cols;
	int i, j;
	int scount;
	int wcount;
	float** mag_arr;
	int_vec* srv;
	int_vec* scv;
	int_vec* wrv;
	int_vec* wcv;
	float mag_val;

	rows = mag->rows;
	cols = mag->cols;
	mag_arr = mag->arr;
	scount = 0;
	wcount = 0;
	srv = create_int_vec(rows*cols);
	scv = create_int_vec(rows*cols);
	wrv = create_int_vec(rows*cols);
	wcv = create_int_vec(rows*cols);
	if (!srv || !scv || !wrv || !wcv)
	{
		destroy_int_vec(&srv);
		destroy_int_vec(&scv);
		destroy_int_vec(&wrv);
		destroy_int_vec(&wcv);
		return;
	}
	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cols; j++)
		{
			mag_val = mag_arr[i][j];
			if (mag_val > high_thres)
			{
				srv->data[scount] = i;
				scv->data[scount] = j;
				scount++;
				mag_arr[i][j] = 1.0;
			}
			else if (mag_val < low_thres)
			{
				mag_arr[i][j] = 0;
			}
			else
			{
				wrv->data[wcount] = i;
				wcv->data[wcount] = j;
				wcount++;
			}
		}
	}
	*_scount = scount;
	*_wcount = wcount;
	*_scv = scv;
	*_srv = srv;
	*_wcv = wcv;
	*_wrv = wrv;
}

void edge_track(real_2d* mag, int_vec* srv, int_vec* scv, int scount)
{
	if (!mag || !srv || !scv)return;
	int i;
	int m, n;
	int rows, cols;
	float val;
	int row, col;
	rows = mag->rows;
	cols = mag->cols;
	for (i = 0; i < scount; i++)
	{
		row = srv->data[i];
		col = scv->data[i];
		for (m = -2; m <= 2; m++)
		{
			for (n = -2; n <= 2; n++)
			{
				if (m + row >= 0 && n + col >= 0 && m + row < rows && n + col < cols)
				{
					val = mag->arr[row + m][col + n];
					if (val > 0.0 && val < 1)
					{
						assert(scount < rows * cols);
						mag->arr[row + m][col + n] = 1;
						srv->data[scount] = row + m;
						scv->data[scount] = col + n;

						mag->arr[row + m][col + n] = 1;
						scount++;
					}
				}
			}
		}
	}
}


void cleanup_weak_edge(real_2d* mag, int_vec* wrv, int_vec* wcv, int wcount)
{
	if (!mag || !wrv || !wcv)return;
	int i;
	int row, col;
	for (i = 0; i < wcount; i++)
	{
		row = wrv->data[i];
		col = wcv->data[i];
		if (mag->arr[row][col] != 1.0)
			mag->arr[row][col] = 0.0;
	}
}

void canny(uint8_t* data, int w, int h, uint8_t** edge)
{
	real_2d* kernel = NULL;
	real_2d* sgx = NULL;
	real_2d* sgy = NULL;
	real_2d* img = uint8_to_real(data, w, h);
	real_2d* dir = NULL;
	real_2d* mag = NULL;
	real_2d* nms = NULL;
	uint8_2d* e = NULL;
	float sigma = 1.4;
	float mag_max = 0.0;
	float low_thred, high_thred;
	float sobelx[3][3] = { { 1, 0, -1 },{ 2, 0, -2 },{ 1, 0, -1 } };
	float sobely[3][3] = { { 1, 2, 1 },{ 0, 0, 0 },{ -1, -2, -1 } };
	if (!img)return;
	kernel = create_gaussian_kernel(sigma);
	if (!kernel)return;
	gaussian_blur(img, kernel, 1);
	sgx = sobel_grad(img, sobelx);
	gaussian_blur(sgx, kernel, 0);
	sgy = sobel_grad(img, sobely);
	gaussian_blur(sgy, kernel, 0);
	calculate_angle_and_mag(sgx, sgy, &dir, &mag);
	non_maximum_supression(dir, mag, &nms, &mag_max);
	mag_max = get_real_2d_max(nms);
	normalize_real_2d(nms, mag_max);
	mag_max = get_real_2d_max(nms);
	high_thred = mag_max * high_thred_ratio;
	low_thred = high_thred * low_thred_ratio;

	int_vec* srv = NULL;
	int_vec* scv = NULL;
	int_vec* wrv = NULL;
	int_vec* wcv = NULL;
	int scount = 0;
	int wcount = 0;
	double_thresholding(nms, low_thred, high_thred, &srv, &scv, &scount, &wrv, &wcv, &wcount);
	edge_track(nms, srv, scv, scount);
	cleanup_weak_edge(nms, wrv, wcv, wcount);
	normalized_to_unnormalize(nms);
	e = real_to_uint8(nms);
	destroy_real_2d(&sgx);
	destroy_real_2d(&sgy);
	destroy_real_2d(&mag);
	destroy_real_2d(&nms);
	destroy_int_vec(&srv);
	destroy_int_vec(&scv);
	destroy_int_vec(&wrv);
	destroy_int_vec(&wcv);
	if (!e)return;
	*edge = e->data;
	e->data = NULL;
	destroy_uint8_2d(&e);
}
