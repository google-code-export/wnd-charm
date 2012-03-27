%module pymfg

%{
#include "cmatrix.h"
%}
namespace mfg
{

  typedef unsigned char byte;

  typedef struct RGBCOLOR
  {
    byte red,green,blue;
  } RGBcolor;

  typedef struct HSVCOLOR
  {
    byte hue,saturation,value;
  } HSVcolor;

  typedef union
  {
    RGBcolor RGB;
    HSVcolor HSV;
  } color;


  typedef struct PIX_DATA
  {
    color clr;
    double intensity;
  } pix_data;

  typedef struct
  {
    int x,y,w,h;
  } rect;

  int compare_doubles (const void *a, const void *b);
  class ImageMatrix
  {
    public:
      pix_data *data;                                   
      /* void CmatrixMessage(); */
      int ColorMode;                                  
      unsigned short bits;                            
      int width,height,depth;                         

      int LoadTIFF(char *filename);                   
      int SaveTiff(char *filename);                   
      int LoadPPM(char *filename, int ColorMode);     
      int OpenImage(char *image_file_name, int downsample, rect *bounding_rect, double mean, double stddev); 
      ImageMatrix();                                  
      ImageMatrix(int width,int height,int depth);    
      ImageMatrix(ImageMatrix *matrix,int x1, int y1, int x2, int y2, int z1, int z2);  
      ~ImageMatrix();                                 
      ImageMatrix *duplicate();                       
      pix_data pixel(int x,int y,int z);              
      void set(int x,int y,int z, pix_data val);             
      void SetInt(int x,int y,int z, double val);        
      void diff(ImageMatrix *matrix);                 
      void normalize(double min, double max, long range, double mean, double stddev); 
      void to8bits();
      void flip();                                    
      void invert();                                  
      void Downsample(double x_ratio, double y_ratio);
      ImageMatrix *Rotate(double angle);              
      void convolve(ImageMatrix *filter);
      void BasicStatistics(double *mean, double *median, double *std, double *min, double *max, double *histogram, int bins);
      void GetColorStatistics(double *hue_avg, double *hue_std, double *sat_avg, double *sat_std, double *val_avg, double *val_std, double *max_color, double *colors);
      void ColorTransform(double *color_hist, int use_hue);
      void histogram(double *bins,unsigned short bins_num, int imhist);
      double Otsu();                                  
      void MultiScaleHistogram(double *out);
      void EdgeTransform();                           
      double fft2();
      void ChebyshevTransform(int N);
      void ChebyshevFourierTransform2D(double *coeff);
      void Symlet5Transform();
      void GradientMagnitude(int span);
      void GradientDirection2D(int span);
      void PerwittMagnitude2D(ImageMatrix *output);
      void PerwittDirection2D(ImageMatrix *output);
      void ChebyshevStatistics2D(double *coeff, int N, int bins_num);
      int CombFirstFourMoments2D(double *vec);
      void EdgeStatistics(long *EdgeArea, double *MagMean, double *MagMedian, double *MagVar, double *MagHist, double *DirecMean, double *DirecMedian, double *DirecVar, double *DirecHist, double *DirecHomogeneity, double *DiffDirecHist, int num_bins);
      void RadonTransform2D(double *vec);
      double OtsuBinaryMaskTransform();
      int BWlabel(int level);
      void centroid(double *x_centroid, double *y_centroid, double *z_centroid);
      void FeatureStatistics(int *count, int *Euler, double *centroid_x, double *centroid_y, double *centroid_z, int *AreaMin, int *AreaMax,
          double *AreaMean, int *AreaMedian, double *AreaVar, int *area_histogram,double *DistMin, double *DistMax,
          double *DistMean, double *DistMedian, double *DistVar, int *dist_histogram, int num_bins);
      void GaborFilters2D(double *ratios);
      void HaarlickTexture2D(double distance, double *out);
      void TamuraTexture2D(double *vec);
      void zernike2D(double *zvalues, long *output_size);
  };

  HSVcolor RGB2HSV(RGBcolor rgb);
  RGBcolor HSV2RGB(HSVcolor hsv);
  TColor RGB2COLOR(RGBcolor rgb);
  double COLOR2GRAY(TColor color);
}

