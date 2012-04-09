%module pymfg

%{
#include "FeatureAlgorithms.h"
%}

%include "std_vector.i"
// Instantiate templates used by example
namespace std {
   %template(IntVector) vector<int>;
   %template(DoubleVector) vector<double>;
}


namespace mfg
{
  class FeatureAlgorithm {
    public:
      std::string name;
      int n_features;
      virtual std::vector<double> calculate( ImageMatrix * IN_matrix ) = 0;
      void print_info() const;
    protected:
      FeatureAlgorithm() {} ;
  };



  class EmptyFeatureAlgorithm : public FeatureAlgorithm {
    public:
      virtual std::vector<double> calculate( ImageMatrix * IN_matrix ) { return std::vector<double>(); }
      EmptyFeatureAlgorithm () { FeatureAlgorithm::name = ""; FeatureAlgorithm::n_features = 1; }
      EmptyFeatureAlgorithm (std::string &s,int i) { FeatureAlgorithm::name = s; FeatureAlgorithm::n_features = i;}
      EmptyFeatureAlgorithm (const char *s,int i) { FeatureAlgorithm::name = s; FeatureAlgorithm::n_features = i;}
  };

  class ChebyshevFourierCoefficients : public FeatureAlgorithm {
    public:
      ChebyshevFourierCoefficients();
      virtual std::vector<double> calculate( ImageMatrix * IN_matrix );
  };

  class ChebyshevCoefficients : public FeatureAlgorithm {
    public:
      ChebyshevCoefficients();
      virtual std::vector<double> calculate( ImageMatrix * IN_matrix );
  };

  class ZernikeCoefficients : public FeatureAlgorithm {
    public:
      ZernikeCoefficients();
      virtual std::vector<double> calculate( ImageMatrix * IN_matrix );
  };

  class HaralickTextures : public FeatureAlgorithm {
    public:
      HaralickTextures();
      virtual std::vector<double> calculate( ImageMatrix * IN_matrix );
  };

  class MultiscaleHistograms : public FeatureAlgorithm {
    public:
      MultiscaleHistograms();
      virtual std::vector<double> calculate( ImageMatrix * IN_matrix );
  };

  class TamuraTextures : public FeatureAlgorithm {
    public:
      TamuraTextures();
      virtual std::vector<double> calculate( ImageMatrix * IN_matrix );
  };

  class CombFirstFourMoments : public FeatureAlgorithm {
    public:
      CombFirstFourMoments();
      virtual std::vector<double> calculate( ImageMatrix * IN_matrix );
  };

  class RadonCoefficients : public FeatureAlgorithm {
    public:
      RadonCoefficients();
      virtual std::vector<double> calculate( ImageMatrix * IN_matrix );
  };

  class FractalFeatures : public FeatureAlgorithm {
    public:
      FractalFeatures();
      virtual std::vector<double> calculate( ImageMatrix * IN_matrix );
  };

  class PixelIntensityStatistics : public FeatureAlgorithm {
    public:
      PixelIntensityStatistics();
      virtual std::vector<double> calculate( ImageMatrix * IN_matrix );
  };

  class EdgeFeatures : public FeatureAlgorithm {
    public:
      EdgeFeatures();
      virtual std::vector<double> calculate( ImageMatrix * IN_matrix );
  };

  class ObjectFeatures : public FeatureAlgorithm {
    public:
      ObjectFeatures();
      virtual std::vector<double> calculate( ImageMatrix * IN_matrix );
  };

  class GaborTextures : public FeatureAlgorithm {
    public:
      GaborTextures();
      virtual std::vector<double> calculate( ImageMatrix * IN_matrix );
  };

  class GiniCoefficient : public FeatureAlgorithm {
    public:
      GiniCoefficient();
      virtual std::vector<double> calculate( ImageMatrix * IN_matrix );
  };
}

