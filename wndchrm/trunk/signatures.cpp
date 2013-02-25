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

// void signatures::AddVector(const std::string &name, const std::string &transform, const std::vector<double> &vec) {
// 	char featuregroup[80], featurename[80];
// 	sprintf (featuregroup, "%s (%s)", name.c_str(), transform.c_str());
// 	for (std::vector<double>::size_type idx = 0; idx < vec.size(); idx++) {
// 		sprintf (featurename, "%s [%u]", featuregroup, (unsigned int)idx);
// 		Add (featurename, vec[idx]);
// 	}
// }
void signatures::AddVector(const FeatureNames::FeatureGroup *fg, const std::vector<double> &vec) {
	for (std::vector<double>::size_type idx = 0; idx < vec.size(); idx++) {
		Add (fg->labels[idx].c_str(), vec[idx]);
	}
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

void signatures::compute(ImageMatrix &matrix, int compute_colors) {
	ImageMatrix FourierTransform,ChebyshevTransform,ChebyshevFourierTransform,WaveletSelector,WaveletFourierSelector;

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

	count=0;      // start counting signatures from 0
	// chebyshev fourier transform (signatures 0 - 63)
	const FeatureNames::FeatureGroup *fg;
	fg = FeatureNames::getGroupByName ("Chebyshev-Fourier Coefficients ()");
	AddVector (fg, fg->algorithm->calculate (&matrix));
	fg = FeatureNames::getGroupByName ("Chebyshev-Fourier Coefficients (Fourier ())");
	AddVector (fg, fg->algorithm->calculate (&FourierTransform));

	// Chebyshev Statistics (signatures 64 - 127) */
	fg = FeatureNames::getGroupByName ("Chebyshev Coefficients ()");
	AddVector (fg, fg->algorithm->calculate (&matrix));
	fg = FeatureNames::getGroupByName ("Chebyshev Coefficients (Fourier ())");
	AddVector (fg, fg->algorithm->calculate (&FourierTransform));

	// Comb4Moments (signatures 128 - 415)
	fg = FeatureNames::getGroupByName ("Comb Moments ()");
	AddVector (fg, fg->algorithm->calculate (&matrix));
	fg = FeatureNames::getGroupByName ("Comb Moments (Chebyshev ())");
	AddVector (fg, fg->algorithm->calculate (&ChebyshevTransform));
	fg = FeatureNames::getGroupByName ("Comb Moments (Chebyshev (Fourier ()))");
	AddVector (fg, fg->algorithm->calculate (&ChebyshevFourierTransform));
	fg = FeatureNames::getGroupByName ("Comb Moments (Fourier ())");
	AddVector (fg, fg->algorithm->calculate (&FourierTransform));
	fg = FeatureNames::getGroupByName ("Comb Moments (Wavelet ())");
	AddVector (fg, fg->algorithm->calculate (&WaveletSelector));
	fg = FeatureNames::getGroupByName ("Comb Moments (Wavelet (Fourier ()))");
	AddVector (fg, fg->algorithm->calculate (&WaveletFourierSelector));

	// edge statistics (signatures 416 - 443)
	fg = FeatureNames::getGroupByName ("Edge Features ()");
	AddVector (fg, fg->algorithm->calculate (&matrix));
 
	// feature statistics (signatures 444 - 477)
	fg = FeatureNames::getGroupByName ("Otsu Object Features ()");
	AddVector (fg, fg->algorithm->calculate (&matrix));
	fg = FeatureNames::getGroupByName ("Inverse-Otsu Object Features ()");
	AddVector (fg, fg->algorithm->calculate (&matrix));

	// gabor filters (signatures 478 - 484)
	fg = FeatureNames::getGroupByName ("Gabor Textures ()");
	AddVector (fg, fg->algorithm->calculate (&matrix));

	// haralick textures (signatures 485 - 652)
	fg = FeatureNames::getGroupByName ("Haralick Textures ()");
	AddVector (fg, fg->algorithm->calculate (&matrix));
	fg = FeatureNames::getGroupByName ("Haralick Textures (Chebyshev ())");
	AddVector (fg, fg->algorithm->calculate (&ChebyshevTransform));
	fg = FeatureNames::getGroupByName ("Haralick Textures (Chebyshev (Fourier ()))");
	AddVector (fg, fg->algorithm->calculate (&ChebyshevFourierTransform));
	fg = FeatureNames::getGroupByName ("Haralick Textures (Fourier ())");
	AddVector (fg, fg->algorithm->calculate (&FourierTransform));
	fg = FeatureNames::getGroupByName ("Haralick Textures (Wavelet ())");
	AddVector (fg, fg->algorithm->calculate (&WaveletSelector));
	fg = FeatureNames::getGroupByName ("Haralick Textures (Wavelet (Fourier ()))");
	AddVector (fg, fg->algorithm->calculate (&WaveletFourierSelector));

	// multi-scale histograms (signatures 653 - 796)
	fg = FeatureNames::getGroupByName ("Multiscale Histograms ()");
	AddVector (fg, fg->algorithm->calculate (&matrix));
	fg = FeatureNames::getGroupByName ("Multiscale Histograms (Chebyshev ())");
	AddVector (fg, fg->algorithm->calculate (&ChebyshevTransform));
	fg = FeatureNames::getGroupByName ("Multiscale Histograms (Chebyshev (Fourier ()))");
	AddVector (fg, fg->algorithm->calculate (&ChebyshevFourierTransform));
	fg = FeatureNames::getGroupByName ("Multiscale Histograms (Fourier ())");
	AddVector (fg, fg->algorithm->calculate (&FourierTransform));
	fg = FeatureNames::getGroupByName ("Multiscale Histograms (Wavelet ())");
	AddVector (fg, fg->algorithm->calculate (&WaveletSelector));
	fg = FeatureNames::getGroupByName ("Multiscale Histograms (Wavelet (Fourier ()))");
	AddVector (fg, fg->algorithm->calculate (&WaveletFourierSelector));

	// radon transform (signatures 797 - 844)
	fg = FeatureNames::getGroupByName ("Radon Coefficients ()");
	AddVector (fg, fg->algorithm->calculate (&matrix));
	fg = FeatureNames::getGroupByName ("Radon Coefficients (Chebyshev ())");
	AddVector (fg, fg->algorithm->calculate (&ChebyshevTransform));
	fg = FeatureNames::getGroupByName ("Radon Coefficients (Chebyshev (Fourier ()))");
	AddVector (fg, fg->algorithm->calculate (&ChebyshevFourierTransform));
	fg = FeatureNames::getGroupByName ("Radon Coefficients (Fourier ())");
	AddVector (fg, fg->algorithm->calculate (&FourierTransform));

	// tamura textures (signatures 845 - 880)
	fg = FeatureNames::getGroupByName ("Tamura Textures ()");
	AddVector (fg, fg->algorithm->calculate (&matrix));
	fg = FeatureNames::getGroupByName ("Tamura Textures (Chebyshev ())");
	AddVector (fg, fg->algorithm->calculate (&ChebyshevTransform));
	fg = FeatureNames::getGroupByName ("Tamura Textures (Chebyshev (Fourier ()))");
	AddVector (fg, fg->algorithm->calculate (&ChebyshevFourierTransform));
	fg = FeatureNames::getGroupByName ("Tamura Textures (Fourier ())");
	AddVector (fg, fg->algorithm->calculate (&FourierTransform));
	fg = FeatureNames::getGroupByName ("Tamura Textures (Wavelet ())");
	AddVector (fg, fg->algorithm->calculate (&WaveletSelector));
	fg = FeatureNames::getGroupByName ("Tamura Textures (Wavelet (Fourier ()))");
	AddVector (fg, fg->algorithm->calculate (&WaveletFourierSelector));


	// zernike (signatures 881 - 1024)
	fg = FeatureNames::getGroupByName ("Zernike Coefficients ()");
	AddVector (fg, fg->algorithm->calculate (&WaveletFourierSelector));
	fg = FeatureNames::getGroupByName ("Zernike Coefficients (Fourier ())");
	AddVector (fg, fg->algorithm->calculate (&FourierTransform));

	if (compute_colors) {
		if (verbosity>3) printf("...ColorFeatures\n");
		CompGroupD(matrix);
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
void signatures::CompGroupA (ImageMatrix &matrix, const std::string &transform_label) {
	std::string featureGroupName;	
	const FeatureNames::FeatureGroup *fg;

	// edge statistics
	fg = FeatureNames::getGroupByName (featureGroupName = "Edge Features " + transform_label);
	AddVector (fg, fg->algorithm->calculate (&matrix));

	// object statistics
	fg = FeatureNames::getGroupByName (featureGroupName = "Otsu Object Features " + transform_label);
	AddVector (fg, fg->algorithm->calculate (&matrix));
	fg = FeatureNames::getGroupByName ("Inverse-Otsu Object Features " + transform_label);
	AddVector (fg, fg->algorithm->calculate (&matrix));

	// gabor filters
	fg = FeatureNames::getGroupByName ("Gabor Textures " + transform_label);
	AddVector (fg, fg->algorithm->calculate (&matrix));
}

/* CompGroupB
   compute group B of image feature (Polynomial Decompositions)
   the features in this group are Chebyshev-Fourier Statistics, Chebyshev Statistics, Zernike Polynomials
   input - an image matrix structure.
         - transform_label - the image transform short description (e.g., wavelet-fourier)
*/
void signatures::CompGroupB (ImageMatrix &matrix, const std::string &transform_label) {
	std::string featureGroupName;	
	const FeatureNames::FeatureGroup *fg;

	// chebyshev fourier coefficients
	fg = FeatureNames::getGroupByName ("Chebyshev-Fourier Coefficients " + transform_label);
	AddVector (fg, fg->algorithm->calculate (&matrix));

	// chebyshev coefficients
	fg = FeatureNames::getGroupByName ("Chebyshev Coefficients " + transform_label);
	AddVector (fg, fg->algorithm->calculate (&matrix));

	// zernike
	fg = FeatureNames::getGroupByName ("Zernike Coefficients " + transform_label);
	AddVector (fg, fg->algorithm->calculate (&matrix));
}

/* CompGroupC
   compute group C of image feature (Statistics and Textures)
   the features in this group are First Four Moments, Haralick Textures, Multiscale Histogram, Tamura Textures, Radon Transform Statistics
   input - an image matrix structure.
         - transform_label - the image transform short description (e.g., wavelet-fourier)
*/
void signatures::CompGroupC (ImageMatrix &matrix, const std::string &transform_label) {
	std::string featureGroupName;	
	const FeatureNames::FeatureGroup *fg;

	// Comb4Moments
	fg = FeatureNames::getGroupByName ("Comb Moments " + transform_label);
	AddVector (fg, fg->algorithm->calculate (&matrix));

	// haralick textures
	fg = FeatureNames::getGroupByName ("Haralick Textures " + transform_label);
	AddVector (fg, fg->algorithm->calculate (&matrix));


	// multiscale histogram of the original image
	fg = FeatureNames::getGroupByName ("Multiscale Histograms " + transform_label);
	AddVector (fg, fg->algorithm->calculate (&matrix));

	// tamura texture
	fg = FeatureNames::getGroupByName ("Tamura Textures " + transform_label);
	AddVector (fg, fg->algorithm->calculate (&matrix));

	// radon transform
	fg = FeatureNames::getGroupByName ("Radon Coefficients " + transform_label);
	AddVector (fg, fg->algorithm->calculate (&matrix));

	// fractal features
	fg = FeatureNames::getGroupByName ("Fractal Features " + transform_label);
	AddVector (fg, fg->algorithm->calculate (&matrix));
   
	// basic statistics
	fg = FeatureNames::getGroupByName ("Pixel Intensity Statistics " + transform_label);
	AddVector (fg, fg->algorithm->calculate (&matrix));
   
	// Gini Coefficient
	fg = FeatureNames::getGroupByName ("Gini Coefficient " + transform_label);
	AddVector (fg, fg->algorithm->calculate (&matrix));
}

/* CompGroupD
   compute group D of image feature (color features)
   the features in this group are HSV statistics, color histogram
   input - an image matrix structure.
         - transform_label - the image transform short description (e.g., wavelet-fourier)
*/
void signatures::CompGroupD (ImageMatrix &matrix) {
	std::string featureGroupName;	
	const FeatureNames::FeatureGroup *fg;

	ImageMatrix ColorTransformMatrix,HueTransformMatrix,HueFFT,HueChebyshev;

	ColorTransformMatrix.copy (matrix);
	ColorTransformMatrix.ColorTransform();
	HueTransformMatrix.copy (matrix);
	HueTransformMatrix.HueTransform();
	HueFFT.copy (HueTransformMatrix);
	HueFFT.fft2();
	HueChebyshev.copy (HueTransformMatrix);
	HueChebyshev.ChebyshevTransform(0);

	// color histogram
	fg = FeatureNames::getGroupByName ("Color Histogram ()");
	AddVector (fg, fg->algorithm->calculate (&matrix));

   /* now compute the groups */
//   CompGroupA(ColorTransformMatrix,"Color Transform");
	CompGroupB(ColorTransformMatrix,"(Color Transform ())");
	CompGroupC(ColorTransformMatrix,"(Color Transform ())");

	CompGroupB(HueTransformMatrix,"(Hue ())");
	CompGroupC(HueTransformMatrix,"(Hue ())");

	CompGroupB(HueFFT,"(Fourier (Hue ()))");
	CompGroupC(HueFFT,"(Fourier (Hue ()))");

	CompGroupB(HueChebyshev,"(Chebyshev (Hue ()))");
	CompGroupC(HueChebyshev,"(Chebyshev (Hue ()))");
}

/* ComputeGroups
   compute the image features
   input - an image matrix structure.
*/
void signatures::ComputeGroups(ImageMatrix &matrix, int compute_colors) {
	ImageMatrix Fourier,Chebyshev,ChebyshevFourier,Wavelet,FourierWavelet;
	ImageMatrix FourierChebyshev,WaveletFourier,ChebyshevWavelet, Edge, FourierEdge, WaveletEdge;

	// Set the feature version
	version = CURRENT_FEATURE_VERSION;

	count=0;      /* start counting signatures from 0 */

	Fourier.copy (matrix);
	Fourier.fft2();
	Chebyshev.copy (matrix);
	Chebyshev.ChebyshevTransform(0);
	ChebyshevFourier.copy (Fourier);
	ChebyshevFourier.ChebyshevTransform(0);
	Wavelet.copy (matrix);
	Wavelet.Symlet5Transform();
	Edge.copy (matrix);
	Edge.EdgeTransform();

	WaveletFourier.copy (Fourier);
	WaveletFourier.Symlet5Transform();
	FourierWavelet.copy (Wavelet);
	FourierWavelet.fft2();
	FourierChebyshev.copy (Chebyshev);
	FourierChebyshev.fft2();
	ChebyshevWavelet.copy (Wavelet);
	ChebyshevWavelet.ChebyshevTransform(0);
	FourierEdge.copy (Edge);
	FourierEdge.fft2();
	WaveletEdge.copy (Edge);  
	WaveletEdge.Symlet5Transform();


	CompGroupA(matrix,"()");
	CompGroupB(matrix,"()");
	CompGroupC(matrix,"()");
	if (compute_colors) {
		CompGroupD(matrix);
		feature_vec_type = fv_long_color;
	} else {
		feature_vec_type = fv_long;
	}

	CompGroupB(Fourier,"(Fourier ())");
	CompGroupC(Fourier,"(Fourier ())");

	CompGroupB(Wavelet,"(Wavelet ())");
	CompGroupC(Wavelet,"(Wavelet ())");

	CompGroupB(Chebyshev,"(Chebyshev ())");
	CompGroupC(Chebyshev,"(Chebyshev ())");

	// Fourier, then Chebyshev
	CompGroupC(ChebyshevFourier,"(Chebyshev (Fourier ()))");

	// Fourier, then Wavelet
	CompGroupC(WaveletFourier,"(Wavelet (Fourier ()))");

	// Wavelet, then Fourier
	CompGroupB(FourierWavelet,"(Fourier (Wavelet ()))");
	CompGroupC(FourierWavelet,"(Fourier (Wavelet ()))");

	// Chebyshev, then Fourier
	CompGroupC(FourierChebyshev,"(Fourier (Chebyshev ()))");

	// Wavelet, then Chebyshev
	CompGroupC(ChebyshevWavelet,"(Chebyshev (Wavelet ()))");

	// Edge
	CompGroupB(Edge,"(Edge ())");
	CompGroupC(Edge,"(Edge ())");

	// Edge, then Fourier
	CompGroupB(FourierEdge,"(Fourier (Edge ()))");
	CompGroupC(FourierEdge,"(Fourier (Edge ()))");

	// Edge, then wavelet
	CompGroupB(WaveletEdge,"(Wavelet (Edge ()))");
	CompGroupC(WaveletEdge,"(Wavelet (Edge ()))");
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


