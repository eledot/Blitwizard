
/* blitwizard 2d engine - source code file

  Copyright (C) 2011 Jonas Thiem

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "png/png.h"

#include "pngloader.h"

struct loadpnginfo {
	const void* source;
	unsigned int sourcesize;
	unsigned int readoffset;
	png_structp png_ptr;
	png_infop info_ptr;
	void** row_pointers;
	void* imgdat;
};

static int pngloader_CheckIfPng(const void* data, unsigned int datasize) {
	int ret;
	if (datasize < 8) {
		return 0;
	}
	ret = png_sig_cmp((png_bytep)data, 0, 8);
	if (ret == 0) {
		return 1;
	}
	return 0;
}

void readdata(png_structp png_ptr, png_bytep data, png_size_t length)  {
	struct loadpnginfo* linfo = (struct loadpnginfo*)(png_ptr->io_ptr);
	memcpy(data, linfo->source+linfo->readoffset, length);
	linfo->readoffset += length;
}

void pngloader_FreeLoadInfo(struct loadpnginfo* linfo) {
	if (linfo->png_ptr) {
		png_destroy_read_struct(&linfo->png_ptr, &linfo->info_ptr, (png_infopp)0);
		linfo->png_ptr = NULL;
		linfo->info_ptr = NULL;
	}
	if (linfo->info_ptr) {
		png_destroy_info_struct(&linfo->png_ptr, &linfo->info_ptr);
		linfo->info_ptr = NULL;
	}
	if (linfo->imgdat) {
		free(linfo->imgdat);
	}
	if (linfo->row_pointers) {
		free(linfo->row_pointers);
	}
	free(linfo);
}
int pngloader_AllocateMembers(struct loadpnginfo* linfo) {
	if (!linfo->png_ptr) {
		linfo->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);;
		if (!linfo->png_ptr) {return 0;}
	}
	if (!linfo->info_ptr) {
		linfo->info_ptr = png_create_info_struct(linfo->png_ptr);
		if (!linfo->info_ptr) {return 0;}
	}
	return 1;
}
int pngloader_LoadRGBA(const char* pngdata, unsigned int pngdatasize, char** imagedata, unsigned int* imagedatasize, int* imagewidth, int* imageheight, int maxwidth, int maxheight) {
	png_uint_32 width, height, channels;
	int bit_depth, color_type;
	  
	//first check
	if (!pngloader_CheckIfPng(pngdata, pngdatasize)) {return 0;}
	
	//get info structs
	struct loadpnginfo* linfo = malloc(sizeof(*linfo));
	if (!linfo) {return 0;}
	memset(linfo,0,sizeof(*linfo));
	if (!pngloader_AllocateMembers(linfo)) {
		pngloader_FreeLoadInfo(linfo);
		return 0;
	}
	linfo->source = pngdata; linfo->sourcesize = pngdatasize;
	
	//set up the very weird error handling
	if (setjmp(png_jmpbuf(linfo->png_ptr))) {
		pngloader_FreeLoadInfo(linfo);
		return 0;
	}
	
	// init structs - THIS IS NOT NEEDED so I believe
	//png_read_init(linfo->png_ptr);
	
	// set up a custom loader
	png_set_read_fn(linfo->png_ptr, linfo, readdata);
	//now read info stuff
	png_read_info(linfo->png_ptr, linfo->info_ptr);
	if (!png_get_IHDR(linfo->png_ptr, linfo->info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL)) {
		pngloader_FreeLoadInfo(linfo);return 0;
	}
	//check whether there is a size limit
	if (maxwidth && width > maxwidth) {pngloader_FreeLoadInfo(linfo);return 0;}
	if (maxheight && height > maxheight) {pngloader_FreeLoadInfo(linfo);return 0;}
	
	//some conversion stuff
	channels = png_get_channels(linfo->png_ptr, linfo->info_ptr);
	switch (color_type) {
		case PNG_COLOR_TYPE_PALETTE:
			png_set_palette_to_rgb(linfo->png_ptr);
			channels = 3;
			break;
		case PNG_COLOR_TYPE_GRAY:
			if (bit_depth < 8) {
				png_set_gray_to_rgb(linfo->png_ptr);
				bit_depth = 8;
			}
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			if (bit_depth < 8) {
				png_set_gray_to_rgb(linfo->png_ptr);
				bit_depth = 8;
			}
			break;
	}
	if (png_get_valid(linfo->png_ptr, linfo->info_ptr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(linfo->png_ptr);
		channels+=1;
	}
	if (bit_depth == 16) {png_set_strip_16(linfo->png_ptr);bit_depth = 8;}
	if (bit_depth != 8) {
		pngloader_FreeLoadInfo(linfo);return 0; // we don't support this :( sorry
	}
	if (channels != 3 && channels != 4) {
		pngloader_FreeLoadInfo(linfo);return 0; //we don't support this :( sorry
	}
	
	//ok let's get it - preparations..
	linfo->row_pointers = malloc(height*sizeof(void*)); //row pointers libpng wants for some odd reason
	if (!linfo->row_pointers) {
		pngloader_FreeLoadInfo(linfo);return 0;
	}
	linfo->imgdat = malloc(channels * width * height); //img data
	if (!linfo->imgdat) {
		pngloader_FreeLoadInfo(linfo);return 0;
	}
	int r = 0;
	while (r < height) {
		linfo->row_pointers[r] = linfo->imgdat + channels * width * r;
		r++;
	}
	
	//loading the image! yeaaa...
	png_read_image(linfo->png_ptr, (png_bytepp)linfo->row_pointers);
	
	free(linfo->row_pointers); linfo->row_pointers = NULL;
	
	//if we have just 3 channels, we really want to expand to 4!
	if (channels == 3) {
		void* newimgdat = malloc(4 * width * height);
		if (!newimgdat) {
			pngloader_FreeLoadInfo(linfo);printf("cannot allocate new info\n");return 0;
		}
		//ugly conversion stuff :p
		r = 0;
		while (r < height) {
			int k = 0;
			while (k < width) {
				memcpy(k*4 + r * width*4 + newimgdat, k*3 + r*width*3 + linfo->imgdat, 3); //copy the three channels we got
				((char*)newimgdat)[k*4+r*width*4+3] = 255; //set alpha to opaque
				k++;
			}
			r++;
		}
		//converted!
		free(linfo->imgdat);
		linfo->imgdat = newimgdat;
		channels = 4;
	}
	
	//ok cleanup and export this
	*imagedatasize = channels * width * height;
	*imagedata = linfo->imgdat;linfo->imgdat = NULL; //of course we want to keep that
	*imagewidth = width;
	*imageheight = height;
	
	pngloader_FreeLoadInfo(linfo);
	return 1;
}