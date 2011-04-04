/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                               */
/*    Copyright (C) 2007 Open Microscopy Environment                             */
/*         Massachusetts Institue of Technology,                                 */
/*         National Institutes of Health,                                        */
/*         University of Dundee                                                  */
/*                                                                               */
/*                                                                               */
/*                                                                               */
/*    This library is free software; you can redistribute it and/or              */
/*    modify it under the terms of the GNU Lesser General Public                 */
/*    License as published by the Free Software Foundation; either               */
/*    version 2.1 of the License, or (at your option) any later version.         */
/*                                                                               */
/*    This library is distributed in the hope that it will be useful,            */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of             */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          */
/*    Lesser General Public License for more details.                            */
/*                                                                               */
/*    You should have received a copy of the GNU Lesser General Public           */
/*    License along with this library; if not, write to the Free Software        */
/*    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  */
/*                                                                               */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*                                                                               */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Written by:  Lior Shamir <shamirl [at] mail [dot] nih [dot] gov>              */
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#ifdef WIN32
#pragma hdrstop
#endif

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h> // for locking stuff
#include <errno.h>
#include <time.h>
#include <unistd.h> // apparently, for close() only?
#include <math.h>
#include <cfloat> // Has definition of DBL_EPSILON, FLT_EPSILON
#define OUR_EPSILON FLT_EPSILON*6
#define FLOAT_EQ(x,v) (((v - FLT_EPSILON) < x) && (x <( v + FLT_EPSILON)))
#define OUR_EQ(x,v) (((v - OUR_EPSILON) < x) && (x <( v + OUR_EPSILON)))
#include "signatures.h"
#include "cmatrix.h"
#include "TrainingSet.h"
#include "colors/FuzzyCalc.h"

#ifndef WIN32
#include <stdlib.h>
#endif


/* global variable */
extern int verbosity;


//---------------------------------------------------------------------------
/*  signatures (constructor)
*/
signatures::signatures()
{  int sig_index;
   for (sig_index=0;sig_index<MAX_SIGNATURE_NUM;sig_index++)
   {  //data[sig_index].name[0]='\0';
      data[sig_index].value=0;
   }
   count=0;
   sample_class=0;
   full_path[0]='\0';
   sample_name[0]='\0';
   NamesTrainingSet=NULL;   
   ScoresTrainingSet=NULL;
}
//---------------------------------------------------------------------------

/* duplicate
*/
signatures *signatures::duplicate()
{  int sig_index;
   signatures *new_samp;
   new_samp=new signatures();
   new_samp->sample_class=sample_class;
   new_samp->sample_value=sample_value;   
   new_samp->interpolated_value=interpolated_value;
   new_samp->count=count;
   new_samp->NamesTrainingSet=NamesTrainingSet;   
   new_samp->ScoresTrainingSet=ScoresTrainingSet;
   strcpy(new_samp->full_path,full_path);
   for (sig_index=0;sig_index<count;sig_index++)
     new_samp->data[sig_index]=data[sig_index];
//   {  new_samp->data[sig_index].value=data[sig_index].value;
//      strcpy(new_samp->data[sig_index].name,data[sig_index].name);
//   }
   return(new_samp);
}

/* Add
   add a signature
   name -char *- the name of the signature (e.g. Multiscale Histogram bin 3)
   value -double- the value to add
*/
void signatures::Add(const char *name,double value)
{
   if (name && NamesTrainingSet) strcpy(((TrainingSet *)(NamesTrainingSet))->SignatureNames[count],name);

   if (value>INF) value=INF;        /* prevent error */
   if (value<-INF) value=-INF;      /* prevent error */
   if (value<1/INF && value>-1/INF) value=0;  /* prevent a numerical error */
   data[count].value=value;
   count++;
}

/* Clear
   clear all signature values
*/
void signatures::Clear()
{  count=0;
}

int signatures::IsNeeded(long start_index, long group_length)
{  int sig_index;
   if (!ScoresTrainingSet) return(1);
   for (sig_index=start_index;sig_index<start_index+group_length;sig_index++)
     if (((TrainingSet *)(ScoresTrainingSet))->SignatureWeights[sig_index]>0) return(1);
   return(0);
}

/* compute
   compute signature set of a given image and add the
   resulting values to the "data" attribute of this class
   input - 
   matrix -ImageMatrix*- an image matrix structure.
   compute_colors -int- 1 to compute color signatures, 0 to ignore color sugnatures   
*/

