/* File : cmatrix.i */

/* Use c-style comments, because these lines get copied into the generated
    c code */

/* module name specified by %module directive */
%module ImageMatrix
%{
/* The lines inside between the %{ }% aren't parsed by SWIG, but are copied verbatim
   into the wrapper c-code */
/* Put headers and other declarations here */   
#include "cmatrix.h"
%}
%include "cmatrix.h"

/* ANSI C/C++ declarations go here */

extern class ImageMatrix;
