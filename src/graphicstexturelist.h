
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

struct graphicstexture* graphicstexturelist_GetTextureByName(const char* name);

void graphicstexturelist_AddTextureToHashmap(struct graphicstexture* gt);

void graphicstexturelist_RemoveTextureFromHashmap(struct graphicstexture* gt);

void graphicstexturelist_TransferTexturesFromHW();

void graphicstexturelist_InvalidateHWTextures();

int graphicstexturelist_TransferTexturesToHW();

struct graphicstexture* graphicstexturelist_GetPreviousTexture(struct graphicstexture* gt);

void graphicstexturelist_AddTextureToList(struct graphicstexture* gt);

void graphicstexturelist_RemoveTextureFromList(struct graphicstexture* gt, struct graphicstexture* prev);

int graphicstexturelist_FreeAllTextures();

void graphicstexturelist_DoForAllTextures(int (*callback)(struct graphicstexture* texture, struct graphicstexture* previoustexture, void* userdata), void* userdata);

#endif //ifdef USE_GRAPHICS