void signatures::compute(ImageMatrix *matrix, int compute_colors)
{  char buffer[80];
   double vec[72];
   int a,b,c;
   double mean, median, std, min, max, histogram[10];
   ImageMatrix *TempMatrix;
   ImageMatrix *FourierTransform,*ChebyshevTransform,*ChebyshevFourierTransform,*WaveletSelector,*WaveletFourierSelector;
   if (verbosity>=2) printf("start processing image...\n");   
   FourierTransform=matrix->duplicate();
   FourierTransform->fft2();
   ChebyshevTransform=matrix->duplicate();
   ChebyshevTransform->ChebyshevTransform(0);
   ChebyshevFourierTransform=FourierTransform->duplicate();
   ChebyshevFourierTransform->ChebyshevTransform(0);
   WaveletSelector=matrix->duplicate();
   WaveletSelector->Symlet5Transform();
   WaveletFourierSelector=FourierTransform->duplicate();
   WaveletFourierSelector->Symlet5Transform();
#ifndef WIN32
   printf("start computing features\n");
#endif
   count=0;      /* start counting signatures from 0 */
   /* chebichev fourier transform (signatures 0 - 63) */
   for (a=0;a<32;a++) vec[a]=0;
   matrix->ChebyshevFourierTransform2D(vec);
   for (a=0;a<32;a++)
   {  sprintf(buffer,"ChebyshevFourierCoefficientHistogram Bin%02d",a);
      Add(buffer,vec[a]);
   }
   FourierTransform->ChebyshevFourierTransform2D(vec);
   for (a=0;a<32;a++)
   {  sprintf(buffer,"ChebyshevFourierCoefficientHistogram_FFT Bin%02d",a);
      Add(buffer,vec[a]);
   }
   /* Chebyshev Statistics (signatures 64 - 127) */
   TempMatrix=matrix->duplicate();
   TempMatrix->ChebyshevStatistics2D(vec,0,32);
   for (a=0;a<32;a++)
   {  sprintf(buffer,"ChebyshevCoefficientHistogram Bin%02d",a);
      Add(buffer,vec[a]);
   }
   delete TempMatrix;
   TempMatrix=FourierTransform->duplicate();
   TempMatrix->ChebyshevStatistics2D(vec,0,32);
   for (a=0;a<32;a++)
   {  sprintf(buffer,"ChebyshevCoefficientHistogram_FFT Bin%02d",a);
      Add(buffer,vec[a]);
   }
   delete TempMatrix;

   /* Comb4Moments (signatures 128 - 415) */
   char four_moments_names[80][80]={"Minus45_Mean_HistBin00","Minus45_Mean_HistBin01","Minus45_Mean_HistBin02","Minus45_Std_HistBin00","Minus45_Std_HistBin01","Minus45_Std_HistBin02",
           "Minus45_Skew_HistBin00","Minus45_Skew_HistBin01","Minus45_Skew_HistBin02","Minus45_Kurt_HistBin00","Minus45_Kurt_HistBin01","Minus45_Kurt_HistBin02",
		   "Plus45_Mean_HistBin00","Plus45_Mean_HistBin01","Plus45_Mean_HistBin02","Plus45_Std_HistBin00","Plus45_Std_HistBin01","Plus45_Std_HistBin02",
		   "Plus45_Skew_HistBin00","Plus45_Skew_HistBin01","Plus45_Skew_HistBin02","Plus45_Kurt_HistBin00","Plus45_Kurt_HistBin01","Plus45_Kurt_HistBin02","90_Mean_HistBin00",
		   "90_Mean_HistBin01","90_Mean_HistBin02","90_Std_HistBin00","90_Std_HistBin01","90_Std_HistBin02","90_Skew_HistBin00","90_Skew_HistBin01","90_Skew_HistBin02",
		   "90_Kurt_HistBin00","90_Kurt_HistBin01","90_Kurt_HistBin02","0_Mean_HistBin00","0_Mean_HistBin01","0_Mean_HistBin02","0_Std_HistBin00","0_Std_HistBin01","0_Std_HistBin02",
		   "0_Skew_HistBin00","0_Skew_HistBin01","0_Skew_HistBin02","0_Kurt_HistBin00","0_Kurt_HistBin01","0_Kurt_HistBin02"};

   matrix->CombFirstFourMoments2D(vec);
   for (a=0;a<48;a++)
   {  sprintf(buffer,"Comb4Orient4MomentsHistogram %s",four_moments_names[a]);
      Add(buffer,vec[a]);
   }
   ChebyshevTransform->CombFirstFourMoments2D(vec);
   for (a=0;a<48;a++)
   {  sprintf(buffer,"Comb4Orient4MomentsHistogram_Chebyshev %s",four_moments_names[a]);
      Add(buffer,vec[a]);
   }
   ChebyshevFourierTransform->CombFirstFourMoments2D(vec);
   for (a=0;a<48;a++)
   {  sprintf(buffer,"Comb4Orient4MomentsHistogram_ChebyshevFFT %s",four_moments_names[a]);
      Add(buffer,vec[a]);
   }
   FourierTransform->CombFirstFourMoments2D(vec);  
   for (a=0;a<48;a++)
   {  sprintf(buffer,"Comb4Orient4MomentsHistogram_FFT %s",four_moments_names[a]);
      Add(buffer,vec[a]);
   }  
   WaveletSelector->CombFirstFourMoments2D(vec);
   for (a=0;a<48;a++)
   {  sprintf(buffer,"Comb4Orient4MomentsHistogram_Wavelet %s",four_moments_names[a]);
      Add(buffer,vec[a]);
   }       
   WaveletFourierSelector->CombFirstFourMoments2D(vec);
   for (a=0;a<48;a++)
   {  sprintf(buffer,"Comb4Orient4MomentsHistogram_WaveletFFT %s",four_moments_names[a]);
      Add(buffer,vec[a]);
   }

   /* edge statistics (signatures 416 - 443) */
   {  long EdgeArea;
      double MagMean, MagMedian, MagVar, MagHist[8], DirecMean, DirecMedian, DirecVar, DirecHist[8], DirecHomogeneity, DiffDirecHist[4];
      matrix->EdgeStatistics(&EdgeArea, &MagMean, &MagMedian, &MagVar, MagHist, &DirecMean, &DirecMedian, &DirecVar, DirecHist, &DirecHomogeneity, DiffDirecHist, 8);
      Add("EdgeArea EdgeArea",EdgeArea);
      for (a=0;a<4;a++)
      {  sprintf(buffer,"EdgeDiffDirecHist Bin%d",a);
         Add(buffer,DiffDirecHist[a]);
      }
      for (a=0;a<8;a++)
      {  sprintf(buffer,"EdgeDirecHist Bin%d",a);
         Add(buffer,DirecHist[a]);
      }
      Add("EdgeDirecHomo DirecHomo",DirecHomogeneity);
      Add("EdgeDirecMean DirecMean",DirecMean);
      Add("EdgeDirecMedian DirecMedian",DirecMedian);
      Add("EdgeDirecVar DirecVar",DirecVar);
      for (a=0;a<8;a++)
      {  sprintf(buffer,"EdgeMagHist Bin%d",a);
         Add(buffer,MagHist[a]);
      }
      Add("EdgeMagMean MagMean",MagMean);
      Add("EdgeMagMedian MagMedian",MagMedian);
      Add("EdgeMagVar MagVar",MagVar);
   }
 
   /*feature statistics (signatures 444 - 477) */
   {  int count, Euler, AreaMin, AreaMax, AreaMedian, area_histogram[10], dist_histogram[10];
      double centroid_x, centroid_y, AreaMean, AreaVar, DistMin, DistMax, DistMean, DistMedian, DistVar;

      matrix->FeatureStatistics(&count, &Euler, &centroid_x, &centroid_y, NULL, &AreaMin, &AreaMax, &AreaMean, &AreaMedian, &AreaVar, area_histogram, &DistMin, &DistMax,
                        &DistMean, &DistMedian, &DistVar, dist_histogram, 10);

      for (a=0;a<10;a++)  /* area histogram */
      {  sprintf(buffer,"FeatureAreaHist Bin%d",a);
         Add(buffer,area_histogram[a]);
      }
      Add("FeatureAreaMax FeatureAreaMax",AreaMax);
      Add("FeatureAreaMean FeatureAreaMean",AreaMean);
      Add("FeatureAreaMedian FeatureAreaMedian",AreaMedian);
      Add("FeatureAreaMin FeatureAreaMin",AreaMin);
      Add("FeatureAreaVar FeatureAreaVar",AreaVar);
      Add("FeatureCentroid X",centroid_x);
      Add("FeatureCentroid Y",centroid_y);
      Add("FeatureCount FeatureCount",count);
      for (a=0;a<10;a++)
      {  sprintf(buffer,"FeatureDistHist Bin%d",a);
         Add(buffer,dist_histogram[a]);
      }
      Add("FeatureDistMax FeatureDistMax",DistMax);
      Add("FeatureDistMean FeatureDistMean",DistMean);
      Add("FeatureDistMedian FeatureDistMedian",DistMedian);
      Add("FeatureDistMin FeatureDistMin",DistMin);
      Add("FeatureDistVar FeatureDistVar",DistVar);
      Add("FeatureEuler FeatureEuler",Euler);
   }  

   /* gabor filters (signatures 478 - 484) */
   matrix->GaborFilters2D(vec);
   for (a=0;a<7;a++)
   {  sprintf(buffer,"GaborTextures Gabor%02d",a+1);
      Add(buffer,vec[a]);
   }

   /* haarlick textures (signatures 485 - 652) */
   char haarlick_names[80][80]={"CoOcMat_AngularSecondMoment","ASM","CoOcMat_Contrast","Contrast","CoOcMat_Correlation","Correlation","CoOcMat_Variance","Variance","CoOcMat_InverseDifferenceMoment","IDM","CoOcMat_SumAverage" ,"SumAvg",
           "CoOcMat_SumVariance","SumVar","CoOcMat_SumEntropy", "SumEntropy","CoOcMat_Entropy" ,"Entropy","CoOcMat_DifferenceEntropy","DiffEntropy","CoOcMat_DifferenceVariance","DiffVar","CoOcMat_FirstMeasureOfCorrelation","MeasCorr1",
		   "CoOcMat_SecondMeasureOfCorrelation","MeasCorr2","CoOcMat_MaximalCorrelationCoefficient" ,"MaxCorrCoef","CoOcMat_AngularSecondMomentDif", "ASM","CoOcMat_ContrastDif" ,"Contrast","CoOcMat_CorrelationDif","Correlation","CoOcMat_VarianceDif","Variance",
		   "CoOcMat_InverseDifferenceMomentDif","IDM","CoOcMat_SumAverageDif","SumAvg","CoOcMat_SumVarianceDif","SumVar","CoOcMat_SumEntropyDif" ,"SumEntropy","CoOcMat_EntropyDif","Entropy","CoOcMat_DifferenceEntropyDif","DiffEntropy","CoOcMat_DifferenceVarianceDif","DiffVar",
		   "CoOcMat_FirstMeasureOfCorrelationDif","MeasCorr1","CoOcMat_SecondMeasureOfCorrelationDif","MeasCorr2","CoOcMat_MaximalCorrelationCoefficientDif","MaxCorrCoef"};

   matrix->HaarlickTexture2D(0,vec);
   for (a=0;a<28;a++)
   {  sprintf(buffer,"%s %s",haarlick_names[a*2],haarlick_names[a*2+1]);
      Add(buffer,vec[a]);
   }
   ChebyshevTransform->HaarlickTexture2D(0,vec);
   for (a=0;a<28;a++)
   {  sprintf(buffer,"%s_Chebyshev %s",haarlick_names[a*2],haarlick_names[a*2+1]);
      Add(buffer,vec[a]);
   }
   ChebyshevFourierTransform->HaarlickTexture2D(0,vec);
   for (a=0;a<28;a++)
   {  sprintf(buffer,"%s_ChebyshevFFT %s",haarlick_names[a*2],haarlick_names[a*2+1]);
      Add(buffer,vec[a]);
   }
   FourierTransform->HaarlickTexture2D(0,vec);
   for (a=0;a<28;a++)
   {  sprintf(buffer,"%s_FFT %s",haarlick_names[a*2],haarlick_names[a*2+1]);
      Add(buffer,vec[a]);
   }
   WaveletSelector->HaarlickTexture2D(0,vec);
   for (a=0;a<28;a++)
   {  sprintf(buffer,"%s_Wavelet %s",haarlick_names[a*2],haarlick_names[a*2+1]);
      Add(buffer,vec[a]);
   }
   WaveletFourierSelector->HaarlickTexture2D(0,vec);
   for (a=0;a<28;a++)
   {  sprintf(buffer,"%s_WaveletFFT %s",haarlick_names[a*2],haarlick_names[a*2+1]);
      Add(buffer,vec[a]);
   }
   /* multiple histogram (signatures 653 - 796) */
   /* ***************************************** */
   /* multiple histogram of the original image (signatures 653 - 676) */
   matrix->MultiScaleHistogram(vec);
   b=3;c=3;
   for (a=0;a<24;a++)
   {  if (a==b) b+=(c=c+2);
      sprintf(buffer,"MultipleScaleHistograms TBins%d_Bin%02d",c,a+c-b);
      Add(buffer,vec[a]);
   }
   /* multiple histogram of the chebyshev transform (signatures 677 - 700) */
   ChebyshevTransform->MultiScaleHistogram(vec);
   b=3;c=3;
   for (a=0;a<24;a++)
   {  if (a==b) b+=(c=c+2);
      sprintf(buffer,"MultipleScaleHistograms_Chebyshev TBins%d_Bin%02d",c,a+c-b);
      Add(buffer,vec[a]);
   }
   /* multiple histogram of the Chebushev Fourier transform (signatures 701 - 724) */
   ChebyshevFourierTransform->MultiScaleHistogram(vec);
   b=3;c=3;
   for (a=0;a<24;a++)
   {  if (a==b) b+=(c=c+2);
      sprintf(buffer,"MultipleScaleHistograms_ChebyshevFFT TBins%d_Bin%02d",c,a+c-b);
      Add(buffer,vec[a]);
   }
   /* multiple histogram of the Fourier transform (signatures 725 - 748) */
   FourierTransform->MultiScaleHistogram(vec);
   b=3;c=3;
   for (a=0;a<24;a++)
   {  if (a==b) b+=(c=c+2);
      sprintf(buffer,"MultipleScaleHistograms_FFT TBins%d_Bin%02d",c,a+c-b);
      Add(buffer,vec[a]);
   }
   /* multiple histogram of the wavelet transform (signatures 749 - 772) */
   WaveletSelector->MultiScaleHistogram(vec);
   b=3;c=3;
   for (a=0;a<24;a++)
   {  if (a==b) b+=(c=c+2);
      sprintf(buffer,"MultipleScaleHistograms_Wavelet TBins%d_Bin%02d",c,a+c-b);
      Add(buffer,vec[a]);
   }
   /* multiple histogram of the wavelet Fourier transform (signatures 773 - 796) */
   WaveletFourierSelector->MultiScaleHistogram(vec);
   b=3;c=3;   
   for (a=0;a<24;a++)
   {  if (a==b) b+=(c=c+2);
      sprintf(buffer,"MultipleScaleHistograms_WaveletFFT TBins%d_Bin%02d",c,a+c-b);
      Add(buffer,vec[a]);
   }

   /* radon transform (signatures 797 - 844) */
   matrix->RadonTransform2D(vec);
   for (a=0;a<12;a++)
   {  sprintf(buffer,"RadonTransformStatistics Orient%d_Bin_%02d",45*(int)(a/3),a % 3);
      Add(buffer,vec[a]);
   }
   ChebyshevTransform->RadonTransform2D(vec);
   for (a=0;a<12;a++)
   {  sprintf(buffer,"RadonTransformStatistics_Chebyshev Orient%d_Bin_%02d",45*(int)(a/3),a % 3);
      Add(buffer,vec[a]);
   }
   ChebyshevFourierTransform->RadonTransform2D(vec);
   for (a=0;a<12;a++)
   {  sprintf(buffer,"RadonTransformStatistics_ChebyshevFFT Orient%d_Bin_%02d",45*(int)(a/3),a % 3);
      Add(buffer,vec[a]);
   }
   FourierTransform->RadonTransform2D(vec);
   for (a=0;a<12;a++)
   {  sprintf(buffer,"RadonTransformStatistics_FFT Orient%d_Bin_%02d",45*(int)(a/3),a % 3);
      Add(buffer,vec[a]);
   }

   char tamura_names[80][80]={"Total_Coarseness","Coarseness_Hist_Bin_00","Coarseness_Hist_Bin_01","Coarseness_Hist_Bin_02","Directionality","Contrast"};
   /* tamura texture (signatures 845 - 880) */
   matrix->TamuraTexture2D(vec);
   for (a=0;a<6;a++)
   {  sprintf(buffer,"TamuraTextures %s",tamura_names[a]);
      Add(buffer,vec[a]);
   }
   ChebyshevTransform->TamuraTexture2D(vec);
   for (a=0;a<6;a++)
   {  sprintf(buffer,"TamuraTextures_Chebyshev %s",tamura_names[a]);
      Add(buffer,vec[a]);
   }
   ChebyshevFourierTransform->TamuraTexture2D(vec);
   for (a=0;a<6;a++)
   {  sprintf(buffer,"TamuraTextures_ChebyshevFFT %s",tamura_names[a]);
      Add(buffer,vec[a]);
   }
   FourierTransform->TamuraTexture2D(vec);
   for (a=0;a<6;a++)
   {  sprintf(buffer,"TamuraTextures_FFT %s",tamura_names[a]);
      Add(buffer,vec[a]);
   }
   WaveletSelector->TamuraTexture2D(vec);
   for (a=0;a<6;a++)
   {  sprintf(buffer,"TamuraTextures_Wavelet %s",tamura_names[a]);
      Add(buffer,vec[a]);
   }
   WaveletFourierSelector->TamuraTexture2D(vec);
   for (a=0;a<6;a++)
   {  sprintf(buffer,"TamuraTextures_WaveletFFT %s",tamura_names[a]);
      Add(buffer,vec[a]);
   }

   /* zernike (signatures 881 - 1024) */
   { long x,y,output_size;   /* output size is normally 72 */
     matrix->zernike2D(vec,&output_size);
     x=0;y=0;
     for (a=0;a<output_size;a++)
     {  sprintf(buffer,"ZernikeMoments Z_%02d_%02d",(int)y,(int)x);
        Add(buffer,vec[a]);
        if (x>=y) x=1-(y++ % 2);
        else x+=2;
     }
    x=0;y=0;
    FourierTransform->zernike2D(vec,&output_size);
    for (a=0;a<output_size;a++)
    {  sprintf(buffer,"ZernikeMoments_FFT Z_%02d_%02d",(int)y,(int)x);
       Add(buffer,vec[a]);
       if (x>=y) x=1-(y++ % 2);
       else x+=2;
     }
   }

   if (compute_colors)
     CompGroupD(matrix,"");

   delete FourierTransform;
   delete ChebyshevTransform;
   delete ChebyshevFourierTransform;
   delete WaveletSelector;
   delete WaveletFourierSelector;
   return;

   
   /* basic statistics (signatures 1025 - 1049) */

   /* basic image statistics */
   matrix->BasicStatistics(&mean, &median, &std, &min, &max, NULL, 10);
   Add("mean",mean);
   Add("median",median);
   Add("stddev",std);
   Add("min",min);
   Add("max",max);

   /* basic chebyshev statistics */
   ChebyshevTransform->BasicStatistics(&mean, &median, &std, &min, &max, NULL, 10);
   Add("Chebyshev mean",mean);
   Add("Chebyshev median",median);
   Add("Chebyshev stddev",std);
   Add("Chebyshev min",min);
   Add("Chebyshev max",max);

   /* basic Fourier statistics */
   FourierTransform->BasicStatistics(&mean, &median, &std, &min, &max, NULL, 10);
   Add("Fourier mean",mean);
   Add("Fourier median",median);
   Add("Fourier stddev",std);
   Add("Fourier min",min);
   Add("Fourier max",max);

   /* basic Wavelet statistics */
   WaveletSelector->BasicStatistics(&mean, &median, &std, &min, &max, NULL, 10);
   Add("Wavelet mean",mean);
   Add("Wavelet median",median);
   Add("Wavelet stddev",std);
   Add("Wavelet min",min);
   Add("Wavelet max",max);

   /* basic Wavelet Fourier statistics */
   WaveletFourierSelector->BasicStatistics(&mean, &median, &std, &min, &max, NULL, 10);
   Add("Wavelet Fourier mean",mean);
   Add("Wavelet Fourier median",median);
   Add("Wavelet Fourier stddev",std);
   Add("Wavelet Fourier min",min);
   Add("Wavelet Fourier max",max);

   delete FourierTransform;
   delete ChebyshevTransform;
   delete ChebyshevFourierTransform;
   delete WaveletSelector;
   delete WaveletFourierSelector;



   return;



/* consider adding also the global centroid (by calling the centroid method) also centroid of Fourier   x and y of max */
   {
      TempMatrix=matrix->duplicate();
      TempMatrix->normalize(min,max,255,-1,-1);
      TempMatrix->BasicStatistics(&mean, &median, &std, &min, &max, histogram, 0);
      Add("normalized mean",mean);
      Add("normalized median",median);
      Add("normalized stddev",std);
      delete TempMatrix;

   }

   /* general color image statistics */
   {  double hue_avg, hue_std, sat_avg, sat_std, val_avg, val_std, max_color, colors[COLORS_NUM];
      matrix->GetColorStatistics(&hue_avg, &hue_std, &sat_avg, &sat_std, &val_avg, &val_std, &max_color, colors);
      Add("hue average",hue_avg);
      Add("hue stddev",hue_std);
      Add("saturation average",sat_avg);
      Add("saturation stddev",sat_std);
      Add("value average",val_avg);
      Add("value stddev",val_std);
      Add("most common color",max_color);
      for (a=0;a<COLORS_NUM;a++)
      {  sprintf(buffer,"color %d",a);
         Add(buffer,colors[a]);
      }
   }

   /* basic edges */
   TempMatrix=matrix->duplicate();
   TempMatrix->EdgeTransform();
   TempMatrix->BasicStatistics(&mean, &median, &std, &min, &max, histogram, 8);
   Add("mean edge",mean);
   Add("median edge",median);
   for (a=0;a<8;a++)
   {  sprintf(buffer,"edge bin %d/8",a);
      Add(buffer,histogram[a]);
   }
   delete TempMatrix;

   /* edge statistics */

   /* peaks of fourier on each axis */

}


