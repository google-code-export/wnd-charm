/* FeatureAlgorithm.cpp */

#include "FeatureAlgorithm.h"
#include "cmatrix.h"
#include "FeatureNames.hpp"
#include "transforms.h"
#include "MatrixMap.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <time.h>
#include <sys/time.h>


//start #including the functions directly once you start pulling them out of cmatrix
//#include "transforms/Chebyshev.h"

#define MIN(a,b) (a<b?a:b)
//#define MAX(a,b) (a>b?a:b)
using namespace std;

void FeatureAlgorithm::print_info() const {
	std::cout << "FeatureAlgorithm: " << name << " (" << n_features << " features) " << std::endl;
}
//======================================================================
void FeatureAlgorithm::dump(std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs ) const
{
	/*
	std::string pixel_plane;

	std::vector<Transform*>::iterator it = run_algorithm_on_this_sequence.begin();
	for( ; it != run_algorithm_on_this_sequence.end() ; ++it ) {
		pixel_plane += (*it)->name;
	}

	time_t ltime;
	struct tm *Tm;
	struct timeval detail_time;

	ltime=time(NULL);
	Tm=localtime(&ltime);

	std::ostringstream filename;

	std::string alg_name_copy = name;
	std::replace( alg_name_copy.begin(), alg_name_copy.end(), ' ', '_');

	filename << alg_name_copy << "_" << pixel_plane << "_" 
		<< setw(2) << setfill('0') << Tm->tm_hour <<  '-'
		<< setw(2) << setfill('0') << Tm->tm_min << '-'
		<< setw(2) << setfill('0') << Tm->tm_sec << ".";
	gettimeofday(&detail_time,NULL);
	filename << setw(2) <<  detail_time.tv_usec / 1000;
	filename << ".txt";
	std::cout << "alg dump: " << filename.str() << std::endl;

	
	ofstream outfile( filename.str().c_str() );
	if( !outfile.good() )
		assert(0);

	int count = 0;
	std::vector<double>::iterator coeff_it = coeffs.begin();
	for( ; coeff_it != coeffs.end(); ++coeff_it )
	{
		outfile << count++ << "\t" << *coeff_it << std::endl;
	}
	
	outfile.close();
	*/
}
//===========================================================================
ChebyshevFourierCoefficients::ChebyshevFourierCoefficients() {
	name = "Chebyshev-Fourier Coefficients";
	n_features = 32;
	//cout << "Instantiating new " << name << " object." << endl;
}

WNDCHRM_ERROR ChebyshevFourierCoefficients::calculate( MatrixMap &saved_pixel_planes, 
    std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs ) const
{
	std::cout << "\t" << name << ":" << std::endl;
  coeffs.clear();
	coeffs.reserve(n_features-1);
	double temp_vec [32];
	int i;

	ImageMatrix* IN_matrix = NULL;
	WNDCHRM_ERROR retval = WC_UNINITIALIZED;
	retval = saved_pixel_planes.obtain_transform( run_algorithm_on_this_sequence, &IN_matrix );
	if( WC_NO_ERROR != retval )
		return retval;
	std::cout << "\t\tcalculating features..." << std::endl;

	for( i = 0; i < n_features; i++ ) temp_vec[i] = 0;
	//IN_matrix->dump();
	IN_matrix->ChebyshevFourierTransform2D(temp_vec);
	coeffs.assign( temp_vec, temp_vec + n_features);

	FeatureAlgorithm::dump(run_algorithm_on_this_sequence, coeffs);

	return WC_NO_ERROR;
}

WNDCHARM_REGISTER_ALGORITHM(ChebyshevFourierCoefficients)

//===========================================================================

ChebyshevCoefficients::ChebyshevCoefficients() {
	name = "Chebyshev Coefficients";
  // nibs_num - (32 is normal)
	n_features = 32;
	//cout << "Instantiating new " << name << " object." << endl;
}

/**
 * Chebyshev Coefficients are calculated by performing a Chebyshev transform,
 * and generating a histogram of pixel intensities.
 *
 */
