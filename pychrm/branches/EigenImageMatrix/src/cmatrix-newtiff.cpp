int ImageMatrix::LoadTIFF(char *filename) {
	unsigned long h=0,w=0,x=0,y=0,z=0;
	unsigned short int spp=0,bps=0;
	unsigned short planarconfig;
	uint32* sbc;



	TIFF *tif = NULL;
	tdata_t buf;
	double max_val;
	pix_data pix;
	TIFFSetWarningHandler(NULL);
	if (! (tif = TIFFOpen(filename, "r")) ) return (0);

	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
	width = w;
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
	height = h;
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);
	bits=bps;
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
	if (!spp) spp=1;  /* assume one sample per pixel if nothing is specified */
	if ((depth=TIFFNumberOfDirectories(tif))<0) return(0);   /* get the number of slices (Zs) */
	TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);

	// only handle 8 and 16 bps
	if (bits == 8) {
		max_val = 255.0;
	} else if (bits == 16) {
		max_val = 65535.0;
	} else {
		return (0);
	}
	/* allocate the intensity data */
	if (! (pix_plane = new pix_data[width*height]) ) return(0); /* memory allocation failed */
	/* allocate the color data */
	if (spp == 3)
		if (! (clr_plane = new clr_data[width*height]) ) return(0); /* memory allocation failed */

	TIFFIsTiled (tif)
	ttile_t TIFFNumberOfTiles(TIFF* tif)
	tsize_t TIFFTileSize(TIFF *tif)
	tsize_t TIFFReadTile(TIFF *tif, tdata_t buf, uint32 x, uint32 y, uint32 z, tsample_t sample)

	tstrip_t TIFFNumberOfStrips(TIFF *tif)
	tsize_t TIFFStripSize(TIFF *tif)
	
	row = 0;
	row_size = TIFFScanlineSize(tif);

	tdata_t buf_off = buf;
	if (config == PLANARCONFIG_SEPARATE) {
		for (sample = 0; sample < spp; sample++) {
			read_bytes = TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, row, sample), buf_off, tsize_t (-1));
			buf_off += read_bytes;
		}
		sample_stride = read_bytes;
	} else {
		read_bytes = TIFFReadEncodedStrip(tif, TIFFComputeStrip(tif, row, 0), bu, tsize_t (-1));
		sample_stride = spp * (bits/8);
	}
	nrows = read_bytes / row_size;
	buf_off = buf;
	ncols = width;
	buf_off = buf;
	uint8 *buf8;
	uint16 *buf16;
	for (row_idx = 0; row_idx < nrows; row_idx++) {
		for (col_idx = 0; col_idx < ncols; col_idx++) {
			row_dat = row+row_idx;
			col_dat = col+col_idx;
			pix_indx = col_dat + (row*width);
			if (spp == 3) {
				if (bits == 8) {
					val8r = *((uint8 *) buf_off);
					val8g = *((uint8 *) (buf_off + sample_stride));
					val8b = *((uint8 *) (buf_off + sample_stride + sample_stride));
					clr_plane[pix_indx].RGB.red   = (uint8)(255*(val8r/max_val));
					clr_plane[pix_indx].RGB.green = (uint8)(255*(val8g/max_val));
					clr_plane[pix_indx].RGB.blue  = (uint8)(255*(val8b/max_val));
				} else {
					val16r = *((uint8 *) buf_off);
					val16g = *((uint8 *) (buf_off + sample_stride));
					val16b = *((uint8 *) (buf_off + sample_stride + sample_stride));
					clr_plane[pix_indx].RGB.red   = (uint8)(255*(val16r/max_val));
					clr_plane[pix_indx].RGB.green = (uint8)(255*(val16g/max_val));
					clr_plane[pix_indx].RGB.blue  = (uint8)(255*(val16b/max_val));
				}
			} else {
				if (bits == 8) {
				} else {
				}
			}
		}
	}
		
	row += nrows;
	col += ncols;





	tsize_t TIFFTileSize(TIFF* tif)


	
	TIFFGetField(tif, TIFFTAG_STRIPBYTECOUNTS, &sbc);



	// allocate scanline buffer
	if (planarconfig == PLANARCONFIG_CONTIG)
		buf = _TIFFmalloc(TIFFScanlineSize(tif)*spp);


	// read/allocate the strip byte counts
	TIFFGetField(tif, TIFFTAG_STRIPBYTECOUNTS, &sbc);
	
	//
	tsize_t TIFFStripSize(TIFF* tif)

	tstrip_t TIFFNumberOfStrips(TIFF* tif)

	/* read TIFF file by scanline */
	TIFFSetDirectory(tif,0);
	for (y = 0; y < height; y++) {
     	int col;
		if (bits==8) TIFFReadScanline(tif, buf8, y);
		else TIFFReadScanline(tif, buf, y);
		x=0;col=0;
		while (x < width) {
			double val0=0, val1=0, val2=0;
			if (bits==8) {
				val0=(double)buf8[col];
				if (spp == 3) {
				
				}
			} else {
				val0=(double)buf16[col+sample_index];

			if (spp == 3) {
				clr_plane.RGB.red=(unsigned char)(255*(val/max_val));
			int sample_index;
			for (sample_index = 0; sample_index < spp; sample_index++) {
				if (bits==8) val=(double)buf8[col+sample_index];
				else val=(double)buf16[col+sample_index];
				if (spp==3) { /* RGB image */
					if (sample_index==0) pix.clr.RGB.red=(unsigned char)(255*(val/max_val));
						if (sample_index==1) pix.clr.RGB.green=(unsigned char)(255*(val/max_val));
						if (sample_index==2) pix.clr.RGB.blue=(unsigned char)(255*(val/max_val));
						if ( ColorMode==cmHSV ) pix.clr.HSV=RGB2HSV(pix.clr.RGB);
				}
			}
			if (spp==3) pix.intensity=COLOR2GRAY(RGB2COLOR(pix.clr.RGB));
			if (spp==1) {
				pix.clr.RGB.red=(unsigned char)(255*(val/max_val));
				pix.clr.RGB.green=(unsigned char)(255*(val/max_val));
				pix.clr.RGB.blue=(unsigned char)(255*(val/max_val));
				pix.intensity=val;
			}
			set(x,y,z,pix);		   
			x++;
			col+=spp;
		}
	}
	if (buf8) _TIFFfree(buf8);
	if (buf16) _TIFFfree(buf16);
	TIFFClose(tif);

	return(1);
}
