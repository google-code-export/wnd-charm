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


#pragma hdrstop

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>

#include "TrainingSet.h"
//#include "cmatrix.h"

#ifndef WIN32
#include <stdlib.h>
#else
#include <dir.h>
#endif


/* global variable */
int print_to_screen=1;

/* compare_two_doubles
   function used for qsort
*/
int compare_two_doubles (const void *a, const void *b)
{
  if (*((double *)a) > *((double*)b)) return(1);
  if (*((double*)a) == *((double*)b)) return(0);
  return(-1);
}

int comp_strings(const void *s1, const void *s2)
{  return(strcmp((char *)s1,(char *)s2));
}

/* check if the file format is supported */
int IsSupportedFormat(char *filename)
{  
  if (strstr(filename,".sig")) return(0);  /* ignore files the extension but are actually .sig files */
#ifdef WIN32	  
  if (strstr(filename,".bmp") || strstr(filename,".BMP")) return(1);
#endif  
  if (strstr(filename,".tif") || strstr(filename,".TIF") || strstr(filename,".ppm") || strstr(filename,".PPM")) return(1);  /* process only image files */
  if (strstr(filename,".dcm") || strstr(filename,".DCM")) return(1);
  return(0);
}

//---------------------------------------------------------------------------

/* constructor of a TrainingSet object
   samples_num -long- a maximal number of samples in the training set
*/
TrainingSet::TrainingSet(long samples_num, long class_num)
{  int signature_index,sample_index;
//   samples=new sample[samples_num];
   samples=new signatures*[samples_num];
   for (sample_index=0;sample_index<samples_num;sample_index++)
     samples[sample_index]=NULL;
   /* initialize */
   for (signature_index=0;signature_index<MAX_SIGNATURE_NUM;signature_index++)
   {  SignatureNames[signature_index][0]='\0';
      SignatureWeights[signature_index]=0.0;
      SignatureMins[signature_index]=INF;
      SignatureMaxes[signature_index]=-INF;
   }
   this->class_num=class_num;
   color_features=0;     /* by default - no color features are used */
   signature_count=0;
   class_labels=new char*[class_num+1];
   for (sample_index=0;sample_index<=class_num;sample_index++)
   {  class_labels[sample_index]=new char[MAX_CLASS_NAME_LENGTH];
      strcpy(class_labels[sample_index],"");
   }
   
//   for (sample_index=0;sample_index<MAX_CLASS_NUM;sample_index++)
//     strcpy(class_labels[sample_index],"");
   count=0;
}

/* destructor of a training set object
*/
TrainingSet::~TrainingSet()
{  int sample_index;
   for (sample_index=0;sample_index<count;sample_index++)
     if (samples[sample_index]) delete samples[sample_index];
   for (sample_index=0;sample_index<=class_num;sample_index++)
     delete class_labels[sample_index];
   delete class_labels;
   delete samples;
}

/* AddSample
   Add the signatures computed from one image to the training set
   new_sample -signatures- the set of signature values
   path -char *- full path to the image file (NULL if n/a)

   returned value -int- 1 if suceeded 0 if failed.
                        can fail due to bad sample class
*/
int TrainingSet::AddSample(signatures *new_sample)
{  int sig_index;
   /* check if the sample can be added */
//   if (new_sample->sample_class>class_num) return(0);
   samples[count]=new_sample;
   signature_count=new_sample->count;
   count++;
   return(1);
}

/* SaveToFile
   filename -char *- the name of the file to save
   returned value -int- 1 is successful, 0 if failed.

   comment: saves the training set into a text file
*/
int TrainingSet::SaveToFile(char *filename)
{  int sample_index, class_index, sig_index;
   FILE *file;
   if (!(file=fopen(filename,"w"))) return(0);
   fprintf(file,"%d\n",class_num);
   fprintf(file,"%d\n",signature_count);
   fprintf(file,"%d\n",count);
   /* write the signature names */
   for (sig_index=0;sig_index<signature_count;sig_index++)
     fprintf(file,"%s\n",SignatureNames[sig_index]);
   /* write the class labels */
   for (class_index=0;class_index<=class_num;class_index++)
     fprintf(file,"%s\n",class_labels[class_index]);
   /* write the samples */
   for (sample_index=0;sample_index<count;sample_index++)
   {
      for (sig_index=0;sig_index<signature_count;sig_index++)
        if (samples[sample_index]->data[sig_index].value==(int)(samples[sample_index]->data[sig_index].value))
      fprintf(file,"%d ",(int)(samples[sample_index]->data[sig_index].value));      /* make the file smaller */
//        else fprintf(file,"%.6f ",samples[sample_index]->data[sig_index].value);
      else fprintf(file,"%.5e ",samples[sample_index]->data[sig_index].value);
      if (class_num==0) fprintf(file,"%f\n",samples[sample_index]->sample_value);  /* if the class is 0, save the continouos value of the sample */
	  else fprintf(file,"%d\n",samples[sample_index]->sample_class);   /* save the class of the sample */
      fprintf(file,"%s\n",samples[sample_index]->full_path);
   }
   fclose(file);
   return(1);
}


/* ReadFromFile
   filename -char *- the name of the file to open
   returned value -int- 1 is successful, 0 if failed.

   comment: reads the training set from a text file
*/
int TrainingSet::ReadFromFile(char *filename)
{  int sample_index, class_index, sample_count,sig_index;
   char buffer[50000];
   FILE *file;
   if (!(file=fopen(filename,"r"))) return(0);
   for (sample_index=0;sample_index<count;sample_index++)
     if (samples[sample_index]) delete samples[sample_index];
   delete samples;
   fgets(buffer,sizeof(buffer),file);
   class_num=atoi(buffer);
   fgets(buffer,sizeof(buffer),file);
   signature_count=atoi(buffer);
   fgets(buffer,sizeof(buffer),file);
   sample_count=atoi(buffer);
   samples=new signatures*[sample_count];
   count=0;         /* initialize the count before adding the samples to the training set */
   color_features=0;
   /* read the signature names */
   for (sig_index=0;sig_index<signature_count;sig_index++)
   {  fgets(buffer,sizeof(buffer),file);
      strcpy(SignatureNames[sig_index],buffer);
      if (strchr(SignatureNames[sig_index],'\n')) *strchr(SignatureNames[sig_index],'\n')='\0';  /* make sure there is no line break in the name */
      if (strstr(SignatureNames[sig_index],"color") || strstr(SignatureNames[sig_index],"Color")) color_features=1;   /* check if color signatures are used */
   }
   /* read the class labels */
   for (class_index=0;class_index<=class_num;class_index++)
   {  fgets(buffer,sizeof(buffer),file);
      strcpy(class_labels[class_index],buffer);
      if (strchr(class_labels[class_index],'\n')) *strchr(class_labels[class_index],'\n')='\0';	  /* make sure there is no line break in the name */
   }
   /* read the samples */
   for (sample_index=0;sample_index<sample_count;sample_index++)
   {  char *p_buffer;
      signatures *one_sample;
      one_sample=new signatures();
      fgets(buffer,sizeof(buffer),file);
      p_buffer=strtok(buffer," \n");
      for (sig_index=0;sig_index<signature_count;sig_index++)
      {  one_sample->Add(SignatureNames[sig_index],atof(p_buffer));
         p_buffer=strtok(NULL," \n");
      }
      one_sample->sample_class=atoi(p_buffer);                  /* read the class of the sample                     */
      if (class_num==0) one_sample->sample_value=atof(p_buffer);/* use the same value as an continouos value        */
      else one_sample->sample_value=atof(class_labels[one_sample->sample_class]); /* use the class label as a value */
      fgets(buffer,sizeof(buffer),file);                        /* read the image path (can also be en ampty line)  */
      if (strchr(buffer,'\n')) *(strchr(buffer,'\n'))='\0';     /* remove the end of line (if there is one)         */
      strcpy(one_sample->full_path,buffer);                     /* copy the full path to the signatures object      */
      AddSample(one_sample);
   }
   fclose(file);
   return(1);
}

/* RemoveClass
   remove a class from the training set
   class_index -long- the index of the class to be removed
*/
void TrainingSet::RemoveClass(long class_index)
{  long index,deleted_count=0;
   /* remove the class label */
   for (index=class_index;index<class_num;index++)
     strcpy(class_labels[index],class_labels[index+1]);
   /* remove the samples of that class */
   for (index=0;index<count;index++)
   { if (samples[index]->sample_class==class_index)
     {  delete samples[index];
        deleted_count++;
     }
     else samples[index-deleted_count]=samples[index];
   }
   count=count-deleted_count;   /* set the new number of samples */   
   /* change the indices of the samples */
   for (index=0;index<count;index++)
     if (samples[index]->sample_class>class_index)
       samples[index]->sample_class=samples[index]->sample_class-1;
   /* change the number of classes and training samples */
   class_num--;
   return;
}

/* SaveWeightVector
   save the weights of the features into a file 
   filename -char *- the name of the file into which the weight values should be written
*/
int TrainingSet::SaveWeightVector(char *filename)
{  FILE *sig_file;
   int sig_index;
   if (!(sig_file=fopen(filename,"w"))) return(0);
   if (print_to_screen) printf("Saving weight vector to file '%s'...\n",filename);   
   for (sig_index=0;sig_index<signature_count;sig_index++)
     fprintf(sig_file,"%f %s\n",SignatureWeights[sig_index],SignatureNames[sig_index]);
   fclose(sig_file);
   return(1);
}

/* LoadWeightVector
   load the weights of the features from a file and assign them to the features of the training set
   filename -char *- the name of the file into which the weight values should be read from
   factor -double- multiple the loaded feature vector and add to the existing vecotr (-1 is subtracting). 0 replaces the existing vector with the loaded vector.
   returned value -double- the square difference between the original weight vector and the imported weight vector
*/
double TrainingSet::LoadWeightVector(char *filename, double factor)
{  FILE *sig_file;
   int sig_index=0;
   char line[128],*p_line;
   double feature_weight_distance=0.0;
   if (!(sig_file=fopen(filename,"r"))) return(0);
   if (print_to_screen) printf("Loading weight vector from file '%s'...\n",filename);
   p_line=fgets(line,sizeof(line),sig_file);
   while (p_line)
   {  if (strlen(p_line)>0)
      {  if (strchr(p_line,' ')) (*strchr(p_line,' '))='\0';
         feature_weight_distance+=pow(SignatureWeights[sig_index]-atof(p_line),2);
         if (factor==0) SignatureWeights[sig_index++]=atof(p_line);
	     else SignatureWeights[sig_index++]+=factor*atof(p_line);
         if (SignatureWeights[sig_index-1]<0) SignatureWeights[sig_index-1]=0;		  
	  }
      p_line=fgets(line,sizeof(line),sig_file);   
   }
   fclose(sig_file);
   if (sig_index!=signature_count) return(-1.0);
   return(sqrt(feature_weight_distance));
}

/* set attrib
   Set the attributes of a given set
*/
void TrainingSet::SetAttrib(TrainingSet *set)
{  int class_index,sig_index;
   set->signature_count=signature_count;
   set->color_features=color_features;
   /* copy the class labels to the train and test */
   for (class_index=0;class_index<=class_num;class_index++)
     strcpy(set->class_labels[class_index],class_labels[class_index]);
   /* copy the signature names to the training and test set */
   for (sig_index=0;sig_index<signature_count;sig_index++)
     strcpy(set->SignatureNames[sig_index],SignatureNames[sig_index]);
}

/*  split
    split randomly into a training set and a test set
    ratio -double- the ratio of the number of test set (e.g., 0.1 means 10% of the data are test data).
    tiles -unsigned short- indicates the number of tiles to which each image was divided into. This means that
                           when splitting to train and test, all tiles of that one image will be either in the
                           test set or training set.
    max_train_samples -int- the maximum number of samples to use for training (0 to ignore and use the proportional part of the set)
    max_test_samples -int- the maximum namber of samples for the test set (0 to ignore and use the proportional part of the set)
    exact_max_train -int- if 1 then the class is removed if its number of samples does not reach the "max_train_samples". (ignored if 0)
*/
void TrainingSet::split(double ratio,TrainingSet *TrainSet,TrainingSet *TestSet, unsigned short tiles, int max_train_samples, int max_test_samples, int exact_max_train)
{  long *class_samples;
   int class_index,sig_index,tile_index;
   int number_of_test_samples;
   long class_counts[MAX_CLASS_NUM];
   class_samples = new long[count];
   
   SetAttrib(TrainSet);      /* copy the same attributes to the training and test set */
   SetAttrib(TestSet); 
   if (tiles<1) tiles=1;    /* make sure the number of tiles is valid */
//class_num=250; /* FERET */   
   TrainSet->class_num=TestSet->class_num=class_num;   
   for (class_index=1;class_index<=class_num+(class_num==0);class_index++)
   {  int sample_index,sample_count=0;
      int class_samples_count=0;
      for (sample_index=0;sample_index<count;sample_index++)
        if (samples[sample_index]->sample_class==class_index || class_num==0)
//if (strstr(samples[sample_index]->full_path,"_fa") || strstr(samples[sample_index]->full_path,"_fb") || strstr(samples[sample_index]->full_path,"_rc") || strstr(samples[sample_index]->full_path,"_rb") || strstr(samples[sample_index]->full_path,"_ql") || strstr(samples[sample_index]->full_path,"_qr"))	  	/* FERET */
          class_samples[class_samples_count++]=sample_index;	  
      class_samples_count/=tiles;
	  class_counts[class_index]=class_samples_count;
      /* add the samples to the test set */
      number_of_test_samples=(int)(class_samples_count*ratio);
	  if (max_train_samples>0) number_of_test_samples=max(0,class_samples_count-max_train_samples);	  
      if (max_test_samples>0 && number_of_test_samples>max_test_samples) number_of_test_samples=max_test_samples;
      for (sample_index=0;sample_index<number_of_test_samples;sample_index++)
      {  long rand_index;
         rand_index=rand() % class_samples_count;            /* find a random sample  */

//int b=0;   /* FERET */
//for (int a=0;a<class_samples_count;a++)
//if (strstr(samples[class_samples[a*tiles]]->full_path,"_fb"))
//{ rand_index=a;
//  b=1;
//  break;
//}
//if (b==0) break;

         for (tile_index=0;tile_index<tiles;tile_index++)    /* add all the tiles of that image */
           TestSet->AddSample(samples[class_samples[rand_index*tiles+tile_index]]->duplicate());   /* add the random sample */		   
         /* remove the index */
         memmove(&(class_samples[rand_index*tiles]),&(class_samples[rand_index*tiles+tiles]),sizeof(long)*(tiles*(class_samples_count-rand_index)));
         class_samples_count--;
      }
	  
      /* now add the remaining samples to the Train Set */	  	  
      for (sample_index=0;sample_index<class_samples_count*tiles && (sample_count<max_train_samples*tiles || max_train_samples<=0);sample_index++)
//if (strstr(samples[class_samples[sample_index]]->full_path,"_fa") || strstr(samples[class_samples[sample_index]]->full_path,"_fb") || strstr(samples[class_samples[sample_index]]->full_path,"_hr") || strstr(samples[class_samples[sample_index]]->full_path,"_hl") || strstr(samples[class_samples[sample_index]]->full_path,"_pr"))	  	/* FERET */
//if (strstr(samples[class_samples[sample_index]]->full_path,"_fa")) /* FERET */
//if (strstr(samples[class_samples[sample_index]]->full_path,"_fa") || strstr(samples[class_samples[sample_index]]->full_path,"_fb") || strstr(samples[class_samples[sample_index]]->full_path,"_rc") || strstr(samples[class_samples[sample_index]]->full_path,"_rb") || strstr(samples[class_samples[sample_index]]->full_path,"_ql") || strstr(samples[class_samples[sample_index]]->full_path,"_qr"))	  	/* FERET */
      {  TrainSet->AddSample(samples[class_samples[sample_index]]->duplicate());
         sample_count++;
      }
   }

   /* remove the class if it doesn't have enough samples */
   class_index=class_num;
   if (exact_max_train)
   while (class_index>0)
   {  if (class_counts[class_index]<=max_train_samples-max_test_samples)
      {  TrainSet->RemoveClass(class_index);
	     TestSet->RemoveClass(class_index);
	     RemoveClass(class_index);
	  }
	  class_index--;
   }
   
   delete class_samples;
}