WNDCHRM_ERROR ChebyshevCoefficients::calculate( MatrixMap &saved_pixel_planes,
    std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs ) const
{
	std::cout << "\t" << name << ":" << std::endl;
  coeffs.clear();
	coeffs.reserve(n_features-1);
	double temp_vec [32];

	ImageMatrix* IN_matrix = NULL;
	WNDCHRM_ERROR retval = WC_UNINITIALIZED;

	// Add a Chebyshev transform to the end of the transform sequence 

	/*
	FeatureNames* phonebook = FeatureNames::get_instance();
	if( NULL == phonebook ) {
		std::cerr << "Error: could not retrieve registry of transforms" << std::endl;
		return retval;
	}
	std::string cheb_name_str = "Chebyshev";

	Transform* Cheb_tform = phonebook->getTransformByName( cheb_name_str );
	if( NULL == Cheb_tform) {
		std::cerr << "Error: could not retrieve Chebyshev Transform" << std::endl;
		return WC_TRANSFORM_NOT_IN_PHONEBOOK;
	}
	std::vector<Transform*> compound_transform_list = run_algorithm_on_this_sequence;
	compound_transform_list.push_back( Cheb_tform ); 
	retval = saved_pixel_planes.obtain_transform( compound_transform_list, &IN_matrix );
	if( WC_NO_ERROR != retval )
		return retval;
	std::cout << "\t\tcalculating features..." << std::endl;
	
  IN_matrix->histogram( temp_vec, 32, 0 );
	coeffs.assign( temp_vec, temp_vec + n_features);
	*/

	retval = saved_pixel_planes.obtain_transform( run_algorithm_on_this_sequence, &IN_matrix );
	if( WC_NO_ERROR != retval )
		return retval;
	std::cout << "\t\tcalculating features..." << std::endl;

	ImageMatrix * temp = IN_matrix->duplicate();
	for( int i = 0; i < n_features; i++ ) temp_vec[i] = 0;
	//temp->dump();
	temp->ChebyshevStatistics2D(temp_vec, 0, 32);
	delete temp;
	coeffs.assign( temp_vec, temp_vec + n_features);

	FeatureAlgorithm::dump(run_algorithm_on_this_sequence, coeffs);

	return WC_NO_ERROR;
}

WNDCHARM_REGISTER_ALGORITHM(ChebyshevCoefficients)

//===========================================================================

ZernikeCoefficients::ZernikeCoefficients() {
	name = "Zernike Coefficients";
	n_features = 72;
	//cout << "Instantiating new " << name << " object." << endl;
}

WNDCHRM_ERROR ZernikeCoefficients::calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs ) const
{
	std::cout << "\t" << name << ":" << std::endl;
  coeffs.clear();
	coeffs.reserve(n_features-1);
	double temp_vec [72];
	int i;
  
	long output_size;   // output size is normally 72

	ImageMatrix* IN_matrix = NULL;
	WNDCHRM_ERROR retval = WC_UNINITIALIZED;
	retval = saved_pixel_planes.obtain_transform( run_algorithm_on_this_sequence, &IN_matrix );
	if( WC_NO_ERROR != retval )
		return retval;
	std::cout << "\t\tcalculating features..." << std::endl;

	for( i = 0; i < n_features; i++ ) temp_vec[i] = 0;
	//IN_matrix->dump();
	IN_matrix->zernike2D(temp_vec, &output_size);
	coeffs.assign( temp_vec, temp_vec + n_features);

	FeatureAlgorithm::dump(run_algorithm_on_this_sequence, coeffs);

	return WC_NO_ERROR;
}

WNDCHARM_REGISTER_ALGORITHM(ZernikeCoefficients)

//===========================================================================

HaralickTextures::HaralickTextures() {
	name = "Haralick Textures";
	n_features = 28;
	//cout << "Instantiating new " << name << " object." << endl;
}

