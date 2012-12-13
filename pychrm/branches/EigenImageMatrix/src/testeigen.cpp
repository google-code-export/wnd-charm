/* compile:
g++ -O3 -o testeigen testeigen.cpp
*/
#include <sys/time.h>
//#include <ctime>
typedef unsigned long long timestamp_t;
static timestamp_t get_timestamp () {
	struct timeval now;
	gettimeofday (&now, NULL);
	return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}

#include <iostream>

#include "Eigen/Dense"
#include <stdio.h>
typedef unsigned char byte;
typedef struct {
	byte r,g,b;
} RGBcolor;
typedef struct {
	byte h,s,v;
} HSVcolor;
typedef Eigen::Matrix< HSVcolor, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > MatrixXhsv;
//typedef Eigen::Matrix< HSVcolor, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor > MatrixXhsv;
typedef MatrixXhsv clrData;


typedef Eigen::Matrix< double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > pixData;

typedef Eigen::Matrix< byte, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > MatrixXb;

#define INF 10E200

// note the const when passing an Eigen matrix
void histogram (const pixData& pix_plane, double *bins, unsigned short bins_num) {
	long a;
	double min = INF,max = -INF;
	int width, height;
	/* find the minimum and maximum */
	min = 0;
	max = pow(2,16) - 1;
	width = pix_plane.cols();
	height = pix_plane.rows();
//	printf ("mat: %p, dat: %p, min: %lf, max: %lf\n", &(pix_plane), pix_plane.data(), pix_plane.minCoeff(), pix_plane.maxCoeff());

	/* initialize the bins */
	for (a = 0; a < bins_num; a++)
		bins[a] = 0;

	/* build the histogram */
// without -O3 this is still fast
// note how the data pointer is declared as const
// 	const double *data;
// 	data = pix_plane.data();
// 	for (a = 0; a < width*height; a++) {
// 		if (data[a] == max) bins[bins_num-1]++;
// 		else bins[(int)(((data[a] - min)/(max - min)) * bins_num)]++;
// 	}

// with -O3 this is as fast as above (also safer?)
// without -O3 this is 15x slower
	for (a = 0; a < width*height; a++) {
		if (pix_plane.array().coeff(a) == max) bins[bins_num-1]++;
		else bins[(int)(((pix_plane.array().coeff(a) - min)/(max - min)) * bins_num)]++;
	}

	return;
}

void test_bigmat() {
	Eigen::MatrixXd randomMat = Eigen::MatrixXd::Random(1000,2000);
	double bins[100];
	pixData pix_plane = (randomMat.array() + 1.0) * ((pow(2,16) - 1) / 2.0);
	printf ("mat: %p, dat: %p, min: %lf, max: %lf\n", (void *)&(pix_plane), (void *)(pix_plane.data()), pix_plane.minCoeff(), pix_plane.maxCoeff());
	
	timestamp_t t0 = get_timestamp();
	for (int i = 0; i < 100; i++) {
		histogram (pix_plane, bins, 100);
	}
	timestamp_t t1 = get_timestamp();

	double msecs = (t1 - t0) / 1000.0L;
	printf ("%d trials, %.4f msecs\n",100,msecs);

// 	for (int i = 0; i < 100; i++) {
// 		printf ("bin [%3d]: %lf\n", i, bins[i]);
// 	}
}

void test_scalar () {
	Eigen::MatrixXd randomMat = Eigen::MatrixXd::Random(10,10);
	Eigen::MatrixXi testmat = ((randomMat.array() + 1.0) * ((pow(2,16) - 1) / 2.0)).cast<int>();

	std::cout << "Here is the testmat coefficient-wise scalars:\n" << testmat << std::endl;

}


void test_access () {
clrData mymat(3,4);

/*
0 0 0 0
0 0 0 0
0 0 0 0
*/
HSVcolor hsv, hsv2, *hsv3;

	hsv.h = 10;
	hsv.s = 20;
	hsv.v = 30;
	
	// this should be the 6th element in a RowMajor row,col matrix
//	mymat (1,2) = hsv;
	(mymat.array())(6) = hsv;
	hsv2 = mymat (1,2);
	std::cout << "Here is the hsv:\n" << (int)hsv2.h << "," << (int)hsv2.s << "," << (int)hsv2.v << std::endl;
	printf ("hsv is (%d,%d,%d)\n",(int)hsv.h, (int)hsv.s, (int)hsv.v);
	printf ("hsv2 is (%d,%d,%d)\n",(int)hsv2.h, (int)hsv2.s, (int)hsv2.v);
	printf ("hsv3 is (%d,%d,%d)\n",(int)(mymat (1,2).h), (int)(mymat (1,2).s), (int)(mymat (1,2).v));
	for (int i = 0; i < 12; i++) {
		printf ("hsv.data[%d] is (%d,%d,%d)\n",i,(int)(mymat.data()[i].h), (int)(mymat.data()[i].s), (int)(mymat.data()[i].v));
		printf ("hsv.array.coeff[%d] is (%d,%d,%d)\n",i,(int)(mymat.array().coeff(i).h), (int)(mymat.array().coeff(i).s), (int)(mymat.array().coeff(i).v));
	}
}

int main (int argc, char **argv) {
	test_access();
//	test_bigmat();
//	test_scalar ();
}
