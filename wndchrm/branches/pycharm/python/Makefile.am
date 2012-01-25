BUILT_SOURCES = $(srcdir)/cmatrix_wrap.cpp
SWIG_SOURCES = cmatrix.i

pycharmpython_PYTHON = pycharm.py
pycharmpyexec_LTLIBRARIES = _pycharm.la
_pycharm_la_SOURCES = $(srcdir)/swig_wrap.cpp $(SWIG_SOURCES)
_pycharm_la_CPPFLAGS = $(SWIG_PYTHON_CPPFLAGS) -I$(top_srcdir)/src
_pycharm_la_LDFLAGS = -module
_pycharm_la_LIBADD = ../src/libproject.la

$(srcdir)/swig_wrap.cpp : $(SWIG_SOURCES)
	$(SWIG) $(SWIG_PYTHON_OPT) -I$(top_srcdir)/src -o $@ $<