WNDCHRM_ERROR HaralickTextures::calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs ) const
{
	std::cout << "\t" << name << ":" << std::endl;
  coeffs.clear();
	coeffs.reserve(n_features-1);
	double temp_vec [28];
	int i;
  
	ImageMatrix* IN_matrix = NULL;
	WNDCHRM_ERROR retval = WC_UNINITIALIZED;
	retval = saved_pixel_planes.obtain_transform( run_algorithm_on_this_sequence, &IN_matrix );
	if( WC_NO_ERROR != retval )
		return retval;
	std::cout << "\t\tcalculating features..." << std::endl;

	for( i = 0; i < n_features; i++ ) temp_vec[i] = 0;
	//IN_matrix->dump();
	IN_matrix->HaarlickTexture2D(0,temp_vec); // Note the misspelling
	coeffs.assign( temp_vec, temp_vec + n_features);

	FeatureAlgorithm::dump(run_algorithm_on_this_sequence, coeffs);

	return WC_NO_ERROR;
}

WNDCHARM_REGISTER_ALGORITHM(HaralickTextures)

//===========================================================================

MultiscaleHistograms::MultiscaleHistograms() {
	name = "Multiscale Histograms";
	n_features = 24;
	//cout << "Instantiating new " << name << " object." << endl;
}

WNDCHRM_ERROR MultiscaleHistograms::calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs ) const
{
	std::cout << "\t" << name << ":" << std::endl;
  coeffs.clear();
	coeffs.reserve(n_features-1);
	double temp_vec [24];
	int i;
  
	ImageMatrix* IN_matrix = NULL;
	WNDCHRM_ERROR retval = WC_UNINITIALIZED;
	retval = saved_pixel_planes.obtain_transform( run_algorithm_on_this_sequence, &IN_matrix );
	if( WC_NO_ERROR != retval )
		return retval;
	std::cout << "\t\tcalculating features..." << std::endl;

	for( i = 0; i < n_features; i++ ) temp_vec[i] = 0;
	//IN_matrix->dump();
	IN_matrix->MultiScaleHistogram(temp_vec);
	coeffs.assign( temp_vec, temp_vec + n_features);

	FeatureAlgorithm::dump(run_algorithm_on_this_sequence, coeffs);

	return WC_NO_ERROR;
}

WNDCHARM_REGISTER_ALGORITHM(MultiscaleHistograms)

//===========================================================================

TamuraTextures::TamuraTextures() {
	name = "Tamura Textures";
	n_features = 6;
	//cout << "Instantiating new " << name << " object." << endl;
}

WNDCHRM_ERROR TamuraTextures::calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs ) const
{
	std::cout << "\t" << name << ":" << std::endl;
  coeffs.clear();
	coeffs.reserve(n_features-1);
	double temp_vec [6];
	int i;
  
	ImageMatrix* IN_matrix = NULL;
	WNDCHRM_ERROR retval = WC_UNINITIALIZED;
	retval = saved_pixel_planes.obtain_transform( run_algorithm_on_this_sequence, &IN_matrix );
	if( WC_NO_ERROR != retval )
		return retval;
	std::cout << "\t\tcalculating features..." << std::endl;

	for( i = 0; i < n_features; i++ ) temp_vec[i] = 0;
	//IN_matrix->dump();
	IN_matrix->TamuraTexture2D(temp_vec);
	coeffs.assign( temp_vec, temp_vec + n_features);

	FeatureAlgorithm::dump(run_algorithm_on_this_sequence, coeffs);

	return WC_NO_ERROR;
}

WNDCHARM_REGISTER_ALGORITHM(TamuraTextures)

//===========================================================================

CombFirstFourMoments::CombFirstFourMoments() {
	name = "Comb Moments";
	n_features = 48;
	//cout << "Instantiating new " << name << " object." << endl;
}

