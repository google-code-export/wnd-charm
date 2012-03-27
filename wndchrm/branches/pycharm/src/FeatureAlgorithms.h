#ifndef __FEATURE_ALGORITHMS_H_
#define __FEATURE_ALGORITHMS_H_

#include "wndchrm_error.h"
#include <vector>
#include <string>


namespace mfg {
class ImageMatrix;

class FeatureAlgorithm {
	public:
		std::string name;
		int n_features;
		virtual WNDCHRM_ERROR calculate( ImageMatrix * IN_matrix, std::vector<double> &coeffs ) = 0;
		void print_info() const;	
	protected:
		FeatureAlgorithm() {} ;
};



class EmptyFeatureAlgorithm : public FeatureAlgorithm {
	public:
		virtual WNDCHRM_ERROR calculate( ImageMatrix * IN_matrix, std::vector<double> &coeffs ) 	{ return WC_NOT_IMPLEMENTED; }
		EmptyFeatureAlgorithm () { FeatureAlgorithm::name = ""; FeatureAlgorithm::n_features = 1; }
		EmptyFeatureAlgorithm (std::string &s,int i) { FeatureAlgorithm::name = s; FeatureAlgorithm::n_features = i;}
		EmptyFeatureAlgorithm (const char *s,int i) { FeatureAlgorithm::name = s; FeatureAlgorithm::n_features = i;}
};

class ChebyshevFourierCoefficients : public FeatureAlgorithm {
	public:
		ChebyshevFourierCoefficients();
		virtual WNDCHRM_ERROR calculate( ImageMatrix * IN_matrix, std::vector<double> &coeffs );
};

class ChebyshevCoefficients : public FeatureAlgorithm {
	public:
		ChebyshevCoefficients();
		virtual WNDCHRM_ERROR calculate( ImageMatrix * IN_matrix, std::vector<double> &coeffs );
};

class ZernikeCoefficients : public FeatureAlgorithm {
	public:
		ZernikeCoefficients();
		virtual WNDCHRM_ERROR calculate( ImageMatrix * IN_matrix, std::vector<double> &coeffs );
};

class HaralickTextures : public FeatureAlgorithm {
	public:
		HaralickTextures();
		virtual WNDCHRM_ERROR calculate( ImageMatrix * IN_matrix, std::vector<double> &coeffs );
};

class MultiscaleHistograms : public FeatureAlgorithm {
	public:
		MultiscaleHistograms();
		virtual WNDCHRM_ERROR calculate( ImageMatrix * IN_matrix, std::vector<double> &coeffs );
};

class TamuraTextures : public FeatureAlgorithm {
	public:
		TamuraTextures();
		virtual WNDCHRM_ERROR calculate( ImageMatrix * IN_matrix, std::vector<double> &coeffs );
};

class CombFirstFourMoments : public FeatureAlgorithm {
	public:
		CombFirstFourMoments();
		virtual WNDCHRM_ERROR calculate( ImageMatrix * IN_matrix, std::vector<double> &coeffs );
};

class RadonCoefficients : public FeatureAlgorithm {
	public:
		RadonCoefficients();
		virtual WNDCHRM_ERROR calculate( ImageMatrix * IN_matrix, std::vector<double> &coeffs );
};

class FractalFeatures : public FeatureAlgorithm {
	public:
		FractalFeatures();
		virtual WNDCHRM_ERROR calculate( ImageMatrix * IN_matrix, std::vector<double> &coeffs );
};

class PixelIntensityStatistics : public FeatureAlgorithm {
	public:
		PixelIntensityStatistics();
		virtual WNDCHRM_ERROR calculate( ImageMatrix * IN_matrix, std::vector<double> &coeffs );
};

class EdgeFeatures : public FeatureAlgorithm {
	public:
		EdgeFeatures();
		virtual WNDCHRM_ERROR calculate( ImageMatrix * IN_matrix, std::vector<double> &coeffs );
};

class ObjectFeatures : public FeatureAlgorithm {
	public:
		ObjectFeatures();
		virtual WNDCHRM_ERROR calculate( ImageMatrix * IN_matrix, std::vector<double> &coeffs );
};

class GaborTextures : public FeatureAlgorithm {
	public:
		GaborTextures();
		virtual WNDCHRM_ERROR calculate( ImageMatrix * IN_matrix, std::vector<double> &coeffs );
};

class GiniCoefficient : public FeatureAlgorithm {
	public:
		GiniCoefficient();
		virtual WNDCHRM_ERROR calculate( ImageMatrix * IN_matrix, std::vector<double> &coeffs );
};

/*
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
*/
} //end namespace mfg
#endif
