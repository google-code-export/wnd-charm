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


#include <string.h>

#include "FeatureStatistics.h"

typedef struct POINT1
{  long x,y,z;
}point;

//---------------------------------------------------------------------------
/*  BWlabel
    label groups of 4-connected pixels.
    This is an implementation of the Matlab function bwlabel
*/
unsigned long bwlabel(ImageMatrix *Im, int level) {
	unsigned long x,y,z,base_x,base_y,base_z,group_counter=1;
	unsigned int stack_count;
	point *stack=new point[Im->width*Im->height];

	for (y = 0; y < Im->height; y++) {
		for (x = 0; x < Im->width; x++) {
			if (Im->pix_plane(y,x) == 1) {
				/* start a new group */
				group_counter++;
				Im->pix_plane(y,x) = group_counter;
				stack[0].x=x;
				stack[0].y=y;
				stack[0].z=z;		   
				stack_count=1;
				while (stack_count > 0) {
					base_x=stack[0].x;
					base_y=stack[0].y;
					base_z=stack[0].z;

					if (base_x > 0 && Im->pix_plane(base_y,base_x-1) == 1) {
						Im->pix_plane(base_y,base_x-1) = group_counter;
						stack[stack_count].x=base_x-1;
						stack[stack_count].y=base_y;
						stack[stack_count].z=base_z;				 
						stack_count++;
					}

					if (base_x<Im->width-1 && Im->pix_plane(base_y,base_x+1) == 1) {
						Im->pix_plane(base_y,base_x+1) = group_counter;
						stack[stack_count].x=base_x+1;
						stack[stack_count].y=base_y;
						stack[stack_count].z=base_z;				 				 
						stack_count++;
					}

					if (base_y>0 && Im->pix_plane(base_y-1,base_x) == 1) {
						Im->pix_plane(base_y-1,base_x) = group_counter;
						stack[stack_count].x=base_x;
						stack[stack_count].y=base_y-1;
						stack[stack_count].z=base_z;				 				 
						stack_count++;
					}

					if (base_y<Im->height-1 && Im->pix_plane(base_y+1,base_x) == 1) {
						Im->pix_plane(base_y+1,base_x) = group_counter;
						stack[stack_count].x=base_x;
						stack[stack_count].y=base_y+1;
						stack[stack_count].z=base_z;				 				 
						stack_count++;
					}

					/* look for 8 connected pixels */
					if (level==8) {
						if (base_x > 0 && base_y > 0 && Im->pix_plane(base_y-1,base_x-1) == 1) {
							Im->pix_plane(base_y-1,base_x-1) = group_counter;
							stack[stack_count].x=base_x-1;
							stack[stack_count].y=base_y-1;
							stack[stack_count].z=base_z;				 					
							stack_count++;
						}

						if (base_x < Im->width-1 && base_y > 0 && Im->pix_plane(base_y-1,base_x+1) == 1) {
							Im->pix_plane(base_y-1,base_x+1) = group_counter;
							stack[stack_count].x=base_x+1;
							stack[stack_count].y=base_y-1;
							stack[stack_count].z=base_z;				 				   
							stack_count++;
						}

						if (base_x > 0 && base_y < Im->height-1 && Im->pix_plane(base_y+1,base_x-1) == 1) {
							Im->pix_plane(base_y+1,base_x-1) = group_counter;
							stack[stack_count].x=base_x-1;
							stack[stack_count].y=base_y+1;
							stack[stack_count].z=base_z;				 				   
							stack_count++;
						}

						if (base_x < Im->width-1 && base_y < Im->height-1 && Im->pix_plane(base_y+1,base_x+1) == 1) {
							Im->pix_plane(base_y+1,base_x+1) = group_counter;
							stack[stack_count].x=base_x+1;
							stack[stack_count].y=base_y+1;
							stack[stack_count].z=base_z;				 				   
							stack_count++;
						}
					}
			  
					stack_count-=1;
					memcpy(stack,&(stack[1]),sizeof(point)*stack_count);
				}
			}
		}
	}

	/* now decrease every non-zero pixel by one because the first group was "2" */
	for (y=0;y<Im->height;y++)
		for (x=0;x<Im->width;x++)
			if (Im->pix_plane(y,x) != 0)
				Im->pix_plane(y,x)--;

	delete [] stack;
	return(group_counter-1);
}

