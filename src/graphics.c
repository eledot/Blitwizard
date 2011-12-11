
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
#include "timefuncs.h"
#include "hash.h"

static SDL_Window* mainwindow;
static SDL_Renderer* mainrenderer;
static int softwarerendering = 0;
static int sdlvideoinit = 0;
static int graphicsvisible = 0;

static struct graphicstexture* texlist = NULL;

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
	if (gt->tex || gt->threadingptr || !gt->name) {return 1;}

	//create texture
	SDL_Texture* t = SDL_CreateTexture(mainrenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, gt->width, gt->height);
	if (!t) {
		fprintf(stderr, "Warning: SDL failed to create texture: %s\n",SDL_GetError());
		return 0;
	}

	//lock texture
	void* pixels; int pitch;
	if (SDL_LockTexture(t, NULL, &pixels, &pitch) != 0) {
		fprintf(stderr, "Warning: SDL failed to lock texture: %s\n",SDL_GetError());
		SDL_DestroyTexture(t);
		return 0;
	}

	//copy pixels into texture
	memcpy(pixels, gt->pixels, gt->width * gt->height * 4);
	
	//unlock texture
	SDL_UnlockTexture(t);
	
	//set blend mode
	if (SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND) < 0) {
		printf("Warning: Blend mode SDL_BLENDMODE_BLEND not applied: %s\n",SDL_GetError());
	}
	
	gt->tex = t;
	return 1;
}

