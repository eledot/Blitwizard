
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
#include <string.h>

#include "vorbis/vorbisfile.h"

#include "audiosource.h"
#include "audiosourceogg.h"

struct audiosourceogg_internaldata {
	struct audiosource* filesource;
	int filesourceeof;
	char fetchedbuf[4096];
	unsigned int fetchedbytes;
	unsigned int fetchedbufreadoffset;
	int eof;
	int returnerroroneof;
	char decodedbuf[512];
	unsigned int decodedbytes;
	
	int vorbisopened;
	OggVorbis_File vorbisfile;
	int vbitstream; //required by libvorbisfile internally
	int vorbiseof;
};

static void audiosourceogg_Rewind(struct audiosource* source) {
	struct audiosourceogg_internaldata* idata = source->internaldata;
	if (!idata->eof || !idata->returnerroroneof) {
		if (idata->vorbisopened) {
			ov_clear(&idata->vorbisfile);
			idata->vorbisopened = 0;
			idata->vbitstream = 0;
		}
		idata->filesource->rewind(idata->filesource);
		idata->filesourceeof = 0;
		idata->eof = 0;
		idata->returnerroroneof = 0;
		idata->fetchedbufreadoffset = 0;
		idata->fetchedbytes = 0;
		idata->decodedbytes = 0;
		idata->vorbiseof = 0;
	}
}

static void audiosourceogg_ReadUndecoded(struct audiosourceogg_internaldata* idata) {
	if (idata->filesourceeof) {
		return;
	}
	
	//move buffer back if required
	if (idata->fetchedbufreadoffset > 0) {
		memmove(idata->fetchedbuf, idata->fetchedbuf + idata->fetchedbufreadoffset, idata->fetchedbytes);
		idata->fetchedbufreadoffset = 0;
	}

	//fetch new bytes
	if (idata->fetchedbytes < sizeof(idata->fetchedbuf)) {
		if (!idata->filesourceeof) {
			int i = idata->filesource->read(idata->filesource, idata->fetchedbuf + idata->fetchedbytes, sizeof(idata->fetchedbuf) - idata->fetchedbytes);
			if (i <= 0) {
				idata->filesourceeof = 1;
				if (i < 0) {
					idata->returnerroroneof = 1;
				}
			}else{
				idata->fetchedbytes += i;
			}
		}
	}
}

//block size reader for libvorbisfile:
static size_t vorbismemoryreader(void *ptr, size_t size, size_t nmemb, void *datasource) {	
	struct audiosourceogg_internaldata* idata = datasource;
	
	unsigned int writtenchunks = 0;
	while (writtenchunks < nmemb) {
		//read new bytes
		if (idata->fetchedbytes < size && idata->filesourceeof != 1) {
			audiosourceogg_ReadUndecoded(idata);
		}
	
		//see how many bytes we have now
		unsigned int amount = size * nmemb;
		if (amount > idata->fetchedbytes) {amount = idata->fetchedbytes;}
		
		//is it sufficient for a chunk?
		if (amount < size) {
			//no, we are done
			return writtenchunks;
		}else{
			//yes, check for amount of chunks
			unsigned int chunks = 0;
			while (chunks * size <= amount - size && chunks < nmemb) {
				chunks += 1;
			}
			
			//write chunks
			writtenchunks += chunks;
			memcpy(ptr, idata->fetchedbuf + idata->fetchedbufreadoffset, chunks * size);
			idata->fetchedbytes -= chunks * size;
			idata->fetchedbufreadoffset += chunks * size;
		}
	}
	return writtenchunks;
}

static int audiosourceogg_InitOgg(struct audiosource* source) {
	struct audiosourceogg_internaldata* idata = source->internaldata;
	if (idata->vorbisopened) {
		return 1;
	}

	ov_callbacks callbacks;
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.read_func = &vorbismemoryreader;
	
	int v = ov_open_callbacks(idata, &idata->vorbisfile, NULL, 0, callbacks);
	if (v != 0) {
		return 0;
	}
	
	return 1;
}

