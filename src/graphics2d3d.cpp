
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

extern "C" {

#if defined(USE_SDL_GRAPHICS) || defined(USE_OGRE_GRAPHICS)

//  various standard headers
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

#ifdef USE_SDL_GRAPHICS
#include "SDL.h"
#include "SDL_syswm.h"
#endif
#ifdef USE_OGRE_GRAPHICS
extern "C++" {
#include <OgreWindowEventUtilities.h>
#include <OgreMaterialManager.h>
#include <OgreMeshSerializer.h>
#include <OgreMeshManager.h>
#include "OIS.h"
}
#endif

#include "graphicstexture.h"
#include "graphics.h"
#include "graphicstexturelist.h"


#ifdef USE_SDL_GRAPHICS
SDL_Window* mainwindow = NULL;
SDL_Renderer* mainrenderer = NULL;
int sdlvideoinit = 0;
#endif
#ifdef USE_OGRE_GRAPHICS
Ogre::Root* mainogreroot = NULL;
Ogre::Camera* mainogrecamera = NULL;
Ogre::SceneManager* mainogrescenemanager = NULL;
Ogre::RenderWindow* mainogrewindow = NULL;
OIS::InputManager* maininput = NULL;
OIS::Mouse* mainogremouse = NULL;
OIS::Keyboard* mainogrekeyboard = NULL;
#endif

int graphicsactive = 0;  // whether graphics are active/opened (1) or not (0)
int inbackground = 0;  // whether program has focus (1) or not (0)
int graphics3d;  // 1: Ogre graphics, 0: SDL graphics
int mainwindowfullscreen; // whether fullscreen (1) or not (0)

int graphics_HaveValidWindow() {
    if (mainwindow) {
        return 1;
    }
    return 0;
}

#ifdef USE_OGRE_GRAPHICS
// event receiver to be used by Ogre:
extern "C++" {
class EventReceiver : public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener {
public:
    EventReceiver(void) {
    }
    ~EventReceiver(void) {
    }
};
}
static EventReceiver* maineventreceiver = NULL;
#endif

void graphics_DestroyHWTexture(struct graphicstexture* gt) {
#ifdef USE_SDL_GRAPHICS
    if (gt->tex.sdltex && !graphics3d) {
        SDL_DestroyTexture(gt->tex.sdltex);
        gt->tex.sdltex = NULL;
    }
#endif
#ifdef USE_OGRE_GRAPHICS

#endif
}

static int graphics_InitVideoSubsystem(char** error) {
#ifdef USE_SDL_GRAPHICS
    char errormsg[512];
    // initialize SDL video if not done yet
    if (!sdlvideoinit) {
        if (SDL_VideoInit(NULL) < 0) {
            snprintf(errormsg,sizeof(errormsg),"Failed to initialize SDL video: %s", SDL_GetError());
            errormsg[sizeof(errormsg)-1] = 0;
            *error = strdup(errormsg);
            return 0;
        }
        sdlvideoinit = 1;
    }
#endif
    return 1;
}

int graphics_Init(char** error) {
    char errormsg[512];

#ifdef USE_SDL_GRAPHICS
    // set scaling settings
    SDL_SetHintWithPriority(SDL_HINT_RENDER_SCALE_QUALITY, "2", SDL_HINT_NORMAL);

    // initialize SDL
    if (SDL_Init(SDL_INIT_TIMER) < 0) {
        snprintf(errormsg,sizeof(errormsg),"Failed to initialize SDL: %s", SDL_GetError());
        errormsg[sizeof(errormsg)-1] = 0;
        *error = strdup(errormsg);
        return 0;
    }
#endif
    return 1;
}

int graphics_TextureToHW(struct graphicstexture* gt) {
#ifdef USE_SDL_GRAPHICS
    if (!graphics3d) {
        if (gt->tex.sdltex || gt->threadingptr || !gt->name) {
            return 1;
        }

        // create texture
        SDL_Texture* t = SDL_CreateTexture(mainrenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, gt->width, gt->height);
        if (!t) {
            printwarning("Warning: SDL failed to create texture: %s\n", SDL_GetError());
            return 0;
        }

        // lock texture
        void* pixels; int pitch;
        if (SDL_LockTexture(t, NULL, &pixels, &pitch) != 0) {
            printwarning("Warning: SDL failed to lock texture: %s\n", SDL_GetError());
            SDL_DestroyTexture(t);
            return 0;
        }

        // copy pixels into texture
        memcpy(pixels, gt->pixels, gt->width * gt->height * 4);

        // unlock texture
        SDL_UnlockTexture(t);

        // set blend mode
        if (SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND) < 0) {
            printf("Warning: Blend mode SDL_BLENDMODE_BLEND not applied: %s\n",SDL_GetError());
        }

        // if on the desktop, discard texture from regular memory
#if !defined(ANDROID)
        free(gt->pixels);
        gt->pixels = NULL;
#endif

        gt->tex.sdltex = t;
    }
#endif
#ifdef USE_OGRE_GRAPHICS
    if (graphics3d) {
        if (gt->tex.ogretex || gt->threadingptr || !gt->name) {
            return 1;
        }
    }
#endif
    return 0;
}

