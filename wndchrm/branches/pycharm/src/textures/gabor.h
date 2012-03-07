//---------------------------------------------------------------------------

#ifndef gaborH
#define gaborH
//---------------------------------------------------------------------------
using namespace mfg;

void GaborTextureFilters2D(ImageMatrix *Im, double *ratios);

void conv2(double *c, double *a, double *b, int ma, int na, int mb, int nb, int plusminus);

#endif
