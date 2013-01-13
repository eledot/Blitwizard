
/* blitwizard game engine - source code file

  Copyright (C) 2011-2013 Jonas Thiem

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

#ifdef USE_GRAPHICS

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
#endif

#include "graphicstexture.h"
#include "graphics.h"
#include "graphicstexturelist.h"

int graphicsactive = 0;

int graphics_AreGraphicsRunning() {
    return graphicsactive;
}

int graphicsrender_Draw(const char* texname, int x, int y, float alpha, unsigned int drawwidth, unsigned int drawheight, int rotationcenterx, int rotationcentery, double rotationangle, int horiflipped, double red, double green, double blue) {
    return graphicsrender_DrawCropped(texname, x, y, alpha, 0, 0, 0, 0, drawwidth, drawheight, rotationcenterx, rotationcentery, rotationangle, horiflipped, red, green, blue);
}


int graphics_GetTextureDimensions(const char* name, unsigned int* width, unsigned int* height) {
    struct graphicstexture* gt = graphicstexturelist_GetTextureByName(name);
    if (!gt || gt->threadingptr) {
        return 0;
    }

    *width = gt->width;
    *height = gt->height;
    return 1;
}

#ifdef SDLRW
static int graphics_AndroidTextureReader(void* buffer, size_t bytes, void* userdata) {
    SDL_RWops* ops = (SDL_RWops*)userdata;
#ifndef NOTHREADEDSDLRW
    int i = ops->read(ops, buffer, 1, bytes);
#else
    // workaround for http:// bugzilla.libsdl.org/show_bug.cgi?id=1422
    int i = main_NoThreadedRWopsRead(ops, buffer, 1, bytes);
#endif
    return i;
}
#endif

int graphics_PromptTextureLoading(const char* texture) {
    // check if texture is already present or being loaded
    struct graphicstexture* gt = graphicstexturelist_GetTextureByName(texture);
    if (gt) {
        // check for threaded loading
        if (gt->threadingptr) {
            // it will be loaded
            return 1;
        }
        return 2; // texture is already present
    }

    // allocate new texture info
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

    // trigger image fetching thread
#ifdef SDLRW
    gt->rwops = SDL_RWFromFile(gt->name, "rb");
    if (!gt->rwops) {
        free(gt->name);
        free(gt);
        return 0;
    }
    gt->threadingptr = img_LoadImageThreadedFromFunction(&graphics_AndroidTextureReader, gt->rwops, 0, 0, "rgba", NULL);
#else
    char* p = file_GetAbsolutePathFromRelativePath(gt->name);
    if (!p) {
        free(gt->name);
        free(gt);
        return 0;
    }
    gt->threadingptr = img_LoadImageThreadedFromFile(p, 0, 0, "rgba", NULL);
    free(p);
#endif
    if (!gt->threadingptr) {
        free(gt->name);
        free(gt);
#ifdef SDLRW
        gt->rwops->close(gt->rwops);
#endif
        return 0;
    }

    // add us to the list
    graphicstexturelist_AddTextureToList(gt);
    graphicstexturelist_AddTextureToHashmap(gt);
    return 1;
}

int graphics_FreeTexture(struct graphicstexture* gt, struct graphicstexture* prev) {
    if (gt->pixels) {
        free(gt->pixels);
        gt->pixels = NULL;
    }
    graphics_DestroyHWTexture(gt);
    if (gt->name) {
        graphicstexturelist_RemoveTextureFromHashmap(gt);
        free(gt->name);
        gt->name = NULL;
    }
    if (gt->threadingptr) {
        return 0;
    }
#ifdef SDLRW
    if (gt->rwops) {
        gt->rwops->close(gt->rwops);
        gt->rwops = NULL;
    }
#endif
    graphicstexturelist_RemoveTextureFromList(gt, prev);
    free(gt);
    return 1;
}


int graphics_FinishImageLoading(struct graphicstexture* gt, struct graphicstexture* gtprev, void (*callback)(int success, const char* texture)) {
    char* data;int width,height;
    img_GetData(gt->threadingptr, NULL, &width, &height, &data);
    img_FreeHandle(gt->threadingptr);
    gt->threadingptr = NULL;

    // check if we succeeded
    int success = 0;
    if (data) {
        gt->pixels = data;
        gt->width = width;
        gt->height = height;
        success = 1;
        if (graphics_AreGraphicsRunning() && gt->name && graphics_HaveValidWindow()) {
            if (!graphics_TextureToHW(gt)) {
                success = 0;
            }
        }
    }

    // do callback
    if (callback) {
        callback(success, gt->name);
    }

    // if this is an empty abandoned or a failed entry, remove
    if (!gt->name || !success) {
         graphics_FreeTexture(gt, gtprev);
         return 0;
    }
    return 1;
}


int graphics_LoadTextureInstantly(const char* texture) {
    // prompt normal async texture loading
    if (!graphics_PromptTextureLoading(texture)) {
        return 0;
    }
    struct graphicstexture* gt = graphicstexturelist_GetTextureByName(texture);
    if (!gt) {
        return 0;
    }

    // wait for loading to finish
    while (!img_CheckSuccess(gt->threadingptr)) {
    }

    // complete image
    return graphics_FinishImageLoading(gt, graphicstexturelist_GetPreviousTexture(gt), NULL);
}

void graphics_UnloadTexture(const char* texname, void (*callback)(int success, const char* texture)) {
    struct graphicstexture* gt = graphicstexturelist_GetTextureByName(texname);
    if (gt) {
        if (!gt->threadingptr) {
            // regularly unload
            struct graphicstexture* gtprev = graphicstexturelist_GetPreviousTexture(gt);
            graphics_FreeTexture(gt, gtprev);
        }else{
            // prompt the image loading callback with an error:
            if (callback) {
                callback(0, gt->name);
            }

            // cancel loading by abandoning the texture
            free(gt->name);
            gt->name = NULL;

            // it will be fully thrown away as soon as the
            // threaded image loading was completed
        }
    }
}

static int graphics_CheckTextureLoadingCallback(struct graphicstexture* gt, struct graphicstexture* gtprev, void* userdata) {
    void (*callback)(int success, const char* texture) = userdata;
    if (gt->threadingptr) {
        // texture which is currently being loaded
        if (img_CheckSuccess(gt->threadingptr)) {
            if (!graphics_FinishImageLoading(gt,gtprev,callback)) {
                // keep old valid gtprev, this entry is deleted now
                return 0;
            }
        }
    }else{
        // texture is a regular loaded texture. check if it was abandoned:
        if (!gt->name) {
            // delete abandoned textures
            graphics_FreeTexture(gt, gtprev);

            // keep old valid gtprev
            return 0;
        }
    }
    return 1;
}

void graphics_CheckTextureLoading(void (*callback)(int success, const char* texture)) {
    graphicstexturelist_DoForAllTextures(&graphics_CheckTextureLoadingCallback, callback);
}


int graphics_IsTextureLoaded(const char* name) {
    // check texture state
    struct graphicstexture* gt = graphicstexturelist_GetTextureByName(name);
    if (gt) {
        // check for threaded loading
        if (gt->threadingptr) {
            // it will be loaded.
            return 1;
        }
        return 2; // texture is already present
    }
    return 0; // not loaded
}

#endif // ifdef USE_GRAPHICS

