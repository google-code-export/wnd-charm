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
typedef MatrixXhsv clrData;


typedef Eigen::Matrix< double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > pixData;

typedef Eigen::Matrix< byte, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor > MatrixXb;

#define INF 10E200


void histogram (pixData pix_data, double *bins, unsigned short bins_num) {
	long a;
	double min=INF,max=-INF;
	int width, height;
	double *data;
	/* find the minimum and maximum */
	min = 0;
	max = pow(2,16) - 1;
	width = pix_data.cols();
	height = pix_data.rows();
	printf ("min: %lf, max: %lf, width: %d. height: %d\n", pix_data.minCoeff(), pix_data.maxCoeff(), width, height);

	/* initialize the bins */
	for (a = 0; a < bins_num; a++)
		bins[a] = 0;

	/* build the histogram */
	data = pix_data.data();
	for (a = 0; a < width*height; a++) {
		if (data[a] == max) bins[bins_num-1]++;
		else bins[(int)(((data[a] - min)/(max - min)) * bins_num)]++;
	}

	return;
}

void test_bigmat() {
	Eigen::MatrixXd randomMat = Eigen::MatrixXd::Random(1000,2000);
	double bins[100];
	pixData pix_data = (randomMat.array() + 1.0) * ((pow(2,16) - 1) / 2.0);
	printf ("min: %lf, max: %lf\n", pix_data.minCoeff(), pix_data.maxCoeff());
	histogram (pix_data, bins, 100);
	for (int i = 0; i < 100; i++) {
		printf ("bin [%3d]: %lf\n", i, bins[i]);
	}
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
	
	mymat (1,2) = hsv;

//std::cout << "Here is the matrix mymat:\n" << mymat << std::endl;
	hsv2 = mymat (1,2);
	std::cout << "Here is the hsv:\n" << (int)hsv2.h << "," << (int)hsv2.s << "," << (int)hsv2.v << std::endl;
	printf ("hsv is (%d,%d,%d)\n",(int)hsv.h, (int)hsv.s, (int)hsv.v);
	printf ("hsv2 is (%d,%d,%d)\n",(int)hsv2.h, (int)hsv2.s, (int)hsv2.v);
	printf ("hsv3 is (%d,%d,%d)\n",(int)(mymat (1,2).h), (int)(mymat (1,2).s), (int)(mymat (1,2).v));
	for (int i = 0; i < 12; i++) {
		printf ("hsv[%d] is (%d,%d,%d)\n",i,(int)(mymat.array().coeff(i).h), (int)(mymat.array().coeff(i).s), (int)(mymat.array().coeff(i).v));
		printf ("hsv[%d]data is (%d,%d,%d)\n",i,(int)(mymat.data()[i].h), (int)(mymat.data()[i].s), (int)(mymat.data()[i].v));
	}
}

int main (int argc, char **argv) {
	test_access();
	test_bigmat();
}
