%module pymfg

%{
#include "FeatureAlgorithms.h"
%}
namespace mfg
{

  class Transform {
    public:
      virtual WNDCHRM_ERROR transform( ImageMatrix * matrix_IN, ImageMatrix ** matrix_OUT_p ) = 0;
      std::string name;
      void print_info();
    protected:
      Transform() {};
  };

  class EmptyTransform : public Transform {
    public:
      EmptyTransform (std::string &s) { name = s;}
      EmptyTransform (const char *s) { name = s;}
      EmptyTransform ();
      virtual WNDCHRM_ERROR transform( ImageMatrix * matrix_IN, ImageMatrix ** matrix_OUT_p );
  };


  class FourierTransform : public Transform {
    public:
      FourierTransform();
      virtual WNDCHRM_ERROR transform( ImageMatrix * matrix_IN, ImageMatrix ** matrix_OUT_p );
  };

  class ChebyshevTransform: public Transform {
    public:
      ChebyshevTransform();
      virtual WNDCHRM_ERROR transform( ImageMatrix * matrix_IN, ImageMatrix ** matrix_OUT_p );
  };

  class WaveletTransform : public Transform {
    public:
      WaveletTransform();
      virtual WNDCHRM_ERROR transform( ImageMatrix * matrix_IN, ImageMatrix ** matrix_OUT_p );
  };

  class EdgeTransform : public Transform {
    public:
      EdgeTransform();
      virtual WNDCHRM_ERROR transform( ImageMatrix * matrix_IN, ImageMatrix ** matrix_OUT_p );
  };

  class ColorTransform : public Transform {
    public:
      ColorTransform();
      vector<double> histogram_vals;
      virtual WNDCHRM_ERROR transform( ImageMatrix * matrix_IN, ImageMatrix ** matrix_OUT_p );
  };

  class HueTransform : public Transform {
    public:
      HueTransform();
      virtual WNDCHRM_ERROR transform( ImageMatrix * matrix_IN, ImageMatrix ** matrix_OUT_p );
  };
}

