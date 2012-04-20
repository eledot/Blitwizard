
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

#include "os.h"

#ifdef USE_SDL_GRAPHICS

// various standard headers
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#ifdef WINDOWS
#include <windows.h>
#endif
#include <stdarg.h>

#include "logging.h"
#include "imgloader.h"
#include "timefuncs.h"
#include "hash.h"
#include "file.h"
#ifdef NOTHREADEDSDLRW
#include "main.h"
#endif

#include "SDL.h"
#include "SDL_syswm.h"

#include "graphicstexture.h"
#include "graphics.h"
#include "graphicstexturelist.h"

static SDL_Window* mainwindow;
static int mainwindowfullscreen;
static SDL_Renderer* mainrenderer;
static int sdlvideoinit = 0;
extern int graphicsvisible;

int graphics_HaveValidWindow() {
    if (mainwindow) {return 1;}
    return 0;
}

void graphics_DestroyHWTexture(struct graphicstexture* gt) {
    if (gt->tex){
        SDL_DestroyTexture(gt->tex);
        gt->tex = NULL;
    }
}

static int graphics_InitVideoSubsystem(char** error) {
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
    return 1;
}

int graphics_Init(char** error) {
    char errormsg[512];
    
    //set scaling settings
    SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, "2", SDL_HINT_NORMAL);
        
    //initialize name string -> texture slot hashmap
    texhashmap = hashmap_New(2048);
    if (!texhashmap) {
        *error = strdup("Failed to allocate texture hash map");
        return 0;
    }
    
    //initialize SDL
    if (SDL_Init(SDL_INIT_TIMER) < 0) {
        snprintf(errormsg,sizeof(errormsg),"Failed to initialize SDL: %s", SDL_GetError());
        errormsg[sizeof(errormsg)-1] = 0;
        *error = strdup(errormsg);
        return 0;
    }
    return 1;
}

