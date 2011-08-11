#ifndef __TRANSFORMS_H_
#define __TRANSFORMS_H_

#include <string>

//using namespace std;

class ImageMatrix;

/*! Transform
 *  defines the interface for all inheriting transform classes
 *  Turns any class that inherits this interface into a singleton
 */
class Transform {
	public:
		virtual int transform( ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT ) = 0;
		std::string name;
		void print_info();
	protected:
		Transform() {};
};

class EmptyTransform : public Transform {
	public:
		EmptyTransform (std::string &s) { name = s;}
		EmptyTransform (const char *s) { name = s;}
		virtual int    transform( ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT );
};


class FourierTransform : public Transform {
	public:
		FourierTransform();
		virtual int    transform( ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT );
};

class ChebyshevTransform: public Transform {
	public:
		ChebyshevTransform();
		virtual int    transform( ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT );
};

class WaveletTransform : public Transform {
	public:
		WaveletTransform();
		virtual int    transform( ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT );
};

class EdgeTransform : public Transform {
	public:
		EdgeTransform();
		virtual int    transform( ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT );
};

class ColorTransform : public Transform {
	public:
		ColorTransform();
		vector<double> histogram_vals;
		virtual int    transform( ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT );
};

class HueTransform : public Transform {
	public:
		HueTransform();
		virtual int    transform( ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT );
};

	
#define WNDCHARM_REGISTER_TRANSFORM(tform_name) \
struct tform_name##TransformRegistrar \
{ \
  tform_name##TransformRegistrar() \
  { \
    FeatureNames *phonebook = FeatureNames::get_instance(); \
		tform_name *tform_instance = new tform_name; \
    phonebook->register_transform( tform_instance->name, dynamic_cast<Transform*>( tform_instance ) ); \
  } \
}; \
static tform_name##TransformRegistrar tform_name##TransformRegistrar_instance;

#endif
