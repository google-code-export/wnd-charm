#ifndef __FEATURE_ALGORITHM_H_
#define __FEATURE_ALGORITHM_H_

#include <vector>
#include <string>
//#include "FeatureNames.hpp"
#include "MatrixMap.h"
using namespace std;

class ImageMatrix;

// The following is the old implementation which used to live in FeatureNames.hpp
#if 0
//=====================================================================
/*!
 * Feature Algorithms
 * This should be a feature algorithm object with an execute() method
 */
class FeatureAlgorithm {
	public:
		std::string name;
		int n_features;
 
		FeatureAlgorithm () : name(""), n_features(1) { }
		FeatureAlgorithm (std::string &s,int i) { name = s; n_features = i;}
		FeatureAlgorithm (const char *s,int i) { name = s; n_features = i;}
		void print_info() const;		
};

#endif

class FeatureAlgorithm {
	public:
		string name;
		int n_features;
		virtual int calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs ) = 0;
		void print_info() const;	
	protected:
		FeatureAlgorithm() {} ;
};



class EmptyFeatureAlgorithm : public FeatureAlgorithm {
	public:
		virtual int calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs ) 	{ return -1; }
		EmptyFeatureAlgorithm () { FeatureAlgorithm::name = ""; FeatureAlgorithm::n_features = 1; }
		EmptyFeatureAlgorithm (std::string &s,int i) { FeatureAlgorithm::name = s; FeatureAlgorithm::n_features = i;}
		EmptyFeatureAlgorithm (const char *s,int i) { FeatureAlgorithm::name = s; FeatureAlgorithm::n_features = i;}
};

class ChebyshevFourierCoefficients : public FeatureAlgorithm {
	public:
		ChebyshevFourierCoefficients();
		virtual int calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs );
};

class ChebyshevCoefficients : public FeatureAlgorithm {
	public:
		ChebyshevCoefficients();
		virtual int calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs );
};

class ZernikeCoefficients : public FeatureAlgorithm {
	public:
		ZernikeCoefficients();
		virtual int calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs );
};

class HaralickTextures : public FeatureAlgorithm {
	public:
		HaralickTextures();
		virtual int calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs );
};

class MultiscaleHistograms : public FeatureAlgorithm {
	public:
		MultiscaleHistograms();
		virtual int calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs );
};

class TamuraTextures : public FeatureAlgorithm {
	public:
		TamuraTextures();
		virtual int calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs );
};

class CombFirstFourMoments : public FeatureAlgorithm {
	public:
		CombFirstFourMoments();
		virtual int calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs );
};

class RadonCoefficients : public FeatureAlgorithm {
	public:
		RadonCoefficients();
		virtual int calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs );
};

class FractalFeatures : public FeatureAlgorithm {
	public:
		FractalFeatures();
		virtual int calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs );
};

class PixelIntensityStatistics : public FeatureAlgorithm {
	public:
		PixelIntensityStatistics();
		virtual int calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs );
};

class EdgeFeatures : public FeatureAlgorithm {
	public:
		EdgeFeatures();
		virtual int calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs );
};

class ObjectFeatures : public FeatureAlgorithm {
	public:
		ObjectFeatures();
		virtual int calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs );
};

class GaborTextures : public FeatureAlgorithm {
	public:
		GaborTextures();
		virtual int calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs );
};

class GiniCoefficient : public FeatureAlgorithm {
	public:
		GiniCoefficient();
		virtual int calculate( MatrixMap &saved_pixel_planes, std::vector<Transform*> &run_algorithm_on_this_sequence, vector<double> &coeffs );
};

#define WNDCHARM_REGISTER_ALGORITHM(alg_name) \
struct alg_name##AlgorithmRegistrar \
{ \
  alg_name##AlgorithmRegistrar() \
  { \
    FeatureNames *phonebook = FeatureNames::get_instance(); \
		alg_name *algorithm_instance = new alg_name; \
    phonebook->register_algorithm( algorithm_instance->name, dynamic_cast<FeatureAlgorithm*>( algorithm_instance ) ); \
  } \
}; \
static alg_name##AlgorithmRegistrar alg_name##AlgorithmRegistrar_instance;

#endif
