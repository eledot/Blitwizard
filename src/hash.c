
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

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "hash.h"

// The hash implementation is from here: http://www.isthe.com/chongo/src/fnv/hash_32.c
// It is slightly modified but not in any substantial way, so the code starting from here
// is public domain:

#define FNV_32_PRIME ((uint32_t)0x01000193)
#define FNV_32_INIT ((uint32_t)0x811c9dc5)

uint32_t fnv_32_buf(const void *buf, size_t len) {
    uint32_t hval = FNV_32_INIT;
    const unsigned char *bp = (const unsigned char *)buf;
    const unsigned char *be = bp + len;

    while (bp < be) {
        hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
        hval ^= (uint32_t)*bp++;
    }

    return hval;
}

uint32_t fnv_32_upper_buf(const void *buf, size_t len) {
    uint32_t hval = FNV_32_INIT;
    const unsigned char *bp = (const unsigned char *)buf;
    const unsigned char *be = bp + len;

    while (bp < be) {
        hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
        hval ^= (uint32_t)toupper(*bp++);
    }

    return hval;
}

// End of public domain code.

hashmap* hashmap_New(uint32_t size) {
    hashmap* hmap = malloc(sizeof(*hmap));
    if (!hmap) {
        return NULL;
    }
    hmap->items = malloc(sizeof(void*)*size);
    if (!hmap->items) {
        free(hmap);
        return NULL;
    }
    memset(hmap->items, 0, sizeof(void*)*size);
    hmap->size = size;
    return hmap;
}

uint32_t hashmap_GetIndex(hashmap* h, const char* buf, size_t len, int ignorecase) {
    uint32_t index;
    if (ignorecase) {
        index = fnv_32_upper_buf(buf, len) % h->size;
    }else{
        index = fnv_32_buf(buf, len) % h->size;
    }
    return index;
}

void hashmap_Free(hashmap* h) {
    if (h->items) {free(h->items);}
    free(h);
}

