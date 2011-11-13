
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

// various standard headers
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "SDL.h"
#include "graphics.h"
#include "imgloader.h"
#include "hash.h"

static SDL_Window* mainwindow;
static SDL_Renderer* mainrenderer;
static int softwarerendering = 0;
static int sdlvideoinit = 0;
static int graphicsvisible = 0;

static struct graphicstexture* texlist = NULL;

struct graphicstexturecallback {
	void* callbackptr;
	struct graphicstexturecallback* next;
};
struct graphicstexture {
	//basic info
	char* name;
	void* pixels; //NULL if not currently stored separately
	unsigned int width,height;
	//threaded PNG loading
	int autodelete;
	void* threadingptr;
	//SDL info
	SDL_Texture* tex; //NULL if not loaded yet
	//callbacks
	struct graphicstexturecallback* callbacks;
	
	//pointer to next list element
	struct graphicstexture* next;
	//pointer to next hashmap bucket element
	struct graphicstexture* hashbucketnext;
};

hashmap* texhashmap = NULL;

static struct graphicstexture* graphics_GetTextureByName(const char* name) {
	uint32_t i = hashmap_GetIndex(texhashmap, name, strlen(name), 1);
	struct graphicstexture* gt = (struct graphicstexture*)(texhashmap->items[i]);
	while (gt && !(strcasecmp(gt->name, name) == 0)) {
		gt = gt->hashbucketnext;
	}
	return gt;
}

int graphics_AreGraphicsRunning() {
	return graphicsvisible;
}

int graphics_Init(char** error) {
	char errormsg[512];
	texhashmap = hashmap_New(2048);
	if (!texhashmap) {
		*error = strdup("Failed to allocate texture hash map");
		return 0;
	}
	if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_AUDIO) < 0) {
		snprintf(errormsg,sizeof(errormsg),"Failed to initialize SDL: %s", SDL_GetError());
		errormsg[sizeof(errormsg)-1] = 0;
		*error = strdup(errormsg);
		return 0;
	}
	return 1;
}

int graphics_TextureToSDL(struct graphicstexture* gt) {
	if (gt->tex || gt->threadingptr || !gt->pixels || !gt->name) {return 1;}
	
	//create SDL surface
	SDL_Surface* sf = SDL_CreateRGBSurface(SDL_SWSURFACE, gt->width, gt->height, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000); //rgba (intel byte-reversed)
	if (!sf) {
		return 0;
	}
	
	//fill SDL surface
	memcpy(sf->pixels, gt->pixels, sf->w * sf->h * 4);
	
	//now, optimize surface:
	SDL_Surface* optimizedImage = NULL;
	if (sf != NULL) {
		if (1) {
			optimizedImage = SDL_DisplayFormatAlpha(sf);
		}else{
			optimizedImage = SDL_DisplayFormat(sf);
		}
		if (optimizedImage) {
			SDL_FreeSurface (sf);
			sf = optimizedImage;
		}
	}
	
	//create texture
	SDL_Texture* t = SDL_CreateTextureFromSurface(mainrenderer, sf);
	if (!t) {
		SDL_FreeSurface(sf);
		return 0;
	}
	SDL_FreeSurface(sf);
	
	//set blend mode
	SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
	
	//wipe out pixels to save some memory
	free(gt->pixels);
	gt->pixels = NULL;
	gt->tex = t;
	return 1;
}

void graphics_TextureFromSDL(struct graphicstexture* gt) {
	if (!gt->tex || gt->threadingptr || !gt->pixels) {return;}
	
	gt->pixels = malloc(gt->width * gt->height * 4);
	if (!gt->pixels) {
		//turn this into a failed-to-load texture
		SDL_DestroyTexture(gt->tex);
		gt->tex = NULL;
	}
	
	//Lock SDL Texture
	void* pixels;int pitch;
	if (SDL_LockTexture(gt->tex, NULL, &pixels, &pitch) != 0) {
		//success, the texture is now officially garbage.
		//can/should we do anything about this? (a purely visual problem)
		SDL_DestroyTexture(gt->tex);
		gt->tex = NULL;
		return;
	}
	
	//Copy texture
	if (pitch == 0) {
		memcpy(gt->pixels, pixels, gt->width * gt->height * 4);
	}
	
	SDL_DestroyTexture(gt->tex);
	gt->tex = NULL;
}

void graphics_AddTextureToHashmap(struct graphicstexture* gt) {
	uint32_t i = hashmap_GetIndex(texhashmap, gt->name, strlen(gt->name), 1);
	gt->hashbucketnext = (struct graphicstexture*)(texhashmap->items[i]);
	texhashmap->items[i] = gt;
}

void graphics_RemoveTextureFromHashmap(struct graphicstexture* gt) {
	uint32_t i = hashmap_GetIndex(texhashmap, gt->name, strlen(gt->name), 1);
	struct graphicstexture* gt2 = (struct graphicstexture*)(texhashmap->items[i]);
	struct graphicstexture* gtprev = NULL;
	while (gt2) {
		if (gt2 == gt) {
			if (gtprev) {
				gtprev->next = gt->hashbucketnext;
			}else{
				texhashmap->items[i] = gt->hashbucketnext;
			}
			gt->hashbucketnext = NULL;
			return;
		}
		
		gtprev = gt2;
		gt2 = gt2->hashbucketnext;
	}
}