WNDCHRM_ERROR CombFirstFourMoments::calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs ) const
{
	std::cout << "\t" << name << ":" << std::endl;
  coeffs.clear();
	coeffs.reserve(n_features-1);
	double temp_vec [48];
	int i;
  
	ImageMatrix* IN_matrix = NULL;
	WNDCHRM_ERROR retval = WC_UNINITIALIZED;
	retval = saved_pixel_planes.obtain_transform( run_algorithm_on_this_sequence, &IN_matrix );
	if( WC_NO_ERROR != retval )
		return retval;
	std::cout << "\t\tcalculating features..." << std::endl;

	for( i = 0; i < n_features; i++ ) temp_vec[i] = 0;
	//IN_matrix->dump();
	IN_matrix->CombFirstFourMoments2D(temp_vec);
	coeffs.assign( temp_vec, temp_vec + n_features);

	FeatureAlgorithm::dump(run_algorithm_on_this_sequence, coeffs);

	return WC_NO_ERROR;
}

WNDCHARM_REGISTER_ALGORITHM(CombFirstFourMoments)

//===========================================================================

RadonCoefficients::RadonCoefficients() {
	name = "Radon Coefficients";
	n_features = 12;
	//cout << "Instantiating new " << name << " object." << endl;
}

WNDCHRM_ERROR RadonCoefficients::calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs ) const
{
	std::cout << "\t" << name << ":" << std::endl;
  coeffs.clear();
	coeffs.reserve(n_features-1);
	double temp_vec [12];
	int i;

	ImageMatrix* IN_matrix = NULL;
	WNDCHRM_ERROR retval = WC_UNINITIALIZED;
	retval = saved_pixel_planes.obtain_transform( run_algorithm_on_this_sequence, &IN_matrix );
	if( WC_NO_ERROR != retval )
		return retval;
	std::cout << "\t\tcalculating features..." << std::endl;

	for( i = 0; i < n_features; i++ ) temp_vec[i] = 0;
	//IN_matrix->dump();
	IN_matrix->RadonTransform2D(temp_vec);
	coeffs.assign( temp_vec, temp_vec + n_features);

	FeatureAlgorithm::dump(run_algorithm_on_this_sequence, coeffs);

	return WC_NO_ERROR;
}

WNDCHARM_REGISTER_ALGORITHM(RadonCoefficients)

//===========================================================================
/* fractal 
   brownian fractal analysis 
   bins - the maximal order of the fractal
   output - array of the size k
   the code is based on: CM Wu, YC Chen and KS Hsieh, Texture features for classification of ultrasonic liver images, IEEE Trans Med Imag 11 (1992) (2), pp. 141Ð152.
   method of approaximation of CC Chen, JS Daponte and MD Fox, Fractal feature analysis and classification in medical imaging, IEEE Trans Med Imag 8 (1989) (2), pp. 133Ð142.
*/
FractalFeatures::FractalFeatures() {
	name = "Fractal Features";
	n_features = 20;
	//cout << "Instantiating new " << name << " object." << endl;
}

WNDCHRM_ERROR FractalFeatures::calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs ) const
{
	std::cout << "\t" << name << ":" << std::endl;
  coeffs.clear();
	coeffs.reserve(n_features-1);
  
	ImageMatrix* IN_matrix = NULL;
	WNDCHRM_ERROR retval = WC_UNINITIALIZED;
	retval = saved_pixel_planes.obtain_transform( run_algorithm_on_this_sequence, &IN_matrix );
	if( WC_NO_ERROR != retval )
		return retval;
	std::cout << "\t\tcalculating features..." << std::endl;

	//IN_matrix->dump();
	retval = calculate( IN_matrix, coeffs );

	FeatureAlgorithm::dump(run_algorithm_on_this_sequence, coeffs);

	return retval;
}