/* CompGroupA
   compute group A of image feature (high contrast features)
   the features in this group are edge statistics, object statistics and Gabor textures
   input - an image matrix structure.
         - transform_label - the image transform short description (e.g., wavelet-fourier)
*/
void signatures::CompGroupA(ImageMatrix *matrix, const char *transform_label)
{  int a;
   char buffer[80];
   double vec[7]={0,0,0,0,0,0,0};
   /* edge statistics */
   {  long EdgeArea=0;
      double MagMean=0, MagMedian=0, MagVar=0, MagHist[8]={0,0,0,0,0,0,0,0}, DirecMean=0, DirecMedian=0, DirecVar=0, DirecHist[8]={0,0,0,0,0,0,0,0}, DirecHomogeneity=0, DiffDirecHist[4]={0,0,0,0};

      if (IsNeeded(count,28))  /* check if this group of signatures is needed */
        matrix->EdgeStatistics(&EdgeArea, &MagMean, &MagMedian, &MagVar, MagHist, &DirecMean, &DirecMedian, &DirecVar, DirecHist, &DirecHomogeneity, DiffDirecHist, 8);
      Add("Edge Area",EdgeArea);
      for (a=0;a<4;a++)
      {  sprintf(buffer,"Edge DiffDirecHist bin %d",a);
         Add(buffer,DiffDirecHist[a]);
      }
      for (a=0;a<8;a++)
      {  sprintf(buffer,"Edge Direction Histogram bin %d",a);
         Add(buffer,DirecHist[a]);
      }
      Add("Edge Direction Homogeneity",DirecHomogeneity);
      Add("Edge Direction Mean",DirecMean);
      Add("Edge Direction Median",DirecMedian);
      Add("Edge Direction Variance",DirecVar);
      for (a=0;a<8;a++)
      {  sprintf(buffer,"Edge Magnitude Histogram bin %d",a);
         Add(buffer,MagHist[a]);
      }
      Add("Edge Magnitude Mean",MagMean);
      Add("Edge Magnitude Median",MagMedian);
      Add("Edge Magnitude Variance",MagVar);
   }

   /* object statistics */
   {  int feature_count=0, Euler=0, AreaMin=0, AreaMax=0, AreaMedian=0, area_histogram[10]={0,0,0,0,0,0,0,0,0,0}, dist_histogram[10]={0,0,0,0,0,0,0,0,0,0};
      double centroid_x=0, centroid_y=0, AreaMean=0, AreaVar=0, DistMin=0, DistMax=0, DistMean=0, DistMedian=0, DistVar=0;

      if (IsNeeded(count,34))  /* check if this group of signatures is needed */
        matrix->FeatureStatistics(&feature_count, &Euler, &centroid_x, &centroid_y, NULL, &AreaMin, &AreaMax, &AreaMean, &AreaMedian, &AreaVar, area_histogram, &DistMin, &DistMax,
                        &DistMean, &DistMedian, &DistVar, dist_histogram, 10);

      for (a=0;a<10;a++)  /* area histogram */
      {  sprintf(buffer,"Feature area histogram bin %d",a);
         Add(buffer,area_histogram[a]);
      }
      Add("Feature AreaMax",AreaMax);
      Add("Feature AreaMean",AreaMean);
      Add("Feature AreaMedian",AreaMedian);
      Add("Feature AreaMin",AreaMin);
      Add("Feature AreaVar",AreaVar);
      Add("Feature X Centroid",centroid_x);
      Add("Feature Y Centroid",centroid_y);
      Add("Feature Count",feature_count);
      for (a=0;a<10;a++)
      {  sprintf(buffer,"Feature dist histogram bin %d",a);
         Add("Feature DistHist",dist_histogram[a]);
      }
      Add("Feature DistMax",DistMax);
      Add("Feature DistMean",DistMean);
      Add("Feature DistMedian",DistMedian);
      Add("Feature DistMin",DistMin);
      Add("Feature DistVar",DistVar);
      Add("Feature Euler",Euler);
   }

   /* gabor filters */
   if (IsNeeded(count,7))
     matrix->GaborFilters2D(vec);
   for (a=0;a<7;a++)
   {  sprintf(buffer,"Gabor Filters bin %d",a);
      Add(buffer,vec[a]);
   }
}

