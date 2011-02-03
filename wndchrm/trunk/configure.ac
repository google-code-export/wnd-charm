AC_INIT(wndchrm,1.27)
AM_INIT_AUTOMAKE
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

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



AC_OUTPUT(Makefile)
