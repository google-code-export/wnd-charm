#include <iostream> // used for debug output from instantiator methods
#include <cmath>
#include <fcntl.h>

#include "cmatrix.h"
#include "FeatureNames.h"
#include "ImageTransforms.h"
#include "colors/FuzzyCalc.h" // for definition of compiler constant COLORS_NUM
#include "transforms/fft/bcb_fftw3/fftw3.h"

/* global variable */
extern int verbosity;

void ImageTransform::print_info() {

}

// storage for static vector of instances
// Done in a static member function holding a static to avoid "static initialization order fiasco"
// FIXME: although this heap memory will be allocated before main() entry,
//   its probably still a good idea to make a destructor to clean it up.
bool ImageTransformInstances::initialized () {
	static std::vector<const ImageTransform *> &instances = getInstances();
	return (!instances.empty());
}
std::vector<const ImageTransform *> &ImageTransformInstances::getInstances () {
	static std::vector<const ImageTransform *> *ImageTransforms = new std::vector<const ImageTransform *>;
	return (*ImageTransforms);
}
bool ImageTransformInstances::add (const ImageTransform *algorithm) {
	static std::vector<const ImageTransform *> &instances = getInstances();

	if (verbosity > 4) std::cout << "Registering ImageTransform " << algorithm->name << std::endl;
	instances.insert (instances.end(), algorithm);
	FeatureNames::registerImageTransform (algorithm);
	return (true);
};



//===========================================================================

void EmptyTransform::execute( const ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT ) const {
	if( !( matrix_IN && matrix_OUT) )
		return;
	matrix_OUT->copy (*matrix_IN);
	if (verbosity > 3) std::cout << name << " transform." << std::endl;
}

//===========================================================================

FourierTransform::FourierTransform () : ImageTransform ("Fourier") {};

/* fft 2 dimensional transform */
// http://www.fftw.org/doc/
void FourierTransform::execute( const ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT ) const {
	if( !( matrix_IN && matrix_OUT) )
		return;
	
	matrix_OUT->copy (*matrix_IN);
	if (verbosity > 3) std::cout << "Performing transform " << name << std::endl;
	matrix_OUT->fft2();
}

// Register a static instance of the class using a namespace for the global bool
namespace ImageTransformReg {
	static const bool FourierTransformReg = ImageTransformInstances::add (new FourierTransform);
}


//===========================================================================

ChebyshevTransform::ChebyshevTransform () : ImageTransform ("Chebyshev") {};

void ChebyshevTransform::execute( const ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT ) const {
	if( !( matrix_IN && matrix_OUT) )
		return;	
	matrix_OUT->copy (*matrix_IN);
	if (verbosity > 3) std::cout << "Performing transform " << name << std::endl;

	matrix_OUT->ChebyshevTransform(0);
}

// Register a static instance of the class using a namespace for the global bool
namespace ImageTransformReg {
	static const bool ChebyshevTransformReg = ImageTransformInstances::add (new ChebyshevTransform);
}

//===========================================================================

WaveletTransform::WaveletTransform () : ImageTransform ("Wavelet") {};

void WaveletTransform::execute( const ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT ) const {
	if( !( matrix_IN && matrix_OUT) )
		return;
	
	matrix_OUT->copy (*matrix_IN);
	if (verbosity > 3) std::cout << "Performing transform " << name << std::endl;
	matrix_OUT->Symlet5Transform();
}

// Register a static instance of the class using a namespace for the global bool
namespace ImageTransformReg {
	static const bool WaveletTransformReg = ImageTransformInstances::add (new WaveletTransform);
}

//===========================================================================

EdgeTransform::EdgeTransform () : ImageTransform ("Edge") {};

void EdgeTransform::execute( const ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT ) const {
	if( !( matrix_IN && matrix_OUT) )
		return;
	
	matrix_OUT->copy (*matrix_IN);
	if (verbosity > 3) std::cout << "Performing transform " << name << std::endl;
	matrix_OUT->EdgeTransform();
}

// Register a static instance of the class using a namespace for the global bool
namespace ImageTransformReg {
	static const bool EdgeTransformReg = ImageTransformInstances::add (new EdgeTransform);
}

//===========================================================================

ColorTransform::ColorTransform () : ImageTransform ("Color") {};

void ColorTransform::execute( const ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT ) const {
	if( !( matrix_IN && matrix_OUT) )
		return;
	
	matrix_OUT->copy (*matrix_IN);
	if (verbosity > 3) std::cout << "Performing transform " << name << std::endl;
	matrix_OUT->ColorTransform();
}

// Register a static instance of the class using a namespace for the global bool
namespace ImageTransformReg {
	static const bool ColorTransformReg = ImageTransformInstances::add (new ColorTransform);
}

//===========================================================================

HueTransform::HueTransform () : ImageTransform ("Hue") {};

void HueTransform::execute( const ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT ) const {
	if( !( matrix_IN && matrix_OUT) )
		return;
	
	matrix_OUT->copy (*matrix_IN);
	if (verbosity > 3) std::cout << "Performing transform " << name << std::endl;
	matrix_OUT->HueTransform();
}

// Register a static instance of the class using a namespace for the global bool
namespace ImageTransformReg {
	static const bool HueTransformReg = ImageTransformInstances::add (new HueTransform);
}