/* SplitAreas
   split into several classifiers based on the area (tile) of the image. E.g., a 4x4 tiling is divided into 16 sets. 
   tiles_num -long- the number of tiles (total number, not the square root of the number of tiles)
   TrainingSets -TrainingSet **- a pointer to an array of training sets.
*/
void TrainingSet::SplitAreas(long tiles_num, TrainingSet **TrainingSets)
{  int samp_index,tile_index;
   for (tile_index=0;tile_index<tiles_num;tile_index++)
   {  TrainingSets[tile_index]=new TrainingSet((long)ceil((double)count/(double)tiles_num),class_num);   /* allocate memory for the new set of each tile location */
      SetAttrib(TrainingSets[tile_index]);
   }
   tile_index=0;
   for (samp_index=0;samp_index<count;samp_index++)
   {  TrainingSets[tile_index]->AddSample(samples[samp_index]->duplicate());
      tile_index++;
      if (tile_index>=tiles_num) tile_index=0;
   }
}


/* AddAllSignatures
   load the image feature values from all files
   filename -char *- the root directory of the data set
   tiles -int- square root of the number of tiles (used only when the images have continouos values).
*/

int TrainingSet::AddAllSignatures(char *filename, int tiles)
{  DIR *class_dir;
   struct dirent *ent;
   FILE *sig_file,*input_file;
   char buffer[512],sig_file_name[512];
   char files_in_class[MAX_FILES_IN_CLASS][256];  /* when compiling in Windows - the stack size must be set to support this structure */
   double values[MAX_FILES_IN_CLASS];
   int res,samp_class=1;
   if (class_num==0) samp_class=0;   /* for continouos values */  
   while (samp_class<=class_num)
   {  int file_index,files_in_class_count=0;
      sprintf(buffer,"%s/%s",filename,class_labels[samp_class]);
      if (class_dir=opendir(buffer)) /* read the files and make sure they are sorted */
      { while (ent = readdir(class_dir))
	      if (strstr(ent->d_name,".sig"))  /* read only the .sig files which store image feature data */
            strcpy(files_in_class[files_in_class_count++],ent->d_name);
        closedir(class_dir);
        qsort(files_in_class,files_in_class_count,sizeof(files_in_class[0]), comp_strings);
        for (file_index=0;file_index<files_in_class_count;file_index++)
        {  strcpy(sig_file_name,files_in_class[file_index]);  
		   sprintf(files_in_class[file_index],"%s/%s",buffer,sig_file_name);		
        }
      }
	  
	  else  /* read the sig files of the images read from a file */
	  {  file_index=0;
         input_file=fopen(filename,"r");
         char *p_line=fgets(sig_file_name,sizeof(sig_file_name),input_file);
         while (p_line)
         {  while (p_line && strchr(p_line,'.')==NULL) p_line=fgets(sig_file_name,sizeof(sig_file_name),input_file); /* skip empty lines */
            if (!p_line) break;
            strtok(sig_file_name," \n\t");
			double value=atof(strtok(NULL," \n\t"));
			*strrchr(sig_file_name,'.')='\0';
			for (int tile_y=0;tile_y<tiles;tile_y++)			
              for (int tile_x=0;tile_x<tiles;tile_x++)
              {  sprintf(files_in_class[file_index],"%s_%d_%d.sig",sig_file_name,tile_x,tile_y);
                 values[file_index]=value;
                 file_index++;
              }
			p_line=fgets(sig_file_name,sizeof(sig_file_name),input_file);
         }
         files_in_class_count=file_index;
         fclose(input_file);
      }
	  	  
      /* now load and add the feature value files */
      for (file_index=0;file_index<files_in_class_count;file_index++)
      {  signatures *ImageSignatures;
//         sprintf(sig_file_name,"%s/%s",buffer,files_in_class[file_index]);
         ImageSignatures = new signatures;
         ImageSignatures->NamesTrainingSet=this;		 
//         if (ImageSignatures->LoadFromFile(sig_file_name))
         if (ImageSignatures->LoadFromFile(files_in_class[file_index]))
         {  ImageSignatures->sample_class=samp_class;          /* make sure the sample has the right class ID */
            if (class_num<=1) ImageSignatures->sample_class=(int)values[file_index];  /* for non-continouos data (classes) read from a file  */
            ImageSignatures->sample_value=values[file_index];  /* read the continouos value */
            AddSample(ImageSignatures);
         }
         else delete ImageSignatures;
      }
      samp_class++;
   }
   return(1);
}


/* LoadFromDir
   load a set of image into the training set
   filename -char *- a root directory - each class is a sub-directory. If points to a file then it reads the images from a text file
   returned value -int- 1 is succeeded 0 if failed
   tiles -int- the number of tiles to break the image to (e.g., 4 means 4x4 = 16 tiles)
   multi_processor -int- 1 if more than one signatures process should be running
   large_set -int- whether to use the large set of image features or not
   bounding_rect -rect *- a sub image area from which features are computed. ignored if NULL.
   overwrite -int- 1 for forcely overwriting pre-computed .sig files
*/


int TrainingSet::LoadFromDir(char *filename, int tiles, int multi_processor, int large_set, int compute_colors, int downsample, double mean, double stddev, rect *bounding_rect, int overwrite)
{  DIR *root_dir,*class_dir;
   struct dirent *ent;
   FILE *sig_file,*input_file=NULL;
   char buffer[512],image_file_name[512];
   char files_in_class[MAX_FILES_IN_CLASS][64];
   char dirs_in_root[MAX_CLASS_NUM][64];
   int dirs_count=0;
   int res,samp_class=1,file_class_num;

   class_num=1;
   file_class_num=1;        /* the number of classes when read from a file (to handle correlations) */
   if (tiles<1) tiles=1;    /* at least one tile */
   color_features=compute_colors;   
   if (filename[strlen(filename)-1]=='*') filename[strlen(filename)-1]='\0';  /* remove a last '*' is there is one       */   
   if (filename[strlen(filename)-1]=='/') filename[strlen(filename)-1]='\0';  /* remove a last '/' is there is one       */
   if (root_dir=opendir(filename))
   {  while (ent = readdir(root_dir))
      {  if (ent->d_name[0]=='.' || strcmp(ent->d_name,"tsv")==0) continue;   /* ignore the '.' and '..' entries or files, and the automatically generated tsv directory */
         sprintf(buffer,"%s/%s",filename,ent->d_name);
         if (class_dir=opendir(buffer)) closedir(class_dir);
         else continue;                                                       /* ignore non-directory enetries                */
         strcpy(files_in_class[samp_class],ent->d_name);                      /* the label of the class is the directory name */
         samp_class++;
      }
      closedir(root_dir);
      class_num=samp_class-1;   
      qsort(&(files_in_class[1]),class_num,sizeof(files_in_class[1]),comp_strings);
      for (samp_class=1;samp_class<=class_num;samp_class++) strcpy(class_labels[samp_class],files_in_class[samp_class]);	  
   }
   else if (!(input_file=fopen(filename,"r"))) return(0);  /* read the images from a file */
   
   samp_class=1;
   while (samp_class<=class_num)
   {  int file_index,files_in_class_count=0;
      char *p_line=(char *)1; 
      /* constract the path of the class files */
      if (!input_file)  /* read from dir */
      {  strcpy(buffer,filename);
		 strcat(buffer,"/");
         strcat(buffer,class_labels[samp_class]);
         /* get the file names */
         class_dir=opendir(buffer);
         while (ent = readdir(class_dir))
         {  if (ent->d_name[0]=='.') continue;          /* ignore the '.' and '..' entries */
            if (!IsSupportedFormat(ent->d_name)) continue;
           strcpy(files_in_class[files_in_class_count++],ent->d_name);
        }
        closedir(class_dir);
        qsort(files_in_class,files_in_class_count,sizeof(files_in_class[0]), comp_strings);		
      }

	  /* process the files in the directory */
      file_index=0;
      while((file_index<files_in_class_count && !input_file) || (p_line && input_file))	  
      {  signatures *ImageSignatures;
         ImageMatrix *matrix=new ImageMatrix;
         double value;
         if (input_file)
         {  p_line=fgets(image_file_name,sizeof(image_file_name),input_file);
            while (p_line && strchr(p_line,'.')==NULL ) p_line=fgets(image_file_name,sizeof(image_file_name),input_file);  /* skip empty lines */
            if (!p_line) break;		 
            strtok(image_file_name," \n\t");
            value=atof(strtok(NULL," \n\t"));
			samp_class=(int)value;
			if (fabs(samp_class-value)>1/INF) file_class_num=0;   /* correlation values - no classes here */
            if (file_class_num>0 && samp_class>file_class_num) file_class_num=samp_class; 
         }
         else
         {  strcpy(image_file_name,buffer);
            strcat(image_file_name,"/");
            strcat(image_file_name,files_in_class[file_index]);
         }
         if (print_to_screen) printf("Loading image %s\n",image_file_name);
         res=matrix->OpenImage(image_file_name,downsample,bounding_rect,mean,stddev);
         if (res)  /* add the image only if it was loaded properly */
         {  int tile_index_x,tile_index_y;
//{  double mean,median,stddev;
//matrix->BasicStatistics(&mean, &median, &stddev, NULL, NULL, NULL, 0);
//for (int x=0;x<matrix->width;x++)
//  for (int y=0;y<matrix->height;y++)
//  if (matrix->data[x][y].intensity<mean+4*stddev) matrix->data[x][y].intensity=0;
//}		 
//matrix->Symlet5Transform();
//matrix->ChebyshevTransform(0);
//printf("image name: %s\n",image_file_name);
//matrix->SaveTiff(image_file_name);

            for (tile_index_y=0;tile_index_y<tiles;tile_index_y++)
              for (tile_index_x=0;tile_index_x<tiles;tile_index_x++)
              {  ImageMatrix *tile_matrix;
                 long tile_x_size=(long)(matrix->width/tiles);
                 long tile_y_size=(long)(matrix->height/tiles);
                 if (tiles>1) tile_matrix=new ImageMatrix(matrix,tile_index_x*tile_x_size,tile_index_y*tile_y_size,(tile_index_x+1)*tile_x_size-1,(tile_index_y+1)*tile_y_size-1,0,0);
                 else tile_matrix=matrix;
                 /* compute the image features */
                 ImageSignatures=new signatures;
                 ImageSignatures->NamesTrainingSet=this;
                 strcpy(ImageSignatures->full_path,image_file_name);
                 /* check if the features for that image were processed by another process */
                 if (multi_processor)
                 {  if (!(sig_file=ImageSignatures->FileOpen(NULL,tile_index_x,tile_index_y,overwrite)))
                    {  delete ImageSignatures;
                       if (tiles>1) delete tile_matrix;
					   continue;
                    }
                 }
                 /* compute the features */						 
                 if (large_set) ImageSignatures->ComputeGroups(tile_matrix,compute_colors);
                 else ImageSignatures->compute(tile_matrix,compute_colors);

                 ImageSignatures->sample_class=samp_class;
                 ImageSignatures->sample_value=value;        /* the value is read from a file */

                 if (!multi_processor)
                   AddSample(ImageSignatures);
                 if (tiles>1) delete tile_matrix;
                 if (multi_processor)
                 {  ImageSignatures->SaveToFile(sig_file,1);
                    ImageSignatures->FileClose(sig_file);
                    delete ImageSignatures;
                 }
            }
         }
         else if (print_to_screen) printf("Could not open '%s' \n",image_file_name);
         delete matrix;
         file_index++;
      }	  
      samp_class++;
    }
	
    /* collect the signature files and join them into one dataset */	
    if (multi_processor)
      AddAllSignatures(filename, tiles);      
	  
    /* check if every class has at least one sample. otherwise - assume correlation values */	  
    if (input_file) 
    {  int sample_index,a_class;
       for (a_class=1;a_class<=file_class_num;a_class++)  
	   {  for (sample_index=0;sample_index<count;sample_index++)
		    if (samples[sample_index]->sample_class==a_class) break;
		  if (sample_index>=count) file_class_num=0;
       }
	   class_num=file_class_num;   /* 0 for correlations, or a number of classes for classification */
	   if (class_num>1)
         for (a_class=1;a_class<=class_num;a_class++) sprintf(class_labels[a_class],"%d",a_class);	  
       fclose(input_file);
    }
	  
    return(1);
}

/* Classify 
   Classify a test sample.
   TestSet -TrainingSet *- one or more tiles of one or more test image
   test_sample_index -int- the index of the image in TestSet that should be tested. if tiles, then the first tile.
   max_tile -int- just use the most similar tile instead of averaging all times
   returned value can be either a class index, or if class_num==0 a contiouos value
*/