WNDCHRM_ERROR FractalFeatures::calculate( ImageMatrix* IN_matrix, vector<double> &coeffs ) const
{
	double temp_vec [20];
	int i;
  
	for( i = 0; i < n_features; i++ ) temp_vec[i] = 0;

	int bins = n_features;
	int width = IN_matrix->width;
	int height = IN_matrix->height;
	int x, y, k, bin = 0;
	int K = MIN( width, height ) / 5;
	int step = (long) floor ( K / bins );
	if( step < 1 )
		step = 1;   // avoid an infinite loop if the image is small
	for( k = 1; k < K; k = k + step )
	{  
		double sum = 0.0;
		for( x = 0; x < width; x++ )
			for( y = 0; y < height - k; y++ )
				sum += fabs( IN_matrix->pixel( x, y, 0 ).intensity - IN_matrix->pixel( x, y+k, 0 ).intensity );
		for( x = 0; x < width - k; x++ )
			for( y = 0; y < height; y++ )
				sum += fabs( IN_matrix->pixel( x, y, 0 ).intensity - IN_matrix->pixel( x + k, y, 0 ).intensity );
		if( bin < bins )
			temp_vec[ bin++ ] = sum / ( width * ( width - k ) + height * ( height - k ) );	  
	}

	coeffs.assign( temp_vec, temp_vec + n_features);



	return WC_NO_ERROR;
}



WNDCHARM_REGISTER_ALGORITHM(FractalFeatures)

//===========================================================================

PixelIntensityStatistics::PixelIntensityStatistics() {
	name = "Pixel Intensity Statistics";
	n_features = 5;
	//cout << "Instantiating new " << name << " object." << endl;
}

WNDCHRM_ERROR PixelIntensityStatistics::calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs ) const
{
	std::cout << "\t" << name << ":" << std::endl;
  coeffs.clear();
	coeffs.reserve(n_features-1);

	double temp_vec[5];
	int j;

	ImageMatrix* IN_matrix = NULL;
	WNDCHRM_ERROR retval = WC_UNINITIALIZED;
	retval = saved_pixel_planes.obtain_transform( run_algorithm_on_this_sequence, &IN_matrix );
	if( WC_NO_ERROR != retval )
		return retval;
	std::cout << "\t\tcalculating features..." << std::endl;

	for( j = 0; j < n_features; j++ ) temp_vec[j] = 0;

	//IN_matrix->dump();
	IN_matrix->BasicStatistics(&temp_vec[0], &temp_vec[1], &temp_vec[2], &temp_vec[3], &temp_vec[4], NULL, 10);

	coeffs.assign( temp_vec, temp_vec + n_features);

	FeatureAlgorithm::dump(run_algorithm_on_this_sequence, coeffs);

	return WC_NO_ERROR;
}

WNDCHARM_REGISTER_ALGORITHM(PixelIntensityStatistics)
	
//===========================================================================

EdgeFeatures::EdgeFeatures() {
	name = "Edge Features";
	n_features = 28;
	//cout << "Instantiating new " << name << " object." << endl;
}

WNDCHRM_ERROR EdgeFeatures::calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs ) const
{
	std::cout << "\t" << name << ":" << std::endl;
	coeffs.clear();
	coeffs.reserve(n_features-1);

	ImageMatrix* IN_matrix = NULL;
	WNDCHRM_ERROR retval = WC_UNINITIALIZED;
	retval = saved_pixel_planes.obtain_transform( run_algorithm_on_this_sequence, &IN_matrix );
	if( WC_NO_ERROR != retval )
		return retval;
	std::cout << "\t\tcalculating features..." << std::endl;
	//IN_matrix->dump();

	long EdgeArea = 0;
	double MagMean=0, MagMedian=0, MagVar=0, MagHist[8]={0,0,0,0,0,0,0,0}, DirecMean=0, DirecMedian=0, DirecVar=0, DirecHist[8]={0,0,0,0,0,0,0,0}, DirecHomogeneity=0, DiffDirecHist[4]={0,0,0,0};

	IN_matrix->EdgeStatistics(&EdgeArea, &MagMean, &MagMedian, &MagVar, MagHist, &DirecMean, &DirecMedian, &DirecVar, DirecHist, &DirecHomogeneity, DiffDirecHist, 8);

	int j;
	int i=0;
	
	double temp_vec[28];
	for( j = 0; j < n_features; j++ ) temp_vec[j] = 0;

	temp_vec[i] = double( EdgeArea ); i++;
	
	for( j=0; j<4; j++ ){
		temp_vec[i] = DiffDirecHist[j]; i++;
	}
	for( j=0; j<8; j++ ){
		temp_vec[i] = DirecHist[j]; i++;
	}

	temp_vec[i] = DirecHomogeneity; i++;
	temp_vec[i] = DirecMean; i++;
	temp_vec[i] = DirecMedian; i++;
	temp_vec[i] = DirecVar; i++;

	for( j=0; j<8; j++ ){
		temp_vec[i] = MagHist[j]; i++;
	}

	temp_vec[i] = MagMean; i++;
	temp_vec[i] = MagMedian; i++;
	temp_vec[i] = MagVar; i++;

	coeffs.assign( temp_vec, temp_vec + n_features);

	FeatureAlgorithm::dump(run_algorithm_on_this_sequence, coeffs);

	return WC_NO_ERROR;
}

