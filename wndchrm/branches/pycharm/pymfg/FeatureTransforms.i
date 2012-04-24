%module pymfg

%{
#include "FeatureTransforms.h"
%}

%include "std_string.i"


namespace mfg
{

  %nodefaultctor Transform;
  class Transform {
    public:
      virtual ImageMatrix* transform( ImageMatrix * matrix_IN ) = 0;
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
      virtual ImageMatrix* transform( ImageMatrix * matrix_IN );
  };


  class FourierTransform : public Transform {
    public:
      FourierTransform();
      virtual ImageMatrix* transform( ImageMatrix * matrix_IN );
  };

  class ChebyshevTransform: public Transform {
    public:
      ChebyshevTransform();
      virtual ImageMatrix* transform( ImageMatrix * matrix_IN );
  };

  class WaveletTransform : public Transform {
    public:
      WaveletTransform();
      virtual ImageMatrix* transform( ImageMatrix * matrix_IN );
  };

  class EdgeTransform : public Transform {
    public:
      EdgeTransform();
      virtual ImageMatrix* transform( ImageMatrix * matrix_IN );
  };

  class ColorTransform : public Transform {
    public:
      ColorTransform();
      vector<double> histogram_vals;
      virtual ImageMatrix* transform( ImageMatrix * matrix_IN );
  };

  class HueTransform : public Transform {
    public:
      HueTransform();
      virtual ImageMatrix* transform( ImageMatrix * matrix_IN );
  };
}