double TrainingSet::ClassifyImage(TrainingSet *TestSet, int test_sample_index,int method, int tiles, int tile_areas, TrainingSet *TilesTrainingSets[], int max_tile, int rank, data_split *split, double *similarities)
{  int predicted_class=0,tile_index,class_index,cand,sample_class,test_tile_index,interpolate=1;
   double probabilities[MAX_CLASS_NUM],probabilities_sum[MAX_CLASS_NUM],normalization_factor,normalization_factor_avg=0;
   signatures *closest_sample=NULL, *tile_closest_sample=NULL,*test_signature;
   char interpolated_value[128],last_path[IMAGE_PATH_LENGTH];
   TrainingSet *ts_selector;
   int most_similar_tile=1,most_similar_predicted_class=0;
   double val,sum_prob,dist,value=0.0,value_sum=0.0,most_similar_value=0.0,closest_value_dist=INF,max_tile_similarity=0.0;  /* use for the continouos value */
   
   /* interpolate only if all class labels are values */
   for (class_index=1;class_index<class_num;class_index++)
     interpolate*=(atof(class_labels[class_index])!=0.0 || class_labels[class_index][0]=='0');
   if (tiles<=0) tiles=1;   /* make sure the number of tiles is valid */
   strcpy(last_path,TestSet->samples[test_sample_index]->full_path);
   sample_class=TestSet->samples[test_sample_index]->sample_class;   /* the ground truth class of the test sample */
   for (class_index=1;class_index<=class_num;class_index++) probabilities_sum[class_index]=0.0;  /* initialize the array */
   for (tile_index=test_sample_index;tile_index<test_sample_index+tiles;tile_index++)
   {  if (print_to_screen) 
      {  printf("Classifying image '%s' ",TestSet->samples[tile_index]->full_path);
         if (TestSet->count/tiles>1) printf("(%d/%d)",1+test_sample_index/tiles,TestSet->count/tiles);   /* so that no image index will be displayed when using the "classify" option */
         printf("...\n");
      }
      test_signature=TestSet->samples[tile_index]->duplicate();
      if (tile_areas==0 || tiles==1) ts_selector=this;
      else ts_selector=TilesTrainingSets[tile_index-test_sample_index];   /* select the TrainingSet of the location of the tile */
      if (class_num==0) /* interpolate the value here */
	  {  val=ts_selector->InterpolateValue(test_signature,method,rank,&closest_sample,&dist);
         value=value+val/(double)tiles;
      }
	  else
	  {  if (method==WNN) predicted_class=ts_selector->WNNclassify(test_signature, probabilities,&normalization_factor,&closest_sample);
         if (method==WND) predicted_class=ts_selector->classify2(test_signature, probabilities,&normalization_factor);
//if (method==WND) predicted_class=this->classify3(test_signature, probabilities, &normalization_factor);
      }

      /* use only the most similar tile */
      if (max_tile) 
	  {  sum_prob=0.0;
         for (class_index=0;class_index<=class_num;class_index++) if (class_index!=predicted_class) sum_prob+=probabilities[class_index];
         if (class_num==0)
		 {  if (dist<closest_value_dist)
            {  closest_value_dist=dist;
               most_similar_value=val;
               most_similar_tile=tile_index;		   
			   tile_closest_sample=closest_sample;
            }
		 }
		 else
         if (probabilities[predicted_class]/sum_prob>max_tile_similarity) 
		 {  max_tile_similarity=probabilities[predicted_class]/sum_prob;
            most_similar_tile=tile_index;
            most_similar_predicted_class=predicted_class;
            tile_closest_sample=closest_sample;			
		 }
	  }
	  
      /* measure the distances between the image to all other images */
      if (split && split->image_similarities)
      {  split->image_similarities[(1+test_sample_index/tiles)]=(double)(test_signature->sample_class);   /* for storing the class of each image in the first row (that is not used for anything else) */
	     for (test_tile_index=0;test_tile_index<TestSet->count;test_tile_index++)
         {  signatures *compare_to;
		    if (max_tile) compare_to=TestSet->samples[most_similar_tile]->duplicate();         /* so that only the most similar tile is used */
		    else compare_to=TestSet->samples[test_tile_index]->duplicate();          
            compare_to->normalize(this);   /* in order to compare two normalized vectors */
            split->image_similarities[(1+test_sample_index/tiles)*(TestSet->count/tiles+1)+test_tile_index/tiles+1]+=(distance(test_signature,compare_to,2.0)/tiles);
            delete compare_to;
         }
      }
	  
      if ((strcmp(last_path,test_signature->full_path)!=0)) printf("inconsistent tile %d of image '%s' \n",tile_index-test_sample_index,test_signature->full_path); /* check that the tile is consistent */
      for (class_index=1;class_index<=class_num;class_index++) 
	    if (max_tile && max_tile_similarity==probabilities[predicted_class]/sum_prob) probabilities_sum[class_index]=probabilities[class_index];  /* take the probabilities of this tile only */
	    else probabilities_sum[class_index]+=(probabilities[class_index]/(double)tiles);  /* sum the marginal probabilities */	  
      normalization_factor_avg+=normalization_factor;	  
      if (split && split->tile_area_accuracy) split->tile_area_accuracy[tile_index-test_sample_index]+=((double)(predicted_class==sample_class))/((double)TestSet->count/(double)tiles); 
      delete test_signature;
   }

   if (max_tile) 
   {  value=most_similar_value;
      predicted_class=most_similar_predicted_class;
   }
   
   if (tiles>1)
     closest_sample=tile_closest_sample;
   
   if (class_num==0) TestSet->samples[test_sample_index]->interpolated_value=value;       
   normalization_factor_avg/=tiles;

   /* find the predicted class based on the rank */
   for (class_index=1;class_index<=class_num;class_index++) probabilities[class_index]=0.0;  /* initialize the array */
   if (class_num>1)
     for (cand=0;cand<rank;cand++)
     {  double max=0.0;
        for (class_index=1;class_index<=class_num;class_index++)
          if (probabilities_sum[class_index]>max && probabilities[class_index]==0.0)
          {  max=probabilities_sum[class_index];
             predicted_class=class_index;
          }	
          probabilities[predicted_class]=1.0;
          if (predicted_class==sample_class) break;  /* class was found among the n closest */
     }

//if (probabilities[1]>0.995) predicted_class=(1);
//else predicted_class=(2);

   /* update confusion and similarity matrices */
   if (split && split->confusion_matrix)  /* update the confusion matrix */
     split->confusion_matrix[class_num*sample_class+predicted_class]++;
   if (split && split->similarity_matrix && class_num>0) /* update the similarity matrix */
     for (class_index=1;class_index<=class_num;class_index++) split->similarity_matrix[class_num*sample_class+class_index]+=probabilities_sum[class_index];

   /* print the report line to a string (for the final report) */
   if (split && split->individual_images)
   {  char buffer[512],closest_image[512],color[128],one_image_string[MAX_CLASS_NUM*15];
      sprintf(one_image_string,"<tr><td>%d</td>",(int)(test_sample_index/tiles)+1);  /* image index */
      if (class_num>0) /* normlization factor */
      {
        if( normalization_factor_avg < 0.001 )
          sprintf( buffer,"<td>%.3e</td>", normalization_factor_avg );
        else
          sprintf( buffer,"<td>%.3f</td>", normalization_factor_avg );
        strcat(one_image_string, buffer);
      } 
      for (class_index=1;class_index<=class_num;class_index++)
      {  if (class_index==sample_class) sprintf(buffer,"<td><b>%.3f</b></td>",probabilities_sum[class_index]);  /* put the actual class in bold */
         else sprintf(buffer,"<td>%.3f</td>",probabilities_sum[class_index]);
         strcat(one_image_string,buffer);
      }
      if (predicted_class==sample_class) sprintf(color,"<font color=\"#00FF00\">Correct</font>");
      else sprintf(color,"<font color=\"#FF0000\">Incorrect</font>");

      /* add the interpolated value */
      if (interpolate)  
      {  
        if( class_num > 0 )  /* interpolate by the values of the class names */
        {
          // Method 1: create an interpolated value based only on the top
          // two marginal probabilities
//          double second_highest_prob = -1.0, min_prob = INF;
//          int second_highest_class;
//          for( class_index = 1; class_index <= class_num; class_index++ )
//          if( probabilities_sum[class_index] < min_prob )
//            min_prob=probabilities_sum[class_index];
//
//          /* subtract the min value from all classes to reduce the noise */
//          for( class_index = 1; class_index <= class_num; class_index++ )
//            probabilities_sum[class_index] -= min_prob;
//
//          for (class_index=1;class_index<=class_num;class_index++)
//            if (probabilities_sum[class_index]>second_highest_prob && class_index!=predicted_class) 
//            {  second_highest_prob=probabilities_sum[class_index];
//               second_highest_class=class_index;
//            }
//          TestSet->samples[test_sample_index]->interpolated_value=(second_highest_prob*atof(class_labels[second_highest_class])+probabilities_sum[predicted_class]*atof(class_labels[predicted_class]))/(second_highest_prob+probabilities_sum[predicted_class]);
          // Method 2: use all the marginal probabilities
          TestSet->samples[test_sample_index]->interpolated_value=0;			
          for( class_index = 1; class_index <= class_num; class_index++ )
            TestSet->samples[ test_sample_index ]->interpolated_value += 
              probabilities_sum[class_index] * atof( class_labels[ class_index ] );

         }
		 sprintf(interpolated_value,"<td>%.3f</td>",TestSet->samples[test_sample_index]->interpolated_value);
      }
      else strcpy(interpolated_value,"");
      
      if (closest_sample) sprintf(closest_image,"<td><A HREF=\"%s\"><IMG WIDTH=40 HEIGHT=40 SRC=\"%s__1\"></A></td>",closest_sample->full_path,closest_sample->full_path);
      else strcpy(closest_image,"");
      if (class_num==0) sprintf(buffer,"<td></td><td>%.3f</td><td>%.3f</td>",TestSet->samples[test_sample_index]->sample_value,TestSet->samples[test_sample_index]->interpolated_value);
      else sprintf(buffer,"<td></td><td>%s</td><td>%s</td><td>%s</td>%s",class_labels[sample_class],class_labels[predicted_class],color,interpolated_value);
      strcat(one_image_string,buffer);
      sprintf(buffer,"<td><A HREF=\"%s\"><IMG WIDTH=40 HEIGHT=40 SRC=\"%s__1\"></A></td>%s</tr>\n",TestSet->samples[test_sample_index]->full_path,TestSet->samples[test_sample_index]->full_path,closest_image); /* add the links to the image */
      strcat(one_image_string,buffer);	  
      strcat(split->individual_images,one_image_string);   /* add the image to the string */
   }  
   /* end of report string */
   
   if (similarities) for (class_index=1;class_index<=class_num;class_index++) similarities[class_index]=probabilities_sum[class_index];
   if (class_num==0) return(value);
   else return(predicted_class);
}

/* Test
   Test the classification accuracy using two sets of signatures
   method -int- 0 - WNN,   1 - WND-5
   split -*data_split- a pointer to a data split structure which contains the similarity matrix, confusion matrix, report string, feature_names, etc. ignored if NULL.
   tiles -int- number of tiles of each image.
   rank -long- the number of first closest classes among which a presence of the right class is considered a match
   max_tile -int- use only the most similar tile
*/
double TrainingSet::Test(TrainingSet *TestSet, int method, int tiles, int tile_areas, TrainingSet *TilesTrainingSets[], int max_tile,long rank, data_split *split)
{  int test_sample_index,class_index,b;//tile_index;
   long accurate_prediction=0;//,interpolate=1;
   if (tiles<1) tiles=1;       /* make sure the number of tiles is at least 1 */
   if (rank<=0) rank=1;  /* set a valid value to rank                */
   if (split && split->individual_images) strcpy(split->individual_images,"");    /* make sure the string is initially empty */
           
   /*initialize the confusion and similarity matrix */
   if (split && split->confusion_matrix)
     for (class_index=0;class_index<(class_num+1)*(class_num+1);class_index++) split->confusion_matrix[class_index]=0;
   if (split && split->similarity_matrix)
     for (class_index=0;class_index<(class_num+1)*(class_num+1);class_index++) split->similarity_matrix[class_index]=0.0;
   if (split && split->image_similarities)
     for (class_index=0;class_index<(TestSet->count/tiles+1)*(TestSet->count/tiles+1);class_index++) split->image_similarities[class_index]=0.0;

   /* test */
//   tile_index=0;
//   for (class_index=0;class_index<=class_num;class_index++) probabilities_sum[class_index]=0;
   /* start going over the test samples */
//TestSet->count=250*tiles;   /* FERET */
   for (test_sample_index=0;test_sample_index<TestSet->count;test_sample_index+=tiles)
   if (class_num==0)   /* continouos values */
   {  double value=ClassifyImage(TestSet,test_sample_index,method,tiles,tile_areas,TilesTrainingSets,max_tile,rank,split,NULL);
      if (print_to_screen) printf("Actual value : %f     Predicted value : %f     (%d/%d)\n",TestSet->samples[test_sample_index]->sample_value,value,(test_sample_index)/tiles+1,TestSet->count/tiles);
   }
   else  /* discrete classes */
   {  long predicted_class=(long)(ClassifyImage(TestSet,test_sample_index,method,tiles,tile_areas,TilesTrainingSets,max_tile,rank,split,NULL));
      if (predicted_class==TestSet->samples[test_sample_index]->sample_class) accurate_prediction++;
      if (print_to_screen) printf("Actual class ID: %d     Predicted class ID: %d      Ac: %f   (%d/%d)\n",TestSet->samples[test_sample_index]->sample_class,predicted_class,(double)(accurate_prediction)/(double)(test_sample_index/tiles+1),accurate_prediction,(test_sample_index)/tiles+1);   
   }

   /* normalize the similarity matrix */
   if (split && split->similarity_matrix)
     for (class_index=1;class_index<=class_num;class_index++)
     { double class_sim;
       int class_test_samples=0;
	   for (b=1;b<=class_num;b++)
	     class_test_samples+=split->confusion_matrix[class_num*class_index+b];
       class_sim=split->similarity_matrix[class_num*class_index+class_index]/class_test_samples; 
       for (b=1;b<=class_num;b++)
         split->similarity_matrix[class_num*class_index+b]/=(class_test_samples*class_sim);
       if (split->similarity_normalization) split->similarity_normalization[class_index]=class_sim;
     }

   /* normalize the image similarities */
   if (split && split->image_similarities)
   {  double min_dist=INF,max_dist=0.0;
      /* subtract the minimum distance */
      for (test_sample_index=0;test_sample_index<TestSet->count;test_sample_index+=tiles)
        for (b=0;b<TestSet->count;b++)
          if (split->image_similarities[(1+test_sample_index/tiles)*(TestSet->count/tiles+1)+b/tiles+1]>0 && split->image_similarities[(1+test_sample_index/tiles)*(TestSet->count/tiles+1)+b/tiles+1]<min_dist) min_dist=split->image_similarities[(1+test_sample_index/tiles)*(TestSet->count/tiles+1)+b/tiles+1]; 
      for (test_sample_index=0;test_sample_index<TestSet->count;test_sample_index+=tiles)
        for (b=0;b<TestSet->count;b+=tiles)
          split->image_similarities[(1+test_sample_index/tiles)*(TestSet->count/tiles+1)+b/tiles+1]-=min_dist;
      /* divide by the maximal distance */
      for (test_sample_index=0;test_sample_index<TestSet->count;test_sample_index+=tiles)
        for (b=0;b<TestSet->count;b+=tiles)
           if (split->image_similarities[(1+test_sample_index/tiles)*(TestSet->count/tiles+1)+b/tiles+1]>max_dist) max_dist=split->image_similarities[(1+test_sample_index/tiles)*(TestSet->count/tiles+1)+b/tiles+1];
      for (test_sample_index=0;test_sample_index<TestSet->count;test_sample_index+=tiles)
        for (b=0;b<TestSet->count;b+=tiles)
          split->image_similarities[(1+test_sample_index/tiles)*(TestSet->count/tiles+1)+b/tiles+1]/=max_dist;
   }

   if (class_num==0) return(0);   /* no classification accuracy if continouos values are used */
   return((double)accurate_prediction/(TestSet->count/tiles));
}