int graphics_TextureToHW(struct graphicstexture* gt) {
    if (gt->tex || gt->threadingptr || !gt->name) {return 1;}

    //create texture
    SDL_Texture* t = SDL_CreateTexture(mainrenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, gt->width, gt->height);
    if (!t) {
        printwarning("Warning: SDL failed to create texture: %s\n",SDL_GetError());
        return 0;
    }

    //lock texture
    void* pixels; int pitch;
    if (SDL_LockTexture(t, NULL, &pixels, &pitch) != 0) {
        printwarning("Warning: SDL failed to lock texture: %s\n",SDL_GetError());
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

    //if on the desktop, discard texture
#if !defined(ANDROID)
    free(gt->pixels);
    gt->pixels = NULL;
#endif
    
    gt->tex = t;
    return 1;
}

void graphics_TextureFromHW(struct graphicstexture* gt) {
    if (!gt->tex || gt->threadingptr || !gt->name) {return;}

    if (!gt->pixels) {  
        gt->pixels = malloc(gt->width * gt->height * 4);
        if (!gt->pixels) {
            //wipe this texture
            SDL_DestroyTexture(gt->tex);
            gt->tex = NULL;
            graphics_RemoveTextureFromHashmap(gt);
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

void graphics_DrawRectangle(int x, int y, int width, int height, float r, float g, float b, float a) {
    SDL_SetRenderDrawColor(mainrenderer, (int)((float)r * 255.0f),
    (int)((float)g * 255.0f), (int)((float)b * 255.0f), (int)((float)a * 255.0f));
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = width;
    rect.h = height;

    SDL_SetRenderDrawBlendMode(mainrenderer, SDL_BLENDMODE_BLEND);  
    SDL_RenderFillRect(mainrenderer, &rect);
}

int graphics_DrawCropped(const char* texname, int x, int y, float alpha, unsigned int sourcex, unsigned int sourcey, unsigned int sourcewidth, unsigned int sourceheight, unsigned int drawwidth, unsigned int drawheight, int rotationcenterx, int rotationcentery, double rotationangle, int horiflipped, double red, double green, double blue) {
    struct graphicstexture* gt = graphicstexturelist_GetTextureByName(texname);
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
    if (drawwidth <= 0 || drawheight <= 0) {
        dest.w = src.w;dest.h = src.h;
    }else{
        dest.w = drawwidth; dest.h = drawheight;
    }
    
    //render
    int i = (int)((float)255.0f * alpha);
    if (SDL_SetTextureAlphaMod(gt->tex, i) < 0) {
        printwarning("Warning: Cannot set texture alpha mod %d: %s\n",i,SDL_GetError());
    }
    SDL_Point p;
    p.x = (int)((double)rotationcenterx * ((double)drawwidth / src.w));
    p.y = (int)((double)rotationcentery * ((double)drawheight / src.h));
    if (red > 1) {red = 1;}
    if (red < 0) {red = 0;}
    if (blue > 1) {blue = 1;}
    if (blue < 0) {blue = 0;}
    if (green > 1) {green = 1;}
    if (green < 0) {green = 0;}
    SDL_SetTextureColorMod(gt->tex, (red * 255.0f), (green * 255.0f), (blue * 255.0f));
    if (horiflipped) {
        SDL_RenderCopyEx(mainrenderer, gt->tex, &src, &dest, rotationangle, &p, SDL_FLIP_HORIZONTAL);
    }else{
        SDL_RenderCopyEx(mainrenderer, gt->tex, &src, &dest, rotationangle, &p, 0);
    }
    return 1;
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



void graphics_Close(int preservetextures) {
    graphicsvisible = 0;
    if (mainrenderer) {
        if (preservetextures) {
            graphicstexturelist_TransferTexturesFromHW();
        }
        SDL_DestroyRenderer(mainrenderer);
        mainrenderer = NULL;
    }
    if (mainwindow) {
        SDL_DestroyWindow(mainwindow);
        mainwindow = NULL;
    }
}

#ifdef ANDROID
void graphics_ReopenForAndroid() {
    //throw away hardware textures:
    graphicstexturelist_InvalidateSDLTextures();
    
    //preserve old window size:
    int w,h;
    w = 0;h = 0;
    graphics_GetWindowDimensions(&w, &h);

    //preserve renderer:
    char renderer[512];
    const char* p = graphics_GetCurrentRendererName();
    int len = strlen(p)+1;
    if (len >= sizeof(renderer)) {len = sizeof(renderer)-1;}
    memcpy(renderer, p, len);
    renderer[sizeof(renderer)-1] = 0;

    //preserve window title:
    char title[512];
    p = graphics_GetWindowTitle();
    len = strlen(p)+1;
    if (len >= sizeof(title)) {len = sizeof(title)-1;}
    memcpy(title, p, len);
    title[sizeof(title)-1] = 0;

    //close window:
    graphics_Close(0);

    //reopen:
    char* e;
    graphics_SetMode(w, h, 1, 0, title, renderer, &e);

    //transfer textures back to hardware:
    graphicstexturelist_TransferTexturesToHW();
}
#endif

const char* graphics_GetWindowTitle() {
    if (!mainrenderer || !mainwindow) {return NULL;}
    return SDL_GetWindowTitle(mainwindow);
}

void graphics_Quit() {
    graphics_Close(0);
    if (sdlvideoinit) {
        SDL_VideoQuit();
        sdlvideoinit = 0;
    }
    SDL_Quit();
}

SDL_RendererInfo info;
#if defined(ANDROID)
static char openglstaticname[] = "opengl";
#endif
const char* graphics_GetCurrentRendererName() {
    if (!mainrenderer) {return NULL;}
    SDL_GetRendererInfo(mainrenderer, &info);
#if defined(ANDROID)
    if (strcasecmp(info.name, "opengles") == 0) {
        //we return "opengl" here aswell, since we want "opengl" to represent
        //the best opengl renderer consistently across all platforms, which is
        //in fact opengles on Android (and opengl for normal desktop platforms).
        return openglstaticname;
    }
#endif
    return info.name;
}

int* videomodesx = NULL;
int* videomodesy = NULL;

static void graphics_ReadVideoModes() {
    //free old video mode data
    if (videomodesx) {
        free(videomodesx);
        videomodesx = 0;
    }
    if (videomodesy) {
        free(videomodesy);
        videomodesy = 0;
    }

    //allocate space for video modes
    int d = SDL_GetNumVideoDisplays();
    if (d < 1) {return;}
    int c = SDL_GetNumDisplayModes(0);
    int i = 0;
    videomodesx = malloc((c+1)*sizeof(int));
    videomodesy = malloc((c+1)*sizeof(int));
    if (!videomodesx || !videomodesy) {
        if (videomodesx) {free(videomodesx);}
        if (videomodesy) {free(videomodesy);}
        return;
    }
    memset(videomodesx, 0, (c+1) * sizeof(int));
    memset(videomodesy, 0, (c+1) * sizeof(int));

    //read video modes
    int lastusedindex = -1;
    while (i < c) {
        SDL_DisplayMode m;
        if (SDL_GetDisplayMode(0, i, &m) == 0) {
            if (m.w > 0 && m.h > 0) {
                //first, check for duplicates
                int isduplicate = 0;
                int j = 0;
                while (j <= lastusedindex && videomodesx[j] > 0 && videomodesy[j] > 0) {
                    if (videomodesx[j] == m.w && videomodesy[j] == m.h) {
                        isduplicate = 1;
                        break;
                    }
                    j++;    
                }
                if (isduplicate) {i++;continue;}

                //add mode
                lastusedindex++;
                videomodesx[lastusedindex] = m.w;
                videomodesy[lastusedindex] = m.h;
            }
        }
        i++;
    }
}

int graphics_GetNumberOfVideoModes() {
    char* error;
    if (!graphics_InitVideoSubsystem(&error)) {
        printwarning("Failed to initialise video subsystem: %s", error);
        if (error) {free(error);}
        return 0;
    }
    graphics_ReadVideoModes();
    int i = 0;
    while (videomodesx && videomodesx[i] > 0 && videomodesy && videomodesy[i] > 0) {
        i++;
    }
    return i;
}

void graphics_GetVideoMode(int index, int* x, int* y) {
    graphics_ReadVideoModes();
    *x = videomodesx[index];
    *y = videomodesy[index];
}

void graphics_GetDesktopVideoMode(int* x, int* y) {
    char* error;
    *x = 0;
    *y = 0;
    if (!graphics_InitVideoSubsystem(&error)) {
        printwarning("Failed to initialise video subsystem: %s", error);
        if (error) {free(error);}
        return;
    }
    SDL_DisplayMode m;
    if (SDL_GetDesktopDisplayMode(0, &m) == 0) {
        *x = m.w;
        *y = m.h;
    }else{
        printwarning("Unable to determine desktop video mode: %s", SDL_GetError());
    }
}

void graphics_MinimizeWindow() {
    if (!mainwindow) {return;}
    SDL_MinimizeWindow(mainwindow);
}

int graphics_IsFullscreen() {
    if (mainwindow) {
        return mainwindowfullscreen;
    }
    return 0;
}

void graphics_ToggleFullscreen() {
    if (!mainwindow) {return;}
    SDL_bool wantfull = SDL_TRUE;
    if (mainwindowfullscreen) {
        wantfull = SDL_FALSE;
    }
    if (SDL_SetWindowFullscreen(mainwindow, wantfull) == 0) {
        if (wantfull == SDL_TRUE) {
            mainwindowfullscreen = 1;
        }else{
            mainwindowfullscreen = 0;
        }
    }
}

#ifdef WINDOWS
HWND graphics_GetWindowHWND() {
    if (!mainwindow) {return NULL;}
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(mainwindow, &info)) {
        if (info.subsystem == SDL_SYSWM_WINDOWS) {
            return info.info.win.window;
        }
    }
    return NULL;
}
#endif

int graphics_SetMode(int width, int height, int fullscreen, int resizable, const char* title, const char* renderer, char** error) {

#if defined(ANDROID)
    if (!fullscreen) {
        //do not use windowed on Android
        *error = strdup("Windowed mode is not supported on Android");
        return 0;
    }
#endif

    char errormsg[512];

    //initialize SDL video if not done yet
    if (!graphics_InitVideoSubsystem(error)) {
        return 0;
    }

    //think about the renderer we want
#ifndef WINDOWS
#ifdef ANDROID
    char preferredrenderer[20] = "opengles";
#else
    char preferredrenderer[20] = "opengl";
#endif
#else
    char preferredrenderer[20] = "direct3d";
#endif
    int softwarerendering = 0;
    if (renderer) {
        if (strcasecmp(renderer, "software") == 0) {
#ifdef ANDROID
            //we don't want software rendering on Android
#else
            softwarerendering = 1;
            strcpy(preferredrenderer, "software");
#endif
        }else{
            if (strcasecmp(renderer, "opengl") == 0) {
#ifdef ANDROID
                //opengles is the opengl we want for android :-)
                strcpy(preferredrenderer, "opengles");
#else
                //regular opengl on desktop platforms
                strcpy(preferredrenderer, "opengl");
#endif
            }
#ifdef WINDOWS
            //only windows knows direct3d obviously
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
        if (count > 0) {
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
    }

    //see if anything changes at all
    unsigned int oldw = 0;
    unsigned int oldh = 0;
    graphics_GetWindowDimensions(&oldw,&oldh);
    if (mainwindow && mainrenderer && width == (int)oldw && height == (int)oldh) {
        SDL_RendererInfo info;
        SDL_GetRendererInfo(mainrenderer, &info);
        if (strcasecmp(preferredrenderer, info.name) == 0) {
            //same renderer and resolution
            if (strcmp(SDL_GetWindowTitle(mainwindow), title) != 0) {
                SDL_SetWindowTitle(mainwindow, title);
            }
            //toggle fullscreen if desired
            if (graphics_IsFullscreen() != fullscreen) {
                graphics_ToggleFullscreen();
            }
            return 1;
        }
    }
    
    //Check if we support the video mode for fullscreen -
    //  This is done to avoid SDL allowing impossible modes and
    //  giving us a fake resized/padded/whatever output we don't want.
    if (fullscreen) {
        //check all video modes in the list SDL returns for us
        int count = graphics_GetNumberOfVideoModes();
        int i = 0;
        int supportedmode = 0;
        while (i < count) {
            int w,h;
            graphics_GetVideoMode(i, &w, &h);
            if (w == width && h == height) {
                supportedmode = 1;
                break;
            }
            i++;
        }
        if (!supportedmode) {
            //check for desktop video mode aswell
            int w,h;
            graphics_GetDesktopVideoMode(&w,&h);
            if (w == 0 || h == 0 || width != w || height != h) {
                *error = strdup("Video mode is not supported");
                return 0;
            }
        }
    }
    
    //preserve textures by managing them on our own for now
    graphics_TransferTexturesFromSDL();
    
    //destroy old window/renderer if we got one
    graphics_Close(1);
    
    //create window
    if (fullscreen) {
        mainwindow = SDL_CreateWindow(title, 0,0, width, height, SDL_WINDOW_FULLSCREEN);
        mainwindowfullscreen = 1;
    }else{
        mainwindow = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, width, height, 0);
        mainwindowfullscreen = 0;
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
        if (!mainrenderer) {
            softwarerendering = 1;
            strcpy(preferredrenderer, "software");
        }
    }
    if (softwarerendering) {
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

void graphics_StartFrame() {
    SDL_SetRenderDrawColor(mainrenderer, 0, 0, 0, 1);
    SDL_RenderClear(mainrenderer);
}

void graphics_CompleteFrame() {
    SDL_RenderPresent(mainrenderer);
}

int lastfingerdownx,lastfingerdowny;

int inbackground = 0;
void graphics_CheckEvents(void (*quitevent)(void), void (*mousebuttonevent)(int button, int release, int x, int y), void (*mousemoveevent)(int x, int y), void (*keyboardevent)(const char* button, int release), void (*textevent)(const char* text), void (*putinbackground)(int background)) {
    SDL_Event e;
    while (SDL_PollEvent(&e) == 1) {
        if (e.type == SDL_QUIT) {
            quitevent();
        }
        if (e.type == SDL_MOUSEMOTION) {
            mousemoveevent(e.motion.x, e.motion.y);
        }
#ifdef ANDROID
        if (e.type == SDL_WINDOWEVENT && (e.window.event == SDL_WINDOWEVENT_MINIMIZED || e.window.event == SDL_WINDOWEVENT_FOCUS_LOST)) {
            //app was put into background
            if (!inbackground) {
                putinbackground(1);
                inbackground = 1;
            }
        }
        if (e.type == SDL_WINDOWEVENT && (e.window.event == SDL_WINDOWEVENT_RESTORED || e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)) {
            //app is pulled back into foreground
            if (inbackground) {
                putinbackground(0);
                inbackground = 0;
            }
        }
#endif
        if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
            int release = 0;
            if (e.type == SDL_MOUSEBUTTONUP) {release = 1;}
            int button = e.button.button;
            mousebuttonevent(button, release, e.button.x, e.button.y);
        }
        if (e.type == SDL_FINGERDOWN || e.type == SDL_FINGERUP) {
            int release = 0;
            int x,y;
            if (e.type == SDL_FINGERUP) {
                //take fingerdown coordinates on fingerup
                x = lastfingerdownx;
                y = lastfingerdowny;
                release = 1;
            }else{
                //remember coordinates on fingerdown
                x = e.tfinger.x;
                y = e.tfinger.y;
                lastfingerdownx = x;
                lastfingerdowny = y;
            }
            //swap coordinates for landscape mode
            int temp = x;
            x = y;
            y = temp;
            int button = SDL_BUTTON_LEFT;
            mousebuttonevent(button, release, x, y);
        }
        if (e.type == SDL_TEXTINPUT) {
            textevent(e.text.text);
        }
        if (e.type == SDL_WINDOWEVENT) {
            if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
#ifndef WINDOWS
#ifdef LINUX
                //if we are a fullscreen window, ensure we are fullscreened
                //FIXME: just a workaround for http://bugzilla.libsdl.org/show_bug.cgi?id=1349
                if (mainwindowfullscreen) {
                    SDL_SetWindowFullscreen(mainwindow, SDL_FALSE);
                    SDL_SetWindowFullscreen(mainwindow, SDL_TRUE);
                }
#endif
#endif
            }
        }
        if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
            int release = 0;
            if (e.type == SDL_KEYUP) {release = 1;}
            char keybuf[30] = "";
            if (e.key.keysym.sym >= SDLK_F1 && e.key.keysym.sym <= SDLK_F12) {
                sprintf(keybuf, "f%d", (e.key.keysym.sym+1)-SDLK_F1);
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

#endif //ifdef USE_SDL_GRAPHICS