void graphics_TextureFromSDL(struct graphicstexture* gt) {
	if (!gt->tex || gt->threadingptr || !gt->name) {return;}

	if (!gt->pixels) {	
		gt->pixels = malloc(gt->width * gt->height * 4);
		if (!gt->pixels) {
			//wipe this texture
			SDL_DestroyTexture(gt->tex);
			gt->tex = NULL;
			free(gt->name);
			gt->name = NULL;
			return;
		}
	
		//Lock SDL Texture
		void* pixels;int pitch;
		if (SDL_LockTexture(gt->tex, NULL, &pixels, &pitch) != 0) {
			//success, the texture is now officially garbage.
			//can/should we do anything about this? (a purely visual problem)
			printf("Warning: SDL_LockTexture() failed\n");
		}else{
	
			//Copy texture
			memcpy(gt->pixels, pixels, gt->width * gt->height * 4);

			//unlock texture again
			SDL_UnlockTexture(gt->tex);

		}
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

void graphics_DrawRectangle(int x, int y, int width, int height, float r, float g, float b, float a) {
	SDL_SetRenderDrawColor(mainrenderer, (int)((float)r * 255.0f),
	(int)((float)g * 255.0f), (int)((float)b * 255.0f), (int)((float)a * 255.0f));
	SDL_Rect rect;
	memset(&rect, 0, sizeof(rect));
	rect.x = x;
	rect.y = y;
	rect.w = width;
	rect.h = height;
	
	SDL_RenderFillRect(mainrenderer, &rect);
}

int graphics_DrawCropped(const char* texname, int x, int y, float alpha, unsigned int sourcex, unsigned int sourcey, unsigned int sourcewidth, unsigned int sourceheight) {
	struct graphicstexture* gt = graphics_GetTextureByName(texname);
	if (!gt || gt->threadingptr || !gt->tex) {
		return 0;
	}

	if (alpha <= 0) {return 1;}
	if (alpha > 1) {alpha = 1;}
	
	//calculate source dimensions
	SDL_Rect src,dest;
	src.x = sourcex;
	src.y = sourcey;

	if (sourcewidth > 0) {
		src.w = sourcewidth;
	}else{
		src.w = gt->width;
	}
	if (sourceheight > 0) {
		src.h = sourceheight;
	}else{
		src.h = gt->height;
	}
	
	//set target dimensinos
	dest.x = x; dest.y = y;
	dest.w = src.w;dest.h = src.h;
	
	//render
	//SDL_SetRenderDrawColor(mainrenderer, 255, 255, 255, 255);
	int i = (int)((float)255.0f * alpha);
	if (SDL_SetTextureAlphaMod(gt->tex, i) < 0) {
		fprintf(stderr,"Warning: Cannot set texture alpha mod %d: %s\n",i,SDL_GetError());
	}
	SDL_RenderCopy(mainrenderer, gt->tex, &src, &dest);
	return 1;
}

int graphics_Draw(const char* texname, int x, int y, float alpha) {
	return graphics_DrawCropped(texname, x, y, alpha, 0, 0, 0, 0);
}

int graphics_GetWindowDimensions(unsigned int* width, unsigned int* height) {
	if (mainwindow) {
		int w,h;
		SDL_GetWindowSize(mainwindow, &w,&h);
		if (w < 0 || h < 0) {return 0;}
		*width = w;
		*height = h;
		return 1;
	}
	return 0;
}

int graphics_GetTextureDimensions(const char* name, unsigned int* width, unsigned int* height) {
	struct graphicstexture* gt = graphics_GetTextureByName(name);
	if (!gt || gt->threadingptr) {return 0;}
	
	*width = gt->width;
	*height = gt->height;
	return 1;
}

int graphics_PromptTextureLoading(const char* texture) {
	//check if texture is already present or being loaded
	struct graphicstexture* gt = graphics_GetTextureByName(texture);
	if (gt) {
		//check for threaded loading
		if (gt->threadingptr) {
			//it will be loaded.
			return 1;
		}
		return 2; //texture is already present
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

	//trigger image fetching thread
        gt->threadingptr = img_LoadImageThreadedFromFile(gt->name, 0, 0, "rgba", NULL);
        if (!gt->threadingptr) {
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


int graphics_FinishImageLoading(struct graphicstexture* gt, struct graphicstexture* gtprev, void (*callback)(int success, const char* texture)) {
    char* data;int width,height;
    img_GetData(gt->threadingptr, NULL, &width, &height, &data);
    img_FreeHandle(gt->threadingptr);
    gt->threadingptr = NULL;

    //check if we succeeded
    int success = 0;
    if (data) {
        gt->pixels = data;
        gt->width = width;
        gt->height = height;
        success = 1;
        if (graphics_AreGraphicsRunning() && gt->name) {
            if (!graphics_TextureToSDL(gt)) {
                success = 0;
            }
        }
    }

    //do callback
    if (callback) {
        callback(success, gt->name);
    }

    //if this is an empty abandoned or a failed entry, remove
    if (!gt->name || !success) {
         graphics_FreeTexture(gt, gtprev);
         return 0;
    }
    return 1;
}

struct graphicstexture* graphics_GetPreviousTexture(struct graphicstexture* gt) {
	struct graphicstexture* gtprev = texlist;
    while (gtprev && !(gtprev->next == gt)) {
        gtprev = gtprev->next;
    }
	return gtprev;
}

int graphics_LoadTextureInstantly(const char* texture) {
	//prompt normal async texture loading
	if (!graphics_PromptTextureLoading(texture)) {
		return 0;
	}
	struct graphicstexture* gt = graphics_GetTextureByName(texture);
	if (!gt) {
		return 0;
	}

	//wait for loading to finish
	while (!img_CheckSuccess(gt->threadingptr)) {time_Sleep(10);}
	
	//complete image
	return graphics_FinishImageLoading(gt, graphics_GetPreviousTexture(gt), NULL);
}

void graphics_UnloadTexture(const char* texname) {
    struct graphicstexture* gt = graphics_GetTextureByName(texname);
    if (gt && !gt->threadingptr) {
		struct graphicstexture* gtprev = graphics_GetPreviousTexture(gt);
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

void graphics_CheckTextureLoading(void (*callback)(int success, const char* texture)) {
	struct graphicstexture* gt = texlist;
	struct graphicstexture* gtprev = NULL;
	while (gt) {
		struct graphicstexture* gtnext = gt->next;
		if (gt->threadingptr) {
			//texture which is currently being loaded
			if (img_CheckSuccess(gt->threadingptr)) {
				graphics_FinishImageLoading(gt,gtprev,callback);
			}
		}
		if (!gt->name) {
			//delete abandoned textures
			graphics_FreeTexture(gt, gtprev);
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
#ifndef WIN
	char preferredrenderer[20] = "opengl";
#else
	char preferredrenderer[20] = "direct3d";
#endif
	softwarerendering = 0;
	if (renderer) {
		if (strcasecmp(renderer, "software") == 0) {
			softwarerendering = 1;
		}else{
			if (strcasecmp(renderer, "opengl") == 0) {
				strcpy(preferredrenderer, "opengl");
			}
#ifdef WIN
			if (strcasecmp(renderer,"direct3d") == 0) {
				strcpy(preferredrenderer, "direct3d");
			}
#endif
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
		if (softwarerendering) {
			snprintf(errormsg,sizeof(errormsg),"Failed to create SDL renderer (backend software): %s", SDL_GetError());
		}else{
			SDL_RendererInfo info;
			SDL_GetRenderDriverInfo(rendererindex, &info);
			snprintf(errormsg,sizeof(errormsg),"Failed to create SDL renderer (backend %s): %s", info.name, SDL_GetError());
		}
		errormsg[sizeof(errormsg)-1] = 0;
		*error = strdup(errormsg);
		return 0;
	}else{
		SDL_RendererInfo info;
		SDL_GetRendererInfo(mainrenderer, &info);
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

int graphics_IsTextureLoaded(const char* name) {
	//check texture state
	struct graphicstexture* gt = graphics_GetTextureByName(name);
	if (gt) {
		//check for threaded loading
		if (gt->threadingptr) {
			//it will be loaded.
			return 1;
		}
		return 2; //texture is already present
	}
	return 0; //not loaded
}

void graphics_StartFrame() {
	SDL_SetRenderDrawColor(mainrenderer, 0, 0, 0, 1);
	SDL_RenderClear(mainrenderer);
}

void graphics_CompleteFrame() {
	SDL_RenderPresent(mainrenderer);
}

void graphics_CheckEvents(void (*quitevent)(void), void (*mousebuttonevent)(int button, int release, int x, int y), void (*mousemoveevent)(int x, int y), void (*keyboardevent)(const char* button, int release), void (*textevent)(const char* text)) {
	SDL_Event e;
        while (SDL_PollEvent(&e) == 1) {
		if (e.type == SDL_QUIT) {
			quitevent();
		}
		if (e.type == SDL_MOUSEMOTION) {
			mousemoveevent(e.motion.x, e.motion.y);
		}
		if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
			int release = 0;
			if (e.type == SDL_MOUSEBUTTONUP) {release = 1;}
			int button = e.button.button;
			mousebuttonevent(button, release, e.button.x, e.button.y);
		}
		if (e.type == SDL_TEXTINPUT) {
			textevent(e.text.text);
		}
		if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
			int release = 0;
			if (e.type == SDL_KEYUP) {release = 1;}
			char keybuf[30] = "";
			if (e.key.keysym.sym >= SDLK_F1 && e.key.keysym.sym <= SDLK_F12) {
                                sprintf(keybuf, "F%d", (e.key.keysym.sym+1)-SDLK_F1);
                        }
			if (e.key.keysym.sym >= SDLK_0 && e.key.keysym.sym <= SDLK_9) {
				sprintf(keybuf, "%d", e.key.keysym.sym - SDLK_0);
			}
			if (e.key.keysym.sym >= SDLK_a && e.key.keysym.sym <= SDLK_z) {
				sprintf(keybuf, "%c", e.key.keysym.sym - SDLK_a + 'a');
			}
			switch (e.key.keysym.sym) {
				case SDLK_SPACE:
					sprintf(keybuf, "space");break;
				case SDLK_BACKSPACE:
					sprintf(keybuf, "backspace");break;
				case SDLK_RETURN:
					sprintf(keybuf, "return");break;
				case SDLK_ESCAPE:
					sprintf(keybuf, "escape");break;
				case SDLK_TAB:
					sprintf(keybuf, "tab");break;
				case SDLK_LSHIFT:
					sprintf(keybuf, "lshift");break;
				case SDLK_RSHIFT:
					sprintf(keybuf, "rshift");break;
				case SDLK_UP:
					sprintf(keybuf,"up");break;
				case SDLK_DOWN:
					sprintf(keybuf,"down");break;
				case SDLK_LEFT:
					sprintf(keybuf,"left");break;
				case SDLK_RIGHT:
					sprintf(keybuf,"right");break;
			}
			if (strlen(keybuf) > 0) {
				keyboardevent(keybuf, release);
			}
		}
	}
}