/* normalize
   normalize the signature in the training set to the interval [0,100]
*/

void TrainingSet::normalize()
{  int sig_index,samp_index,max_value_index;
   double *sig_data,min_value,max_value;
   sig_data=new double[count];
   for (sig_index=0;sig_index<signature_count;sig_index++)
   {  for (samp_index=0;samp_index<count;samp_index++)
        sig_data[samp_index]=samples[samp_index]->data[sig_index].value;
      qsort(sig_data,count,sizeof(double),compare_two_doubles);
      max_value_index=count-1;
      while (sig_data[max_value_index]==INF && max_value_index>0) max_value_index--;  /* make sure the maximum value is not SIG_INF */
      max_value=sig_data[(int)(0.975*max_value_index)];
      min_value=sig_data[(int)(0.025*count)];
      SignatureMaxes[sig_index]=max_value;   /* these values of min and max can be used for normalizing a test vector */
      SignatureMins[sig_index]=min_value;
      for (samp_index=0;samp_index<count;samp_index++)
      { if (samples[samp_index]->data[sig_index].value>=INF) samples[samp_index]->data[sig_index].value=0;
        else
        if (samples[samp_index]->data[sig_index].value<min_value) samples[samp_index]->data[sig_index].value=min_value;
        else
        if (samples[samp_index]->data[sig_index].value>max_value) samples[samp_index]->data[sig_index].value=max_value;
        else
        if (min_value>=max_value) samples[samp_index]->data[sig_index].value=0; /* prevent possible division by zero */
        else
        samples[samp_index]->data[sig_index].value=100*(samples[samp_index]->data[sig_index].value-min_value)/(max_value-min_value);
      }
//      if (class_num<=1)  /* normalize by the values */
//      {  double mean_ground=0,stddev_ground=0,mean=0,stddev=0;
//         for (samp_index=0;samp_index<count;samp_index++)  /* compute the mean of the interpolated values */
//           mean_ground+=(samples[samp_index]->interpolated_value/((double)count));
//         for (samp_index=0;samp_index<count;samp_index++)  /* compute the stddev of the interpolated values */
//           stddev_ground+=pow(samples[samp_index]->interpolated_value-mean_ground,2);
//         stddev_ground=sqrt(stddev_ground/count);

//         for (samp_index=0;samp_index<count;samp_index++) 
//            if (samples[samp_index]->data[sig_index].value>=INF) samples[samp_index]->data[sig_index].value=0;		 

//         for (samp_index=0;samp_index<count;samp_index++)  /* compute the mean of the original signature values */
//           mean+=(samples[samp_index]->data[sig_index].value/((double)count));
//         for (samp_index=0;samp_index<count;samp_index++)  /* compute the stddev of the original signature values */
//           stddev+=pow(samples[samp_index]->data[sig_index].value-mean,2);
//         stddev=sqrt(stddev/count);		   
//         for (samp_index=0;samp_index<count;samp_index++)
//         {  samples[samp_index]->data[sig_index].value-=(mean-mean_ground);
//			if (stddev>0) samples[samp_index]->data[sig_index].value=mean_ground+(samples[samp_index]->data[sig_index].value-mean_ground)*(stddev_ground/stddev);	  
//         }		 
//      }
   }
   delete sig_data;
}


/* SetFisherScores
   Compute the fisher score of each signature
   used_signatures -double- what fraction of the signatures should be used (a value between 0 and 1).
   sorted_feature_names -char *- a text of the names and scores of the features (NULL to ignore)
   int method - 0 for Fisher Scores. 1 for Pearson Correlation scores (with the ground truth).
*/

void TrainingSet::SetmRMRScores(double used_signatures, double used_mrmr)
{  FILE *mrmr_file;
   char buffer[512],*p_buffer; 
   int sig_index,sample_index;

   /* use mRMR (if an executable file "mrmr" exists) */
   if (mrmr_file=fopen("mrmr","r")) fclose(mrmr_file);
   if (mrmr_file)  /* use mrmr */
   {  /* first create a csv file for mrmr */
      mrmr_file=fopen("mrmr_sigs.csv","w");
      fprintf(mrmr_file,"class");
	  for (sig_index=0;sig_index<signature_count;sig_index++)
         if (SignatureWeights[sig_index]>0) fprintf(mrmr_file,",%d",sig_index);
      fprintf(mrmr_file,"\n");
      for (sample_index=0;sample_index<count;sample_index++)
      {  fprintf(mrmr_file,"%d",samples[sample_index]->sample_class);
	     for (sig_index=0;sig_index<signature_count;sig_index++)
           if (SignatureWeights[sig_index]>0) fprintf(mrmr_file,",%.0f",samples[sample_index]->data[sig_index]);
         fprintf(mrmr_file,"\n");
      }
	  fclose(mrmr_file);
      sprintf(buffer,"./mrmr -i mrmr_sigs.csv -n %d -s %d -v %d > mrmr_output",(long)(used_mrmr*used_signatures*signature_count),count,signature_count);
      printf("%s\n",buffer);
	  system(buffer);	  
      remove("mrmr_sigs.csv");
      /* now read the mRMR output file */
	  for (sig_index=0;sig_index<signature_count;sig_index++)  /* first set all scores to zero */
         SignatureWeights[sig_index]=0.0;	  
      if (!(mrmr_file=fopen("mrmr_output","r"))) printf("Cannot open file 'mrmr_sigs.csv'\n");
	  p_buffer=fgets(buffer,sizeof(buffer),mrmr_file); /* skip the first lines */
      while (p_buffer && strstr(p_buffer,"mRMR")==NULL) p_buffer=fgets(buffer,sizeof(buffer),mrmr_file);
      if (!p_buffer) printf("Cannot parse file 'mrmr_output'\n");	  
	  p_buffer=fgets(buffer,sizeof(buffer),mrmr_file); /* skip the first line */	  
	  p_buffer=fgets(buffer,sizeof(buffer),mrmr_file); /* skip the first line */	  	  
      while(p_buffer && strlen(p_buffer)>8)
	  {  long sig_num;
         double weight;
	     strtok(p_buffer," \t\n");
         strtok(NULL," \t\n");		 
         sig_num=atoi(strtok(NULL," \t\n"));
        weight=atof(strtok(NULL," \t\n"));
        if (weight<0) weight=0.0;   /* make sure the values are not negative */
		if (weight>0) SignatureWeights[sig_num]=pow(weight,1);
         p_buffer=fgets(buffer,sizeof(buffer),mrmr_file);	  
	  }
	  fclose(mrmr_file);
      remove("mrmr_output");	 
   }   
}

void TrainingSet::SetFisherScores(double used_signatures, double used_mrmr, data_split *split)
{  int sample_index,sig_index,class_index;
   double mean,var,class_dev_from_mean,mean_inner_class_var;
   double *class_mean,*class_var,*class_count;
   double signature_weight_values[MAX_SIGNATURE_NUM],threshold;   

   double FeatureGroupValues[MAX_SIGNATURE_NUM];

   int FeatureGroupCount[MAX_SIGNATURE_NUM];
   for( int ii = 0; ii < MAX_SIGNATURE_NUM; ii++ ) 
     FeatureGroupCount[ii] = 0;

   char FeatureGroupNames[MAX_SIGNATURE_NUM][SIGNATURE_NAME_LENGTH];
   int fg_index = 0;

   double sum_of_group=0.0;
   char current_name[256]={'\0'},last_name[256]={'\0'},full_last_name[256],feature_string[128];  
   class_mean=new double[class_num+1];
   class_var=new double[class_num+1];
   class_count=new double[class_num+1];

   if (split && split->feature_groups) strcpy(split->feature_groups,"");

   /* use Fisher scores (for classes) or correlation scores (for correlations) */
   for (sig_index=0;sig_index<signature_count;sig_index++)
   {
      if (class_num>0)  /* Fisher Scores */
      {  /* initialize */
         for (class_index=0;class_index<=class_num;class_index++)
         {  class_mean[class_index]=0.0;
            class_var[class_index]=0.0;
            class_count[class_index]=0.0;
         }
         mean=var=0.0;
         /* find the means */
         for (sample_index=0;sample_index<count;sample_index++)
         {  class_mean[samples[sample_index]->sample_class]+=samples[sample_index]->data[sig_index].value;
            class_count[samples[sample_index]->sample_class]+=1;
         }

         for (class_index=1;class_index<=class_num;class_index++)
           if (class_count[class_index])
             class_mean[class_index]/=class_count[class_index];

         /* find the variance */
         for (sample_index=0;sample_index<count;sample_index++)
           class_var[samples[sample_index]->sample_class]+=pow(samples[sample_index]->data[sig_index].value-class_mean[samples[sample_index]->sample_class],2);

         for (class_index=1;class_index<=class_num;class_index++)
           if (class_count[class_index])
             class_var[class_index]/=class_count[class_index];

         /* compute fisher score */

         /* find the mean of all means */
         for (class_index=1;class_index<=class_num;class_index++)
           mean+=class_mean[class_index];
         mean/=class_num;
         /* find the variance of all means */
         class_dev_from_mean=0;
         for (class_index=1;class_index<=class_num;class_index++)
           class_dev_from_mean+=pow(class_mean[class_index]-mean,2);
         if (class_num>1) class_dev_from_mean/=(class_num-1);
	     else class_dev_from_mean=0;

         mean_inner_class_var=0;
         for (class_index=1;class_index<=class_num;class_index++)
           mean_inner_class_var+=class_var[class_index];
         mean_inner_class_var/=class_num;
         if (mean_inner_class_var==0) mean_inner_class_var+=0.000001;   /* avoid division by zero - and avoid INF values */

         SignatureWeights[sig_index]=class_dev_from_mean/mean_inner_class_var;
///*
//char *p1,*p2;
//p1=strrchr(SignatureNames[sig_index],'#');
//p2=strrchr(SignatureNames[sig_index],'_');
//if (!p1) SignatureWeights[sig_index]=0;
//if (p1) if (((long)p2-(long)p1)>3) SignatureWeights[sig_index]=0;
//*/		 

//if (strchr(SignatureNames[sig_index],'(') && (strstr(SignatureNames[sig_index],"()")==NULL)) SignatureWeights[sig_index]=0;
//if ( (1)
//&& (strstr(SignatureNames[sig_index],"lick")==NULL)
//&& (strstr(SignatureNames[sig_index],"oment")==NULL) 
//&& (strstr(SignatureNames[sig_index],"dge")==NULL) 
//&& (strstr(SignatureNames[sig_index],"ernike")==NULL) 
//&& (strstr(SignatureNames[sig_index],"eature")==NULL) 
//&& (strstr(SignatureNames[sig_index],"amura")==NULL) 
//&& (strstr(SignatureNames[sig_index],"abor")==NULL) 
//&& (strstr(SignatureNames[sig_index],"istogram")==NULL) 
//&& (strstr(SignatureNames[sig_index],"hebyshev")==NULL) 
//&& (strstr(SignatureNames[sig_index],"adon")==NULL) 
//) SignatureWeights[sig_index]=0;
//if (SignatureWeights[sig_index]>0) printf("%s\n",SignatureNames[sig_index]);

      }  /* end of method 0 (Fisher Scores) */

      /* Pearson Correlation scores */
      if (class_num==0)
      {  double mean_ground=0,stddev_ground=0,mean=0,stddev=0,z_score_sum=0;
         for (sample_index=0;sample_index<count;sample_index++)  /* compute the mean of the continouos values */
           mean_ground+=(samples[sample_index]->sample_value/((double)count));
         for (sample_index=0;sample_index<count;sample_index++)  /* compute the stddev of the continouos values */
           stddev_ground+=pow(samples[sample_index]->sample_value-mean_ground,2);	  
         stddev_ground=sqrt(stddev_ground/count);
         for (sample_index=0;sample_index<count;sample_index++)
           mean+=(samples[sample_index]->data[sig_index].value/((double)count));
         for (sample_index=0;sample_index<count;sample_index++)  /* compute the stddev of the continouos values */
           stddev+=pow(samples[sample_index]->data[sig_index].value-mean,2);	  
         stddev=sqrt(stddev/count);	
         for (sample_index=0;sample_index<count;sample_index++)
           if (stddev>0 && stddev_ground>0) z_score_sum+=((samples[sample_index]->sample_value-mean_ground)/stddev_ground)*((samples[sample_index]->data[sig_index].value-mean)/stddev);
         SignatureWeights[sig_index]=pow(fabs(z_score_sum/count),1);
//printf("%d Fisher Score: %f\n",class_num,SignatureWeights[sig_index]);		 
	  } /* end of method 1 (Pearson Correlation) */
	  
      /* add the sums of the scores of each group of features */
      if( split && split->feature_groups && 
          SignatureNames[ sig_index ][ 0 ] >= 'A' && SignatureNames[ sig_index ][ 0 ] <= 'Z' )
      {
        strcpy( current_name, SignatureNames[ sig_index ] );
        if( strchr( current_name, ' ' ) )
        {
          *( strchr( current_name,' ' ) ) = '\0';
        }
        if( ( strcmp( current_name, last_name ) != 0 ) && ( sig_index > 0 ) )
        {  
          int char_index;
          for( char_index = 0; char_index < strlen( full_last_name ); char_index++ )
          {
            if( isdigit( full_last_name[ char_index ] ) )
              full_last_name[ char_index ] = ' ';
          }
          if( strstr( full_last_name, "bin " ) )
            strncpy( strstr( full_last_name, "bin " ), "   ", 3 );
          if( strncmp( full_last_name, "Feature", 7 ) == 0 )
            strcpy( full_last_name, "Feature Statistics" );
          if( strncmp( full_last_name, "Edge", 4 ) == 0 )
            strcpy( full_last_name, "Edge Statistics" );

          //sprintf( feature_string, "%d. %s: %f\n", group,full_last_name, sum_of_group );
          //strcat( split->feature_groups, feature_string );
          
          strcpy( FeatureGroupNames[fg_index], full_last_name );
          FeatureGroupValues[fg_index] = sum_of_group;
          fg_index++;

          sum_of_group = 0.0;
        }
        sum_of_group += SignatureWeights[ sig_index ];
        FeatureGroupCount[fg_index]++;
        strcpy( last_name, current_name );
        strcpy( full_last_name, SignatureNames[ sig_index ] );
      }
   }

   // Insert feature group sorting code here:

   /* copy the feature names and scores into a string. the features will be ordered by their scores */

   double sortedFeatGroupValues[MAX_SIGNATURE_NUM];

   for( sig_index = 0; sig_index < signature_count; sig_index++ )
      sortedFeatGroupValues[sig_index ] = FeatureGroupValues[ sig_index ];

   qsort( sortedFeatGroupValues, signature_count, sizeof(double), compare_two_doubles );

   if( split && split->feature_names )
   {  
     int sig_index2;
     split->feature_groups[0] = '\0';
     for( sig_index = signature_count - 1; sig_index >= 0; sig_index-- )
        for( sig_index2 = 0; sig_index2 < signature_count; sig_index2++ )
          // This method assumes all weight values are unique
          if( sortedFeatGroupValues[ sig_index ] == FeatureGroupValues[ sig_index2 ] &&
              sortedFeatGroupValues[ sig_index ] > 0 )
          { 
            sprintf( feature_string, "%d. %s: %f [%d]\n",
                        signature_count - sig_index,
                        FeatureGroupNames[ sig_index2 ],
                        FeatureGroupValues[ sig_index2 ],
                        FeatureGroupCount[ sig_index2 ] );
            strcat( split->feature_groups, feature_string );
            break;                                         /* no need to complete the loop */
          }
   }   

   /* now set to 0 all signatures that are below the threshold */
   for (sig_index=0;sig_index<signature_count;sig_index++)
     signature_weight_values[sig_index]=SignatureWeights[sig_index];
   qsort(signature_weight_values,signature_count,sizeof(double),compare_two_doubles);
   threshold=signature_weight_values[(int)((1-used_signatures)*signature_count)];
   for (sig_index=0;sig_index<signature_count;sig_index++)
     if (SignatureWeights[sig_index]<threshold) SignatureWeights[sig_index]=0.0;

   if (used_mrmr>0) SetmRMRScores(used_signatures,used_mrmr);  /* filter the most informative features using mrmr */

   /* copy the feature names and scores into a string. the features will be ordered by their scores */
   if( split && split->feature_names )
   { 
     int sig_index2;
     split->feature_names[0]='\0';

     for( sig_index = signature_count - 1;
          sig_index >= (long)( ( 1 - used_signatures ) * signature_count ) - 5 ;
          sig_index-- )
     {
       for( sig_index2 = 0; sig_index2 < signature_count; sig_index2++ )
       {
         if( signature_weight_values[sig_index] == SignatureWeights[sig_index2] &&
             signature_weight_values[sig_index] > 0 )
         {  
           sprintf( feature_string, "%d. %s: %f\n",
                      signature_count - sig_index,
                      SignatureNames[sig_index2],
                      SignatureWeights[sig_index2] );
           strcat( split->feature_names, feature_string );
           break;   // no need to complete the loop
         }
       }
     }
   }

   delete class_mean;
   delete class_var;
   delete class_count;
}


