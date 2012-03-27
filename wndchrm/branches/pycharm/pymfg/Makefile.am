
BUILT_SOURCES = $(srcdir)/swig_wrap.cpp
SWIG_SOURCES = swig.i cmatrix.i wndchrm_error.i FeatureAlgorithms.i FeatureTransforms.i

pkgpython_PYTHON = pymfg.py __init__.py
pkgpyexec_LTLIBRARIES = _pymfg.la
_pymfg_la_SOURCES = $(srcdir)/swig_wrap.cpp $(SWIG_SOURCES)
_pymfg_la_CPPFLAGS = $(SWIG_PYTHON_CPPFLAGS) -I$(top_srcdir)/src
_pymfg_la_LDFLAGS = -module -bundle 
_pymfg_la_LIBADD = ../src/libmfgraph.la

$(srcdir)/swig_wrap.cpp : $(SWIG_SOURCES)
	 $(SWIG) $(SWIG_PYTHON_OPT) -I$(top_srcdir)/src -o $@ $<