/* CompGroupB
   compute group B of image feature (Polynomial Decompositions)
   the features in this group are Chebyshev-Fourier Statistics, Chebyshev Statistics, Zernike Polynomials
   input - an image matrix structure.
         - transform_label - the image transform short description (e.g., wavelet-fourier)
*/
void signatures::CompGroupB(ImageMatrix *matrix, const char *transform_label)
{  int a;
   double vec[72];
   char buffer[80];
   ImageMatrix *TempMatrix;

   /* chebichev fourier transform (signatures 0 - 63) */
   for (a=0;a<72;a++) vec[a]=0;
   if (IsNeeded(count,32))
     matrix->ChebyshevFourierTransform2D(vec);
   for (a=0;a<32;a++)
   {  sprintf(buffer,"Chebishev Fourier Transform bin %d (%s)",a,transform_label);
      Add(buffer,vec[a]);
   }

   /* Chebyshev Statistics (signatures 64 - 127) */
   TempMatrix=matrix->duplicate();
   if (IsNeeded(count,32))
     TempMatrix->ChebyshevStatistics2D(vec,0,32);
   for (a=0;a<32;a++)
   {  sprintf(buffer,"Chebishev Statistics bin %d (%s)",a,transform_label);
      Add(buffer,vec[a]);
   }
   delete TempMatrix;

   /* zernike (signatures 881 - 1024) */
   long output_size;   /* output size is normally 72 */
   if (IsNeeded(count,72))
     matrix->zernike2D(vec,&output_size);
   for (a=0;a<72;a++)
   {  sprintf(buffer,"Zernike %d (%s)",a,transform_label);
      Add(buffer,vec[a]);
   }
}