/* IgnoreFeatureGroup
   classify without using one of the feature groups (identified by 'index'). This function is used for assessing the contribution of the different image features.
   index -long- the index (in the group order) of the group to be ignored
   group_name -char *- the name of the ignored group. This output variable is ignored if NULL.
*/
int TrainingSet::IgnoreFeatureGroup(long index,char *group_name)
{  int group=0,sig_index=0,char_index;
   char current_name[256]={'\0'},last_name[256]={'\0'};
   
   while(group<=index)
   {  if (sig_index>=signature_count) return(0);   /* no more image features */
      while (SignatureNames[sig_index][0]<'A' || SignatureNames[sig_index][0]>'Z') sig_index++;
      strcpy(current_name,SignatureNames[sig_index]);
      if (strchr(current_name,' ')) *(strchr(current_name,' '))='\0';
      if (strcmp(current_name,last_name)!=0) group++;
	  strcpy(last_name,current_name);
	  if (group==index) 
	  {  SignatureWeights[sig_index]=0;
         if (group_name) strcpy(group_name,SignatureNames[sig_index]);   /* return the name of the group */	  
         for (char_index=0;char_index<strlen(group_name);char_index++) if (isdigit(group_name[char_index])) group_name[char_index]=' ';          
      }
	  sig_index++;
   }
   return(1);
}

/* distance 
   Find the weighted Euclidean distance between two samples
*/
double TrainingSet::distance(signatures *sample1, signatures *sample2, double power)
{   double dist=0;
    int sig_index;	
      for (sig_index=0;sig_index<signature_count;sig_index++)
        dist=dist+pow(SignatureWeights[sig_index],1)*pow(sample1->data[sig_index].value-sample2->data[sig_index].value,power);
    return(pow(dist,1/power));
}

/* WNNclassify
   classify a given sample using weighted nearest neioghbor
   test_sample -signature *- a given sample to classify
   probabilities -array of double- an array (size num_classes) marginal probabilities of the given sample from each class. (ignored if NULL).
   normalization_factor -double *- the normalization factor used to compute the marginal probabilities from the distances normalization_factor=1/(sum_dist*marginal_prob). ignored if NULL.
   closest_sample -signatures **- a pointer to the closest sample found. (ignored if NULL).
   returned value -long- the predicted class of the sample

   comment: must set weights before calling to this function
*/
long TrainingSet::WNNclassify(signatures *test_sample, double *probabilities, double *normalization_factor,signatures **closest_sample)
{  int class_index,sample_index,sig_index;
   long most_probable_class;
   double closest_dist=INF;

   /* initialize the probabilities */
   if (probabilities)
     for (class_index=0;class_index<=class_num;class_index++)
        probabilities[class_index]=INF;

   /* normalize the test sample */
   test_sample->normalize(this);
   for (sample_index=0;sample_index<count;sample_index++)
   {  double dist=distance(test_sample,samples[sample_index],2.0);
      if ((dist<1/INF) || (strcmp(samples[sample_index]->full_path,test_sample->full_path)==0)) dist=INF;    /* ignore images that are 100% identical */
//if (strstr(samples[sample_index]->full_path,"1948")==NULL) dist=INF;	  
      if (dist<closest_dist)
      {  closest_dist=dist;
         most_probable_class=samples[sample_index]->sample_class;
         if (closest_sample) *closest_sample=samples[sample_index];		 
      }
      /* set the distance from classes */
      if (probabilities)
        if (dist<probabilities[samples[sample_index]->sample_class])
          probabilities[samples[sample_index]->sample_class]=dist;
   }
    
   /* normalize the marginal probabilities */
   if (probabilities)
   {  double sum_dists=0;
      for (class_index=1;class_index<=class_num;class_index++)
        if (probabilities[class_index]!=0)
          sum_dists+=1/probabilities[class_index];
      for (class_index=1;class_index<=class_num;class_index++)
        if (sum_dists==0) probabilities[class_index]=0;    /* protect from division by zero */
        else
          if (probabilities[class_index]==0) probabilities[class_index]=1.0; /* exact match */
          else probabilities[class_index]=(1/probabilities[class_index])/sum_dists;
      if (normalization_factor) *normalization_factor=sum_dists;
   }

   return(most_probable_class);
}


/* classify2
   classify a given sample
   test_sample -signature *- a given sample to classify
   probabilities -array of double- an array (size num_classes) marginal probabilities of the given sample from each class. (ignored if NULL).
   normalization_factor -double *- the normalization factor used to compute the marginal probabilities from the distances normalization_factor=1/(dist*marginal_prob). Ignored if NULL.   
   returned value -long- the predicted class of the sample

   comment: must set weights before calling to this function
*/
long TrainingSet::classify2(signatures *test_sample, double *probabilities, double *normalization_factor)
{  int sample_index,class_index,sig_index;
   long most_probable_class;
   double samp_sum,*class_sum; //,*samples_num;
   double dist,closest_dist=INF;
   int *samples_num;

   /* normalize the test sample */
   test_sample->normalize(this);

   /* allocate and initialize memory */
   class_sum=new double[class_num+1];
   samples_num=new int[class_num+1];
   for (class_index=0;class_index<=class_num;class_index++)
   {  
     class_sum[class_index]=0.0;
     samples_num[class_index]=0;
   }

   for (sample_index=0;sample_index<count;sample_index++)
   {
     samp_sum=0.0;
     for (sig_index=0;sig_index<signature_count;sig_index++)
     {
       samp_sum+=pow(SignatureWeights[sig_index],2)*pow(test_sample->data[sig_index].value-samples[sample_index]->data[sig_index].value,2);
     }
     //if( samp_sum == 0.0 ) continue; /* ignore images that are 100% identical */
     if( samp_sum < 0.1 ) continue; // try to weed out matches that got through due to floating point error 
     class_sum[samples[sample_index]->sample_class]+=pow(samp_sum,-5);
     samples_num[samples[sample_index]->sample_class]++;
     /* printf( "\ttest img index %i, test img class: %i, dist w/o ^-5 %f, dist w/ ^-5 %e, class sum so far: %e, number of test images from this class seen so far: %d\n",
         sample_index, samples[sample_index]->sample_class, samp_sum, pow(samp_sum, -5), class_sum[samples[sample_index]->sample_class], samples_num[samples[sample_index]->sample_class]);
      */
   }
	 
   for (class_index=1;class_index<=class_num;class_index++)
   {
     if( samples_num[class_index]==0 )
       class_sum[class_index]=INF;   /* no samples for this class */
     else
       class_sum[class_index]/=samples_num[class_index];    /* find the average distance per sample */
     
     // printf( "Dist to class %d = %e = %e class_sum / %d samples\n", class_index, class_sum[class_index]/=samples_num[class_index], class_sum[class_index], samples_num[class_index] );

     dist=class_sum[class_index];
     if( dist < closest_dist )
     {  
       closest_dist=dist;
       most_probable_class=class_index;
     }
   }
   
   /* normalize the marginal probabilities */
   if (probabilities)
   {  
     double sum_dists=0;

     //printf( "\n\n" );

     for( class_index = 1; class_index <= class_num; class_index++ )
       sum_dists += class_sum[class_index];

     // printf( "Sum of all distances = %e\n", sum_dists );

     for( class_index = 1; class_index <= class_num; class_index++ )
       probabilities[class_index]=class_sum[class_index]/sum_dists;
     
     if (normalization_factor) *normalization_factor=sum_dists;
   }

   delete class_sum;
   delete samples_num;
   return(most_probable_class);
}

/* InterpolateValue
   Compute the interpolated value of a given test sample
   method -int- 0 for nearest neighbors, 1 for two closest samples
   N -int- number of neighbors to use
   closest_dist -double *- if not NULL holds the distance to the closest sample
*/
double TrainingSet::InterpolateValue(signatures *test_sample, int method, int N, signatures **closest_sample, double *closest_dist)
{  int sample_index,close_index;
   double *min_dists,*min_dists_values,val=0.0,sum=0.0;
// double min_dist_up=INF,min_dist_down=-INF,min_val_up,min_val_down;

   /* normalize the test sample */
   test_sample->normalize(this);
      
//   if (method==0)
   {  min_dists=new double[N];
      min_dists_values=new double[N];
      for (close_index=0;close_index<N;close_index++)
        min_dists[close_index]=INF;

      /* find the closest samples */
      for (sample_index=0;sample_index<count;sample_index++)
      {  double dist=distance(test_sample,samples[sample_index],2.0);
//printf("dist: %f   %f\n",dist,samples[sample_index]->sample_value);	  
         if (closest_sample && dist<min_dists[0]) *closest_sample=samples[sample_index];  /* for returning the closest sample */	  
         if (closest_dist && dist<min_dists[0]) *closest_dist=dist;                       /* for returning the distanmce to the closest sample */	  		 
         for (close_index=0;close_index<N;close_index++)
         if (dist<min_dists[close_index])
         {  memmove(&(min_dists[close_index+1]),&(min_dists[close_index]),sizeof(double)*(N-1-close_index));
            memmove(&(min_dists_values[close_index+1]),&(min_dists_values[close_index]),sizeof(long)*(N-1-close_index));
            min_dists[close_index]=dist;
            min_dists_values[close_index]=samples[sample_index]->sample_value;
//printf("%d %f %f %f %f\n",N,min_dists_values[0],min_dists[0],min_dists_values[1],min_dists[1]);				
            break;
         }
      }

      /* compute the weighted average value */
      for (close_index=0;close_index<N;close_index++)   
        if (min_dists[close_index]<INF)
        {  val+=min_dists_values[close_index]*(1/min_dists[close_index]);
           sum+=(1/min_dists[close_index]);
//printf("%d %f %f\n",close_index,min_dists_values[close_index],min_dists[close_index]);		   
        }
//printf("%d %f %f %f %f\n",N,min_dists_values[0],min_dists[0],min_dists_values[1],min_dists[1]);		
      delete min_dists;
      delete min_dists_values;
//printf("%f %f %f\n",val,sum,val/sum);	  		
      return(val/sum);
   }
//   if (method==1)
//   {  for (sample_index=0;sample_index<count;sample_index++)
//      {  if (distance(test_sample,samples[sample_index],1.0)>=0 && distance(test_sample,samples[sample_index],1.0)<min_dist_up) 
//         {  min_dist_up=distance(test_sample,samples[sample_index],1.0);
//            min_val_up=samples[sample_index]->sample_value;
//printf("val up: %f %f\n",min_val_up,min_dist_up);
//            if (closest_sample) if (min_dist_up<fabs(min_dist_down)) *closest_sample=samples[sample_index];					 
//         }
//         if (distance(test_sample,samples[sample_index],1.0)<=0 && distance(test_sample,samples[sample_index],1.0)>min_dist_down) 
//         {  min_dist_down=distance(test_sample,samples[sample_index],1.0);
//            min_val_down=samples[sample_index]->sample_value;
//            if (closest_sample) if (min_dist_down<fabs(min_dist_up)) *closest_sample=samples[sample_index];					 			
//         }
//      }
//printf("%f %f %f %f\n",min_val_down,min_val_up,min_dist_down,min_dist_up);	  
//      if (min_dist_up==INF) return(min_val_down);
//      else if (min_dist_down==INF) return(min_val_up);	  
//      else return(min_val_down+(min_val_up-min_val_down)*((-1*min_dist_down)/(min_dist_up+min_dist_down*-1)));	  
//   }   
}

