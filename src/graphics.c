
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

#include "SDL1.3/SDL.h"
#include "graphics.h"
#include "imgloader.h"

// various standard headers
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if (!defined(WIN)) && (!defined(MAC))
static unsigned int fullscreenhack = 1;
#else
static unsigned int fullscreenhack = 0;
#endif

static SDL_Window* mainwindow;
static SDL_Renderer* mainrenderer;
static int softwarerendering = 0;
static int sdlvideoinit = 0;

static struct graphicstexture* texlist = NULL;

struct graphicstexture {
	//basic info
	void* texpixels; //NULL if not currently stored separately
	unsigned int width,height;
	//threaded PNG loading
	void* threadingptr;
	//SDL info
	SDL_Texture* tex; //NULL if not loaded yet
};

int graphics_InitSDL() {
	if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_AUDIO) < 0) {
		flog(LC_ERROR, "Failed to initialize %s: %s", backendname, SDL_GetError());
		return 0;
	}
	return 1;
}

void graphics_PromptTextureLoading(const char* texture, void* callbackptr) {

}

int graphics_SetMode(int width, int height, int fullscreen, int resizable, const char* renderer) {
	//initialize SDL video if not done yet
	if (!sdlvideoinit) {
		if (SDL_VideoInit(NULL) < 0) {
			flog(LC_ERROR, "Failed to initialize %s (SDL_VideoInit): %s", backendname, SDL_GetError());
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
	//create window
	if (fullscreen) {
		if (fullscreenhack) {
		 	//a hack since X11 fullscreen is so buggy
			mainwindow = SDL_CreateWindow(titlebar, 0,0, width, height,0);
			delayedfullscreen = time_GetMilliseconds() + 500;
		}else{
			mainwindow = SDL_CreateWindow(titlebar, 0,0, width, height, SDL_WINDOW_FULLSCREEN);
		}
	}else{
		mainwindow = SDL_CreateWindow(titlebar, SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, width, height, 0);
	}
	if (!mainwindow) {
		flog(LC_ERROR, "Failed to create SDL video window with %dx%dx32 - fullscreen: %s: %s (%s)", width, height, yesnostr(fullscreen), SDL_GetError(), backendname);
		SDL_VideoQuit();
		return 0;
	}
	return 1;
}