/* CompGroupC
   compute group C of image feature (Statistics and Textures)
   the features in this group are First Four Moments, Haralick Textures, Multiscale Histogram, Tamura Textures, Radon Transform Statistics
   input - an image matrix structure.
         - transform_label - the image transform short description (e.g., wavelet-fourier)
*/
void signatures::CompGroupC(ImageMatrix *matrix, const char *transform_label)
{  int a;
   double vec[48];
   char buffer[80];
   double mean=0, median=0, std=0, min=0, max=0;

   for (a=0;a<48;a++) vec[a]=0;
   /* Comb4Moments */
   if (IsNeeded(count,48))
     matrix->CombFirstFourMoments2D(vec);
   for (a=0;a<48;a++)
   {  sprintf(buffer,"CombFirstFourMoments %d (%s)",a,transform_label);
      Add(buffer,vec[a]);
   }

   /* haarlick textures */
   if (IsNeeded(count,28))
     matrix->HaarlickTexture2D(0,vec);
   for (a=0;a<28;a++)
   {  sprintf(buffer,"Haarlick Texture %d (%s)",a,transform_label);
      Add(buffer,vec[a]);
   }

   /* multiscale histogram of the original image */
   if (IsNeeded(count,24))
     matrix->MultiScaleHistogram(vec);
   for (a=0;a<24;a++)
   {  sprintf(buffer,"MultiScale Histogram bin %d (%s)",a,transform_label);
      Add(buffer,vec[a]);
   }

   /* tamura texture */
   if (IsNeeded(count,6))
     matrix->TamuraTexture2D(vec);
   for (a=0;a<6;a++)
   {  sprintf(buffer,"Tamura Texture %d (%s)",a,transform_label);
      Add(buffer,vec[a]);
   }

   /* radon transform */
   if (IsNeeded(count,12))
     matrix->RadonTransform2D(vec);
   for (a=0;a<12;a++)
   {  sprintf(buffer,"Radon bin %d (%s)",a,transform_label);
      Add(buffer,vec[a]);
   }

   /* fractal features */
   if (IsNeeded(count,20))
     matrix->fractal2D(20,vec);
   for (a=0;a<20;a++)
   {  sprintf(buffer,"Fractal %d (%s)",a,transform_label);
      Add(buffer,vec[a]);
   }
   
   /* basic statistics */
   if (IsNeeded(count,5))
     matrix->BasicStatistics(&mean, &median, &std, &min, &max, NULL, 10);
   sprintf(buffer,"mean (%s)",transform_label);
   Add(buffer,mean);
   sprintf(buffer,"median (%s)",transform_label);
   Add(buffer,median);
   sprintf(buffer,"stddev (%s)",transform_label);
   Add(buffer,std);
   sprintf(buffer,"min (%s)",transform_label);
   Add(buffer,min);
   sprintf(buffer,"max (%s)",transform_label);
   Add(buffer,max);
}

/* CompGroupD
   compute group D of image feature (color features)
   the features in this group are HSV statistics, color histogram
   input - an image matrix structure.
         - transform_label - the image transform short description (e.g., wavelet-fourier)
*/
void signatures::CompGroupD(ImageMatrix *matrix, const char *transform_label)
{  int color_index;
   char buffer[80];
   double color_hist[COLORS_NUM+1];

//   ImageMatrix *ColorTransformMatrix,*ColorFFT,*ColorChebyshev,*ColorWavelet;
   ImageMatrix *ColorTransformMatrix,*HueTransformMatrix,*HueFFT,*HueChebyshev;

   ColorTransformMatrix=matrix->duplicate();
   ColorTransformMatrix->ColorTransform(color_hist,0);
//   ColorFFT=ColorTransformMatrix->duplicate();
//   ColorFFT->fft2();
//   ColorChebyshev=ColorTransformMatrix->duplicate();
//   ColorChebyshev->ChebyshevTransform(0);
//   ColorWavelet=ColorTransformMatrix->duplicate();
//   ColorWavelet->Symlet5Transform();
   HueTransformMatrix=matrix->duplicate();
   HueTransformMatrix->ColorTransform(NULL,1);
   HueFFT=HueTransformMatrix->duplicate();
   HueFFT->fft2();
   HueChebyshev=HueTransformMatrix->duplicate();
   HueChebyshev->ChebyshevTransform(0);

   /* color histogram */
   for (color_index=1;color_index<=COLORS_NUM;color_index++)
   {  sprintf(buffer,"color histogram bin %d (%s)",color_index,transform_label);
      Add(buffer,color_hist[color_index]);
   }

   /* now compute the groups */
//   CompGroupA(ColorTransformMatrix,"Color Transform");
   CompGroupB(ColorTransformMatrix,"Color Transform");
   CompGroupC(ColorTransformMatrix,"Color Transform");

   CompGroupB(HueTransformMatrix,"Hue Transform");
   CompGroupC(HueTransformMatrix,"Hue Transform");

   CompGroupB(HueFFT,"Hue FFT Transform");
   CompGroupC(HueFFT,"Hue FFT Transform");

   CompGroupB(HueChebyshev,"Hue Chebyshev Transform");
   CompGroupC(HueChebyshev,"Hue Chebyshev Transform");

//   CompGroupB(ColorFFT,"Color FFT Transform");
//   CompGroupC(ColorFFT,"Color FFT Transform");

//   CompGroupB(ColorChebyshev,"Color Chebyshev Transform");
//   CompGroupC(ColorChebyshev,"Color Chebyshev Transform");

//   CompGroupB(ColorWavelet,"Color Wavelet Transform");
//   CompGroupC(ColorWavelet,"Color Wavelet Transform");

   delete ColorTransformMatrix;
   delete HueTransformMatrix;
   delete HueFFT;
   delete HueChebyshev;
//   delete ColorFFT;
//   delete ColorChebyshev;
//   delete ColorWavelet;

   ColorTransformMatrix=matrix->duplicate();
   ColorTransformMatrix->ColorTransform(color_hist,0);
//   ColorFFT=ColorTransformMatrix->duplicate();
//   ColorFFT->fft2();
//   ColorChebyshev=ColorTransformMatrix->duplicate();
//   ColorChebyshev->ChebyshevTransform(0);
//   ColorWavelet=ColorTransformMatrix->duplicate();
//   ColorWavelet->Symlet5Transform();
}