WNDCHARM_REGISTER_ALGORITHM(EdgeFeatures)

//===========================================================================

ObjectFeatures::ObjectFeatures() {
	name = "Object Features";
	n_features = 34;
	//cout << "Instantiating new " << name << " object." << endl;
}

WNDCHRM_ERROR ObjectFeatures::calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs ) const
{
	std::cout << "\t" << name << ":" << std::endl;
  coeffs.clear();
	coeffs.reserve(n_features-1);
	
	ImageMatrix* IN_matrix = NULL;
	WNDCHRM_ERROR retval = WC_UNINITIALIZED;
	retval = saved_pixel_planes.obtain_transform( run_algorithm_on_this_sequence, &IN_matrix );
	if( WC_NO_ERROR != retval )
		return retval;
	std::cout << "\t\tcalculating features..." << std::endl;
	//IN_matrix->dump();

	int feature_count=0, Euler=0, AreaMin=0, AreaMax=0, AreaMedian=0,
			area_histogram[10]={0,0,0,0,0,0,0,0,0,0},
			dist_histogram[10]={0,0,0,0,0,0,0,0,0,0};

	double centroid_x=0, centroid_y=0, AreaMean=0, AreaVar=0, DistMin=0,
				 DistMax=0, DistMean=0, DistMedian=0, DistVar=0;

	IN_matrix->FeatureStatistics(&feature_count, &Euler, &centroid_x, &centroid_y,
	                             NULL, &AreaMin, &AreaMax, &AreaMean, &AreaMedian,
	                             &AreaVar, area_histogram, &DistMin, &DistMax,
	                             &DistMean, &DistMedian, &DistVar, dist_histogram, 10);

	double temp_vec[34];
	int j = 0;
	int i = 0;

	for( j = 0; j < n_features; j++ ) temp_vec[j] = 0;

	for( j = 0; j < 10; j++ ){
		temp_vec[i] = area_histogram[j]; i++;
	}

	temp_vec[i] = AreaMax; i++;
	temp_vec[i] = AreaMean; i++;
	temp_vec[i] = AreaMedian; i++;
	temp_vec[i] = AreaMin; i++;
	temp_vec[i] = AreaVar; i++;
	temp_vec[i] = centroid_x; i++;
	temp_vec[i] = centroid_y; i++;
	temp_vec[i] = feature_count; i++;

	for( j = 0; j < 10; j++ ) {
		temp_vec[i] = dist_histogram[j]; i++;
	}

	temp_vec[i] = DistMax; i++;
	temp_vec[i] = DistMean; i++;
	temp_vec[i] = DistMedian; i++;
	temp_vec[i] = DistMin; i++;
	temp_vec[i] = DistVar; i++;
	temp_vec[i] = Euler; i++;

	coeffs.assign( temp_vec, temp_vec + n_features);

	FeatureAlgorithm::dump(run_algorithm_on_this_sequence, coeffs);

	return WC_NO_ERROR;
}

