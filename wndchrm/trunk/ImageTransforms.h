#ifndef __TRANSFORMS_H_
#define __TRANSFORMS_H_

#include <string>
#include <vector>
#include "cmatrix.h"



/*! ImageTransform
 *  defines the interface for all inheriting transform classes
 *  Turns any class that inherits this interface into a singleton
 */
class ImageMatrix; // forward declaration
class ImageTransform {
	public:
		std::string name;
		virtual void execute( const ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT ) const = 0;
		void print_info();
	protected:
		ImageTransform (const std::string &s) { name = s;}
		ImageTransform (const char *s) { name = s;}
		ImageTransform() {};
};

// The purpose of this is to hold instances of the transform algorithms below
// so that we can get references (pointers) to them later for looking them up by name using a map.
// Done in a static member function holding a static to avoid "static initialization order fiasco"
// FIXME: although this heap memory will be allocated before main() entry,
//   its probably still a good idea to make a destructor to clean it up.
class ImageTransformInstances {
	public:
		static bool initialized ();
		static bool add (const ImageTransform *algorithm);
		static std::vector<const ImageTransform *> &getInstances ();
};

class EmptyTransform : public ImageTransform {
	public:
		EmptyTransform () : ImageTransform ("Empty") {};
		EmptyTransform (const std::string &s) : ImageTransform (s) {};
		EmptyTransform (const char *s) : ImageTransform (s) {};
		virtual void execute( const ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT ) const;
};

class FourierTransform : public ImageTransform {
	public:
		FourierTransform();
		virtual void execute( const ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT ) const;
};

class ChebyshevTransform: public ImageTransform {
	public:
		ChebyshevTransform();
		virtual void execute( const ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT ) const;
};

class WaveletTransform : public ImageTransform {
	public:
		WaveletTransform();
		virtual void execute( const ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT ) const;
};

class EdgeTransform : public ImageTransform {
	public:
		EdgeTransform();
		virtual void execute( const ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT ) const;
};

class ColorTransform : public ImageTransform {
	public:
		ColorTransform();
		virtual void execute( const ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT ) const;
};

class HueTransform : public ImageTransform {
	public:
		HueTransform();
		virtual void execute( const ImageMatrix * matrix_IN, ImageMatrix * matrix_OUT ) const;
};

#endif