/* ComputeGroups
   compute the image features
   input - an image matrix structure.
*/
void signatures::ComputeGroups(ImageMatrix *matrix, int compute_colors)
{
  ImageMatrix *FourierTransform,*ChebyshevTransform,*ChebyshevFourierTransform,*WaveletSelector,*FourierWaveletSelector;
  ImageMatrix *FourierChebyshev,*WaveletFourier,*ChebyshevWavelet, *EdgeTransform, *EdgeFourier, *EdgeWavelet;

  count=0;      /* start counting signatures from 0 */


/*

// first level
  CompGroupB(matrix,"raw");
  CompGroupC(matrix,"raw");

// second level
  TempMatrix=matrix->duplicate();
  TempMatrix->fft2();
  CompGroupB(TempMatrix,"#f_");
  CompGroupC(TempMatrix,"#f_");
  delete TempMatrix;

  TempMatrix=matrix->duplicate();
  TempMatrix->ChebyshevTransform(0);
  CompGroupB(TempMatrix,"#c_");
  CompGroupC(TempMatrix,"#c_");
  delete TempMatrix;

  TempMatrix=matrix->duplicate();
  TempMatrix->Symlet5Transform();
  CompGroupB(TempMatrix,"#w_");
  CompGroupC(TempMatrix,"#w_");
  delete TempMatrix;

  
// fourier / chebyshev

  TempMatrix=matrix->duplicate();
  TempMatrix->fft2();
  TempMatrix->ChebyshevTransform(0);
  CompGroupB(TempMatrix,"#fc_");
  CompGroupC(TempMatrix,"#fc_");
  delete TempMatrix;

  TempMatrix=matrix->duplicate();
  TempMatrix->fft2();
  TempMatrix->ChebyshevTransform(0);
  TempMatrix->fft2();  
  CompGroupB(TempMatrix,"#fcf_");
  CompGroupC(TempMatrix,"#fcf_");
  delete TempMatrix;

  TempMatrix=matrix->duplicate();
  TempMatrix->fft2();
  TempMatrix->ChebyshevTransform(0);
  TempMatrix->fft2();  
  TempMatrix->ChebyshevTransform(0);  
  CompGroupB(TempMatrix,"#fcfc_");
  CompGroupC(TempMatrix,"#fcfc_");
  delete TempMatrix;
  
// chebyshev / fourier

  TempMatrix=matrix->duplicate();
  TempMatrix->ChebyshevTransform(0);
  TempMatrix->fft2();
  CompGroupB(TempMatrix,"#cf_");
  CompGroupC(TempMatrix,"#cf_");
  delete TempMatrix;

  TempMatrix=matrix->duplicate();
  TempMatrix->ChebyshevTransform(0);  
  TempMatrix->fft2();
  TempMatrix->ChebyshevTransform(0);  
  CompGroupB(TempMatrix,"#cfc_");
  CompGroupC(TempMatrix,"#cfc_");
  delete TempMatrix;

  TempMatrix=matrix->duplicate();
  TempMatrix->ChebyshevTransform(0);
  TempMatrix->fft2();  
  TempMatrix->ChebyshevTransform(0);
  TempMatrix->fft2();    
  CompGroupB(TempMatrix,"#cfcf_");
  CompGroupC(TempMatrix,"#cfcf_");
  delete TempMatrix;

// wavelet / fourier

  TempMatrix=matrix->duplicate();
  TempMatrix->Symlet5Transform();
  TempMatrix->fft2();
  CompGroupB(TempMatrix,"#wf_");
  CompGroupC(TempMatrix,"#wf_");
  delete TempMatrix;

  TempMatrix=matrix->duplicate();
  TempMatrix->Symlet5Transform();  
  TempMatrix->fft2();
  TempMatrix->Symlet5Transform();  
  CompGroupB(TempMatrix,"#wfw_");
  CompGroupC(TempMatrix,"#wfw_");
  delete TempMatrix;

  TempMatrix=matrix->duplicate();
  TempMatrix->Symlet5Transform();
  TempMatrix->fft2();  
  TempMatrix->Symlet5Transform();
  TempMatrix->fft2();    
  CompGroupB(TempMatrix,"#wfwf_");
  CompGroupC(TempMatrix,"#wfwf_");
  delete TempMatrix;

// fourier / wavelet

  TempMatrix=matrix->duplicate();
  TempMatrix->fft2();
  TempMatrix->Symlet5Transform();
  CompGroupB(TempMatrix,"#fw_");
  CompGroupC(TempMatrix,"#fw_");
  delete TempMatrix;

  TempMatrix=matrix->duplicate();
  TempMatrix->fft2();  
  TempMatrix->Symlet5Transform();  
  TempMatrix->fft2();
  CompGroupB(TempMatrix,"#fwf_");
  CompGroupC(TempMatrix,"#fwf_");
  delete TempMatrix;

  TempMatrix=matrix->duplicate();
  TempMatrix->fft2();  
  TempMatrix->Symlet5Transform();
  TempMatrix->fft2();    
  TempMatrix->Symlet5Transform();  
  CompGroupB(TempMatrix,"#fwfw_");
  CompGroupC(TempMatrix,"#fwfw_");
  delete TempMatrix;

// chebyshev / wavelet

  TempMatrix=matrix->duplicate();
  TempMatrix->ChebyshevTransform(0);
  TempMatrix->Symlet5Transform();
  CompGroupB(TempMatrix,"#cw_");
  CompGroupC(TempMatrix,"#cw_");
  delete TempMatrix;

  TempMatrix=matrix->duplicate();
  TempMatrix->ChebyshevTransform(0);
  TempMatrix->Symlet5Transform();  
  TempMatrix->ChebyshevTransform(0);
  CompGroupB(TempMatrix,"#cwc_");
  CompGroupC(TempMatrix,"#cwc_");
  delete TempMatrix;

  TempMatrix=matrix->duplicate();
  TempMatrix->ChebyshevTransform(0);
  TempMatrix->Symlet5Transform();
  TempMatrix->ChebyshevTransform(0);
  TempMatrix->Symlet5Transform();  
  CompGroupB(TempMatrix,"#cwcw_");
  CompGroupC(TempMatrix,"#cwcw_");
  delete TempMatrix;

  return;
*/




  FourierTransform=matrix->duplicate();
  FourierTransform->fft2();
  ChebyshevTransform=matrix->duplicate();
  ChebyshevTransform->ChebyshevTransform(0);
  ChebyshevFourierTransform=FourierTransform->duplicate();
  ChebyshevFourierTransform->ChebyshevTransform(0);
  WaveletSelector=matrix->duplicate();
  WaveletSelector->Symlet5Transform();
  FourierWaveletSelector=FourierTransform->duplicate();
  FourierWaveletSelector->Symlet5Transform();
  FourierChebyshev=ChebyshevTransform->duplicate();
  FourierChebyshev->fft2();
  WaveletFourier=WaveletSelector->duplicate();
  WaveletFourier->fft2();
  ChebyshevWavelet=WaveletSelector->duplicate();
  ChebyshevWavelet->ChebyshevTransform(0);
  EdgeTransform=matrix->duplicate();
  EdgeTransform->EdgeTransform();
  EdgeFourier=EdgeTransform->duplicate();
  EdgeFourier->fft2();
  EdgeWavelet=EdgeTransform->duplicate();  
  EdgeWavelet->Symlet5Transform();


  CompGroupA(matrix,"");
  CompGroupB(matrix,"");
  CompGroupC(matrix,"");
  if (compute_colors) CompGroupD(matrix,"");

  CompGroupB(FourierTransform,"Fourier");
  CompGroupC(FourierTransform,"Fourier");

  CompGroupB(WaveletSelector,"Wavelet");
  CompGroupC(WaveletSelector,"Wavelet");

  CompGroupB(ChebyshevTransform,"Chebyshev");
  CompGroupC(ChebyshevTransform,"Chebyshev");
// Fourier, then Chebyshev
  CompGroupC(ChebyshevFourierTransform,"Chebyshev Fourier");
// Fourier, then Wavelet
  CompGroupC(FourierWaveletSelector,"Wavelet Fourier");
/**/
// Wavelet, then Fourier
  CompGroupB(WaveletFourier,"Fourier Wavelet");
  CompGroupC(WaveletFourier,"Fourier Wavelet");

// Chebyshev, then Fourier
  CompGroupC(FourierChebyshev,"Fourier Chebyshev");
// Wavelet, then Chebyshev
  CompGroupC(ChebyshevWavelet,"Chebyshev Wavelet");

  CompGroupB(EdgeTransform,"Edge Transform");
  CompGroupC(EdgeTransform,"Edge Transform");
// Edge, then fourier - named in wrong order!
  CompGroupB(EdgeFourier,"Edge Fourier Transform");
  CompGroupC(EdgeFourier,"Edge Fourier Transform");

// Edge, then wavelet - named in wrong order!
  CompGroupB(EdgeWavelet,"Edge Wavelet Transform");
//printf("5.5\n");  
  CompGroupC(EdgeWavelet,"Edge Wavelet Transform");
//printf("6\n");
  delete FourierTransform;
  delete WaveletSelector;
  delete ChebyshevTransform;
  delete ChebyshevFourierTransform;
  delete FourierWaveletSelector;
  delete WaveletFourier;
  delete ChebyshevWavelet;
  delete EdgeTransform;
  delete EdgeFourier;
  delete EdgeWavelet;
  delete FourierChebyshev;
//printf("7\n");  
}