WNDCHARM_REGISTER_ALGORITHM(ObjectFeatures)

//===========================================================================

GaborTextures::GaborTextures() {
	name = "Gabor Textures";
	n_features = 7;
	//cout << "Instantiating new " << name << " object." << endl;
}

WNDCHRM_ERROR GaborTextures::calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs ) const
{
	std::cout << "\t" << name << ":" << std::endl;
	coeffs.clear();
	coeffs.reserve(n_features-1);

	double temp_vec [7];
	int i;
  
	ImageMatrix* IN_matrix = NULL;
	WNDCHRM_ERROR retval = WC_UNINITIALIZED;
	retval = saved_pixel_planes.obtain_transform( run_algorithm_on_this_sequence, &IN_matrix );
	if( WC_NO_ERROR != retval )
		return retval;
	std::cout << "\t\tcalculating features..."<< std::endl;
	//IN_matrix->dump();

	for( i = 0; i < n_features; i++ ) temp_vec[i] = 0;
	IN_matrix->GaborFilters2D(temp_vec);
	coeffs.assign( temp_vec, temp_vec + n_features);

	FeatureAlgorithm::dump(run_algorithm_on_this_sequence, coeffs);

	return WC_NO_ERROR;
}

WNDCHARM_REGISTER_ALGORITHM(GaborTextures)

//===========================================================================

/* gini
   compute the gini coefficient
   
   paper reference: Roberto G. Abraham, Sidney van den Bergh, Preethi Nair, A NEW APPROACH TO GALAXY MORPHOLOGY. I. ANALYSIS OF THE SLOAN DIGITAL SKY
	SURVEY EARLY DATA RELEASE, The Astrophysical Journal, vol. 588, p. 218-229, 2003.
*/
GiniCoefficient::GiniCoefficient() {
	name = "Gini Coefficient";
	n_features = 1;
	//cout << "Instantiating new " << name << " object." << endl;
}

WNDCHRM_ERROR GiniCoefficient::calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs ) const
{
	std::cout << "\t" << name << ":" << std::endl;
	coeffs.clear();
	coeffs.reserve(n_features-1);

	ImageMatrix* IN_matrix = NULL;
	WNDCHRM_ERROR retval = WC_UNINITIALIZED;
	retval = saved_pixel_planes.obtain_transform( run_algorithm_on_this_sequence, &IN_matrix );
	if( WC_NO_ERROR != retval )
		return retval;
	std::cout << "\t\tcalculating features..." << std::endl;
	//IN_matrix->dump();

	double temp_vec [1];
	int j;
	for( j = 0; j < n_features; j++ ) temp_vec[j] = 0;

	long pixel_index, num_pixels;
	double *pixels, mean = 0.0, g = 0.0;
	long i, count = 0;

	num_pixels = IN_matrix->height * IN_matrix->width * IN_matrix->depth;
	pixels = new double[ num_pixels ];

	for( pixel_index = 0; pixel_index < num_pixels; pixel_index++ )
		if( IN_matrix->data[ pixel_index ].intensity > 0 )
		{
			pixels[ count ] = IN_matrix->data[ pixel_index ].intensity;
			mean += IN_matrix->data[ pixel_index ].intensity;
			count++;
		}
	if( count > 0 )
		mean = mean / count;
	qsort( pixels, count, sizeof(double), compare_doubles );

	for( i = 1; i <= count; i++)
		g += (2. * i - count - 1.) * pixels[i-1];
	delete [] pixels;

	if( count <= 1 || mean <= 0.0 )
		temp_vec[0] = 0.0;   // avoid division by zero
	else
		temp_vec[0] = g / ( mean * count * ( count-1 ) );

	coeffs.assign( temp_vec, temp_vec + n_features);

	FeatureAlgorithm::dump(run_algorithm_on_this_sequence, coeffs);

	return WC_NO_ERROR;
}

WNDCHARM_REGISTER_ALGORITHM(GiniCoefficient)



