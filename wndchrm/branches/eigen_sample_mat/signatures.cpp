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
#define DEBUG 1

#ifdef WIN32
#pragma hdrstop
#endif
#include "TrainingSet.h"
#include <map> // only a standard map will take a vector<Transform*> as a key
#include <iostream>
#include <string>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h> // for locking stuff
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <unistd.h> // apparently, for close() only?
#include <stdlib.h> // for exit(), used for debug, comment out if release build
#include <math.h>
#include <cfloat> // Has definition of DBL_EPSILON, FLT_EPSILON
#include <fstream>
#define OUR_EPSILON FLT_EPSILON*6
#define FLOAT_EQ(x,v) (((v - FLT_EPSILON) < x) && (x <( v + FLT_EPSILON)))
#define OUR_EQ(x,v) (((v - OUR_EPSILON) < x) && (x <( v + OUR_EPSILON)))
#include "signatures.h"
#include "cmatrix.h"
#include "colors/FuzzyCalc.h"
#include "MAP.h"
#include "MatrixMap.h"
#include "FeatureAlgorithm.h"
#include "transforms.h"

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
	data = new signature[MAX_SIGNATURE_NUM];
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
	sample_col = -1;
	sample_mat = NULL;
}

//---------------------------------------------------------------------------
/*  ~signatures (destructor)
*/
signatures::~signatures() {
	if (data) delete [] data;
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
   new_samp->sample_mat = sample_mat;
   new_samp->sample_col = sample_col;
   if (data) for (sig_index=0;sig_index<count;sig_index++)
     new_samp->data[sig_index]=data[sig_index];
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

	// FIXME:  Alternative - make invalid values -DBL_MAX
	// this will make them 0 when normalized.
	// This will happen to all non-numerical doubles (INF, NEG_INF, NAN, etc).
	// For calculating ranges, only values in the range of OUR_DBL_MIN - OUR_DBL_MAX could be considered,
	// Where, OUR_DBL_MIN = -DBL_MAX + DBL_EPSILON and OUR_DBL_MAX = DBL_MAX - DBL_EPSILON
	// if (value > -DBL_MAX && value < DBL_MAX) data[count].value=value;
	// else data[count].value=-DBL_MAX;
   if (value>INF) value=INF;        /* prevent error */
   if (value<-INF) value=-INF;      /* prevent error */
   if (value<1/INF && value>-1/INF) value=0;  /* prevent a numerical error */
   data[count].value=value;
   count++;
}

/* Finalize
   store the_sample_indx, and clear out data
*/
void signatures::Finalize(Eigen::MatrixXd &the_mat, int the_col) {
	sample_mat = &the_mat;
	sample_col = the_col;
//	if (data) delete data;
//	data = NULL;
}

/* Clear
   clear all signature values
*/
void signatures::Clear()
{
	count=0;
	if (data) {
		delete [] data;
		data = new signature[MAX_SIGNATURE_NUM];
		for (int sig_index=0;sig_index<MAX_SIGNATURE_NUM;sig_index++)
		{  //data[sig_index].name[0]='\0';
			data[sig_index].value=0;
		}
	}
}
//============================================================
// added June 2011 
// This function essentially generates a work order for wndcharm to
// iterate over and calculate.
// input:
//   int large_set = true or false
//   int compute_colors = true or false
// output:
//   double<FeatureGroup> & group_list = a vector of FeatureGroup objects.
//int signatures::GenerateStandardFeatureGroupList( int long_chain, int compute_colors, vector<const FeatureGroup*> &group_list )
int signatures::GenerateStandardFeatureGroupList( int long_chain, int compute_colors, vector<FeatureGroup*> &group_list )
{
	// First, add feature groups that are common to all sets:
	group_list.clear();
	FeatureNames* phonebook = FeatureNames::get_instance();
	if( NULL == phonebook ) return -1;
	string temp_str;
	
	ifstream small_feature ( "small.txt" );
	if( small_feature.fail() ) {
		std::cout << "Failed to open small.txt" << std::endl;
		exit(1);
	}
	while( small_feature.good() ) {
		temp_str.clear();
		getline( small_feature, temp_str );
		if( !temp_str.empty() ) 
			group_list.push_back( phonebook->getGroupByName( temp_str ) );
	}
	small_feature.close();

	if( long_chain ) {
		ifstream large_feature( "large.txt" ); //, ifstream::in );
		if( large_feature.fail() ) {
			std::cout << "Failed to open large.txt" << endl;
			exit(1);
		}
		while( large_feature.good() ) {
		temp_str.clear();
		getline( large_feature, temp_str );
		if( !temp_str.empty() ) 
			group_list.push_back( phonebook->getGroupByName( temp_str ) );
		}
		large_feature.close();
	}
	return 0;
}

int signatures::IsNeeded(long start_index, long group_length)
{  int sig_index;
   if (!ScoresTrainingSet) return(1);
   for (sig_index=start_index;sig_index<start_index+group_length;sig_index++)
     if (((TrainingSet *)(ScoresTrainingSet))->SignatureWeights[sig_index]>0) return(1);
   return(0);
}