/* classify3
   test_sample -signature *- a given sample to classify
   probabilities -array of double- an array (size num_classes) marginal probabilities of the given sample from each class. (ignored if NULL).
   normalization_factor -double *- the normalization factor used to compute the marginal probabilities from the distances normalization_factor=1/(dist*marginal_prob). Ignored if NULL.
*/
long TrainingSet::classify3(signatures *test_sample, double *probabilities,double *normalization_factor)
{  int dist_index,class_index,sig_index,sample_index;
   long *num_samples,*close_samples,min_samples=10000000;
   int max_class;
   double *min_dists;
   long *min_dists_classes;
   double *sig_probs;
   int most_probable_class;
   long double probs[MAX_CLASS_NUM];
   double dist;
   long size_of_class;

   /* initialize the probabilities */
   for (class_index=0;class_index<=class_num;class_index++)
     probs[class_index]=1;

   /* find the number of samples of the smallest class */
   num_samples=new long[class_num+1];
   close_samples=new long[class_num+1];
   for (class_index=0;class_index<=class_num;class_index++)
     num_samples[class_index]=0;
   for (sample_index=0;sample_index<count;sample_index++)
     num_samples[samples[sample_index]->sample_class]+=1;
   for (class_index=1;class_index<=class_num;class_index++)
     if (num_samples[class_index]<min_samples) min_samples=num_samples[class_index];

   min_dists=new double[count];
   min_dists_classes=new long[count];
   for (sig_index=0;sig_index<signature_count;sig_index++)
   {  int close_index,total_num_min_dist;
      for (dist_index=0;dist_index<min_samples;dist_index++)
        min_dists[dist_index]=INF;
      for (sample_index=0;sample_index<count;sample_index++)
      {
         dist=fabs(test_sample->data[sig_index].value-samples[sample_index]->data[sig_index].value);
         /* check if this dist should be in the close list */
         for (close_index=0;close_index<count;close_index++)
         if (dist<min_dists[close_index])
         {  memmove(&(min_dists[close_index+1]),&(min_dists[close_index]),sizeof(double)*(count-1-close_index));
            memmove(&(min_dists_classes[close_index+1]),&(min_dists_classes[close_index]),sizeof(long)*(count-1-close_index));
            min_dists[close_index]=dist;
            min_dists_classes[close_index]=samples[sample_index]->sample_class;
            break;
         }
      }

      /* find the actual range of the closest sample */
      sample_index=min_samples-1;
      dist=min_dists[sample_index];
      while (sample_index<count && min_dists[sample_index]==dist)
        sample_index++;
      size_of_class=sample_index;
      if (size_of_class>=count) continue; /* no point in continuing if they all equally close */

      /* find the number of times each class appears */
      for (class_index=1;class_index<=class_num;class_index++)
        close_samples[class_index]=0;
      for (close_index=0;close_index<size_of_class;close_index++)
        close_samples[min_dists_classes[close_index]]+=1;
      /* find the max class */
      max_class=0;
      for (class_index=1;class_index<=class_num;class_index++)
        if (close_samples[class_index]>max_class) max_class=close_samples[class_index];
      /* now find the probability of each class */
      if ((double)max_class/(double)min_samples>pow(1/(double)class_num,1.0/2.0))
      for (class_index=1;class_index<=class_num;class_index++)
      {  long double class_prob;
         class_prob=((double)size_of_class/(double)(num_samples[class_index]))*(double)(close_samples[class_index])/(double)size_of_class;
//if (class_prob<1.0/(double)class_num) class_prob=1.0/(double)class_num;
         probs[class_index]=probs[class_index]*class_prob;
      }

   }

   /* normalize the results and find the most probable class */
   if (probabilities)
   {  long double sum_dists=0.0;
      long double highest_prob=0.0;
      most_probable_class=0;
      for (class_index=1;class_index<=class_num;class_index++)
        if (probs[class_index]>highest_prob)
        {  highest_prob=probs[class_index];
           most_probable_class=class_index;
        }

      for (dist_index=1;dist_index<=class_num;dist_index++)
        if (probs[dist_index]!=0)
           sum_dists+=probs[dist_index];
      for (dist_index=1;dist_index<=class_num;dist_index++)
        if (sum_dists==0 || probs[dist_index]==0) probabilities[dist_index]=0;    /* protect from division by zero */
        else probabilities[dist_index]=(probs[dist_index])/sum_dists;
     if (normalization_factor) *normalization_factor=sum_dists;				
   }

   delete num_samples;
   delete min_dists;
   delete min_dists_classes;
   delete close_samples;
   return(most_probable_class);
}

/*  Pearson
    compute pearson correlation
	This function is used by wndchrm.cpp if all class labels are numerical.
	The class labels are used as the values of one variable, and the interpolated values are used as the other
    tiles -int- the number of tiles
    avg_abs_dif -double *- the average absolute difference from the predicted and actual value
*/
double gamma(double z)   /* implementation of the gamma function -  used for computing the P value */
{  double res=1;
   for (int n=1;n<1000000;n++)
     res*=(1/(1+z/n))*pow(2.7183,z/n);
   return(res*(pow(2.7183,-1*z*0.57721)/z));
}

double TrainingSet::pearson(int tiles, double *avg_abs_dif, double *p_value)
{  double mean=0,stddev=0,mean_ground=0,stddev_ground=0,z_score_sum=0,pearson_cor,N;
   int test_sample_index,class_index;
   if (tiles<=0) tiles=1;
   N=(double)count/(double)tiles;
   if (avg_abs_dif) *avg_abs_dif=0.0;
   /* check if the data can be interpolated (all class labels are numbers) */
   for (class_index=1;class_index<=class_num;class_index++)
     if (atof(class_labels[class_index])==0.0 && class_labels[class_index][0]!='0') return(0);
   /* compute the mean */
   for (test_sample_index=0;test_sample_index<count;test_sample_index+=tiles)
   {  mean+=samples[test_sample_index]->interpolated_value;
      if (class_num==0) mean_ground+=samples[test_sample_index]->sample_value;
      else mean_ground+=atof(class_labels[samples[test_sample_index]->sample_class]);
      if (avg_abs_dif) *avg_abs_dif=*avg_abs_dif+fabs(samples[test_sample_index]->sample_value-samples[test_sample_index]->interpolated_value)/N;
   }
   mean=mean/N;
   mean_ground=mean_ground/N;
   /* compute the stddev */
   for (test_sample_index=0;test_sample_index<count;test_sample_index+=tiles)
   {  stddev+=pow(samples[test_sample_index]->interpolated_value-mean,2);
      if (class_num==0) stddev_ground+=pow(samples[test_sample_index]->sample_value-mean_ground,2);
	  else stddev_ground+=pow(atof(class_labels[samples[test_sample_index]->sample_class])-mean_ground,2);
   }
   stddev=sqrt(stddev/(N-1));
   stddev_ground=sqrt(stddev_ground/(N-1));   
   /* now compute the pearson correlation */
   for (test_sample_index=0;test_sample_index<count;test_sample_index+=tiles)
     if (class_num==0) z_score_sum+=((samples[test_sample_index]->interpolated_value-mean)/stddev)*((samples[test_sample_index]->sample_value-mean_ground)/stddev_ground);
	 else z_score_sum+=((samples[test_sample_index]->interpolated_value-mean)/stddev)*((atof(class_labels[samples[test_sample_index]->sample_class])-mean_ground)/stddev_ground);
   pearson_cor=z_score_sum/(N-1);

   if (p_value) /* compute the P value of the pearson correlation */
   {  double t=pearson_cor*(sqrt(N-2)/sqrt(1-pearson_cor*pearson_cor));
      *p_value=(gamma(((N-2)+1)/2)/(sqrt((N-2)*3.14159265)*gamma((N-2)/2)))  *  pow((1+pow(t,2)/(N-2)),-1*(N-2+1)/2);
   }
   return(pearson_cor);
}

/* dendrogram
   generate a dendrogram 
filename -char *- a file name for   
sim_method -unsigned short- the method of transforming the similarity values into a single distance (0 - min, 1 - average. 2 - top triangle, 3 - bottom triangle).
phylip_algorithm -unsigned short- the method used by phylip
*/
long TrainingSet::dendrogram(FILE *output_file, char *data_set_name, char *phylib_path, int nodes_num,double *similarity_matrix, char **labels, unsigned short sim_method,unsigned short phylip_algorithm)
{  FILE *dend_file;
   int label_index,algorithm_index;
   char file_path[256],alg[16];
   sprintf(file_path,"%s/dend_file.txt",phylib_path);
   if (!(dend_file=fopen(file_path,"w"))) return(0);
   fprintf(dend_file,"%d\n",nodes_num);
   /* print the labels */
   for (label_index=1;label_index<=nodes_num;label_index++)
   {  char label[128];
      double dist=0.0;
      int label_index2;
      strcpy(label,labels[label_index]);
      if (strlen(label)>8) strcpy(label,&(label[strlen(label)-8]));  /* make sure the labels are shorter or equal to 8 characters in length */
      if (!isalnum(label[strlen(label)-1])) label[strlen(label)-1]='\0';
      fprintf(dend_file,"%s                 ",label);
      for (label_index2=1;label_index2<=nodes_num;label_index2++)	  
      {  if (sim_method==0) dist=max(1-similarity_matrix[label_index*nodes_num+label_index2],1-similarity_matrix[label_index2*nodes_num+label_index]);
         if (sim_method==1) dist=((1-similarity_matrix[label_index*nodes_num+label_index2])+(1-similarity_matrix[label_index2*nodes_num+label_index]))/2;
         if (sim_method==2) dist=(1-similarity_matrix[label_index*nodes_num+label_index2])*(label_index2>=label_index)+(1-similarity_matrix[label_index2*nodes_num+label_index])*(label_index2<label_index);  /* top triangle    */
         if (sim_method==3) dist=(1-similarity_matrix[label_index*nodes_num+label_index2])*(label_index2<=label_index)+(1-similarity_matrix[label_index2*nodes_num+label_index])*(label_index2>label_index);  /* bottom triangle */
         if (sim_method==4) dist=(similarity_matrix[label_index*nodes_num+label_index2]+similarity_matrix[label_index2*nodes_num+label_index])/2;
#ifndef WIN32
         if (isnan (dist)) dist=0;
#endif
         fprintf(dend_file,"%.4f       ",fabs(dist*(dist>=0)));
      }
	  fprintf(dend_file,"\n");
   }
   fclose(dend_file);

   /* *** generate a dendrogram *** */
   sprintf(file_path,"%s/fitch.infile",phylib_path);
   /* create fith.infile */   
   if (!(dend_file=fopen(file_path,"w"))) return(0);
   fprintf(dend_file,"%s/dend_file.txt\nJ\n97\n10\nY\n",phylib_path);
   fclose(dend_file);
   /* create drawtree.infile */			
   sprintf(file_path,"%s/drawtree.infile",phylib_path);
   if (!(dend_file=fopen(file_path,"w"))) return(0);
   alg[0]='\0';
   for (algorithm_index=0;algorithm_index<phylip_algorithm;algorithm_index++)
     strcat(alg,"I\n");
   fprintf(dend_file,"outtree\n%s/exe/font1\n%sV\nN\nY\n",phylib_path,alg);     //D\n
   fclose(dend_file);
   /* create the dendrogram */   
   system("rm plotfile");
   sprintf(file_path,"%s/exe/fitch < %s/fitch.infile",phylib_path,phylib_path);
   system(file_path);
   sprintf(file_path,"%s/exe/drawtree < %s/drawtree.infile",phylib_path,phylib_path);
   system(file_path);
   sprintf(file_path,"mv plotfile ./%s.ps",data_set_name);
   system(file_path);			
   sprintf(file_path,"convert ./%s.ps ./%s.jpg",data_set_name,data_set_name);
   system(file_path);
   system("rm outfile outtree");  /* delete files from last run */			
   fprintf(output_file,"<A HREF=\"%s.ps\"><IMG SRC=\"%s.jpg\"></A><br>",data_set_name,data_set_name);
   fprintf(output_file,"<A HREF=\"%s.ps\">%s.ps</A><br>",data_set_name,data_set_name);	/* the image files are copied in the file "wndchrm.cpp" */
   return(1);
}

/*
PrintMatrix
print the confusion or similarity matrix
output_file -FILE *- the file to print into (can be stdout to print to screen)
confusion_matrix -unsigned short *- the confusion matrixvalues to print
                 NULL - don't print confusion matrix
similarity_matrix -double *- the similarity matrix values to print
                 NULL - don't print similarity matrix
dend_file -unsigned short- create a dendogram input file
method -unsigned short- the method of transforming the similarity values into a single distance (0 - min, 1 - average. 2 - top triangle, 3 - bottom triangle).

returned values -long- 1 if successful, 0 if failed
*/
long TrainingSet::PrintConfusion(FILE *output_file,unsigned short *confusion_matrix, double *similarity_matrix)//,unsigned short dend_file, unsigned short method)
{  int class_index1,class_index2;
//   if (dend_file) fprintf(output_file,"%d\n",class_num);
//   else
   {  fprintf(output_file,"%18s"," ");
      for (class_index1=1;class_index1<=class_num;class_index1++)
        fprintf(output_file,"%16s",class_labels[class_index1]);
      fprintf(output_file,"\n");
   }
   for (class_index1=1;class_index1<=class_num;class_index1++)
   {
//      if (dend_file)
//      {   char label[128];
//          strcpy(label,class_labels[class_index1]);
//          if (strlen(label)>8) strcpy(label,&(label[strlen(label)-8]));  /* make sure the labels are shorter or equal to 8 characters in length */
//          if (!isalnum(label[strlen(label)-1])) label[strlen(label)-1]='\0';
//          fprintf(output_file,"%s                 ",label);
//      }
//      else 
      fprintf(output_file,"%16s",class_labels[class_index1]);
      for (class_index2=1;class_index2<=class_num;class_index2++)
      {  if (confusion_matrix) // && !dend_file)
           fprintf(output_file,"%16d",confusion_matrix[class_index1*class_num+class_index2]);
//         if (dend_file && similarity_matrix)
//         {  double dist=0;
//            if (method==0) dist=max(1-similarity_matrix[class_index1*class_num+class_index2],1-similarity_matrix[class_index2*class_num+class_index1]);
//            if (method==1) dist=((1-similarity_matrix[class_index1*class_num+class_index2])+(1-similarity_matrix[class_index2*class_num+class_index1]))/2;
//            if (method==2) dist=(1-similarity_matrix[class_index1*class_num+class_index2])*(class_index2>=class_index1)+(1-similarity_matrix[class_index2*class_num+class_index1])*(class_index2<class_index1);  /* top triangle    */
//            if (method==3) dist=(1-similarity_matrix[class_index1*class_num+class_index2])*(class_index2<=class_index1)+(1-similarity_matrix[class_index2*class_num+class_index1])*(class_index2>class_index1);  /* bottom triangle */
//#ifndef WIN32
//            if (dist==NAN) dist=0;
//#endif
//            if (dend_file) fprintf(output_file,"%1.4f%7s",fabs(dist*(dist>=0))," ");
            else
            fprintf(output_file,"%9s%1.5f"," ",similarity_matrix[class_index1*class_num+class_index2]);
//         }
      }
      fprintf(output_file,"\n");
   }
   fprintf(output_file,"\n");
   return(1);
}

