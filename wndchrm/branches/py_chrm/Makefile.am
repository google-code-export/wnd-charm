# EXTRA_DIST 
EXTRA_DIST = SVNREV
distdir = $(PACKAGE)-$(VERSION).$(SVNREV)

# Required because of the SWIG m4 script ax_pkg_swig.m4
ACLOCAL_AMFLAGS = -I m4

noinst_LTLIBRARIES = libimfit.a

libimfit_a_SOURCES = \
	gsl/specfunc.cpp \
	gsl/specfunc.h \
	wndchrm_error.cpp \
	wndchrm_error.h \
	colors/FuzzyCalc.cpp \
	colors/FuzzyCalc.h \
	statistics/CombFirst4Moments.cpp \
	statistics/CombFirst4Moments.h \
	statistics/FeatureStatistics.cpp \
	statistics/FeatureStatistics.h \
	textures/gabor.cpp \
	textures/gabor.h \
	textures/haarlick/CVIPtexture.cpp \
	textures/haarlick/CVIPtexture.h \
	textures/haarlick/haarlick.cpp \
	textures/haarlick/haarlick.h \
	textures/haarlick/mapkit.cpp \
	textures/haarlick/mapkit.h \
	textures/haarlick/mapkit_generic.cpp \
	textures/haarlick/mapkit_generic.h \
	textures/tamura.cpp \
	textures/tamura.h \
	textures/zernike/complex.cpp \
	textures/zernike/complex.h \
	textures/zernike/zernike.cpp \
	textures/zernike/zernike.h \
	transforms/ChebyshevFourier.cpp \
	transforms/ChebyshevFourier.h \
	transforms/chebyshev.cpp \
	transforms/chebyshev.h \
	transforms/fft/bcb_fftw3/fftw3.h \
	transforms/fft/fftw3.h \
	transforms/radon.cpp \
	transforms/radon.h \
	transforms/wavelet/Common.cpp \
	transforms/wavelet/Common.h \
	transforms/wavelet/convolution.cpp \
	transforms/wavelet/convolution.h \
	transforms/wavelet/DataGrid.h \
	transforms/wavelet/DataGrid2D.cpp \
	transforms/wavelet/DataGrid2D.h \
  transforms/wavelet/DataGrid3D.cpp \
	transforms/wavelet/DataGrid3D.h \
	transforms/wavelet/Filter.cpp \
	transforms/wavelet/Filter.h \
	transforms/wavelet/FilterSet.cpp \
	transforms/wavelet/FilterSet.h \
	transforms/wavelet/Symlet5.cpp \
	transforms/wavelet/Symlet5.h \
	transforms/wavelet/Wavelet.cpp \
	transforms/wavelet/Wavelet.h \
	transforms/wavelet/WaveletHigh.cpp \
	transforms/wavelet/WaveletHigh.h \
	transforms/wavelet/WaveletLow.cpp \
	transforms/wavelet/WaveletLow.h \
	transforms/wavelet/WaveletMedium.cpp \
	transforms/wavelet/WaveletMedium.h \
	transforms/wavelet/wt.cpp \
	transforms/wavelet/wt.h \
	transforms.cpp \
	transforms.h \
	FeatureAlgorithm.cpp \
	FeatureAlgorithm.h \
	Channel.cpp \
	Channel.h \
	FeatureGroup.h \
	FeatureGroup.cpp \
	cmatrix.cpp \
	cmatrix.h \

AM_CPPFLAGS = -fPIC

bin_PROGRAMS=wndchrm util/color_deconvolution

wndchrm_SOURCES = wndchrm.cpp

wndchrm_LDADD = libFeatureNames.a libimfit.a -lm -ltiff -L. -lFeatureNames -limfit -lfftw3

util_color_deconvolution_SOURCES = 	\
	util/readTiffData.c \
	util/readTIFF.h \
	util/color_deconvolution.c

util_color_deconvolution_LDADD = -lm -ltiff