void graphics_TextureFromHW(struct graphicstexture* gt) {
    if (!gt->tex.sdltex || gt->threadingptr || !gt->name) {
        return;
    }

    if (!gt->pixels) {
        gt->pixels = malloc(gt->width * gt->height * 4);
        if (!gt->pixels) {
            // wipe this texture
            SDL_DestroyTexture(gt->tex.sdltex);
            gt->tex.sdltex = NULL;
            graphicstexturelist_RemoveTextureFromHashmap(gt);
            free(gt->name);
            gt->name = NULL;
            return;
        }

        // Lock SDL Texture
        void* pixels;int pitch;
        if (SDL_LockTexture(gt->tex.sdltex, NULL, &pixels, &pitch) != 0) {
            // success, the texture is now officially garbage.
            // can/should we do anything about this? (a purely visual problem)
            printf("Warning: SDL_LockTexture() failed\n");
        }else{

            // Copy texture
            memcpy(gt->pixels, pixels, gt->width * gt->height * 4);

            // unlock texture again
            SDL_UnlockTexture(gt->tex.sdltex);

        }
    }

    SDL_DestroyTexture(gt->tex.sdltex);
    gt->tex.sdltex = NULL;
}


int graphics_GetWindowDimensions(unsigned int* width, unsigned int* height) {
#ifdef USE_SDL_GRAPHICS
    if (mainwindow) {
        int w,h;
        SDL_GetWindowSize(mainwindow, &w,&h);
        if (w < 0 || h < 0) {
            return 0;
        }
        *width = w;
        *height = h;
        return 1;
    }
#endif
    return 0;
}