long double factorial(long num)
{  long double res=1.0;
   for (int i=1; i<=num; ++i) res=res*i;
   return(res);
}

long TrainingSet::report(FILE *output_file, char *output_file_name,char *data_set_name, data_split *splits, unsigned short split_num, int tiles, int max_train_images, char *phylib_path, int phylip_algorithm, int export_tsv, char *path_to_test_set, int image_similarities)
{  int class_index,class_index2,sample_index,split_index,a,test_set_size,train_set_size;
   int test_images[MAX_CLASS_NUM];
   double *avg_similarity_matrix,*avg_similarity_normalization;
   double splits_accuracy,splits_class_accuracy,avg_pearson=0.0,avg_abs_dif=0.0,avg_p=0.0;
   FILE *tsvfile;
   char tsv_filename[512];
   
   /* create a directory for the files */
#ifndef WIN32
   if (export_tsv) mkdir("tsv",0755);
#else
   if (export_tsv) mkdir("tsv");
#endif

   /* print the header */
   fprintf(output_file,"<HTML>\n<HEAD>\n<TITLE> %s </TITLE>\n </HEAD> \n <BODY> \n <br> WNDCHRM "PACKAGE_VERSION"\n <br><br> <h1>%s</h1><br>\n ",output_file_name,data_set_name);
   if (path_to_test_set)  fprintf(output_file,"Testing with data file: %s<br>",path_to_test_set);
   fprintf(output_file,"<hr/><CENTER>\n");
   
   /* print the number of samples table */
   fprintf(output_file,"<table border=\"1\" cellspacing=\"0\" cellpadding=\"3\" align=\"center\">\" \n <caption>%d Images in the dataset (tiles per image=%d)</caption> \n <tr>",(long)(count/pow(tiles,2)),tiles*tiles);
   for (class_index=0;class_index<=class_num;class_index++)
     fprintf(output_file,"<td>%s</td>\n",class_labels[class_index]);
   fprintf(output_file,"<td>total</td></tr>\n");
   test_set_size=0;
   fprintf(output_file,"<tr><td>Testing</td>\n");
   if (class_num==0) test_set_size=splits[0].confusion_matrix[0];
   else
   for (class_index=1;class_index<=class_num;class_index++)
   {  int inst_num=0;
      for (class_index2=1;class_index2<=class_num;class_index2++)
        inst_num+=(splits[0].confusion_matrix[class_index*class_num+class_index2]);
      fprintf(output_file,"<td>%d</td>\n",inst_num);
      test_images[class_index]=inst_num;
      test_set_size+=inst_num;
   }
   fprintf(output_file,"<td>%d</td></tr>\n",test_set_size); /* add the total number of test samples */
   train_set_size=0;
   fprintf(output_file,"<tr>\n<td>Training</td>\n");
   if (class_num==0) 
   {  train_set_size=count/(tiles*tiles)-test_set_size;
      if (max_train_images!=0 && max_train_images<train_set_size) train_set_size=max_train_images;
   }
   for (class_index=1;class_index<=class_num;class_index++)
   {  int inst_num=0;
      for (sample_index=0;sample_index<count;sample_index++)
        if (samples[sample_index]->sample_class==class_index) inst_num++;
      inst_num=(int)(inst_num/(tiles*tiles))-test_images[class_index]*(path_to_test_set==NULL);
      if (max_train_images>0 && inst_num>max_train_images) inst_num=max_train_images;
      fprintf(output_file,"<td>%d</td>\n",inst_num);
      train_set_size+=inst_num;
   }
   fprintf(output_file,"<td>%d</td>\n",train_set_size); /* add the total number of training samples */
   fprintf(output_file,"</tr> \n </table><br>\n");          /* close the number of samples table */
   
   /* print the splits */
   splits_accuracy=0.0;
   splits_class_accuracy=0.0;
   fprintf(output_file,"<h2>Results</h2> \n <table border=\"1\" align=\"center\"><caption></caption> \n");
   for (split_index=0;split_index<split_num;split_index++)
   {  unsigned short *confusion_matrix;
      double *similarity_matrix,P=0.0;
      double avg_accuracy=0.0,plus_minus=0;

      confusion_matrix=splits[split_index].confusion_matrix;
      similarity_matrix=splits[split_index].similarity_matrix;

      for (class_index=1;class_index<=class_num;class_index++)
      {  double class_avg=0.0,class_sum=0.0;
         for (class_index2=1;class_index2<=class_num;class_index2++)
           if (class_index==class_index2) class_avg+=confusion_matrix[class_index*class_num+class_index2];
           else class_sum+=confusion_matrix[class_index*class_num+class_index2];
         if (class_avg+class_sum>0) avg_accuracy=avg_accuracy+(class_avg/(class_avg+class_sum))/class_num;
      }
      for (int correct=(long)ceil(avg_accuracy*test_set_size);correct<=test_set_size;correct++)  /* find the P */
        P+=pow((1/(double)class_num),correct)*pow(1-1/(double)class_num,test_set_size-correct)*factorial(test_set_size)/(factorial(correct)*factorial(test_set_size-correct));		 		   
//      for (int correct=(long)class_avg;correct<=class_avg+class_sum;correct++)  /* find the P */
//        P+=pow((1/(double)class_num),correct)*pow(1-1/(double)class_num,(long)(class_avg+class_sum)-correct)*factorial((long)(class_avg+class_sum))/(factorial(correct)*factorial((long)(class_avg+class_sum)-correct));		 		   
//      avg_accuracy/=class_num;
      for (class_index=1;class_index<=class_num;class_index++)
      {  double class_avg=0.0,class_sum=0.0;
         for (class_index2=1;class_index2<=class_num;class_index2++)
           if (class_index==class_index2) class_avg+=confusion_matrix[class_index*class_num+class_index2];
           else class_sum+=confusion_matrix[class_index*class_num+class_index2];
         if (fabs((class_avg/(class_avg+class_sum))-avg_accuracy)>plus_minus) plus_minus=fabs((class_avg/(class_avg+class_sum))-avg_accuracy);
      }

      fprintf(output_file,"<tr> <td>Split %d</td> \n <td align=\"center\" valign=\"top\"> \n",split_index+1);
      if (class_num>0)
      {  fprintf(output_file,"Accuracy: <b>%.2f of total (P=%e) </b><br> \n",splits[split_index].accuracy,P);	  
         fprintf(output_file,"<b>%.2f &plusmn; %.1f Avg per Class Correct of total</b><br> \n",avg_accuracy,plus_minus);
      }
	  if (splits[split_index].pearson_coefficient!=0) 
      {  fprintf(output_file,"Pearson correlation coefficient: %.2f (P=%e) <br>\n",splits[split_index].pearson_coefficient,splits[split_index].pearson_p_value);
         fprintf(output_file,"Average absolute difference: %.4f <br>\n",splits[split_index].avg_abs_dif);
//      }
//	  else
//      {  
	     avg_pearson+=splits[split_index].pearson_coefficient;
         avg_abs_dif+=splits[split_index].avg_abs_dif; 
         avg_p+=splits[split_index].pearson_p_value;
      }
      if (splits[split_index].feature_weight_distance>=0)
	    fprintf(output_file,"Feature weight distance: %.2f<br>\n",splits[split_index].feature_weight_distance);	  	  
      fprintf(output_file,"<a href=\"#split%d\">Full details</a><br> \n",split_index);
//      fprintf(output_file,"<a href=\"#features%d\">Features used</a><br> </td> </tr> \n",split_index);
      splits_accuracy+=splits[split_index].accuracy;
	  splits_class_accuracy+=avg_accuracy;
   }
   /* average of all splits */
   fprintf(output_file,"<tr> <td>Total</td> \n <td align=\"center\" valign=\"top\"> \n");
   if (class_num>0)
   {  double avg_p2=0.0;
//      for (int correct=(long)((test_set_size+train_set_size)*(splits_accuracy/split_num));correct<=test_set_size+train_set_size;correct++)
//	    avg_p2+=pow((1/(double)class_num),correct)*pow(1-1/(double)class_num,test_set_size+train_set_size-correct)*factorial(test_set_size+train_set_size)/(factorial(correct)*factorial(test_set_size+train_set_size-correct));
      for (int correct=(long)((long)(count/pow(tiles,2))*(splits_accuracy/split_num));correct<=(long)(count/pow(tiles,2));correct++)
	    avg_p2=avg_p2+pow((1/(double)class_num),correct)*pow(1-1/(double)class_num,(long)(count/pow(tiles,2))-correct)*factorial((long)(count/pow(tiles,2)))/(factorial(correct)*factorial((long)(count/pow(tiles,2))-correct));		
//printf("%i %i %f %i %i %f %f\n",class_num,count,splits_accuracy/split_num,(long)(count/tiles),(long)((long)(count/tiles)*(splits_accuracy/split_num)),0.0,avg_p2);		
      fprintf(output_file,"<b>%.2f Avg per Class Correct of total</b><br> \n",splits_class_accuracy/split_num);
      fprintf(output_file,"Accuracy: <b>%.2f of total (P=%e)</b><br> \n",splits_accuracy/split_num,avg_p2);
   }
   if (avg_pearson!=0) 
   {  fprintf(output_file,"Pearson correlation coefficient: %.2f (avg P=%e) <br>\n",avg_pearson/split_num,avg_p/split_num);
      fprintf(output_file,"Average absolute difference: %.4f <br>\n",avg_abs_dif/split_num);
   }   
   
   fprintf(output_file,"</table>\n");   /* close the splits table */
   fprintf(output_file,"<br><br><br><br><br><br> \n\n\n\n\n\n\n\n");

   /* average (sum) confusion matrix */
   sprintf(tsv_filename,"tsv/avg_confusion.tsv");                /* determine the tsv file name           */
   tsvfile=NULL;                                                 /* keep it null if the file doesn't open */
   if (export_tsv) tsvfile=fopen(tsv_filename,"w");              /* open the file for tsv                 */
   if (class_num>0) fprintf(output_file,"<table border=\"1\" align=\"center\"><caption>Confusion Matrix (sum of all splits)</caption> \n <tr><td></td> ");
   if (tsvfile) fprintf(tsvfile,"\t");         /* space (in the tsv file) */
   for (class_index=1;class_index<=class_num;class_index++)
   {  fprintf(output_file,"<td><b>%s</b></td> ",class_labels[class_index]);   /* print to the html file  */
      if (tsvfile) fprintf(tsvfile,"%s\t",class_labels[class_index]);         /* print into the tsv file */
   }
   fprintf(output_file,"</tr>\n");         /* end of the classes names */
   if (tsvfile) fprintf(tsvfile,"\n");     /* end of the classes names in the tsv file */
   for (class_index=1;class_index<=class_num;class_index++)
   {  fprintf(output_file,"<tr><td><b>%s</b></td> ",class_labels[class_index]);  /* print the class name                   */
      if (tsvfile) fprintf(tsvfile,"%s\t",class_labels[class_index]);            /* print the class name into the tsv file */   
      for (class_index2=1;class_index2<=class_num;class_index2++)
      {  double sum=0.0;
	     char bgcolor[64];
         for (split_index=0;split_index<split_num;split_index++)
           sum+=splits[split_index].confusion_matrix[class_index*class_num+class_index2];
		 if (class_index==class_index2) strcpy(bgcolor," bgcolor=#D5D5D5");
		 else strcpy(bgcolor,"");  
         if ((double)((long)(sum/split_num))==sum/split_num) fprintf(output_file,"<td%s>%d</td>\n",bgcolor,(long)(sum/*/split_num*/));
         else fprintf(output_file,"<td%s>%.0f</td> ",bgcolor,sum/*/split_num*/);
         if (tsvfile) fprintf(tsvfile,"%.0f\t",sum/*/split_num*/);     /* print the values to the tsv file (for the tsv machine readable file a %.2f for all values should be ok) */		 
      }
      fprintf(output_file,"</tr>\n");         /* end of the line in the html report */
      if (tsvfile) fprintf(tsvfile,"\n");     /* end of the line in the tsv file    */	  
   }
   fprintf(output_file,"</table> \n <br><br><br><br> \n");  /* end of average confusion matrix */
   if (tsvfile) fclose(tsvfile);

   /* average similarity matrix */
   sprintf(tsv_filename,"tsv/avg_similarity.tsv");                 /* determine the tsv file name               */
   tsvfile=NULL;                                                   /* keep it null if the file doesn't open     */
   if (export_tsv) tsvfile=fopen(tsv_filename,"w");                /* open the file for tsv                     */   
   avg_similarity_matrix=new double[(class_num+1)*(class_num+1)];  /* this is used for creating the dendrograms */
   avg_similarity_normalization=new double[class_num+1];
   if (class_num>0) fprintf(output_file,"<table><tr><td><table border=\"1\" align=\"center\"><caption>Average Similarity Matrix</caption>\n <tr><td></td> ");
   if (tsvfile) fprintf(tsvfile,"\t");         /* space */   
   for (class_index=1;class_index<=class_num;class_index++)
   {  fprintf(output_file,"<td><b>%s</b></td> ",class_labels[class_index]);   /* print to the html file  */
      if (tsvfile) fprintf(tsvfile,"%s\t",class_labels[class_index]);         /* print into the tsv file */
   }
   fprintf(output_file,"</tr>\n");         /* end of the classes names */
   if (tsvfile) fprintf(tsvfile,"\n");     /* end of the classes names in the tsv file */
   for (class_index=1;class_index<=class_num;class_index++)
   {  fprintf(output_file,"<tr><td><b>%s</b></td> ",class_labels[class_index]);
      if (tsvfile) fprintf(tsvfile,"%s\t",class_labels[class_index]);         /* print the class name into the tsv file */
      for (class_index2=1;class_index2<=class_num;class_index2++)
      {  double sum=0.0;
         for (split_index=0;split_index<split_num;split_index++)
           sum+=splits[split_index].similarity_matrix[class_index*class_num+class_index2];
         avg_similarity_matrix[class_index*class_num+class_index2]=sum/split_num;    /* remember this value for the dendrogram file */
         fprintf(output_file,"<td>%.2f</td> ",sum/split_num);
         if (tsvfile) fprintf(tsvfile,"%.2f\t");              /* print the values to the tsv file (for the tsv machine readable file a %.2f for all values should be ok) */		 		 
      }
      fprintf(output_file,"</tr>\n");                         /* end of the line in the html report   */
      if (tsvfile) fprintf(tsvfile,"\n");                     /* end of the line in the tsv file      */	  
	  /* compute the avg normalization factor */
	  avg_similarity_normalization[class_index]=0.0;
	  for (split_index=0;split_index<split_num;split_index++) 
	    if (splits[split_index].similarity_normalization)
	      avg_similarity_normalization[class_index]+=(splits[split_index].similarity_normalization[class_index]/split_num);
   }
   fprintf(output_file,"</table></td>");   /* end of average similarity matrix */
   if (tsvfile) fclose(tsvfile);
   /* write the normalization factors for each class */
//   if (splits[0].similarity_normalization)
//   {  fprintf(output_file,"<td>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp</td><td><table border=\"1\" align=\"center\"><caption>Norm. Factor</caption>\n");
//      for (class_index=1;class_index<=class_num;class_index++)
//      {  fprintf(output_file,"<TR><TD>%.3f</TD></TR><br>\n",avg_similarity_normalization[class_index]);
//      }
//	  fprintf(output_file,"</td></table> \n");  /* end of average similarity normalization factors list */
//   }
   fprintf(output_file,"</tr></table><br><br><br><br> \n");   
   
   /* *** generate a dendrogram *** */
   if (phylib_path && class_num>0)  /* generate a dendrogram only if phlyb path was specified */
   {   dendrogram(output_file,data_set_name, phylib_path, class_num,avg_similarity_matrix, class_labels,1,phylip_algorithm);
       if (export_tsv)   /* write the phylip file to the tsv directory */
       {  sprintf(tsv_filename,"cp %s/dend_file tsv/dend_file.txt",phylib_path);
          system(tsv_filename);
	   }   
   }

   
//      FILE *dend_file;
//      char file_path[256],alg[16];
//	  int algorithm_index;	  
//      /* write "dend_file.txt" */
//      sprintf(file_path,"%s/dend_file.txt",phylib_path);
//      dend_file=fopen(file_path,"w");
//      if (dend_file)
//	  {  PrintConfusion(dend_file,splits[0].confusion_matrix,avg_similarity_matrix,1,1);  /* print the dendrogram to a the "dend_file.txt" file */
//         fclose(dend_file);
//		 if (export_tsv)   /* write the phylip file to the tsv directory */
//		 {  sprintf(file_path,"tsv/dend_file.txt");
//		    dend_file=fopen(file_path,"w");
//			PrintConfusion(dend_file, splits[0].confusion_matrix,avg_similarity_matrix,1,1);  /* print the dendrogram to a "dend_file.txt" file */
//			fclose(dend_file);
//		 }
//         sprintf(file_path,"%s/fitch.infile",phylib_path);
//         dend_file=fopen(file_path,"w");
//         if (dend_file)
//         {  /* create fith.infile */
//            fprintf(dend_file,"%s/dend_file.txt\nJ\n97\n10\nY\n",phylib_path);
//            fclose(dend_file);
//            /* create drawtree.infile */			
//            sprintf(file_path,"%s/drawtree.infile",phylib_path);
//            dend_file=fopen(file_path,"w");
//			alg[0]='\0';
//			for (algorithm_index=0;algorithm_index<phylip_algorithm;algorithm_index++)
//			  strcat(alg,"I\n");
//            fprintf(dend_file,"outtree\n%s/exe/font1\n%sV\nN\nY\n",phylib_path,alg);     //D\n
//            fclose(dend_file);
//			/* create the dendrogram */
//			system("rm plotfile");
//            sprintf(file_path,"%s/exe/fitch < %s/fitch.infile",phylib_path,phylib_path);
//            system(file_path);
//            sprintf(file_path,"%s/exe/drawtree < %s/drawtree.infile",phylib_path,phylib_path);
//            system(file_path);
//            sprintf(file_path,"mv plotfile ./%s.ps",data_set_name);
//            system(file_path);			
//            sprintf(file_path,"convert ./%s.ps ./%s.jpg",data_set_name,data_set_name);
//            system(file_path);
//            system("rm outfile outtree");  /* delete files from last run */			
//            fprintf(output_file,"<A HREF=\"%s.ps\"><IMG SRC=\"%s.jpg\"></A><br>",data_set_name,data_set_name);
//            fprintf(output_file,"<A HREF=\"%s.ps\">%s.ps</A><br>",data_set_name,data_set_name);			
//         }
//	}		   
//   }
//   delete avg_similarity_matrix;  /* free the memory allocated for the dendrogram similarity matrix */
//   delete avg_similarity_normalization;

   /* print the average accuracies of the tile areas */
   if (splits[0].tile_area_accuracy)
   {  fprintf(output_file,"<br><table border=\"1\" align=\"center\"><caption>Tile Areas Accuracy</caption> \n");
      for (int y=0;y<tiles;y++) 
      {   fprintf(output_file,"<tr>\n");
          for (int x=0;x<tiles;x++)
	      {  splits_accuracy=0.0;
             for (split_index=0;split_index<split_num;split_index++)
               splits_accuracy+=splits[split_index].tile_area_accuracy[y*tiles+x];
             fprintf(output_file,"<td>%.3f</td>\n",splits_accuracy/(double)split_num);
         }
         fprintf(output_file,"</tr>\n");
      }
      fprintf(output_file,"</table><br>\n");
   }
   
   /* *** print the confusion/similarity matrices, feature names and individual images for the splits *** */
   for (split_index=0;split_index<split_num;split_index++)
   {  unsigned short *confusion_matrix;
      double *similarity_matrix;
      char feature_names[60000],*p_feature_names;
      unsigned short features_num=0,class_index;

      confusion_matrix=splits[split_index].confusion_matrix;
      similarity_matrix=splits[split_index].similarity_matrix;

      fprintf(output_file,"<HR><BR><A NAME=\"split%d\">\n",split_index);   /* for the link to the split */
      fprintf(output_file,"<B>Split %d</B><br><br>\n",split_index+1);

	  /* print the confusion matrix */
      if (class_num>0) 
	  {  fprintf(output_file,"<table border=\"1\" align=\"center\"><caption>Confusion Matrix</caption> \n <tr><td></td>\n");
         for (class_index=1;class_index<=class_num;class_index++)
           fprintf(output_file,"<td><b>%s</b></td>\n",class_labels[class_index]);
         fprintf(output_file,"</tr>\n");
         for (class_index=1;class_index<=class_num;class_index++)
         {  fprintf(output_file,"<tr><td><b>%s</b></td>\n",class_labels[class_index]);
            for (class_index2=1;class_index2<=class_num;class_index2++)
              fprintf(output_file,"<td>%d</td>\n",confusion_matrix[class_index*class_num+class_index2]);
            fprintf(output_file,"</tr>\n");
         }
         fprintf(output_file,"</table> \n <br><br> \n");
      }
      
	  /* print the similarity matrix */
      if (class_num>0) 
	  {  fprintf(output_file,"<table><tr><td><table border=\"1\" align=\"center\"><caption>Similarity Matrix</caption> \n <tr><td></td>\n");   
         for (class_index=1;class_index<=class_num;class_index++)
           fprintf(output_file,"<td><b>%s</b></td>\n",class_labels[class_index]);   
         fprintf(output_file,"</tr>\n");
         for (class_index=1;class_index<=class_num;class_index++)
         {  fprintf(output_file,"<tr><td><b>%s</b></td>\n",class_labels[class_index]);
            for (class_index2=1;class_index2<=class_num;class_index2++)
              fprintf(output_file,"<td>%.2f</td>\n",similarity_matrix[class_index*class_num+class_index2]);
            fprintf(output_file,"</tr>\n");
         }
		fprintf(output_file,"</table></td>\n");
      }
      /* write the normalization factors for each class */
//      if (splits[split_index].similarity_normalization)
//      {  fprintf(output_file,"<td>&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp</td><td><table border=\"1\" align=\"center\"><caption>Norm. Factor</caption>\n");
//         for (class_index=1;class_index<=class_num;class_index++)
//          fprintf(output_file,"<TR><TD>%.2f</TD></TR>\n",splits[split_index].similarity_normalization[class_index]);
//         fprintf(output_file,"</td></table> \n");  /* end of average similarity normalization factors list */
//      }
      fprintf(output_file,"</tr></table><br><br><br><br> \n");    

      /* add a dendrogram of the image similarities */
      if (image_similarities && splits[split_index].image_similarities)
      {  char file_name[256],**labels;
         int test_image_index;
         sprintf(file_name,"%s_%d",data_set_name,split_index);
         labels=new char *[test_set_size+1];
         for (test_image_index=1;test_image_index<=test_set_size;test_image_index++)
         {  labels[test_image_index]=new char[MAX_CLASS_NAME_LENGTH];
            strcpy(labels[test_image_index],class_labels[(int)(splits[split_index].image_similarities[test_image_index])]);
		 }
         dendrogram(output_file,file_name, phylib_path, test_set_size,(double *)(splits[split_index].image_similarities), labels,4,phylip_algorithm);	    
         for (test_image_index=1;test_image_index<=test_set_size;test_image_index++) delete labels[test_image_index];
         delete labels;
      }
	  
      /* add the sorted features */
      strncpy(feature_names,splits[split_index].feature_names,sizeof(feature_names));
      feature_names[sizeof(feature_names)-1]='\0';  /* make sure the string is null-terminated */
      a=0;
      while (feature_names[a]!='\0') /* first count the features */
        features_num+=(feature_names[a++]=='\n');
      if (features_num>0) fprintf(output_file,"<br>%d features selected (out of %d features computed).<br>  <a href=\"#\" onClick=\"sigs_used=document.getElementById('FeaturesUsed_split%d'); if (sigs_used.style.display=='none'){ sigs_used.style.display='inline'; } else { sigs_used.style.display='none'; } return false; \">Toggle feature names</a><br><br>\n",features_num,signature_count,split_index);
      fprintf(output_file,"<TABLE ID=\"FeaturesUsed_split%d\" border=\"1\" style=\"display: none;\">\n",split_index);
      p_feature_names=strtok(feature_names,"\n");
      while (p_feature_names)
      {  fprintf(output_file,"<tr><td>%s</td></tr>\n",p_feature_names);
         p_feature_names=strtok(NULL,"\n");
      }
      fprintf(output_file,"</table><br>\n"); 
	  
      /* add the feature groups */
      if( splits[split_index].feature_groups )
      {
        strncpy( feature_names, splits[split_index].feature_groups,
                 sizeof( feature_names ) );

        feature_names[ sizeof( feature_names ) - 1 ] = '\0'; // make sure the string is null-terminated
        if( features_num > 0 )
          fprintf( output_file, "<a href=\"#\" onClick=\"sigs_used=document.getElementById('FeaturesGroups_split%d'); if (sigs_used.style.display=='none'){ sigs_used.style.display='inline'; } else { sigs_used.style.display='none'; } return false; \">Feature groups analysis (sum of Fisher scores for each family) </a><br><br>\n",split_index);
        fprintf(output_file,"<TABLE ID=\"FeaturesGroups_split%d\" border=\"1\" style=\"display: none;\">\n",split_index);
        fprintf( output_file,"<tr><td>NOTE: Number of component features in each group is given in brackets</td></tr>\n",p_feature_names );
        
        p_feature_names=strtok(feature_names,"\n");
        while( p_feature_names )
        {
          fprintf( output_file,"<tr><td>%s</td></tr>\n",p_feature_names );
          p_feature_names = strtok( NULL, "\n" );
        }
        fprintf(output_file,"</table><br>\n");
      }
	  
      /* individual image predictions */
      if (splits[split_index].individual_images)
      {  char closest_image[256],interpolated_value[256];
	     int interpolate=1;
         
         /* add the most similar image if WNN and no tiling */
         if ((splits[split_index].method==WNN || class_num==0) && tiles==1) strcpy(closest_image,"<td><b>Most similar image</b></td>");
         else strcpy(closest_image,"");
		 
         fprintf(output_file,"<a href=\"#\" onClick=\"sigs_used=document.getElementById('IndividualImages_split%d'); if (sigs_used.style.display=='none'){ sigs_used.style.display='inline'; } else { sigs_used.style.display='none'; } return false; \">Individual image predictions</a><br>\n",split_index);
         fprintf(output_file,"<TABLE ID=\"IndividualImages_split%d\" border=\"1\" style=\"display: none;\">\n       <tr><td><b>Image No.</b></td>",split_index);
		 if (class_num>0) fprintf(output_file,"<td><b>Normalization<br>Factor</b></td>");
         for (class_index=1;class_index<=class_num;class_index++)
         {  fprintf(output_file,"<td><b>%s</b></td>",class_labels[class_index]);
            interpolate*=(atof(class_labels[class_index])!=0.0 || class_labels[class_index][0]=='0');		 /* interpolate only if all class labels are values */ 
		 }
   	     if (interpolate) strcpy(interpolated_value,"<td><b>Interpolated<br>Value</b></td>");
         else strcpy(interpolated_value,"");
         if (class_num==0) fprintf(output_file,"<td>&nbsp</td><td><b>Actual<br>Value</b></td><td><b>Predicted<br>Value</b></td>");
         else fprintf(output_file,"<td>&nbsp</td><td><b>Actual<br>Class</b></td><td><b>Predicted<br>Class</b></td><td><b>Classification<br>Correctness</b></td>%s",interpolated_value);
         fprintf(output_file,"<td><b>Image</b></td>%s</tr>\n",closest_image);		 
         fprintf(output_file,splits[split_index].individual_images);
         fprintf(output_file,"</table><br><br>\n");
      }
   }

   fprintf(output_file,"<br><br><br><br><br><br> \n\n\n\n\n\n\n\n");

   fprintf(output_file,"</CENTER> \n </BODY> \n </HTML>\n");
}

#pragma package(smart_init)