/* normalize
   normalize the signature values using the maximum and minimum values of the training set
   ts -TrainingSet *- the training set according which the signature values should be normalized
*/
void signatures::normalize(void *TrainSet)
{
	int sig_index;
	TrainingSet *ts;
	ts=(TrainingSet *)TrainSet;
	for( sig_index = 0; sig_index < count; sig_index++ )
	{
		if( data[ sig_index ].value >= INF )
			data[sig_index].value=0;
		else if( data[ sig_index ].value < ts->SignatureMins[ sig_index ] )
			data[ sig_index ].value = ts->SignatureMins[ sig_index ];
		else if( data[ sig_index ].value > ts->SignatureMaxes[ sig_index ] )
			data[ sig_index ].value = ts->SignatureMaxes[ sig_index ];
		if( ts->SignatureMins[ sig_index ] >= ts->SignatureMaxes[ sig_index ] )
			data[ sig_index ].value = 0; /* prevent possible division by zero */
		else
			data[ sig_index ].value=100*(data[sig_index].value-ts->SignatureMins[sig_index])/(ts->SignatureMaxes[sig_index]-ts->SignatureMins[sig_index]);
	}
}


/* ComputeFromDouble
   compute signature set of a given image and add the
   resulting values to the "data" attribute of this class.
   This function is the same as "compute" but accepts doubles as input
   instead of ImageMatrix

   input - data -double *- the pixel values.
                the data is organized such that the first data are the first row
                (y moves faster, x moves slower. z is faster than y if used)
           width - the image width (X)
           height - the image height (Y)
		   depth - the image depth (Z)
*/
void signatures::ComputeFromDouble(double *data, int width, int height, int depth, int compute_color)
{  ImageMatrix *matrix;
   long x,y,z;
   matrix=new ImageMatrix(width,height,depth);
   for (x=0;x<width;x++)
     for (y=0;y<height;y++)
       for (z=0;z<depth;z++)
         matrix->SetInt(x,y,z,data[x*height*depth + y * depth + z]);   
 
	compute(matrix,compute_color);
   delete matrix;
}


/* FileOpen
   Creates and opens a files before writing feature value content
   path -char *- path to the file to be opened.
     If NULL then open path is the original file name up to the last '.', followed by '_', the sample_name and ".sig".
   int overwrite - 1 for forceably overwritting existing .sig files
*/
FILE *signatures::FileOpen(char *path, int overwrite)
{  char filename[512];
   FILE *ret;
   if (path && *path != '\0') strcpy(filename,path);
   else GetFileName (filename);
   /*
     FIXME: this sets up a race condition.
       another process can try open-for-read between this process' open-for-read and open-for-write,
       resulting in the other process also failing open-for-read and opening this same file for write.
       Solution: The sig file acts as a lockfile.  The existence test and the file creation have to be in one atomic transaction.
       If we are able to get a file handle this way, it means that the file did not previously exist, and no other process has it open.
       If we cannot get the handle, it means some other process has this file opened.
   */
   if ( (ret=fopen(filename,"r")) )
   {  struct stat ft;
      fclose(ret);
      if (overwrite) 
	  {  stat(filename,&ft); /* check the file date only if overwrite is specified */
         if (time(NULL)-ft.st_mtime>7200) return(fopen(filename,"w"));  /* check if the file is forced to be overwritten */
      }   //st_mtimespec.tv_sec
      else return(NULL);   /* file already exists */
   }
   return(fopen(filename,"w"));
}

/* FileClose
   Closes a value file.  This is the closing command for files opened with ReadFromFile.
   This closes the stream as well as filedescriptor
*/
void signatures::FileClose(FILE *value_file)
{
	if (!value_file) return;
	int fd = fileno (value_file);
	fclose(value_file);
	close (fd);
}

int signatures::SaveToFile(FILE *value_file, int save_feature_names)
{  int sig_index;
   if (!value_file) {printf("Cannot write to .sig file\n");return(0);}
   if (NamesTrainingSet && ((TrainingSet *)(NamesTrainingSet))->is_continuous) {
   	fprintf(value_file,"%f\n",sample_value);  /* save the continouos value */
   } else fprintf(value_file,"%d\n",sample_class);  /* save the class index */
   fprintf(value_file,"%s\n",full_path);
   for (sig_index=0;sig_index<count;sig_index++)
     if (save_feature_names && NamesTrainingSet) fprintf(value_file,"%f %s\n",data[sig_index].value,((TrainingSet *)NamesTrainingSet)->SignatureNames[sig_index]);
	 else fprintf(value_file,"%f\n",data[sig_index].value);   
   return(1);
}

