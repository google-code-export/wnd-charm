/*------------------------------------------------------------------------------
 *
 *  Copyright (C) 2003 Open Microscopy Environment
 *      Massachusetts Institute of Technology,
 *      National Institutes of Health,
 *      University of Dundee
 *
 *
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *------------------------------------------------------------------------------
 */




/*------------------------------------------------------------------------------
 *
 * Written by:    Ilya G. Goldberg <igg@nih.gov>
 * 
 *------------------------------------------------------------------------------
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tiffio.h>
#include "readTIFF.h"

int main (int argc, char **argv) {

int arg,ret;
TIFF* tif;
unsigned char *buf;
uint16 bits;
uint32 width,height;
char outfile[256],basename[256],error[256],*cp;


	if (argc < 2) {
		fprintf (stderr,"Parameters are one or more RGB TIFF files\n");
		exit (-1);
	}
	
	// suppress warnings
	TIFFSetWarningHandler (NULL);
	for (arg = 1; arg < argc; arg++) {
		fprintf (stdout,"File: %s\n",argv[arg]);
		tif = TIFFOpen(argv[arg],"r");
		if (!tif) {
			fprintf (stderr,"Could not open %s\n",argv[arg]);
			continue;
		}
		
		// Check if its proper RGB
		if ( (ret = isRGB (tif)) < 0) {
			GetReadTIFFError (ret,error);
			fprintf (stderr, "Malformed RGB TIFF %s: %s\n",argv[arg],error);
			TIFFClose (tif);
			continue;
		} else if (ret == 0) {
			fprintf (stderr, "TIFF %s is Grayscale\n",argv[arg]);
			TIFFClose (tif);
			continue;
		}
		
		TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bits);
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
		buf = (unsigned char *)malloc (width*height*(bits/8)*3);

		if ( (ret = ReadRGBTIFFData(tif,buf)) != 0) {
			fprintf (stderr,"Error reading file %s. Return: %d\n",argv[arg],ret);
			free (buf);
			TIFFClose (tif);
			continue;
		}
		
		strncpy (basename,argv[arg],255);
		if ( (cp = strrchr(basename,'.')) != NULL)
			*cp = '\0';

		snprintf (outfile,255,"%s_R.tiff",basename);
		if ( (ret = WriteTIFFData (outfile,buf,bits,height, width)) != 0) {
			fprintf (stderr,"Error writing Red file %s. Return: %d\n",outfile,ret);
			free (buf);
			TIFFClose (tif);
			continue;
		}

		snprintf (outfile,255,"%s_G.tiff",basename);
		if ( (ret = WriteTIFFData (outfile,buf+(width*height*(bits/8)),bits,height, width)) != 0) {
			fprintf (stderr,"Error writing Green file %s. Return: %d\n",outfile,ret);
			free (buf);
			TIFFClose (tif);
			continue;
		}

		snprintf (outfile,255,"%s_B.tiff",basename);
		if ( (ret = WriteTIFFData (outfile,buf+(width*height*(bits/8)*2),bits,height, width)) != 0) {
			fprintf (stderr,"Error writing Blue file %s. Return: %d\n",outfile,ret);
			free (buf);
			TIFFClose (tif);
			continue;
		}


		free (buf);
		TIFFClose (tif);
	}
	
	return (0);
}
