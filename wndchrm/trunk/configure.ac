AC_INIT(wndchrm,1.27)
AM_INIT_AUTOMAKE(wndchrm,1.27)
AM_CONFIG_HEADER(config.h)
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

AC_CHECK_HEADER(math.h, [],
        AC_MSG_ERROR([the <math.h> header file could not be found.]))

dnl ------------------------------------------------------------
dnl check for FFTW3
dnl ------------------------------------------------------------
AC_CHECK_LIB(fftw3,fftw_execute,[AC_DEFINE(HAVE_FFTW3,1,"Turn on FFTW3 support.")],
	[AC_MSG_ERROR([FFTW3 library not found])]
)
AC_CHECK_HEADER(fftw3.h, [],
        AC_MSG_ERROR([the <fftw3.h> header file from FFTW3 could not be found.]))


dnl ------------------------------------------------------------
dnl check for libtiff
dnl ------------------------------------------------------------
AC_CHECK_LIB(tiff, TIFFReadScanline)
AC_CHECK_HEADER(tiffio.h,LIBTIFF='yes',AC_MSG_ERROR([
*** WND_CHARM requires libtiff; (Required TIFF headers not found) ***]))



AC_OUTPUT(Makefile)