void graphics_Close(int preservetextures) {
    // close graphics, and destroy textures if instructed to do so
    graphicsactive = 0;
    if (mainrenderer) {
        if (preservetextures) {
            // preserve textures if not instructed to destroy them
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
    // throw away hardware textures:
    graphicstexturelist_InvalidateSDLTextures();

    // preserve old window size:
    int w,h;
    w = 0;h = 0;
    graphics_GetWindowDimensions(&w, &h);

    // preserve renderer:
    char renderer[512];
    const char* p = graphics_GetCurrentRendererName();
    int len = strlen(p)+1;
    if (len >= sizeof(renderer)) {
        len = sizeof(renderer)-1;
    }
    memcpy(renderer, p, len);
    renderer[sizeof(renderer)-1] = 0;

    // preserve window title:
    char title[512];
    p = graphics_GetWindowTitle();
    len = strlen(p)+1;
    if (len >= sizeof(title)) {
        len = sizeof(title)-1;
    }
    memcpy(title, p, len);
    title[sizeof(title)-1] = 0;

    // close window:
    graphics_Close(0);

    // reopen:
    char* e;
    graphics_SetMode(w, h, 1, 0, title, renderer, &e);

    // transfer textures back to hardware:
    graphicstexturelist_TransferTexturesToHW();
}
#endif

const char* graphics_GetWindowTitle() {
    if (!mainrenderer || !mainwindow) {
        return NULL;
    }
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
    if (!mainrenderer) {
        return NULL;
    }
    SDL_GetRendererInfo(mainrenderer, &info);
#if defined(ANDROID)
    if (strcasecmp(info.name, "opengles") == 0) {
        // we return "opengl" here aswell, since we want "opengl" to represent
        // the best opengl renderer consistently across all platforms, which is
        // in fact opengles on Android (and opengl for normal desktop platforms).
        return openglstaticname;
    }
#endif
    return info.name;
}

int* videomodesx = NULL;
int* videomodesy = NULL;

static void graphics_ReadVideoModes() {
    // free old video mode data
    if (videomodesx) {
        free(videomodesx);
        videomodesx = 0;
    }
    if (videomodesy) {
        free(videomodesy);
        videomodesy = 0;
    }

    // allocate space for video modes
    int d = SDL_GetNumVideoDisplays();
    if (d < 1) {
        return;
    }
    int c = SDL_GetNumDisplayModes(0);
    int i = 0;
    videomodesx = (int*)malloc((c+1)*sizeof(int));
    videomodesy = (int*)malloc((c+1)*sizeof(int));
    if (!videomodesx || !videomodesy) {
        if (videomodesx) {
            free(videomodesx);
        }
        if (videomodesy) {
            free(videomodesy);
        }
        return;
    }
    memset(videomodesx, 0, (c+1) * sizeof(int));
    memset(videomodesy, 0, (c+1) * sizeof(int));

    // read video modes
    int lastusedindex = -1;
    while (i < c) {
        SDL_DisplayMode m;
        if (SDL_GetDisplayMode(0, i, &m) == 0) {
            if (m.w > 0 && m.h > 0) {
                // first, check for duplicates
                int isduplicate = 0;
                int j = 0;
                while (j <= lastusedindex && videomodesx[j] > 0 && videomodesy[j] > 0) {
                    if (videomodesx[j] == m.w && videomodesy[j] == m.h) {
                        isduplicate = 1;
                        break;
                    }
                    j++;
                }
                if (isduplicate) {
                    i++;
                    continue;
                }

                // add mode
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
        if (error) {
            free(error);
        }
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
        if (error) {
            free(error);
        }
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
    if (!mainwindow) {
        return;
    }
    SDL_MinimizeWindow(mainwindow);
}

int graphics_IsFullscreen() {
    if (mainwindow) {
        return mainwindowfullscreen;
    }
    return 0;
}

void graphics_ToggleFullscreen() {
    if (!mainwindow) {
        return;
    }
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
    if (!mainwindow) {
        return NULL;
    }
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
        // do not use windowed on Android
        *error = strdup("Windowed mode is not supported on Android");
        return 0;
    }
#endif

    char errormsg[512];

    // initialize SDL video if not done yet
    if (!graphics_InitVideoSubsystem(error)) {
        return 0;
    }

    // think about the renderer we want
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
            // we don't want software rendering on Android
#else
            softwarerendering = 1;
            strcpy(preferredrenderer, "software");
#endif
        }else{
            if (strcasecmp(renderer, "opengl") == 0) {
#ifdef ANDROID
                // opengles is the opengl we want for android :-)
                strcpy(preferredrenderer, "opengles");
#else
                // regular opengl on desktop platforms
                strcpy(preferredrenderer, "opengl");
#endif
            }
#ifdef WINDOWS
            // only windows knows direct3d obviously
            if (strcasecmp(renderer,"direct3d") == 0) {
                strcpy(preferredrenderer, "direct3d");
            }
#endif
        }
    }

    // get renderer index
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

    //  see if anything changes at all
    unsigned int oldw = 0;
    unsigned int oldh = 0;
    graphics_GetWindowDimensions(&oldw,&oldh);
    if (mainwindow && mainrenderer && width == (int)oldw && height == (int)oldh) {
        SDL_RendererInfo info;
        SDL_GetRendererInfo(mainrenderer, &info);
        if (strcasecmp(preferredrenderer, info.name) == 0) {
            //  same renderer and resolution
            if (strcmp(SDL_GetWindowTitle(mainwindow), title) != 0) {
                SDL_SetWindowTitle(mainwindow, title);
            }
            //  toggle fullscreen if desired
            if (graphics_IsFullscreen() != fullscreen) {
                graphics_ToggleFullscreen();
            }
            return 1;
        }
    }

    //  Check if we support the video mode for fullscreen -
    //   This is done to avoid SDL allowing impossible modes and
    //   giving us a fake resized/padded/whatever output we don't want.
    if (fullscreen) {
        //  check all video modes in the list SDL returns for us
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
            //  check for desktop video mode aswell
            int w,h;
            graphics_GetDesktopVideoMode(&w,&h);
            if (w == 0 || h == 0 || width != w || height != h) {
                *error = strdup("Video mode is not supported");
                return 0;
            }
        }
    }

    //  preserve textures by managing them on our own for now
    graphicstexturelist_TransferTexturesFromHW();

    //  destroy old window/renderer if we got one
    graphics_Close(1);

    //  create window
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

    // Create renderer
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
        // we failed to create the renderer
        if (mainwindow) {
            // destroy window aswell in case it is open
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

    // Transfer textures back to SDL
    if (!graphicstexturelist_TransferTexturesToHW()) {
        SDL_RendererInfo info;
        SDL_GetRendererInfo(mainrenderer, &info);
        snprintf(errormsg,sizeof(errormsg),"Failed to create SDL renderer (backend %s): Cannot recreate textures", info.name);
        *error = strdup(errormsg);
        SDL_DestroyRenderer(mainrenderer);
        SDL_DestroyWindow(mainwindow);
        return 0;
    }

    // Re-focus window if previously focussed
    if (!inbackground) {
        SDL_RaiseWindow(mainwindow);
    }

    graphicsactive = 1;
    return 1;
}

int lastfingerdownx,lastfingerdowny;

void graphics_CheckEvents(void (*quitevent)(void), void (*mousebuttonevent)(int button, int release, int x, int y), void (*mousemoveevent)(int x, int y), void (*keyboardevent)(const char* button, int release), void (*textevent)(const char* text), void (*putinbackground)(int background)) {
#ifdef USE_SDL_GRAPHICS
    SDL_Event e;
    while (SDL_PollEvent(&e) == 1) {
        if (e.type == SDL_QUIT) {
            quitevent();
        }
        if (e.type == SDL_MOUSEMOTION) {
            mousemoveevent(e.motion.x, e.motion.y);
        }
        if (e.type == SDL_WINDOWEVENT && (e.window.event == SDL_WINDOWEVENT_MINIMIZED || e.window.event == SDL_WINDOWEVENT_FOCUS_LOST)) {
            // app was put into background
            if (!inbackground) {
                putinbackground(1);
                inbackground = 1;
            }
        }
        if (e.type == SDL_WINDOWEVENT && (e.window.event == SDL_WINDOWEVENT_RESTORED || e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)) {
            // app is pulled back into foreground
            if (inbackground) {
                putinbackground(0);
                inbackground = 0;
            }
        }
        if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
            int release = 0;
            if (e.type == SDL_MOUSEBUTTONUP) {
                release = 1;
            }
            int button = e.button.button;
            mousebuttonevent(button, release, e.button.x, e.button.y);
        }
        if (e.type == SDL_FINGERDOWN || e.type == SDL_FINGERUP) {
            int release = 0;
            int x,y;
            if (e.type == SDL_FINGERUP) {
                // take fingerdown coordinates on fingerup
                x = lastfingerdownx;
                y = lastfingerdowny;
                release = 1;
            }else{
                // remember coordinates on fingerdown
                x = e.tfinger.x;
                y = e.tfinger.y;
                lastfingerdownx = x;
                lastfingerdowny = y;
            }
            // swap coordinates for landscape mode
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
                // if we are a fullscreen window, ensure we are fullscreened
                // FIXME: just a workaround for http:// bugzilla.libsdl.org/show_bug.cgi?id=1349
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
            if (e.type == SDL_KEYUP) {
                release = 1;
            }
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
#endif
#ifdef USE_OGRE_GRAPHICS
    WindowEventUtilities::messagePump();
    mainmouse->capture();
    mainkeyboard->capture();    
#endif
}

#endif // if defined(USE_SDL_GRAPHICS) || defined(USE_OGRE_GRAPHICS)


