
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

#include "vorbis/vorbisfile.h"

#include "audiosource.h"
#include "audiosourceogg.h"

struct audiosourceogg_internaldata {
	struct audiosource* filesource;
	char fetchedbuf[512];
	int fetchedbytes;
	int fetchedbufreadoffset;
	int eof;
	int reporterroroneof;
	char decodedbuf[512];
	int decodedbytes;
	
	int vorbisopen;
	OggVorbis_File vorbisfile;
	int vbitstream; //required by libvorbisfile internally
	int vorbiseof;
};

static void audiosourceogg_ReadUndecoded(struct audiosourceogg_internaldata* idata) {
	//move buffer back if required
	if (idata->fetchedbufreadoffset > 0) {
		memmove(idata->fetchedbuf, idata->fetchedbuf + idata->fetchedbufreadoffset, idata->fetchedbytes);
		idata->fetchedbufreadoffset = 0;
	}

	//fetch new bytes
	if (idata->fetchedbytes < sizeof(idata->fetchedbuf)) {
		if (idata->filesource) {
			int i = idata->filesource->read(idata->filesource, sizeof(idata->fetchedbuf) - idata->fetchedbytes, idata->fetchedbuf + idata->fetchedbytes);
			if (i <= 0) {
				idata->filesource->close(idata->filesource);
				idata->filesource = NULL;
				if (i < 0) {
					idata->reporterroroneof = 1;
				}
			}
		}
	}
}

//block size reader for libvorbisfile:
static size_t vorbismemoryreader(void *ptr, size_t size, size_t nmemb, void *datasource) {	
	struct audiosourceogg_internaldata* idata = datasource;
	
	int writtenchunks = 0;
	while (writtenchunks < nmemb) {
		//read new bytes
		if (idata->fetchedbytes < size) {
			audiosourceogg_ReadUndecoded(idata);
		}
	
		//see how many bytes we have now
		int amount = size * nmemb;
		if (amount > idata->fetchedbytes) {amount = idata->fetchedbytes;}
		
		//is it sufficient for a chunk?
		if (amount < nmemb) {
			//no, we are done
			return writtenchunks;
		}else{
			//yes, check for amount of chunks
			int chunks = 0;
			while (chunks * nmemb < amount) {
				chunks += 1;
			}
			
			//write chunks
			writtenchunks += chunks;
			memcpy(ptr, idata->fetchedbuf + idata->fetchedbufreadoffset, chunks * nmemb);
			idata->fetchedbytes -= chunks * nmemb;
			idata->fetchedbufreadoffset += chunks * nmemb;
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
	
	if (ov_open_callbacks(idata, &idata->vorbisfile, NULL, 0, callbacks) != 0) {
		return 0;
	}
	
	return 1;
}

static int audiosourceogg_Read(struct audiosource* source, unsigned int bytes, char* buffer) {
	if (source->eof) {
		return -1;
	}
	
	struct audiosourceogg_internaldata* idata = source->internaldata;
	
	//open up ogg file if we don't have one yet
	if (!idata->vorbisopened) {
		if (!audiosourceogg_InitOgg(source)) {
			source->eof = 1;
			source->samplerate = -1;
			source->channels = -1;
			return -1;
		}
		vorbis_info* vi = ov_info(&idata->vorbisfile, -1);
		source->samplerate = vi->rate;
		source->channels = vi->channels;
		if (source->samplerate != 48000 && source->samplerate != 44100 && source->samplerate != 22050) {
			//incompatible sample rate
			source->eof = 1;
			source->samplerate = -1;
			source->channels = -1;
			ov_clear(&idata->vorbisfile);
			return -1;
		}
		if (source->channels < 1 || source->channels > 2) {
			//incompatible channel count
			source->eof = 1;
			source->samplerate = -1;
			source->channels = -1;
			ov_clear(&idata->vorbisfile);
			return -1;
		}
	}
	
	//if no bytes were requested, don't do anything
	if (bytes <= 0) {
		return 0;
	}
	
	//read bytes
	audiosourceogg_ReadUndecoded(idata);
	
	int byteswritten = 0;
	while (bytes > 0) {
		//see how much we want to decode now
		int decodeamount = bytes;
		if (decodeamount > sizeof(idata->decodedbuf)) {
			decodeamount = sizeof(idata->decodedbuf);
		}
		
		//decode from encoded fetched bytes
		int decodesamples = decodebytes/(source->channels * 2);
		while (!idata->vorbiseof && decodesamples > 0) {
			float **pcm;
			
			long ret = ov_read_float(&idata->vorbisfile, pcm, decodesamples, &source->vbitstream);
		
			int fileend = 0;
			if (ret < 0) {
				if (ret == OV_HOLE) {
					//a "jump" or temporary decoding problem - ignore this
					continue;
				}
				//some vorbis error we want to report
				idata->reporterroroneof = 1;
				idata->vorbiseof = 1;
			}else{
				if (ret > 0) {
					//success
					int i = 0;
					while (i < ret) {
						int j = 0;
						while (j < source->channels) {
							memcpy(i->decodedbuf + i->decodedbytes, &pcm[j][0], sizeof(float));
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
		int amount = idata->decodedbytes;
		if (amount > bytes) {
			//never copy more than we want
			amount = bytes;
		}
		
		//see vorbis end of file
		if (amount <= 0) {
			if (byteswritten <= 0) {
				source->eof = 1;
				if (idata->reporterroroneof) {
					return -1;
				}
				return 0;
			}else{
				return byteswritten;
			}
		}
		
		//copy
		memcpy(buffer, decodedbuf, amount);
		idata->decodedbuf += amount;
		idata->decodedbytes -= amount;
		bytes -= amount;
	}
	return byteswritten;
}

static void audiosourceogg_Close(struct audiosource* source) {
	struct audiosourceogg_internaldata* idata = source->internaldata;

	//close audio file source we might have opened
	struct audiosource* a = idata->filesource;
	if (a) {
		a->close(a);
	}
	
	//close ogg file if we have it open
	if (idata->vorbisopened) {
		ov_clear(&idata->vorbisfile);
	}
	
	//free all structs
	if (source->internaldata) {
		free(source->internaldata);
	}
	free(source);
}

struct audiosource* audiosourceogg_Create(struct audiosource* filesource) {
	if (!filesource) {return NULL;}
	struct audiosource* a = malloc(sizeof(*a));
	if (!a) {
		filesource->close(filesource);
		return NULL;
	}
	memset(a,0,sizeof(*a));
	
	a->internaldata = malloc(sizeof(struct audiosourceogg_internaldata));
	if (!a->internaldata) {
		free(a);
		filesource->close(filesource);
		return NULL;
	}
	
	a->read = &audiosourceogg_Read;
	a->close = &audiosourceogg_Close;
	
	//ensure proper initialisation of sample rate
	audiosourceogg_Read(a, 0, NULL);
	
	return NULL;
}