/* the input should be a binary image */
void GlobalCentroid(ImageMatrix *Im, double *x_centroid, double *y_centroid) {
	unsigned long x,y;
	double x_mass=0,y_mass=0,mass=0;

	for (y=0;y<Im->height;y++)
		for (x=0;x<Im->width;x++)
			if (Im->pix_plane(y,x) > 0) {
				x_mass=x_mass+x+1;    /* the "+1" is only for compatability with matlab code (where index starts from 1) */
				y_mass=y_mass+y+1;    /* the "+1" is only for compatability with matlab code (where index starts from 1) */
				mass++;
			}
	if (mass) {
		*x_centroid=x_mass/mass;
		*y_centroid=y_mass/mass;
	} else *x_centroid=*y_centroid=0;
}

/* find the centroid of a certain feature
   the input image should be a bwlabel transform of a binary image
   the retruned value is the area of the feature
*/
unsigned long FeatureCentroid(ImageMatrix *Im, double object_index,double *x_centroid, double *y_centroid) {
	unsigned long x,y;
	unsigned long x_mass=0,y_mass=0,mass=0;

	for (y=0;y<Im->height;y++)
		for (x=0;x<Im->width;x++)
			if (Im->pix_plane(y,x)==object_index) {
				x_mass=x_mass+x+1;      /* the "+1" is only for compatability with matlab code (where index starts from 1) */
				y_mass=y_mass+y+1;      /* the "+1" is only for compatability with matlab code (where index starts from 1) */
				mass++;
			}
	if (x_centroid) *x_centroid=(double)x_mass/(double)mass;
	if (y_centroid) *y_centroid=(double)y_mass/(double)mass;
	return(mass);



//   for (y=0;y<Im->height;y++)
//     for (x=0;x<Im->width;x++)
//       if (Im->data[x][y].intensity==object_index) mass++;

   /* find the x coordinate of the centroid */
//   for (x=0;x<Im->width;x++)
//   {  for (y=0;y<Im->height;y++)
//        if (Im->data[x][y].intensity==object_index)
//          x_mass++;
//        if (x_mass>=mass/2)
//        {  *x_centroid=x;
//            break;
//        }
//   }

   /* find the y coordinate of the centroid */
//   for (y=0;y<Im->height;y++)
//   {  for (x=0;x<Im->width;x++)
//        if (Im->data[x][y].intensity==object_index)
//          y_mass++;
//        if (y_mass>=mass/2)
//        {  *y_centroid=y;
//           break;
//        }
//   }
//   return(mass);
}

/* the number of pixels that are above the threshold
   the input image is a binary image
*/
unsigned long area(ImageMatrix *Im) {
	unsigned long x,y,sum=0;
	for (y=0;y<Im->height;y++)
		for (x=0;x<Im->width;x++)
			sum=sum+(Im->pix_plane(y,x) > 0);
	return(sum);
}

/* EulerNumber
   The input image should be a binary image
*/
unsigned long EulerNumber(ImageMatrix *Im, unsigned long FeatureNumber) {
	unsigned long x,y,HolesNumber;
	ImageMatrix *cp;
	cp = new ImageMatrix (Im);

	/* inverse the image */
	for (y=0;y<cp->height;y++)
		for (x=0;x<cp->width;x++)
			if (cp->pix_plane(y,x) > 0)
				cp->pix_plane(y,x) = 0;
			else cp->pix_plane(y,x) = 1;
	HolesNumber=cp->BWlabel(8);

	delete cp;
	return(FeatureNumber-HolesNumber-1);
}