static int audiosourceogg_Read(struct audiosource* source, char* buffer, unsigned int bytes) {
	struct audiosourceogg_internaldata* idata = source->internaldata;
	if (idata->eof) {
		return -1;
	}
	
	//open up ogg file if we don't have one yet
	if (!idata->vorbisopened) {
		if (!audiosourceogg_InitOgg(source)) {
			idata->eof = 1;
			source->samplerate = -1;
			source->channels = -1;
			return -1;
		}
		vorbis_info* vi = ov_info(&idata->vorbisfile, -1);
		source->samplerate = vi->rate;
		source->channels = vi->channels;
		if (source->samplerate != 48000 && source->samplerate != 44100 && source->samplerate != 22050) {
			//incompatible sample rate
			idata->eof = 1;
			source->samplerate = -1;
			source->channels = -1;
			ov_clear(&idata->vorbisfile);
			return -1;
		}
		if (source->channels < 1 || source->channels > 2) {
			//incompatible channel count
			idata->eof = 1;
			source->samplerate = -1;
			source->channels = -1;
			ov_clear(&idata->vorbisfile);
			return -1;
		}
		idata->vorbisopened = 1;
	}
	
	//if no bytes were requested, don't do anything
	if (bytes <= 0) {
		return 0;
	}

	//read bytes
	audiosourceogg_ReadUndecoded(idata);
	
	unsigned int byteswritten = 0;
	while (bytes > 0) {
		//see how much we want to decode now
		unsigned int decodeamount = bytes;
		if (decodeamount > sizeof(idata->decodedbuf)) {
			decodeamount = sizeof(idata->decodedbuf);
		}
		
		//decode from encoded fetched bytes
		unsigned int decodesamples = decodeamount/(source->channels * sizeof(float));
		if (decodesamples * (source->channels * sizeof(float)) < decodeamount && decodesamples * (source->channels * sizeof(float)) <= (sizeof(idata->decodedbuf) - sizeof(float) * source->channels)) {
			//make sure we cover all desired bytes even for half-sample requests or something
			decodesamples++;
		}
		while (!idata->vorbiseof && decodesamples > 0 && !idata->vorbiseof) {
			float **pcm;
			
			long ret = ov_read_float(&idata->vorbisfile, &pcm, decodesamples, &idata->vbitstream);
			if (ret < 0) {
				if (ret == OV_HOLE) {
					//a "jump" or temporary decoding problem - ignore this
					continue;
				}
				//some vorbis error we want to report
				idata->returnerroroneof = 1;
				idata->vorbiseof = 1;
			}else{
				if (ret > 0) {
					//success
					unsigned int i = 0;
					while (i < (unsigned int)ret) {
						unsigned int j = 0;
						while (j < source->channels) {
							memcpy(idata->decodedbuf + idata->decodedbytes, &pcm[j][0], sizeof(float));
							idata->decodedbytes += sizeof(float);
							j++;
						}
						i++;
					}
				}else{
					//regular eof
					idata->vorbiseof = 1;
				}
			}
			break;
		}
		
		//check how much we want and can copy
		unsigned int amount = idata->decodedbytes;
		if (amount > bytes) {
			//never copy more than we want
			amount = bytes;
		}
		
		//see vorbis end of file
		if (amount <= 0) {
			if (byteswritten <= 0) {
				idata->eof = 1;
				if (idata->returnerroroneof) {
					return -1;
				}
				return 0;
			}else{
				return byteswritten;
			}
		}
		
		//output bytes
		memcpy(buffer, idata->decodedbuf, amount);
		byteswritten += amount;
		buffer += amount;

		//empty our decode buffer
		memmove(idata->decodedbuf, idata->decodedbuf + amount, sizeof(idata->decodedbuf) - amount);
		idata->decodedbytes -= amount;
		bytes -= amount;
	}
	return byteswritten;
}

static void audiosourceogg_Close(struct audiosource* source) {
	struct audiosourceogg_internaldata* idata = source->internaldata;
	
	//close ogg file if we have it open
	if (idata->vorbisopened) {
		ov_clear(&idata->vorbisfile);
	}
	
	//close file source if we have one
	if (idata->filesource) {
		idata->filesource->close(idata->filesource);
	}
	
	//free all structs
	if (source->internaldata) {
		free(source->internaldata);
	}
	free(source);
}

struct audiosource* audiosourceogg_Create(struct audiosource* filesource) {
	if (!filesource) {return NULL;}
	//main data structure
	struct audiosource* a = malloc(sizeof(*a));
	if (!a) {
		filesource->close(filesource);
		return NULL;
	}
	memset(a,0,sizeof(*a));
	
	//internal data structure
	a->internaldata = malloc(sizeof(struct audiosourceogg_internaldata));
	if (!a->internaldata) {
		free(a);
		filesource->close(filesource);
		return NULL;
	}
	
	//internal data values
	struct audiosourceogg_internaldata* idata = a->internaldata;
	memset(idata, 0, sizeof(*idata));
	idata->filesource = filesource;
	
	//function pointers
	a->read = &audiosourceogg_Read;
	a->close = &audiosourceogg_Close;
	a->rewind = &audiosourceogg_Rewind;
	
	//ensure proper initialisation of sample rate + channels variables
	audiosourceogg_Read(a, NULL, 0);
	if (idata->eof && idata->returnerroroneof) {
		//There was an error reading this ogg file - probably not ogg (or broken ogg)
		audiosourceogg_Close(a);
		return NULL;
	}
	
	return a;
}

