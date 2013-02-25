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


#ifndef signaturesH
#define signaturesH
//---------------------------------------------------------------------------

#include <stdio.h>

#include "cmatrix.h"
#include "FeatureNames.h"
#include "WORMfile.h"

#define MAX_SIGNATURE_NUM 5000
#define SIGNATURE_NAME_LENGTH 80
#define TRANSFORM_NAME_LENGTH 32
#define MAX_TRANSFORM_DEPTH 6
#define IMAGE_PATH_LENGTH 512
#define SAMPLE_NAME_LENGTH 128

// #define NUM_LC_FEATURES  4008
// #define NUM_L_FEATURES   2873
// #define NUM_C_FEATURES   2160
// #define NUM_DEF_FEATURES 1025

#define NUM_LC_FEATURES  4042
#define NUM_L_FEATURES   2907
#define NUM_C_FEATURES   2194
#define NUM_DEF_FEATURES 1059

#define CURRENT_FEATURE_VERSION 2

#define NO_SIGS_IN_FILE -2

#define MIN_SIG_VAL -FLT_MAX
#define MAX_SIG_VAL FLT_MAX

struct signature
{
  public:
   double value;
};

class signatures
{
  private:
    int IsNeeded(long start_index, long group_length);  /* check if the group of signatures is needed */
  public:
    signature *data;
    enum feature_vec_types {
    	fv_unknown = 0,  // this must evaluate to false
    	fv_short = 1,
    	fv_long = 2,
    	fv_short_color = 3,
    	fv_long_color = 4
    };
    int feature_vec_type;              // stores the integer value of the feature_vec_types enum.
    int version;                       // The major version of the sig file (1 for wndchrm versions prior to 1.33 , 2 for wndchrm versions > 1.33).
                                       // The full version designation is version.feature_vec_type
    unsigned short sample_class;        /* the class of the sample             */
    double sample_value;                /* a continous value (if TrainingSet->is_continuous is true, sample_value = 1 for known samples, and 0 for unknown samples */      
	double interpolated_value;          /* a predicted continous value if class_num==1, or an interploated class value if class labels are all numerical */
    long count;
    long allocated;
    static long max_sigs;
    char full_path[IMAGE_PATH_LENGTH];  /* optional - full path the the image file     */
    char sample_name[SAMPLE_NAME_LENGTH];  /* A string to identify the image sample (e.g. tile). For .sig files, added before last '.' of the image name */
	void *NamesTrainingSet;             /* the training set in which this set of signatures belongs - is assigned so that the signature names will be added */
    void *ScoresTrainingSet;            /* a pointer to a training set with computed Fisher scores (to avoid computing 0-scored signatures)                 */
	WORMfile *wf;                       // class for mutex'ed files for storing sig values
    signatures();                       // constructor
    ~signatures();                      // destructor
    signatures *duplicate();            // create an identical signature vector object */
    void Allocate(size_t nsigs);        // call before adding sigs
    void Add(const char *name, double value);
	void AddVector(const FeatureNames::FeatureGroup *fg, const std::vector<double> &vec);
	void SetFeatureVectorType();
    void Clear();
    void compute(ImageMatrix &matrix, int compute_colors);
    void CompGroupA(ImageMatrix &matrix, const std::string &transform_label);
    void CompGroupB(ImageMatrix &matrix, const std::string &transform_label);
    void CompGroupC(ImageMatrix &matrix, const std::string &transform_label);
    void CompGroupD(ImageMatrix &matrix);
    void ComputeGroups(ImageMatrix &matrix, int compute_colors);
    void normalize(void *TrainSet);                /* normalize the signatures based on the values of the training set */
    void FileClose();
    int SaveToFile(int save_feature_names);
    int LoadFromFile(char *filename);
    void LoadFromFilep (FILE *value_file); // implementation for LoadFromFile using a pre-existing FILE*
	int ReadFromFile (bool wait); // load if exists, or lock and set fpp.
	char *GetFileName(char *buffer);
	int CompareToFile (ImageMatrix &matrix, char *filename, int compute_colors, int large_set);
};

#endif