int signatures::LoadFromFile(char *filename)
{  char buffer[IMAGE_PATH_LENGTH+SAMPLE_NAME_LENGTH+1],*p_buffer;
   FILE *value_file;

	if (!filename || *filename == '\0')
		GetFileName (buffer);
	else strncpy (buffer,filename,sizeof(buffer));

   if (!(value_file=fopen(buffer,"r")))
     return(0);
   /* read the class or value */
   fgets(buffer,sizeof(buffer),value_file);   
   if (NamesTrainingSet && ((TrainingSet *)(NamesTrainingSet))->is_continuous) {
   	sample_value=atof(buffer);   /* continouos value */
   	sample_class = 1;
   } else sample_class=atoi(buffer);                /* class index      */

   /* read the path */
   fgets(buffer,sizeof(buffer),value_file);
   chomp (buffer);
   strcpy(full_path,buffer);

   /* read the feature values */
   p_buffer=fgets(buffer,sizeof(buffer),value_file);
   chomp (p_buffer);
   while (p_buffer)
   {  char *p_name;
      p_name=strchr(buffer,' ');
      if (p_name)    /* if there is a feature name in the file */
	  {  *p_name='\0';
         p_name++;
	  }
      Add(p_name,atof(buffer));
      p_buffer=fgets(buffer,sizeof(buffer),value_file);
      chomp (p_buffer);
   }
   fclose(value_file);
   return(1);
}



/*
  Yet another variant of reading from a file.
  In this case, the filename is computed from full_path and sample_name using GetFileName
  The fp (FILE **) will be set to the successfully opened and locked file (return 0).
  If another process has a lock, return 0 (fpp = NULL).
  If the file exists, and is not locked, the sigs will be loaded from it, and no lock will be issued. (return 1, *fpp = NULL)
  If an error occurs in obtaining the lock (if necessary) or creating the file (if necessary) or reading it (if possible), return -1.
*/
int signatures::ReadFromFile (FILE **fpp, bool wait) {
	char buffer[IMAGE_PATH_LENGTH+SAMPLE_NAME_LENGTH+1];
	int fd;
	struct flock fl;
	struct stat stat_buf;
	// or, use 	mode_t mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	mode_t mask = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

	// This is non-null only if we have a lock on an empty file.
	if (fpp) *fpp = NULL;


	// We will never read from this fd or from its fp
    if ( (fd = open(GetFileName (buffer), wait ? (O_RDONLY) : (O_WRONLY | O_CREAT),mask)) < 0 )
		return (-1);
    // Make a non-blocking request for a whole-file write lock
    fl.l_type = wait ? F_RDLCK : F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;

	if (fcntl(fd, wait ? F_SETLKW : F_SETLK, &fl) == -1) {
		if (errno == EACCES || errno == EAGAIN) {
		// locked by another process
			errno = 0;
			close (fd);
			return (0);
		} else {
		// an unexpected error
			close (fd);
			return (-1);
        }
	} else {
	// got the lock, check if it was just created and empty
		if (fstat(fd, &stat_buf)) {
			close (fd);
			return (-1);
		}
		errno = 0;
		if (stat_buf.st_size > 1) {
		// This file has stuff in it - release the lock and read it.
			close (fd); // this releases our lock
			Clear(); // reset sample count
			LoadFromFile (buffer);
			// Of course, if it was empty afterall, its an error.
			if (count < 1) return (NO_SIGS_IN_FILE);
			else return (1);
		} else {
		// We just made an empty file. Open it as a stream, keeping the lock
		// Call FileClose to close the file.  It closes the stream and the filedes.  Probably not necessary.
			if (fpp) *fpp = wait ? (fdopen (fd, "r")) : (fdopen (fd, "w"));
			return (0);
		}
	}
}

/*
  get the filename for storing the signature.
  The filename is generated from the full path of the image, plus a sample name.
  The sample name (i.e. _0_0 for tile 0,0) is set externally depending on what sample of the image (referred to by full_path) the signature pertains to.
  It is stored internally so that a signature object can get to its own file without additional information.
*/
char *signatures::GetFileName (char *buffer) {
	char *char_p;

	strcpy(buffer,full_path);
	char_p = strrchr(buffer,'.');
	if (!char_p) char_p=buffer+strlen(buffer);

	sprintf(char_p,"%s.sig",sample_name);
	return (buffer);
}


// Based on
// Usable AlmostEqual function
// By Bruce Dawson
// http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
// returns the ulps (floating point representations) b/w the two inputs
// uses a union for punning to avoid strict aliasing rules
int diffUlps(float A, float B)
{
	union fi_union {
	int32_t i;
	float f;
	};
	fi_union fiA,fiB;
	fiA.f = A;
	fiB.f = B;

    int32_t aInt = fiA.i;
    // Make aInt lexicographically ordered as a twos-complement int
    if (aInt < 0)
        aInt = 0x80000000 - aInt;
    // Make bInt lexicographically ordered as a twos-complement int
    int32_t bInt = fiB.i;
    if (bInt < 0)
        bInt = 0x80000000 - bInt;
    int intDiff = abs(aInt - bInt);
    return (intDiff);
}


/*
  This function is used to determine (somewhat quickly) if the features stored in a file match those
  that would be calculated from the passed-in matrix.
  A partial signature calculation is done to determine the match.  An exact match (within FLT_EPSILON) of every feature is required to return 1.
  If the file can't be opened, or if the match is inexact, 0 is returned.
*/
int signatures::CompareToFile (ImageMatrix *matrix, char *filename, int compute_colors, int large_set) {
	signatures file_sigs;
	double vec[72];
	int i,file_index;

	if (! file_sigs.LoadFromFile (filename) ) return (0);
	if (verbosity>=2) printf ("compare %s to computed\n",filename);

	// 20 features long: 323-342, standard: N/A
	if (large_set) {
		matrix->fractal2D(20,vec);
		file_index = 323;
// for (i = 0; i< 20; i++) printf ("fractal2D computed %15.10f\tfrom file: %15.10f\tdiff: %f\tulps: %d\n",vec[i],file_sigs.data[file_index+i].value,
// (file_sigs.data[file_index+i].value - vec[i])/FLT_EPSILON
// ,diffUlps(file_sigs.data[file_index+i].value,vec[i])
// );
		for (i = 0; i< 20; i++) if (!OUR_EQ(file_sigs.data[file_index+i].value,vec[i])) {
			if (verbosity>=2) printf ("fractal2D mismatch computed %15.10f\tfrom file: %15.10f\n",vec[i],file_sigs.data[file_index+i].value);
			return (0);
		}
	}
	if (verbosity>=2) printf ("fractal2D match\n");
	
	// 28 features long: 253-280, standard: 485-512
	matrix->HaarlickTexture2D(0,vec);
	if (large_set) file_index = 253;
	else file_index = 485;
	for (i = 0; i < 28; i++) if (!OUR_EQ(file_sigs.data[file_index+i].value,vec[i])) {
		if (verbosity>=2) printf ("HaarlickTexture2D mismatch computed %15.10f\tfrom file: %15.10f\n",vec[i],file_sigs.data[file_index+i].value);
		return (0);
	}
	if (verbosity>=2) printf ("HaarlickTexture2D match\n");
	
	// 72 features long: 133-204, standard: 881-952
// 	long output_size;   /* output size is normally 72 */
// 	matrix->zernike2D(vec,&output_size);
// 	if (large_set) file_index = 133;
// 	else file_index = 881;
// 	for (i = 0; i < 72; i++) if (!OUR_EQ(file_sigs.data[file_index+i].value,vec[i])) {
// 		if (verbosity>=2) printf ("zernike2D mismatch computed %15.10f\tfrom file: %15.10f\n",vec[i],file_sigs.data[file_index+i].value);
// 		return (0);
// 	}
// 	if (verbosity>=2) printf ("zernike2D match.\n");

	if (verbosity>=2) printf ("Match found.\n");
	return (1);
}


#ifdef WIN32
#pragma package(smart_init)
#endif

