#define FLT_MAX 3.402823e+38
#define FLT_MIN 1.175494e-38
#define SOL_COE_SIZE 5
#ifndef M_PI			/* in case M_PI not found in math.h */
#define M_PI 3.1415927
#endif
typedef struct real_2d_array_t
{
	float** arr;
	float*  data;
	int     rows;
	int     cols;
}real_2d;

typedef struct real_uint8_array_t
{
	uint8_t** arr;
	uint8_t*  data;
	int     rows;
	int     cols;
}uint8_2d;

typedef struct int_vector_t
{
	float* data;
	int    size;
}int_vec;

real_2d*  create_real_2d(int rows, int cols);
void      destroy_real_2d(real_2d** rd);

uint8_2d*  create_uint8_2d(int rows, int cols);
void      destroy_uint8_2d(uint8_2d** rd);

int_vec*  create_int_vec(int size);
void       destroy_int_vec(int_vec** rv);

real_2d* uint8_to_real(uint8_t* img, int w, int h);
uint8_2d* real_to_uint8(real_2d* img);

float get_real_2d_max(real_2d* rd);
void normalize_real_2d(real_2d* rd, float max_val);
void normalized_to_unnormalize(real_2d* rd);

void cleanup_weak_edge(real_2d* mag, int_vec* wrv, int_vec* wcv, int wcount);
void edge_track(real_2d* mag, int_vec* srv, int_vec* scv, int scount);
void double_thresholding(real_2d* mag, float low_thres, float high_thres,
	int_vec** _srv, int_vec** _scv, int* _scount,
	int_vec** _wrv, int_vec** _wcv, int* _wcount);
void  non_maximum_supression(real_2d* dir, real_2d* mag, real_2d** nms, float* mag_max);
void calculate_angle_and_mag(real_2d* sgx, real_2d* sgy, real_2d** dir, real_2d** mag);
real_2d* sobel_grad(real_2d* img, float sobel[SOL_COE_SIZE][SOL_COE_SIZE]);
void gaussian_blur(real_2d* img, real_2d* kernel, int r);
real_2d* create_gaussian_kernel(float sigma);

void canny(uint8_t* data, int w, int h, uint8_t** edge);


float low_thred_ratio = 0.4;
float high_thred_ratio = 0.7;
