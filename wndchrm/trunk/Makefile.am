EXTRA_DIST = SVNREV
distdir = $(PACKAGE)-$(VERSION).$(SVNREV)

noinst_LIBRARIES = libimfit.a libFeatureNames.a

# This is set up as a separate target because g++ barfs compiling this with optimization.
# gcc 4.2.1 tries to allocate several gigs of RAM, and fails with an out-of-memory error
# Apparently fixed in 4.3 or 4.4, but have not checked.
libFeatureNames_a_SOURCES = \
	FeatureNames.cpp \
	FeatureNames.hpp

# Our special optimization flags, which over-ride AM_CXXFLAGS.
# Note that the user's CXXFLAGS always go at the end, which
# get set to "-g -O2" if they are not set, which blows up.
# I thought they were strictly for the user, so why are they set to a default?
# configure.ac has some checks to not stomp on the user's CXXFLAGS, but also
# not use the defaults provided by AC_PROG_CXX (which is where -g -O2 comes from).
# The default options -g -O2 are set in AM_CXXFLAGS, which is as it should be.
# The result is that it only blows up if the user has -O set in their CXXFLAGS.
libFeatureNames_a_CXXFLAGS = -g -O0

libimfit_a_SOURCES = \
	cmatrix.cpp \
	cmatrix.h \
	colors/FuzzyCalc.cpp \
	colors/FuzzyCalc.h \
	signatures.cpp \
	signatures.h \
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
	TrainingSet.cpp \
	TrainingSet.h \
	transforms/ChebishevFourier.cpp \
	transforms/ChebishevFourier.h \
	transforms/chevishev.cpp \
	transforms/chevishev.h \
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
	transforms/wavelet/wt.h

AM_CPPFLAGS = -fPIC

bin_PROGRAMS=wndchrm util/color_deconvolution

wndchrm_SOURCES = wndchrm.cpp

wndchrm_LDADD = libimfit.a libFeatureNames.a -lm -ltiff -L. -limfit -lfftw3

util_color_deconvolution_SOURCES = 	\
	util/readTiffData.c \
	util/readTIFF.h \
	util/color_deconvolution.c

util_color_deconvolution_LDADD = -lm -ltiff

