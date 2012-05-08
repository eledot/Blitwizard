
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

#ifdef USE_GRAPHICS

// various standard headers
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#ifdef WINDOWS
#include <windows.h>
#endif
#include <stdarg.h>

#include "os.h"
#include "logging.h"
#ifdef USE_SDL_GRAPHICS
#include "SDL.h"
#include "SDL_syswm.h"
#endif
#include "graphicstexture.h"
#include "graphics.h"
#include "graphicstexturelist.h"
#include "imgloader.h"
#include "timefuncs.h"
#include "hash.h"
#include "file.h"
#ifdef NOTHREADEDSDLRW
#include "main.h"
#endif

static struct graphicstexture* texlist = NULL;
hashmap* texhashmap = NULL;

void graphicstexturelist_InitializeHashmap() {
    if (texhashmap) {return;}
    texhashmap = hashmap_New(1024 * 1024);
}

void graphicstexturelist_AddTextureToList(struct graphicstexture* gt) {
    gt->next = texlist;
    texlist = gt;
}

void graphicstexturelist_RemoveTextureFromList(struct graphicstexture* gt, struct graphicstexture* prev) {
    if (prev) {
        prev->next = gt->next;
    }else{
        texlist = gt->next;
    }
}

struct graphicstexture* graphicstexturelist_GetTextureByName(const char* name) {
    graphicstexturelist_InitializeHashmap();
    uint32_t i = hashmap_GetIndex(texhashmap, name, strlen(name), 1);
    struct graphicstexture* gt = (struct graphicstexture*)(texhashmap->items[i]);
    while (gt && !(strcasecmp(gt->name, name) == 0)) {
        gt = gt->hashbucketnext;
    }
    return gt;
}

void graphicstexturelist_AddTextureToHashmap(struct graphicstexture* gt) {
    graphicstexturelist_InitializeHashmap();
    uint32_t i = hashmap_GetIndex(texhashmap, gt->name, strlen(gt->name), 1);
    gt->hashbucketnext = (struct graphicstexture*)(texhashmap->items[i]);
    texhashmap->items[i] = gt;
}

void graphicstexturelist_RemoveTextureFromHashmap(struct graphicstexture* gt) {
    graphicstexturelist_InitializeHashmap();
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

void graphicstexturelist_TransferTexturesFromHW() {
    struct graphicstexture* gt = texlist;
    while (gt) {
        graphics_TextureFromHW(gt);
        gt = gt->next;
    }
}

void graphicstexturelist_InvalidateHWTextures() {
    struct graphicstexture* gt = texlist;
    while (gt) {
        graphics_DestroyHWTexture(gt);
        gt = gt->next;
    }
}

int graphicstexturelist_TransferTexturesToHW() {
    struct graphicstexture* gt = texlist;
    while (gt) {
        if (!graphics_TextureToHW(gt)) {
            return 0;
        }
        gt = gt->next;
    }
    return 1;
}


struct graphicstexture* graphicstexturelist_GetPreviousTexture(struct graphicstexture* gt) {
    struct graphicstexture* gtprev = texlist;
    while (gtprev && !(gtprev->next == gt)) {
        gtprev = gtprev->next;
    }
    return gtprev;
}

int graphicstexturelist_FreeAllTextures() {
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

void graphicstexturelist_DoForAllTextures(int (*callback)(struct graphicstexture* texture, struct graphicstexture* previoustexture, void* userdata), void* userdata) {
    struct graphicstexture* gt = texlist;
    struct graphicstexture* gtprev = NULL;
    while (gt) {
        struct graphicstexture* gtnext = gt->next;
        if (callback(gt, gtprev, userdata)) {
            //entry is still valid (callback return 1), remember it as prev
            gtprev = gt;
        }
        gt = gtnext;
    }
}

#endif //ifdef USE_GRAPHICS
