#ifndef __TRANSFORMS_H_
#define __TRANSFORMS_H_

#include <string>
#include "wndchrm_error.h"

class ImageMatrix;

/*! Transform
 *  defines the interface for all inheriting transform classes
 *  Turns any class that inherits this interface into a singleton
 */
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

	
#define WNDCHARM_REGISTER_TRANSFORM(tform_name) \
struct tform_name##TransformRegistrar \
{ \
  tform_name##TransformRegistrar() \
  { \
    FeatureNames *phonebook = FeatureNames::get_instance(); \
		tform_name *tform_instance = new tform_name; \
		int retval = phonebook->register_transform( tform_instance->name, dynamic_cast<Transform*>( tform_instance ) ); \
  } \
}; \
static tform_name##TransformRegistrar tform_name##TransformRegistrar_instance;
//std::cout << "call to register_transform " << #tform_name << " returned " << retval << std::endl; \

#endif