void graphics_TransferTexturesFromSDL() {
	struct graphicstexture* gt = texlist;
	while (gt) {
		graphics_TextureFromSDL(gt);
		gt = gt->next;
	}
}

int graphics_TransferTexturesToSDL() {
	struct graphicstexture* gt = texlist;
	while (gt) {
		if (!graphics_TextureToSDL(gt)) {
			return 0;
		}
		gt = gt->next;
	}
	return 1;
}

int graphics_DrawCropped(const char* texname, int x, int y, float alpha, unsigned int sourcex, unsigned int sourcey, unsigned int sourcewidth, unsigned int sourceheight) {
	struct graphicstexture* gt = graphics_GetTextureByName(texname);
	if (!gt || gt->threadingptr || !gt->tex) {return 0;}
	return 1;
}

int graphics_Draw(const char* texname, int x, int y, float alpha) {
	return graphics_DrawCropped(texname, x, y, alpha, 0, 0, -1, -1);
}


int graphics_PromptTextureLoading(const char* texture, void* callbackptr) {
	//check if texture is already present or being loaded
	struct graphicstexture* gt = texlist;
	while (gt) {
		if (gt->name && strcasecmp(gt->name, texture) == 0) {
			//check for threaded loading
			if (gt->threadingptr) {
				//add us to the callback queue
				struct graphicstexturecallback* gtcallback = malloc(sizeof(*gtcallback));
				if (!gtcallback) {
					return 0;
				}
				gtcallback->callbackptr = callbackptr;
				gtcallback->next = gt->callbacks;
				gt->callbacks = gtcallback;
				return 1;
			}
			return 2; //texture is already present
		}
		gt = gt->next;
	}
	
	//allocate new texture info
	gt = malloc(sizeof(*gt));
	if (!gt) {
		return 0;
	}
	memset(gt, 0, sizeof(*gt));
	gt->name = strdup(texture);
	if (!gt->name) {
		free(gt);
		return 0;
	}

	struct graphicstexturecallback* gtcallback = malloc(sizeof(*gtcallback));
	gtcallback->callbackptr = callbackptr;
	gtcallback->next = NULL;
	if (!gtcallback) {
		free(gt->name);
		free(gt);
	}
	gt->callbacks = gtcallback;	

	//trigger image fetching thread
        gt->threadingptr = img_LoadImageThreadedFromFile(gt->name, 0, 0, "rgba", NULL);
        if (!gt->threadingptr) {
		free(gt->callbacks);
                free(gt->name);
                free(gt);
                return 0;
        }
	
	//add us to the list
	gt->next = texlist;
	texlist = gt;
	graphics_AddTextureToHashmap(gt);
	return 1;
}

int graphics_FreeTexture(struct graphicstexture* gt, struct graphicstexture* prev) {
	while (gt->callbacks) {
		struct graphicstexturecallback* gtcallback = gt->callbacks;
		gt->callbacks = gt->callbacks->next;
		free(gtcallback);
	}
	gt->callbacks = NULL;
	if (gt->pixels) {
		free(gt->pixels);
		gt->pixels = NULL;
	}
	if (gt->tex){
		SDL_DestroyTexture(gt->tex);
		gt->tex = NULL;
	}
	if (gt->name) {
		graphics_RemoveTextureFromHashmap(gt);
		free(gt->name);
		gt->name = NULL;
	}
	if (gt->threadingptr) {
		return 0;
	}
	if (prev) {
		prev->next = gt->next;
	}else{
		texlist = gt->next;
	}
	free(gt);
	return 1;
}

void graphics_UnloadTexture(const char* texname) {
        struct graphicstexture* gt = graphics_GetTextureByName(texname);
        if (gt && !gt->threadingptr) {
		struct graphicstexture* gtprev = texlist;
		while (gtprev && !(gtprev->next == gt)) {
			gtprev = gtprev->next;
		}
                graphics_FreeTexture(gt, gtprev);
        }
}

int graphics_FreeAllTextures() {
	int fullycleaned = 1;
	struct graphicstexture* gt = texlist;
	struct graphicstexture* gtprev = NULL;
	while (gt) {
		if (!graphics_FreeTexture(gt, gtprev)) {
			fullycleaned = 0;
		}
		gtprev = gt;
		gt = gt->next;
	}
	return fullycleaned;
}

