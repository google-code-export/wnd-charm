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


#ifndef TrainingSetH
#define TrainingSetH
//---------------------------------------------------------------------------

#include "signatures.h"
#include "config.h" // for version info

#define MAX_CLASS_NUM 1024
#define MAX_CLASS_NAME_LENGTH 50
#define MAX_FILES_IN_CLASS 16384

#define WNN 0
#define WND 1


typedef struct
{  double accuracy;
   double *tile_area_accuracy;            /* used for the different accuracies of the different tile areas     */
   unsigned short *confusion_matrix;      
   double *similarity_matrix;             /* matrix - used for the similarities between the classes            */
   double *similarity_normalization;
   double *image_similarities;            /* matrix - used for the similarity values between all test images   */
   char *feature_names;
   char *feature_groups;
   double feature_weight_distance;
   char *individual_images;                /* a string of the individual image predictions. used for the report */
   unsigned short method; 
   double pearson_coefficient;             /* pearson correlation between the predicted and actual value        */
   double avg_abs_dif;                     /* average absolute difference between the actual and the predicted values */
   double pearson_p_value;
}data_split;

class TrainingSet
{
public:
/* properties */
   signatures **samples;                                           /* samples data                              */
   char SignatureNames[MAX_SIGNATURE_NUM][SIGNATURE_NAME_LENGTH];  /* names of the signatures (e.g. "MultiScale Histogram bin 3) */
   double SignatureWeights[MAX_SIGNATURE_NUM];                     /* weights of the samples                    */
   double SignatureMins[MAX_SIGNATURE_NUM];                        /* minimum value of each signature           */
   double SignatureMaxes[MAX_SIGNATURE_NUM];                       /* maximum value of each signature           */
   long class_num;                                                 /* number of classes                         */
//   char class_labels[MAX_CLASS_NUM][MAX_CLASS_NAME_LENGTH];        /* labels of the classes                     */
   char **class_labels;                                            /* labels of the classes                     */
   long count;                                                     /* the number of samples in the training set */
   long signature_count;                                           /* the number of signatures (< MAX_SIGNATURE_NUM) */
   long color_features;                                            /* color signatures are used                 */
/* methods */
   TrainingSet(long samples_num, long class_num);                  /* constructor                               */
   ~TrainingSet();                                                 /* destructor                                */
   int AddAllSignatures(char *filename, int tiles);                /* load the image feature values from all files */
   int LoadFromDir(char *filename, int tiles, int multi_processor, int large_set, int compute_colors, int downsample, double mean, double stddev, rect *bounding_rect, int overwrite);  /* load images from a root directory   */
   double ClassifyImage(TrainingSet *TestSet, int test_sample_index,int method, int tiles, int tile_areas, TrainingSet *TilesTrainingSets[], int max_tile,int rank, data_split *split, double *similarities);  /* classify one or more images */
   double Test(TrainingSet *TestSet, int method, int tiles, int tile_areas, TrainingSet *TilesTrainingSets[], int max_tile,long rank, data_split *split);     /* test      */
   int SaveToFile(char *filename);                                 /* save the training set values to a file    */
   int ReadFromFile(char *filename);                               /* read the training set values from a file  */
   int SaveWeightVector(char *filename);                           /* save the weights of the features into a file */
   double LoadWeightVector(char *filename, double factor);         /* load the weights of the features from a file and assign them to the features of the training set */
   void SetAttrib(TrainingSet *set);                               /* copy the attributes from one training set to another */   
   void split(double ratio,TrainingSet *TrainSet,TrainingSet *TestSet, unsigned short tiles, int max_train_samples, int max_test_samples,int exact_max_train); /* random split to train and test */
   void SplitAreas(long tiles_num, TrainingSet **TrainingSets);    /* split a tiled dataset into several datasets such that each dataset is one tile location */
   void RemoveClass(long class_index);                             /* remove a class                            */
   int AddSample(signatures *new_sample);                          /* add signatures computed from one image    */
   void normalize();                                               /* normalize the values of the signatures to [0,100] */
   void SetmRMRScores(double used_signatures,double used_mrmr);                     /* set mRMR scores to the features           */
   void SetFisherScores(double used_signatures, double used_mrmr, data_split *split);/* compute the fisher scores for the signatures  */
   int IgnoreFeatureGroup(long index,char *group_name);            /* set the Fisher Score of a group of image features to zero */
   double distance(signatures *sample1, signatures *sample2,double power);  /* Find the weighted Euclidean distance between two samples  */
   long WNNclassify(signatures *test_sample, double *probabilities, double *normalization_factor, signatures **closest_sample);/* classify a sample using weighted nearest neighbor */
   long classify2(signatures *test_sample, double *probabilities,double *normalization_factor); /* classify using -5                         */
   double InterpolateValue(signatures *test_sample, int method, int N, signatures **closest_sample, double *closest_dist);  /* interpolate a value */
   long classify3(signatures *test_sample, double *probabilities,double *normalization_factor);
   double pearson(int tiles,double *avg_abs_dif,double *p_value);                  /* a pearson correlation of the interpolated and the class labels (if all labels are numeric) */
   long PrintConfusion(FILE *output_file, unsigned short *confusion_matrix, double *similarity_matrix);//, unsigned short dend_file, unsigned short method);  /* print a confusion or similarity matrix */
   long dendrogram(FILE *output_file, char *data_set_name, char *phylib_path, int nodes_num,double *similarity_matrix, char **labels,unsigned short sim_method,unsigned short phylip_algorithm);  /* create a dendrogram */
   long report(FILE *output_file, char *output_file_name, char *data_set_name, data_split *splits, unsigned short split_num, int tiles, int max_train_images,char *phylib_path, int phylip_algorithm, int export_tsv, char *path_to_test_set,int image_similarities);  /* report on few splits */
};


#endif
