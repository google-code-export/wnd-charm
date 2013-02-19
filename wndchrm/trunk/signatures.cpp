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


#include "signatures.h"

#include <cmath>
#include <cfloat> // Has definition of DBL_EPSILON, FLT_EPSILON
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h> // for locking stuff
#include <errno.h>
#include <time.h>
#include <unistd.h> // apparently, for close() only?
#define OUR_EPSILON FLT_EPSILON*6
#define FLOAT_EQ(x,v) (((v - FLT_EPSILON) < x) && (x <( v + FLT_EPSILON)))
#define OUR_EQ(x,v) (((v - OUR_EPSILON) < x) && (x <( v + OUR_EPSILON)))
#include "cmatrix.h"
#include "TrainingSet.h"
#include "colors/FuzzyCalc.h"

#ifndef WIN32
#include <stdlib.h>
#endif


/* global variable */
extern int verbosity;

// static signatures::max_sigs
long signatures::max_sigs = NUM_DEF_FEATURES;

//---------------------------------------------------------------------------
/*  signatures (constructor)
*/
signatures::signatures() {
	data = NULL;
	version = 0;
	feature_vec_type = fv_unknown;
	count=0;
	allocated = 0;
	sample_class=0;
	full_path[0]='\0';
	sample_name[0]='\0';
	NamesTrainingSet=NULL;   
	ScoresTrainingSet=NULL;
	wf = NULL;
}
//---------------------------------------------------------------------------

signatures::~signatures() {
	if (data) delete data;
	data = NULL;
	if (wf) delete wf;
	wf = NULL;
}
/* duplicate
*/
signatures *signatures::duplicate() {
	signatures *new_samp;
	new_samp=new signatures();
	new_samp->sample_class=sample_class;
	new_samp->sample_value=sample_value;   
	new_samp->interpolated_value=interpolated_value;
	new_samp->count=count;
	new_samp->NamesTrainingSet=NamesTrainingSet;   
	new_samp->ScoresTrainingSet=ScoresTrainingSet;
	strcpy(new_samp->full_path,full_path);

	new_samp->Allocate (count);
	memcpy (new_samp->data, data, sizeof (signature) * count );
	wf = NULL;
	new_samp->version = version;
	new_samp->feature_vec_type = feature_vec_type;
	return(new_samp);
}

/* Allocate
   Allocate memory for specified number of signatures
   nsigs -size_t - number of signatures to preallocate
*/
void signatures::Allocate(size_t nsigs) {
	if (data) delete data;
	data = new signature[nsigs];
	if (data) {
		memset (data,0,sizeof(signature)*nsigs);
		allocated = nsigs;
	}
}

/* Add
   add a signature
   name -char *- the name of the signature (e.g. Multiscale Histogram bin 3)
   value -double- the value to add
*/
void signatures::Add(const char *name,double value) {
	if (name && NamesTrainingSet) strcpy(((TrainingSet *)(NamesTrainingSet))->SignatureNames[count],name);
	
	
	if (count == 0 && allocated == 0) {
		Allocate (max_sigs);
	} else if (count >= allocated) {
		signature *old_data = data;
		data = NULL; // Avoid Allocate calling delete on this
		Allocate (count + 1024);
		memcpy (data, old_data, count * sizeof (signature));
		delete old_data;
	}
	data[count].value=value;
	count++;
	if (count > max_sigs) max_sigs = count;
}


void signatures::SetFeatureVectorType () {
	if (feature_vec_type == fv_unknown) {
		switch (count) {
			case NUM_LC_FEATURES:
				feature_vec_type = fv_long_color;
			break;
			case NUM_L_FEATURES:
				feature_vec_type = fv_long;
			break;
			case NUM_C_FEATURES:
				feature_vec_type = fv_short_color;
			break;
			case NUM_DEF_FEATURES:
				feature_vec_type = fv_short;
			break;
			default:
			break;
		}
	}
}

