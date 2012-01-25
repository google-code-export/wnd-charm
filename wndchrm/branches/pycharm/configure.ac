
# AC_INIT( [name of package], [version], [contact address for bug reports] )
AC_INIT( [pycharm], [0.01], [christopher.coletta@nih.gov] )

# AM_INIT_AUTOMAKE( [optional compiler flags] )
# just a standard macro to write out automake stuff
AM_INIT_AUTOMAKE( [-Wall -Werror foreign] )

# AC_PROG_CC - macro to look for cc
AC_PROG_CC

# AC_PROG_RANLIB - macro required if any libraries are built in the package
AC_PROG_RANLIB

# AC_PROG_CXX - macro required if there's any c++ source
AC_PROG_CXX

AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AM_PATH_PYTHON
AC_PROG_SWIG
SWIG_ENABLE_CXX
SWIG_PYTHON


AX_PKG_SWIG([major.minor.micro], [action-if-found], [action-if-not-found])


SVNREV=`( svnversion $srcdir | sed 's/:/_/' ) 2>/dev/null`
if { ! ( svnversion ) >/dev/null 2>&1 || test "$SVNREV" = "exported"; } ;
	then SVNREV=`cat $srcdir/SVNREV`
	else echo $SVNREV>$srcdir/SVNREV
fi
AC_SUBST(SVNREV)

AC_DEFINE_UNQUOTED([SVNREV],["$SVNREV"], [SVN Revision from svnversion command])
AC_DEFINE_UNQUOTED([PACKAGE_VERSION],["$VERSION-$SVNREV"], [Version string, including SVNREV])
AC_DEFINE_UNQUOTED([PACKAGE_STRING],["$PACKAGE_NAME $VERSION-$SVNREV"], [Package string with version+SVNREV])


AM_CONFIG_HEADER(config.h)

AC_CHECK_HEADER(math.h, [],
        AC_MSG_ERROR([the <math.h> header file could not be found.]))

dnl ------------------------------------------------------------
dnl check for FFTW3
dnl ------------------------------------------------------------
AC_CHECK_LIB(fftw3,fftw_execute,[AC_DEFINE(HAVE_FFTW3,1,"Turn on FFTW3 support.")],
	[AC_MSG_ERROR([
    FFTW3 library not found.
    Please download+install FFTW 3.x from http://www.fftw.org/download.html
])]
)
AC_CHECK_HEADER(fftw3.h, [],
        AC_MSG_ERROR([
    the <fftw3.h> header file from FFTW3 could not be found.
    Please download+install FFTW 3.x from http://www.fftw.org/download.html
]))


dnl ------------------------------------------------------------
dnl check for libtiff
dnl ------------------------------------------------------------
AC_CHECK_LIB(tiff, TIFFReadScanline)
AC_CHECK_HEADER(tiffio.h,LIBTIFF='yes',AC_MSG_ERROR([
    *** WND_CHARM requires libtiff; (Required TIFF headers not found) ***
    Please download+install libtiff from http://www.libtiff.org/
]))


# AC_CONFIG_HEADERS( [name of config file that you #included in your source code] ) 

# AC_CONFIG_FILES( [ newline delimited list of paths to all applicable Makefile.am's ] ) 


# AC_SUBST( VAR ) - Pass this variable to every generated Makefile.in file
# AC_DEFINE( variable, value [, description ) - define C preprocessor macro to value (verbatim) 
# AC_DEFINE_UNQUOTED - same as AC_DEFINE, but not verbatim (do variable exapnsions)

# AC_OUTPUT - the closing macro that produces all the rest of the code.
AC_OUTPUT