//int signatures::ComputeFromGroupList( ImageMatrix *matrix, vector<const FeatureGroup*> &feature_groups)
int signatures::ComputeFromGroupList( ImageMatrix *untransformed_matrix, std::vector<FeatureGroup*> &feature_groups)
{
	// So that the pre-main transform and algorithm registration operations
	// don't get optimized out.
	FourierTransform * no_op = new FourierTransform; delete no_op;
	MultiscaleHistograms * noop = new  MultiscaleHistograms; delete noop;

	MatrixMap saved_pixel_planes (untransformed_matrix);
	// MatrixMap is essentially = map< vector<Transform const *>, ImageMatrix*>
	// I know, it looks kinda weird to have a vector as the key in a map
	// The MatrixMap will delete any ImageMatrices that are stored in it
	// as part of it's ~MatrixMap destructor

	WNDCHRM_ERROR retval;
	FeatureInfo* feature_info = NULL;
	vector<double> coeffs;
	int i;
	string feature_name;
	vector<FeatureInfo*> feature_list;
	
	#if DEBUG
	std::string group_name;
	#endif
	int group_count = 0;

	vector<FeatureGroup*>::const_iterator grp_it = feature_groups.begin();
	for( ; grp_it != feature_groups.end(); grp_it++ ) {
		if( NULL == (*grp_it) ) {
			std::cout << "Signatures::ComputeFromGroupList(): group " << group_count++ << " is corrupted." << std::endl;
			continue;
		}

		#if DEBUG
		(*grp_it)->print_info();
		#endif

		if( NULL == (*grp_it)->algorithm )
			continue;

		if( (retval = (*grp_it)->calculate_coefficients( saved_pixel_planes, coeffs )) != WC_NO_ERROR )
		{
			std::cout << "Signatures::ComputeFromGroupList(): call to algorithm->calculate returned value " << retval << std::endl;
			continue;
		}

		for( i = 0; i < coeffs.size(); i++ ) {
			if( NULL == (feature_info = new FeatureInfo( (*grp_it), i )) ) return -1;
			if( !( feature_info->get_name( feature_name ) ) ) return -1;
			Add( feature_name.c_str(), coeffs[i] );
			feature_list.push_back( feature_info );
		}
		++group_count;
	} // end iterating over feature groups

	vector<FeatureInfo*>::iterator fi_it = feature_list.begin();

	#if DEBUG
	int count = 0;
	for( ; fi_it != feature_list.end(); ++fi_it )
	{
		string temp_str;
		(*fi_it)->get_name( temp_str );
		cout << ++count << ". " << temp_str << endl;
	}
	#endif

	// For right now, don't save the feature_infos, cause it's a lot of memory
	// (~5MB for 12 images)
	
	for( fi_it = feature_list.begin(); fi_it != feature_list.end(); ++fi_it )
		delete (*fi_it);

	return 1;
}


/* normalize
   normalize the signature values using the maximum and minimum values of the training set
   ts -TrainingSet *- the training set according which the signature values should be normalized
*/
void signatures::normalize(void *TrainSet, Eigen::VectorXd &sample) {
	TrainingSet *ts;
	ts=(TrainingSet *)TrainSet;
	Eigen::VectorXd &mins = ts->SignatureMins;
	Eigen::VectorXd &ranges = ts->SignatureRanges;
	Eigen::VectorXi &FeatureIndexes = ts->ReducedFeatureIndexes;
	int sig_index, nsigs = FeatureIndexes.size();
	sample.resize(nsigs);
	double sample_feature;
	int orig_index;
	for (sig_index = 0; sig_index < nsigs; sig_index++) {
		orig_index = FeatureIndexes[sig_index];
		sample_feature = (*sample_mat)( orig_index, sample_col );
		if ( sample_feature > -DBL_MAX && sample_feature < DBL_MAX && ranges[orig_index] > DBL_EPSILON ) {
			sample[sig_index] = 100*((sample_feature - mins[orig_index])/ranges[orig_index]);
// 			if (sample[sig_index] > 100) sample[sig_index] = 100;
// 			if (sample[sig_index] < 0) sample[sig_index] = 0;
		} else {
			sample[sig_index] = 0;
		}
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
/*void signatures::ComputeFromDouble(double *data, int width, int height, int depth, int compute_color)
{  
	ImageMatrix *matrix;
   long x,y,z;
   matrix=new ImageMatrix(width,height,depth);
   for (x=0;x<width;x++)
     for (y=0;y<height;y++)
       for (z=0;z<depth;z++)
         matrix->SetInt(x,y,z,data[x*height*depth + y * depth + z]);   
 
	compute(matrix,compute_color);
   delete matrix;
}
*/

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
	std::vector<double> coeffs;
	int i,file_index;

	if (! file_sigs.LoadFromFile (filename) ) return (0);
	if (verbosity>=2) printf ("compare %s to computed\n",filename);
	
	FeatureNames* phonebook = FeatureNames::get_instance();
	if( NULL == phonebook )
		assert(0);

	// 20 features long: 323-342, standard: N/A
	if (large_set) {
		std::string fractal_name = "Fractal Features"; 
		FeatureAlgorithm* base_itf = phonebook->getFeatureAlgorithmByName( fractal_name );
		if( NULL == base_itf )
			assert(0);
		FractalFeatures* Fractal2D = dynamic_cast<FractalFeatures*>(base_itf);
		if( NULL == Fractal2D )
			assert(0);
		Fractal2D->calculate( matrix, coeffs );
		file_index = 323;
// for (i = 0; i< 20; i++) printf ("fractal2D computed %15.10f\tfrom file: %15.10f\tdiff: %f\tulps: %d\n",vec[i],file_sigs.data[file_index+i].value,
// (file_sigs.data[file_index+i].value - vec[i])/FLT_EPSILON
// ,diffUlps(file_sigs.data[file_index+i].value,vec[i])
// );
		for (i = 0; i< 20; i++) if (!OUR_EQ(file_sigs.data[file_index+i].value,coeffs[i])) {
			if (verbosity>=2) printf ("fractal2D mismatch computed %15.10f\tfrom file: %15.10f\n",coeffs[i],file_sigs.data[file_index+i].value);
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