/* Clear
   clear all signature values
*/
void signatures::Clear() {
	if (data) delete data;
	data = NULL;
	allocated = 0;
	count = 0;
	feature_vec_type = fv_unknown;
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

void signatures::compute(ImageMatrix &matrix, int compute_colors)
{  char buffer[80];
   double vec[72];
   int a,b,c;
   ImageMatrix FourierTransform,ChebyshevTransform,ChebyshevFourierTransform,WaveletSelector,WaveletFourierSelector;
   ImageMatrix TempMatrix;

	version = CURRENT_FEATURE_VERSION;

   if (verbosity>=2) printf("start processing image...\n");   
   if (verbosity>3) printf("transforms...\n");
   if (verbosity>3) printf("...fft2\n");
   FourierTransform.copy (matrix);
   FourierTransform.fft2();
   ChebyshevTransform.copy (matrix);
   if (verbosity>3) printf("...ChebyshevTransform\n");
   ChebyshevTransform.ChebyshevTransform(0);
   ChebyshevFourierTransform.copy (FourierTransform);
   if (verbosity>3) printf("...ChebyshevFourierTransform\n");
   ChebyshevFourierTransform.ChebyshevTransform(0);
   WaveletSelector.copy (matrix);
   if (verbosity>3) printf("...Symlet5Transform\n");
   WaveletSelector.Symlet5Transform();
   WaveletFourierSelector.copy (FourierTransform);
   if (verbosity>3) printf("...WaveletFourierSelector\n");
   WaveletFourierSelector.Symlet5Transform();
   if (verbosity>3) printf("start computing features\n");
   count=0;      /* start counting signatures from 0 */
   /* chebyshev fourier transform (signatures 0 - 63) */
   for (a=0;a<32;a++) vec[a]=0;
   if (verbosity>3) printf("...ChebyshevFourierCoefficientHistogram\n");
   matrix.ChebyshevFourierTransform2D(vec);
   for (a=0;a<32;a++)
   {  sprintf(buffer,"ChebyshevFourierCoefficientHistogram Bin%02d",a);
      Add(buffer,vec[a]);
   }
   if (verbosity>3) printf("...ChebyshevFourierCoefficientHistogram_FFT\n");
   FourierTransform.ChebyshevFourierTransform2D(vec);
   for (a=0;a<32;a++)
   {  sprintf(buffer,"ChebyshevFourierCoefficientHistogram_FFT Bin%02d",a);
      Add(buffer,vec[a]);
   }
   /* Chebyshev Statistics (signatures 64 - 127) */
   TempMatrix.copy (matrix);
   if (verbosity>3) printf("...ChebyshevCoefficientHistogram\n");
   TempMatrix.ChebyshevStatistics2D(vec,0,32);
   for (a=0;a<32;a++)
   {  sprintf(buffer,"ChebyshevCoefficientHistogram Bin%02d",a);
      Add(buffer,vec[a]);
   }

   TempMatrix.copy (FourierTransform);
   if (verbosity>3) printf("...ChebyshevCoefficientHistogram_FFT\n");
   TempMatrix.ChebyshevStatistics2D(vec,0,32);
   for (a=0;a<32;a++)
   {  sprintf(buffer,"ChebyshevCoefficientHistogram_FFT Bin%02d",a);
      Add(buffer,vec[a]);
   }

   /* Comb4Moments (signatures 128 - 415) */
//    char four_moments_names[80][80]={"Minus45_Mean_HistBin00","Minus45_Mean_HistBin01","Minus45_Mean_HistBin02","Minus45_Std_HistBin00","Minus45_Std_HistBin01","Minus45_Std_HistBin02",
//            "Minus45_Skew_HistBin00","Minus45_Skew_HistBin01","Minus45_Skew_HistBin02","Minus45_Kurt_HistBin00","Minus45_Kurt_HistBin01","Minus45_Kurt_HistBin02",
// 		   "Plus45_Mean_HistBin00","Plus45_Mean_HistBin01","Plus45_Mean_HistBin02","Plus45_Std_HistBin00","Plus45_Std_HistBin01","Plus45_Std_HistBin02",
// 		   "Plus45_Skew_HistBin00","Plus45_Skew_HistBin01","Plus45_Skew_HistBin02","Plus45_Kurt_HistBin00","Plus45_Kurt_HistBin01","Plus45_Kurt_HistBin02","90_Mean_HistBin00",
// 		   "90_Mean_HistBin01","90_Mean_HistBin02","90_Std_HistBin00","90_Std_HistBin01","90_Std_HistBin02","90_Skew_HistBin00","90_Skew_HistBin01","90_Skew_HistBin02",
// 		   "90_Kurt_HistBin00","90_Kurt_HistBin01","90_Kurt_HistBin02","0_Mean_HistBin00","0_Mean_HistBin01","0_Mean_HistBin02","0_Std_HistBin00","0_Std_HistBin01","0_Std_HistBin02",
// 		   "0_Skew_HistBin00","0_Skew_HistBin01","0_Skew_HistBin02","0_Kurt_HistBin00","0_Kurt_HistBin01","0_Kurt_HistBin02"};
	char four_moments_names[80][80]={
		"Minus45_Mean_HistBin00", "Minus45_Mean_HistBin01", "Minus45_Mean_HistBin02",
		"Minus45_Std_HistBin00",  "Minus45_Std_HistBin01",  "Minus45_Std_HistBin02",
		"Minus45_Skew_HistBin00", "Minus45_Skew_HistBin01", "Minus45_Skew_HistBin02",
		"Minus45_Kurt_HistBin00", "Minus45_Kurt_HistBin01", "Minus45_Kurt_HistBin02",
		"Plus45_Mean_HistBin00",  "Plus45_Mean_HistBin01",  "Plus45_Mean_HistBin02",
		"Plus45_Std_HistBin00",   "Plus45_Std_HistBin01",   "Plus45_Std_HistBin02",
		"Plus45_Skew_HistBin00",  "Plus45_Skew_HistBin01",  "Plus45_Skew_HistBin02",
		"Plus45_Kurt_HistBin00",  "Plus45_Kurt_HistBin01",  "Plus45_Kurt_HistBin02",
		"90_Mean_HistBin00",      "90_Mean_HistBin01",      "90_Mean_HistBin02",
		"90_Std_HistBin00",       "90_Std_HistBin01",       "90_Std_HistBin02",
		"90_Skew_HistBin00",      "90_Skew_HistBin01",      "90_Skew_HistBin02",
		"90_Kurt_HistBin00",      "90_Kurt_HistBin01",      "90_Kurt_HistBin02",
		"0_Mean_HistBin00",       "0_Mean_HistBin01",       "0_Mean_HistBin02",
		"0_Std_HistBin00",        "0_Std_HistBin01",        "0_Std_HistBin02",
		"0_Skew_HistBin00",       "0_Skew_HistBin01",       "0_Skew_HistBin02",
		"0_Kurt_HistBin00",       "0_Kurt_HistBin01",       "0_Kurt_HistBin02"
	};

   if (verbosity>3) printf("...Comb4Orient4MomentsHistogram\n");
   matrix.CombFirstFourMoments2D(vec);
   for (a=0;a<48;a++)
   {  sprintf(buffer,"Comb4Orient4MomentsHistogram %s",four_moments_names[a]);
      Add(buffer,vec[a]);
   }
   if (verbosity>3) printf("...Comb4Orient4MomentsHistogram_Chebyshev\n");
   ChebyshevTransform.CombFirstFourMoments2D(vec);
   for (a=0;a<48;a++)
   {  sprintf(buffer,"Comb4Orient4MomentsHistogram_Chebyshev %s",four_moments_names[a]);
      Add(buffer,vec[a]);
   }
   if (verbosity>3) printf("...Comb4Orient4MomentsHistogram_ChebyshevFFT\n");
   ChebyshevFourierTransform.CombFirstFourMoments2D(vec);
   for (a=0;a<48;a++)
   {  sprintf(buffer,"Comb4Orient4MomentsHistogram_ChebyshevFFT %s",four_moments_names[a]);
      Add(buffer,vec[a]);
   }
   if (verbosity>3) printf("...Comb4Orient4MomentsHistogram_FFT\n");
   FourierTransform.CombFirstFourMoments2D(vec);  
   for (a=0;a<48;a++)
   {  sprintf(buffer,"Comb4Orient4MomentsHistogram_FFT %s",four_moments_names[a]);
      Add(buffer,vec[a]);
   }  
   if (verbosity>3) printf("...Comb4Orient4MomentsHistogram_Wavelet\n");
   WaveletSelector.CombFirstFourMoments2D(vec);
   for (a=0;a<48;a++)
   {  sprintf(buffer,"Comb4Orient4MomentsHistogram_Wavelet %s",four_moments_names[a]);
      Add(buffer,vec[a]);
   }       
   if (verbosity>3) printf("...Comb4Orient4MomentsHistogram_WaveletFFT\n");
   WaveletFourierSelector.CombFirstFourMoments2D(vec);
   for (a=0;a<48;a++)
   {  sprintf(buffer,"Comb4Orient4MomentsHistogram_WaveletFFT %s",four_moments_names[a]);
      Add(buffer,vec[a]);
   }

   if (verbosity>3) printf("...EdgeStatistics\n");
   /* edge statistics (signatures 416 - 443) */
   {  unsigned long EdgeArea;
      double MagMean, MagMedian, MagVar, MagHist[8], DirecMean, DirecMedian, DirecVar, DirecHist[8], DirecHomogeneity, DiffDirecHist[4];
      matrix.EdgeStatistics(&EdgeArea, &MagMean, &MagMedian, &MagVar, MagHist, &DirecMean, &DirecMedian, &DirecVar, DirecHist, &DirecHomogeneity, DiffDirecHist, 8);
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
 
   if (verbosity>3) printf("...FeatureStatistics\n");
   /*feature statistics (signatures 444 - 477) */
	{
		unsigned long count, AreaMin, AreaMax;
		long Euler;
		unsigned int AreaMedian, area_histogram[10], dist_histogram[10];
		double centroid_x, centroid_y, AreaMean, AreaVar, DistMin, DistMax, DistMean, DistMedian, DistVar;

		matrix.FeatureStatistics(&count, &Euler, &centroid_x, &centroid_y, &AreaMin, &AreaMax, &AreaMean, &AreaMedian, &AreaVar, area_histogram, &DistMin, &DistMax,
			&DistMean, &DistMedian, &DistVar, dist_histogram, 10);
		// area histogram
		for (a=0;a<10;a++) {
	      	sprintf(buffer,"Otsu Feature area histogram bin %d",a);
			Add(buffer,area_histogram[a]);
		}
		Add("Otsu Feature AreaMax",AreaMax);
		Add("Otsu Feature AreaMean",AreaMean);
		Add("Otsu Feature AreaMedian",AreaMedian);
		Add("Otsu Feature AreaMin",AreaMin);
		Add("Otsu Feature AreaVar",AreaVar);
		Add("Otsu Feature X Centroid",centroid_x);
		Add("Otsu Feature Y Centroid",centroid_y);
		Add("Otsu Feature Count",count);
		for (a=0;a<10;a++) {
			sprintf(buffer,"Otsu Feature dist histogram bin %d",a);
			Add(buffer,dist_histogram[a]);
		}
		Add("Otsu Feature DistMax",DistMax);
		Add("Otsu Feature DistMean",DistMean);
		Add("Otsu Feature DistMedian",DistMedian);
		Add("Otsu Feature DistMin",DistMin);
		Add("Otsu Feature DistVar",DistVar);
		Add("Otsu Feature Euler",Euler);

		ImageMatrix matrixInv;
		matrixInv.copy (matrix);
		matrixInv.invert();
		matrixInv.FeatureStatistics(&count, &Euler, &centroid_x, &centroid_y, &AreaMin, &AreaMax, &AreaMean, &AreaMedian, &AreaVar, area_histogram, &DistMin, &DistMax,
			&DistMean, &DistMedian, &DistVar, dist_histogram, 10);
		// area histogram
		for (a=0;a<10;a++) {
	      	sprintf(buffer,"Inverse Otsu Feature area histogram bin %d",a);
			Add(buffer,area_histogram[a]);
		}
		Add("Inverse Otsu Feature AreaMax",AreaMax);
		Add("Inverse Otsu Feature AreaMean",AreaMean);
		Add("Inverse Otsu Feature AreaMedian",AreaMedian);
		Add("Inverse Otsu Feature AreaMin",AreaMin);
		Add("Inverse Otsu Feature AreaVar",AreaVar);
		Add("Inverse Otsu Feature X Centroid",centroid_x);
		Add("Inverse Otsu Feature Y Centroid",centroid_y);
		Add("Inverse Otsu Feature Count",count);
		for (a=0;a<10;a++) {
			sprintf(buffer,"Inverse Otsu Feature dist histogram bin %d",a);
			Add(buffer,dist_histogram[a]);
		}
		Add("Inverse Otsu Feature DistMax",DistMax);
		Add("Inverse Otsu Feature DistMean",DistMean);
		Add("Inverse Otsu Feature DistMedian",DistMedian);
		Add("Inverse Otsu Feature DistMin",DistMin);
		Add("Inverse Otsu Feature DistVar",DistVar);
		Add("Inverse Otsu Feature Euler",Euler);
   }  


   if (verbosity>3) printf("...GaborTextures\n");
   /* gabor filters (signatures 478 - 484) */
   matrix.GaborFilters2D(vec);
   for (a=0;a<7;a++)
   {  sprintf(buffer,"GaborTextures Gabor%02d",a+1);
      Add(buffer,vec[a]);
   }

   /* haralick textures (signatures 485 - 652) */
   char haralick_names[80][80]={"CoOcMat_AngularSecondMoment","ASM","CoOcMat_Contrast","Contrast","CoOcMat_Correlation","Correlation","CoOcMat_Variance","Variance","CoOcMat_InverseDifferenceMoment","IDM","CoOcMat_SumAverage" ,"SumAvg",
           "CoOcMat_SumVariance","SumVar","CoOcMat_SumEntropy", "SumEntropy","CoOcMat_Entropy" ,"Entropy","CoOcMat_DifferenceEntropy","DiffEntropy","CoOcMat_DifferenceVariance","DiffVar","CoOcMat_FirstMeasureOfCorrelation","MeasCorr1",
		   "CoOcMat_SecondMeasureOfCorrelation","MeasCorr2","CoOcMat_MaximalCorrelationCoefficient" ,"MaxCorrCoef","CoOcMat_AngularSecondMomentDif", "ASM","CoOcMat_ContrastDif" ,"Contrast","CoOcMat_CorrelationDif","Correlation","CoOcMat_VarianceDif","Variance",
		   "CoOcMat_InverseDifferenceMomentDif","IDM","CoOcMat_SumAverageDif","SumAvg","CoOcMat_SumVarianceDif","SumVar","CoOcMat_SumEntropyDif" ,"SumEntropy","CoOcMat_EntropyDif","Entropy","CoOcMat_DifferenceEntropyDif","DiffEntropy","CoOcMat_DifferenceVarianceDif","DiffVar",
		   "CoOcMat_FirstMeasureOfCorrelationDif","MeasCorr1","CoOcMat_SecondMeasureOfCorrelationDif","MeasCorr2","CoOcMat_MaximalCorrelationCoefficientDif","MaxCorrCoef"};

   if (verbosity>3) printf("...HaralickTextures\n");
   matrix.HaralickTexture2D(0,vec);
   for (a=0;a<28;a++)
   {  sprintf(buffer,"%s %s",haralick_names[a*2],haralick_names[a*2+1]);
      Add(buffer,vec[a]);
   }
   if (verbosity>3) printf("...HaralickTextures_Chebyshev\n");
   ChebyshevTransform.HaralickTexture2D(0,vec);
   for (a=0;a<28;a++)
   {  sprintf(buffer,"%s_Chebyshev %s",haralick_names[a*2],haralick_names[a*2+1]);
      Add(buffer,vec[a]);
   }
   if (verbosity>3) printf("...HaralickTextures_ChebyshevFFT\n");
   ChebyshevFourierTransform.HaralickTexture2D(0,vec);
   for (a=0;a<28;a++)
   {  sprintf(buffer,"%s_ChebyshevFFT %s",haralick_names[a*2],haralick_names[a*2+1]);
      Add(buffer,vec[a]);
   }
   if (verbosity>3) printf("...HaralickTextures_FFT\n");
   FourierTransform.HaralickTexture2D(0,vec);
   for (a=0;a<28;a++)
   {  sprintf(buffer,"%s_FFT %s",haralick_names[a*2],haralick_names[a*2+1]);
      Add(buffer,vec[a]);
   }
   if (verbosity>3) printf("...HaralickTextures_Wavelet\n");
   WaveletSelector.HaralickTexture2D(0,vec);
   for (a=0;a<28;a++)
   {  sprintf(buffer,"%s_Wavelet %s",haralick_names[a*2],haralick_names[a*2+1]);
      Add(buffer,vec[a]);
   }
   if (verbosity>3) printf("...HaralickTextures_WaveletFFT\n");
   WaveletFourierSelector.HaralickTexture2D(0,vec);
   for (a=0;a<28;a++)
   {  sprintf(buffer,"%s_WaveletFFT %s",haralick_names[a*2],haralick_names[a*2+1]);
      Add(buffer,vec[a]);
   }
   /* multiple histogram (signatures 653 - 796) */
   /* ***************************************** */
   /* multiple histogram of the original image (signatures 653 - 676) */
   if (verbosity>3) printf("...MultipleScaleHistograms\n");
   matrix.MultiScaleHistogram(vec);
   b=3;c=3;
   for (a=0;a<24;a++)
   {  if (a==b) b+=(c=c+2);
      sprintf(buffer,"MultipleScaleHistograms TBins%d_Bin%02d",c,a+c-b);
      Add(buffer,vec[a]);
   }
   /* multiple histogram of the chebyshev transform (signatures 677 - 700) */
   if (verbosity>3) printf("...MultipleScaleHistograms_Chebyshev\n");
   ChebyshevTransform.MultiScaleHistogram(vec);
   b=3;c=3;
   for (a=0;a<24;a++)
   {  if (a==b) b+=(c=c+2);
      sprintf(buffer,"MultipleScaleHistograms_Chebyshev TBins%d_Bin%02d",c,a+c-b);
      Add(buffer,vec[a]);
   }
   /* multiple histogram of the Chebushev Fourier transform (signatures 701 - 724) */
   if (verbosity>3) printf("...MultipleScaleHistograms_ChebyshevFFT\n");
   ChebyshevFourierTransform.MultiScaleHistogram(vec);
   b=3;c=3;
   for (a=0;a<24;a++)
   {  if (a==b) b+=(c=c+2);
      sprintf(buffer,"MultipleScaleHistograms_ChebyshevFFT TBins%d_Bin%02d",c,a+c-b);
      Add(buffer,vec[a]);
   }
   /* multiple histogram of the Fourier transform (signatures 725 - 748) */
   if (verbosity>3) printf("...MultipleScaleHistograms_FFT\n");
   FourierTransform.MultiScaleHistogram(vec);
   b=3;c=3;
   for (a=0;a<24;a++)
   {  if (a==b) b+=(c=c+2);
      sprintf(buffer,"MultipleScaleHistograms_FFT TBins%d_Bin%02d",c,a+c-b);
      Add(buffer,vec[a]);
   }
   /* multiple histogram of the wavelet transform (signatures 749 - 772) */
   if (verbosity>3) printf("...MultipleScaleHistograms_Wavelet\n");
   WaveletSelector.MultiScaleHistogram(vec);
   b=3;c=3;
   for (a=0;a<24;a++)
   {  if (a==b) b+=(c=c+2);
      sprintf(buffer,"MultipleScaleHistograms_Wavelet TBins%d_Bin%02d",c,a+c-b);
      Add(buffer,vec[a]);
   }
   /* multiple histogram of the wavelet Fourier transform (signatures 773 - 796) */
   if (verbosity>3) printf("...MultipleScaleHistograms_WaveletFFT\n");
   WaveletFourierSelector.MultiScaleHistogram(vec);
   b=3;c=3;   
   for (a=0;a<24;a++)
   {  if (a==b) b+=(c=c+2);
      sprintf(buffer,"MultipleScaleHistograms_WaveletFFT TBins%d_Bin%02d",c,a+c-b);
      Add(buffer,vec[a]);
   }

   /* radon transform (signatures 797 - 844) */
   if (verbosity>3) printf("...RadonTransformStatistics\n");
   matrix.RadonTransform2D(vec);
   for (a=0;a<12;a++)
   {  sprintf(buffer,"RadonTransformStatistics Orient%d_Bin_%02d",45*(int)(a/3),a % 3);
      Add(buffer,vec[a]);
   }
   if (verbosity>3) printf("...RadonTransformStatistics_Chebyshev\n");
   ChebyshevTransform.RadonTransform2D(vec);
   for (a=0;a<12;a++)
   {  sprintf(buffer,"RadonTransformStatistics_Chebyshev Orient%d_Bin_%02d",45*(int)(a/3),a % 3);
      Add(buffer,vec[a]);
   }
   if (verbosity>3) printf("...RadonTransformStatistics_ChebyshevFFT\n");
   ChebyshevFourierTransform.RadonTransform2D(vec);
   for (a=0;a<12;a++)
   {  sprintf(buffer,"RadonTransformStatistics_ChebyshevFFT Orient%d_Bin_%02d",45*(int)(a/3),a % 3);
      Add(buffer,vec[a]);
   }
   if (verbosity>3) printf("...RadonTransformStatistics_FFT\n");
   FourierTransform.RadonTransform2D(vec);
   for (a=0;a<12;a++)
   {  sprintf(buffer,"RadonTransformStatistics_FFT Orient%d_Bin_%02d",45*(int)(a/3),a % 3);
      Add(buffer,vec[a]);
   }

   char tamura_names[80][80]={"Coarseness_Hist_Bin_00","Coarseness_Hist_Bin_01","Coarseness_Hist_Bin_02","Contrast","Directionality","Total_Coarseness"};
   /* tamura texture (signatures 845 - 880) */
   if (verbosity>3) printf("...TamuraTextures\n");
   matrix.TamuraTexture2D(vec);
   for (a=0;a<6;a++)
   {  sprintf(buffer,"TamuraTextures %s",tamura_names[a]);
      Add(buffer,vec[a]);
   }
   if (verbosity>3) printf("...TamuraTextures_Chebyshev\n");
   ChebyshevTransform.TamuraTexture2D(vec);
   for (a=0;a<6;a++)
   {  sprintf(buffer,"TamuraTextures_Chebyshev %s",tamura_names[a]);
      Add(buffer,vec[a]);
   }
   if (verbosity>3) printf("...TamuraTextures_ChebyshevFFT\n");
   ChebyshevFourierTransform.TamuraTexture2D(vec);
   for (a=0;a<6;a++)
   {  sprintf(buffer,"TamuraTextures_ChebyshevFFT %s",tamura_names[a]);
      Add(buffer,vec[a]);
   }
   if (verbosity>3) printf("...TamuraTextures_FFT\n");
   FourierTransform.TamuraTexture2D(vec);
   for (a=0;a<6;a++)
   {  sprintf(buffer,"TamuraTextures_FFT %s",tamura_names[a]);
      Add(buffer,vec[a]);
   }
   if (verbosity>3) printf("...TamuraTextures_Wavelet\n");
   WaveletSelector.TamuraTexture2D(vec);
   for (a=0;a<6;a++)
   {  sprintf(buffer,"TamuraTextures_Wavelet %s",tamura_names[a]);
      Add(buffer,vec[a]);
   }
   if (verbosity>3) printf("...TamuraTextures_WaveletFFT\n");
   WaveletFourierSelector.TamuraTexture2D(vec);
   for (a=0;a<6;a++)
   {  sprintf(buffer,"TamuraTextures_WaveletFFT %s",tamura_names[a]);
      Add(buffer,vec[a]);
   }

   if (verbosity>3) printf("...ZernikeMoments\n");
   /* zernike (signatures 881 - 1024) */
   { long x,y,output_size;   /* output size is normally 72 */
     matrix.zernike2D(vec,&output_size);
     x=0;y=0;
     for (a=0;a<output_size;a++)
     {  sprintf(buffer,"ZernikeMoments Z_%02d_%02d",(int)y,(int)x);
        Add(buffer,vec[a]);
        if (x>=y) x=1-(y++ % 2);
        else x+=2;
     }
    x=0;y=0;
   if (verbosity>3) printf("...ZernikeMoments_FFT\n");
    FourierTransform.zernike2D(vec,&output_size);
    for (a=0;a<output_size;a++)
    {  sprintf(buffer,"ZernikeMoments_FFT Z_%02d_%02d",(int)y,(int)x);
       Add(buffer,vec[a]);
       if (x>=y) x=1-(y++ % 2);
       else x+=2;
     }
   }

	if (compute_colors) {
		if (verbosity>3) printf("...ColorFeatures\n");
		CompGroupD(matrix,"");
		feature_vec_type = fv_short_color;
	} else {
		feature_vec_type = fv_short;
	}

   return;
}


/* CompGroupA
   compute group A of image feature (high contrast features)
   the features in this group are edge statistics, object statistics and Gabor textures
   input - an image matrix structure.
         - transform_label - the image transform short description (e.g., wavelet-fourier)
*/
void signatures::CompGroupA(ImageMatrix &matrix, const char *transform_label)
{  int a;
   char buffer[80];
   double vec[7]={0,0,0,0,0,0,0};
   /* edge statistics */
   {  unsigned long EdgeArea=0;
      double MagMean=0, MagMedian=0, MagVar=0, MagHist[8]={0,0,0,0,0,0,0,0}, DirecMean=0, DirecMedian=0, DirecVar=0, DirecHist[8]={0,0,0,0,0,0,0,0}, DirecHomogeneity=0, DiffDirecHist[4]={0,0,0,0};

      if (IsNeeded(count,28))  /* check if this group of signatures is needed */
        matrix.EdgeStatistics(&EdgeArea, &MagMean, &MagMedian, &MagVar, MagHist, &DirecMean, &DirecMedian, &DirecVar, DirecHist, &DirecHomogeneity, DiffDirecHist, 8);

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
   {
		unsigned long count, AreaMin, AreaMax;
		long Euler;
		unsigned int AreaMedian, area_histogram[10], dist_histogram[10];
		double centroid_x, centroid_y, AreaMean, AreaVar, DistMin, DistMax, DistMean, DistMedian, DistVar;

		matrix.FeatureStatistics(&count, &Euler, &centroid_x, &centroid_y, &AreaMin, &AreaMax, &AreaMean, &AreaMedian, &AreaVar, area_histogram, &DistMin, &DistMax,
			&DistMean, &DistMedian, &DistVar, dist_histogram, 10);
		for (a=0;a<10;a++) { /* area histogram */
	      	sprintf(buffer,"Otsu Feature area histogram bin %d",a);
			Add(buffer,area_histogram[a]);
		}
		Add("Otsu Feature AreaMax",AreaMax);
		Add("Otsu Feature AreaMean",AreaMean);
		Add("Otsu Feature AreaMedian",AreaMedian);
		Add("Otsu Feature AreaMin",AreaMin);
		Add("Otsu Feature AreaVar",AreaVar);
		Add("Otsu Feature X Centroid",centroid_x);
		Add("Otsu Feature Y Centroid",centroid_y);
		Add("Otsu Feature Count",count);
		for (a=0;a<10;a++) {
			sprintf(buffer,"Otsu Feature dist histogram bin %d",a);
			Add(buffer,dist_histogram[a]);
		}
		Add("Otsu Feature DistMax",DistMax);
		Add("Otsu Feature DistMean",DistMean);
		Add("Otsu Feature DistMedian",DistMedian);
		Add("Otsu Feature DistMin",DistMin);
		Add("Otsu Feature DistVar",DistVar);
		Add("Otsu Feature Euler",Euler);

		ImageMatrix matrixInv;
		matrixInv.copy (matrix);
		matrixInv.invert();
		matrixInv.FeatureStatistics(&count, &Euler, &centroid_x, &centroid_y, &AreaMin, &AreaMax, &AreaMean, &AreaMedian, &AreaVar, area_histogram, &DistMin, &DistMax,
			&DistMean, &DistMedian, &DistVar, dist_histogram, 10);
		for (a=0;a<10;a++) { /* area histogram */
	      	sprintf(buffer,"Inverse Otsu Feature area histogram bin %d",a);
			Add(buffer,area_histogram[a]);
		}
		Add("Inverse Otsu Feature AreaMax",AreaMax);
		Add("Inverse Otsu Feature AreaMean",AreaMean);
		Add("Inverse Otsu Feature AreaMedian",AreaMedian);
		Add("Inverse Otsu Feature AreaMin",AreaMin);
		Add("Inverse Otsu Feature AreaVar",AreaVar);
		Add("Inverse Otsu Feature X Centroid",centroid_x);
		Add("Inverse Otsu Feature Y Centroid",centroid_y);
		Add("Inverse Otsu Feature Count",count);
		for (a=0;a<10;a++) {
			sprintf(buffer,"Inverse Otsu Feature dist histogram bin %d",a);
			Add(buffer,dist_histogram[a]);
		}
		Add("Inverse Otsu Feature DistMax",DistMax);
		Add("Inverse Otsu Feature DistMean",DistMean);
		Add("Inverse Otsu Feature DistMedian",DistMedian);
		Add("Inverse Otsu Feature DistMin",DistMin);
		Add("Inverse Otsu Feature DistVar",DistVar);
		Add("Inverse Otsu Feature Euler",Euler);
   }

   /* gabor filters */
   if (IsNeeded(count,7))
     matrix.GaborFilters2D(vec);
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
void signatures::CompGroupB(ImageMatrix &matrix, const char *transform_label)
{  int a;
   double vec[72];
   char buffer[80];
   ImageMatrix TempMatrix;

   /* chebyshev fourier transform (signatures 0 - 63) */
   for (a=0;a<72;a++) vec[a]=0;
   if (IsNeeded(count,32))
     matrix.ChebyshevFourierTransform2D(vec);
   for (a=0;a<32;a++)
   {  sprintf(buffer,"Chebyshev Fourier Transform bin %d (%s)",a,transform_label);
      Add(buffer,vec[a]);
   }

   /* Chebyshev Statistics (signatures 64 - 127) */
   TempMatrix.copy (matrix);
   if (IsNeeded(count,32))
     TempMatrix.ChebyshevStatistics2D(vec,0,32);
   for (a=0;a<32;a++)
   {  sprintf(buffer,"Chebyshev Statistics bin %d (%s)",a,transform_label);
      Add(buffer,vec[a]);
   }

   /* zernike (signatures 881 - 1024) */
   long output_size;   /* output size is normally 72 */
   if (IsNeeded(count,72))
     matrix.zernike2D(vec,&output_size);
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
void signatures::CompGroupC(ImageMatrix &matrix, const char *transform_label)
{  int a;
   double vec[48];
   char buffer[80];
   double mean=0, median=0, std=0, min=0, max=0;

   for (a=0;a<48;a++) vec[a]=0;
   /* Comb4Moments */
   if (IsNeeded(count,48))
     matrix.CombFirstFourMoments2D(vec);
   for (a=0;a<48;a++)
   {  sprintf(buffer,"CombFirstFourMoments %d (%s)",a,transform_label);
      Add(buffer,vec[a]);
   }

   /* haralick textures */
   if (IsNeeded(count,28))
     matrix.HaralickTexture2D(0,vec);
   for (a=0;a<28;a++)
   {  sprintf(buffer,"Haralick Texture %d (%s)",a,transform_label);
      Add(buffer,vec[a]);
   }

   /* multiscale histogram of the original image */
   if (IsNeeded(count,24))
     matrix.MultiScaleHistogram(vec);
   for (a=0;a<24;a++)
   {  sprintf(buffer,"MultiScale Histogram bin %d (%s)",a,transform_label);
      Add(buffer,vec[a]);
   }

   /* tamura texture */
   if (IsNeeded(count,6))
     matrix.TamuraTexture2D(vec);
   for (a=0;a<6;a++)
   {  sprintf(buffer,"Tamura Texture %d (%s)",a,transform_label);
      Add(buffer,vec[a]);
   }

   /* radon transform */
   if (IsNeeded(count,12))
     matrix.RadonTransform2D(vec);
   for (a=0;a<12;a++)
   {  sprintf(buffer,"Radon bin %d (%s)",a,transform_label);
      Add(buffer,vec[a]);
   }

   /* fractal features */
   if (IsNeeded(count,20))
     matrix.fractal2D(20,vec);
   for (a=0;a<20;a++)
   {  sprintf(buffer,"Fractal %d (%s)",a,transform_label);
      Add(buffer,vec[a]);
   }
   
   /* basic statistics */
   if (IsNeeded(count,5))
     matrix.BasicStatistics(&mean, &median, &std, &min, &max, NULL, 10);
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
void signatures::CompGroupD(ImageMatrix &matrix, const char *transform_label)
{  int color_index;
   char buffer[80];
   double color_hist[COLORS_NUM+1];

   ImageMatrix ColorTransformMatrix,HueTransformMatrix,HueFFT,HueChebyshev;

   ColorTransformMatrix.copy (matrix);
   ColorTransformMatrix.ColorTransform(color_hist,0);
   HueTransformMatrix.copy (matrix);
   HueTransformMatrix.ColorTransform(NULL,1);
   HueFFT.copy (HueTransformMatrix);
   HueFFT.fft2();
   HueChebyshev.copy (HueTransformMatrix);
   HueChebyshev.ChebyshevTransform(0);

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
}

/* ComputeGroups
   compute the image features
   input - an image matrix structure.
*/
void signatures::ComputeGroups(ImageMatrix &matrix, int compute_colors)
{
  ImageMatrix FourierTransform,ChebyshevTransform,ChebyshevFourierTransform,WaveletSelector,FourierWaveletSelector;
  ImageMatrix FourierChebyshev,WaveletFourier,ChebyshevWavelet, EdgeTransform, EdgeFourier, EdgeWavelet;

	// Set the feature version
	version = CURRENT_FEATURE_VERSION;

  count=0;      /* start counting signatures from 0 */

  FourierTransform.copy (matrix);
  FourierTransform.fft2();
  ChebyshevTransform.copy (matrix);
  ChebyshevTransform.ChebyshevTransform(0);
  ChebyshevFourierTransform.copy (FourierTransform);
  ChebyshevFourierTransform.ChebyshevTransform(0);
  WaveletSelector.copy (matrix);
  WaveletSelector.Symlet5Transform();
  FourierWaveletSelector.copy (FourierTransform);
  FourierWaveletSelector.Symlet5Transform();
  FourierChebyshev.copy (ChebyshevTransform);
  FourierChebyshev.fft2();
  WaveletFourier.copy (WaveletSelector);
  WaveletFourier.fft2();
  ChebyshevWavelet.copy (WaveletSelector);
  ChebyshevWavelet.ChebyshevTransform(0);
  EdgeTransform.copy (matrix);
  EdgeTransform.EdgeTransform();
  EdgeFourier.copy (EdgeTransform);
  EdgeFourier.fft2();
  EdgeWavelet.copy (EdgeTransform);  
  EdgeWavelet.Symlet5Transform();


  CompGroupA(matrix,"");
  CompGroupB(matrix,"");
  CompGroupC(matrix,"");
	if (compute_colors) {
		CompGroupD(matrix,"");
		feature_vec_type = fv_long_color;
	} else {
		feature_vec_type = fv_long;
	}

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
	double sig_val, sig_min, sig_max;
	for( sig_index = 0; sig_index < count; sig_index++ ) {
		sig_val = data[ sig_index ].value;
		sig_min = ts->SignatureMins[ sig_index ];
		sig_max = ts->SignatureMaxes[ sig_index ];

		if (std::isnan(sig_val) || sig_val < sig_min || (sig_max - sig_min) < DBL_EPSILON) sig_val = 0; 
		else if( sig_val > sig_max )
			sig_val = 100;
		else
			sig_val = 100 * ( (sig_val - sig_min) / (sig_max - sig_min) );

		data[ sig_index ].value = sig_val;
	}
}


/* FileClose
   Closes a value file.  This is the closing command for files opened with ReadFromFile.
   This closes the stream as well as filedescriptor
*/
void signatures::FileClose()
{
	if (wf) {
		wf->finish();
	}
}

int signatures::SaveToFile (int save_feature_names) {
	int sig_index;
	char buffer[IMAGE_PATH_LENGTH+SAMPLE_NAME_LENGTH+1];

	if (!wf) {
		if (strlen (full_path) > 0)
			wf = new WORMfile (GetFileName (buffer));
		else {
			fprintf(stderr, "Cannot write to .sig file - full_path not set.\n");
			return(0);
		}
	}

	if (!wf || !(wf->status == WORMfile::WORM_WR) ) {
		printf("Cannot write to .sig file: cannot open sigfile for writing\n");
		return(0);
	}
	FILE *wf_fp = wf->fp();

	if ( NamesTrainingSet && ((TrainingSet *)(NamesTrainingSet))->is_continuous ) {
		fprintf(wf_fp,"%f\t%d.%d\n",sample_value,version,feature_vec_type);  /* save the continouos value */
	} else {
		fprintf(wf_fp,"%d\t%d.%d\n",sample_class,version,feature_vec_type);  /* save the class index */
	}
	fprintf(wf_fp,"%s\n",full_path);
	for (sig_index=0; sig_index < count; sig_index++) {
		if (save_feature_names && NamesTrainingSet)
			fprintf(wf_fp,"%f %s\n",data[sig_index].value,((TrainingSet *)NamesTrainingSet)->SignatureNames[sig_index]);
		else
			fprintf(wf_fp,"%f\n",data[sig_index].value);
	}
   return(1);
}


int signatures::LoadFromFile(char *filename) {
	char buffer[IMAGE_PATH_LENGTH+SAMPLE_NAME_LENGTH+1];
	WORMfile *wf_temp = NULL;
	int ret = 0;

	if (!filename || *filename == '\0')
		GetFileName (buffer);
	else strncpy (buffer,filename,sizeof(buffer));

	wf_temp = new WORMfile (buffer, true); // readonly
	if (wf_temp->status == WORMfile::WORM_RD) {
		LoadFromFilep (wf_temp->fp());
		ret = 1;
	}
	delete wf_temp;  // closes readonly, unlinks write-locked.
	return (ret);
}

void signatures::LoadFromFilep (FILE *value_file) {
	char buffer[IMAGE_PATH_LENGTH+SAMPLE_NAME_LENGTH+1],*p_buffer;
	int version_maj = 0, version_min = 0;

	/* read the class or value and version */
	fgets(buffer,sizeof(buffer),value_file);
	if (NamesTrainingSet && ((TrainingSet *)(NamesTrainingSet))->is_continuous) {
		sscanf (buffer, "%lf%*[\t ]%d.%d", &sample_value, &version_maj, &version_min);
		sample_class = 1;
	} else {
		sscanf (buffer, "%hu%*[\t ]%d.%d", &sample_class, &version_maj, &version_min);
	}
	// If we did not read a version, then it is 1.0
	if (version_maj == 0) {
		version = 1;
		feature_vec_type = fv_unknown;
	} else {
		version = version_maj;
		feature_vec_type = version_min;
	}
	/* read the path */
	fgets(buffer,sizeof(buffer),value_file);
	chomp (buffer);
	strcpy(full_path,buffer);
	
	/* read the feature values */
	p_buffer=fgets(buffer,sizeof(buffer),value_file);
	chomp (p_buffer);
	while (p_buffer) {
		char *p_name;
		p_name=strchr(buffer,' ');
		if (p_name) {    /* if there is a feature name in the file */
			*p_name='\0';
			p_name++;
		}
		Add(p_name,atof(buffer));
		p_buffer=fgets(buffer,sizeof(buffer),value_file);
		chomp (p_buffer);
	}

	// FIXME: There is opportunity here to check for inconsistent number of features if minor version is specified.
	SetFeatureVectorType();
}



/*
  Yet another variant of reading from a file.
  The filename is computed from full_path and sample_name using GetFileName
  If the file is successfully opened and write-locked, return 0 (wf.status = WORMfile::WORM_WR).
  If another process has a lock, return 0 (wf.status = WORMfile::WORM_BUSY).
  If the file exists, and is not locked, the sigs will be loaded from it, and no lock will be issued. (return 1, (wf.status = WORMfile::WORM_FINISHED))
  If an error occurs in obtaining the lock (if necessary) or creating the file (if necessary) or reading it (if possible), return -1.
*/
int signatures::ReadFromFile (bool wait) {
	char buffer[IMAGE_PATH_LENGTH+SAMPLE_NAME_LENGTH+1];

	if (!wf) wf = new WORMfile (GetFileName (buffer), wait, wait);
	else wf->reopen(wait, wait);

	if (!wf) return (-1);
	if (wf->status == WORMfile::WORM_BUSY) {
		return (0);
	} else if (wf->status == WORMfile::WORM_WR) {
		return (0);
	} else if (wf->status == WORMfile::WORM_RD) {
		Clear(); // reset sample count
		LoadFromFilep (wf->fp());
		wf->finish(); // this unlocks, closes, etc.
		// Of course, if it was empty afterall, its an error.
		if (count < 1) {
			return (NO_SIGS_IN_FILE);
		} else {
			return (1);
		}
	} else {
	// I/O error
		return (-1);
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

	if (wf) {
		strcpy (buffer, wf->path.c_str());
		return buffer;
	}

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
/*
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
*/

/*
  This function is used to determine (somewhat quickly) if the features stored in a file match those
  that would be calculated from the passed-in matrix.
  A partial signature calculation is done to determine the match.  An exact match (within FLT_EPSILON) of every feature is required to return 1.
  If the file can't be opened, or if the match is inexact, 0 is returned.
*/
int signatures::CompareToFile (ImageMatrix &matrix, char *filename, int compute_colors, int large_set) {
	signatures file_sigs;
	double vec[72];
	int i,file_index;

	if (! file_sigs.LoadFromFile (filename) ) return (0);
	if (verbosity>=2) printf ("compare %s to computed\n",filename);

	// 20 features long: 323-342, standard: N/A
	if (large_set) {
		matrix.fractal2D(20,vec);
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
	matrix.HaralickTexture2D(0,vec);
	if (large_set) file_index = 253;
	else file_index = 485;
	for (i = 0; i < 28; i++) if (!OUR_EQ(file_sigs.data[file_index+i].value,vec[i])) {
		if (verbosity>=2) printf ("HaralickTexture2D mismatch computed %15.10f\tfrom file: %15.10f\n",vec[i],file_sigs.data[file_index+i].value);
		return (0);
	}
	if (verbosity>=2) printf ("HaralickTexture2D match\n");
	
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


