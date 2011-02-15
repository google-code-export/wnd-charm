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

#define MAX_SIGNATURE_NUM 5000
#define SIGNATURE_NAME_LENGTH 80
#define IMAGE_PATH_LENGTH 256
#define SAMPLE_NAME_LENGTH 64

#define NO_SIGS_IN_FILE -2

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
    signature data[MAX_SIGNATURE_NUM];
    unsigned short sample_class;        /* the class of the sample             */
    double sample_value;                /* a continous value (if TrainingSet->is_continuous is true, sample_value = 1 for known samples, and 0 for unknown samples */      
	double interpolated_value;          /* a predicted continous value if class_num==1, or an interploated class value if class labels are all numerical */
    long count;
    char full_path[IMAGE_PATH_LENGTH];  /* optional - full path the the image file     */
    char sample_name[SAMPLE_NAME_LENGTH];  /* A string to identify the image sample (e.g. tile). For .sig files, added before last '.' of the image name */
	void *NamesTrainingSet;             /* the training set in which this set of signatures belongs - is assigned so that the signature names will be added */
    void *ScoresTrainingSet;            /* a pointer to a training set with computed Fisher scores (to avoid computing 0-scored signatures)                 */
    signatures();                       /* constructor                                 */
    signatures *duplicate();            /* create an identical signature vector object */
    void Add(const char *name, double value);
    void Clear();
    void compute(ImageMatrix *matrix, int compute_colors);
    void CompGroupA(ImageMatrix *matrix, const char *transform_label);
    void CompGroupB(ImageMatrix *matrix, const char *transform_label);
    void CompGroupC(ImageMatrix *matrix, const char *transform_label);
    void CompGroupD(ImageMatrix *matrix, const char *transform_label);
    void ComputeGroups(ImageMatrix *matrix, int compute_colors);
    void normalize(void *TrainSet);                /* normalize the signatures based on the values of the training set */
    void ComputeFromDouble(double *data, int width, int height, int depth, int compute_color);  /* compute the feature values from an array of doubles */
    FILE *FileOpen(char *path, int overwrite);
    void FileClose(FILE *value_file);
    int SaveToFile(FILE *value_file,int save_feature_names);
    int LoadFromFile(char *filename);
	int ReadFromFile (FILE **fpp, bool wait); // load if exists, or lock and set fpp.
	char *GetFileName(char *buffer);
	int CompareToFile (ImageMatrix *matrix, char *filename, int compute_colors, int large_set);
};

#endif


