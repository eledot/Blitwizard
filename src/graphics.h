
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

int graphics_SetMode(int width, int height, int fullscreen, int resizable, const char* renderer, char** error);
//Set or change graphics mode.
//This can possibly cause all textures to become unloaded and reloaded,
//so is a possibly very slow operation.
//Renderer can be NULL (for any), "software", "opengl" or "direct3d".
//If you want the best for your system, go for NULL.

int graphics_Init(char** error);

int graphics_PromptTextureLoading(const char* texture, void* callbackptr);
//Prompt texture loading and sign up with callback info.
//Returns 0 on fatal error (e.g. out of memory), 1 for operation in progress,
//2 for image already loaded.
//Only for operation in progress, you'll get a callback later with graphics_CheckTextureLoading().
//All other return values mean there is no later callback happening.

void graphics_CheckTextureLoading(void (*callbackinfo)(int success, const char* texture, void* callbackptr));
//Check texture loading state and get the newest callbacks

void graphics_Quit();
//Quit the graphics subsystem

int graphics_DrawCropped(const char* texname, int x, int y, float alpha, unsigned int sourcex, unsigned int sourcey, unsigned int sourcewidth, unsigned int sourceheight);
//Draw a texture cropped. Returns 1 on success, 0 when there is no such texture.

int graphics_Draw(const char* texname, int x, int y, float alpha);
//Draw a texture. Returns 1 on success, 0 when there is no such texture.

void graphics_UnloadTexture(const char* texname);
//Unload the given texture if loaded currently. Does nothing if texture is currently being loaded!