void graphics_CheckTextureLoading(void (*callbackinfo)(int success, const char* texture, void* callbackptr)) {
	struct graphicstexture* gt = texlist;
	struct graphicstexture* gtprev = NULL;
	while (gt) {
		struct graphicstexture* gtnext = gt->next;
		if (gt->threadingptr) {
			//texture which is currently being loaded
			if (img_CheckSuccess(gt->threadingptr)) {
				char* data;int width,height;
				img_GetData(gt->threadingptr, NULL, &width, &height, &data);
				img_FreeHandle(gt->threadingptr);
				gt->threadingptr = NULL;
				
				//now do the callbacks
				int success = 0;
				if (data) {
					success = 1;
					if (!graphics_TextureToSDL(gt)) {
						free(data);
						success = 0;
					}
				}
				while (gt->callbacks) {
					struct graphicstexturecallback* gtcallback = gt->callbacks;
					gt->callbacks = gt->callbacks->next;
					callbackinfo(success, gt->name, gtcallback->callbackptr);
					free(gtcallback);
				}
				gt->callbacks = NULL;
				
				//if this is an empty abandoned entry, remove
				if (!gt->name) {
					graphics_FreeTexture(gt, gtprev);
				}
			}
		}else{
			if (!gt->name) {
				//delete abandoned textures
				graphics_FreeTexture(gt, gtprev);
			}
		}
		gtprev = gt;
		gt = gtnext;
	}
}

void graphics_Quit() {
	graphicsvisible = 0;
	if (mainrenderer) {
		graphics_TransferTexturesFromSDL();
		SDL_DestroyRenderer(mainrenderer);
		mainrenderer = NULL;
	}
	if (mainwindow) {
		SDL_DestroyWindow(mainwindow);
		mainwindow = NULL;
	}
}

int graphics_SetMode(int width, int height, int fullscreen, int resizable, const char* title, const char* renderer, char** error) {
	char errormsg[512];
	//initialize SDL video if not done yet
	if (!sdlvideoinit) {
		if (SDL_VideoInit(NULL) < 0) {
			snprintf(errormsg,sizeof(errormsg),"Failed to initialize SDL video: %s", SDL_GetError());
			errormsg[sizeof(errormsg)-1] = 0;
			*error = strdup(errormsg);
			return 0;
		}
		sdlvideoinit = 1;
	}
	//think about the renderer we want
	char preferredrenderer[20] = "";
	softwarerendering = 0;
	if (renderer) {
		if (strcasecmp(renderer, "software") == 0) {
			softwarerendering = 1;
		}else{
			if (strcasecmp(renderer, "opengl") == 0) {
				strcpy(preferredrenderer, "opengl");
			}else{
		#ifdef WIN
				strcpy(preferredrenderer, "direct3d");
		#endif
			}
		}
	}
	//get renderer index
	int rendererindex = -1;
	if (strlen(preferredrenderer) > 0 && !softwarerendering) {
		int count = SDL_GetNumRenderDrivers();
		int r = 0;
		while (r < count) {
			SDL_RendererInfo info;
			SDL_GetRenderDriverInfo(r, &info);
			if (strcasecmp(info.name, preferredrenderer) == 0) {
				rendererindex = r;
				break;
			}
			r++;
		}
	}
	//preserve textures by managing them on our own for now
	graphics_TransferTexturesFromSDL();
	//destroy old window/renderer if we got one
	graphics_Quit();
	//create window
	if (fullscreen) {
		mainwindow = SDL_CreateWindow(title, 0,0, width, height, SDL_WINDOW_FULLSCREEN);
	}else{
		mainwindow = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, width, height, 0);
	}
	if (!mainwindow) {
		snprintf(errormsg,sizeof(errormsg),"Failed to open SDL window: %s", SDL_GetError());
		errormsg[sizeof(errormsg)-1] = 0;
		*error = strdup(errormsg);
		return 0;
	}
	//Create renderer
	if (!softwarerendering) {
		mainrenderer = SDL_CreateRenderer(mainwindow, rendererindex, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
	}else{
		mainrenderer = SDL_CreateRenderer(mainwindow, -1, SDL_RENDERER_SOFTWARE);
	}
	if (!mainrenderer) {
		if (mainwindow) {
			SDL_DestroyWindow(mainwindow);
			mainwindow = NULL;
		}
		SDL_RendererInfo info;
                SDL_GetRenderDriverInfo(rendererindex, &info);
		snprintf(errormsg,sizeof(errormsg),"Failed to create SDL renderer (backend %s): %s", info.name, SDL_GetError());
		errormsg[sizeof(errormsg)-1] = 0;
		*error = strdup(errormsg);
		return 0;
	}else{
		SDL_RendererInfo info;
		SDL_GetRendererInfo(mainrenderer, &info);
		//printf("Graphics renderer is: %s (hardware acceleration: %s)", info.name, yesnostr((int)(info.flags & SDL_RENDERER_ACCELERATED)));
	}
	//Transfer textures back to SDL
	if (!graphics_TransferTexturesToSDL()) {
		SDL_RendererInfo info;
		SDL_GetRendererInfo(mainrenderer, &info);
		snprintf(errormsg,sizeof(errormsg),"Failed to create SDL renderer (backend %s): Cannot recreate textures", info.name);
		*error = strdup(errormsg);
		SDL_DestroyRenderer(mainrenderer);
		SDL_DestroyWindow(mainwindow);
		return 0;
	}
	graphicsvisible = 1;
	return 1;
}
