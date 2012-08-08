
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

#ifdef __cplusplus
extern "C" {
#endif

int graphics_AreGraphicsRunning();
// Returns 1 if the graphics are open/active, otherwise 0.

int graphics_SetMode(int width, int height, int fullscreen, int resizable, const char* title, const char* renderer, char** error);
// Set or change graphics mode.
// This can possibly cause all textures to become unloaded and reloaded,
// so is a possibly very slow operation.
// Renderer can be NULL (for any), "software", "opengl" or "direct3d".
// If you want the best for your system, go for NULL.

const char* graphics_GetCurrentRendererName();
// Get the renderer currently used for the active graphics mode.
// Returns NULL when no mode has been set.

int graphics_Init(char** error);

#ifdef WINDOWS
HWND graphics_GetWindowHWND(); // get win32 HWND handle for the window
#endif

int graphics_PromptTextureLoading(const char* texture);
// Prompt texture loading.
// Returns 0 on fatal error (e.g. out of memory), 1 for operation in progress,
// 2 for image already loaded.

int graphics_LoadTextureInstantly(const char* texture);
// Prompts texture loading and blocks until the texture is loaded or cancelled.
// Returns 0 on any error (both fatal out of memory or image loading failure),
// 1 when the image has been loaded successfully.

void graphics_CheckTextureLoading(void (*callback)(int success, const char* texture));
// Check texture loading state and get the newest callbacks.
// For the callback, success will be 1 for images loaded successfully,
// otherwise 0. texture will contain the texure's name.

int graphics_GetNumberOfVideoModes();
// Get the number of supported video modes (= the modes usable in fullscreen)

void graphics_GetVideoMode(int index, int* width, int* height);
// Get the video mode at the given index (0..graphics_GetNumberOfVideoMode()-1)

void graphics_GetDesktopVideoMode(int* x, int* y);
// Get the current video mode of the desktop

void graphics_Close(int preservetextures);
// Close the graphics. preservetextures 1: keep them available for use,
// 0: dispose of them

void graphics_Quit();
// Quit the graphics completely

int graphics_IsFullscreen();
// Return if the graphics are currently running at full screen.
// 1: yes, 0: no. Undefined result when no graphics mode set

void graphics_MinimizeWindow();
// Minimize the window

void graphicsrender_StartFrame();
// Clears the screen to prepare for the next frame.

int graphicsrender_DrawCropped(const char* texname, int x, int y, float alpha, unsigned int sourcex, unsigned int sourcey, unsigned int sourcewidth, unsigned int sourceheight, unsigned int drawwidth, unsigned int drawheight, int rotationcenterx, int rotationcentery, double rotationangle, int horiflipped, double red, double green, double blue);
// Draw a texture cropped. Returns 1 on success, 0 when there is no such texture.

int graphicsrender_Draw(const char* texname, int x, int y, float alpha, unsigned int drawwidth, unsigned int drawheight, int rotationcenterx, int rotationcentery, double rotationangle, int horiflipped, double red, double green, double blue);
// Draw a texture. Returns 1 on success, 0 when there is no such texture.

void graphicsrender_DrawRectangle(int x, int y, int width, int height, float r, float g, float b, float a);
// Draw a colored rectangle.

void graphicsrender_CompleteFrame();
// Update the current drawing changes to screen.
// Use this always after completing one frame.

void graphics_UnloadTexture(const char* texname, void (*callback)(int success, const char* texture));
// Unload the given texture if loaded currently.
// If the texture is currently being loaded, loading will be cancelled and,
// if the provided callback is not NULL, the callback will be called.

int graphics_IsTextureLoaded(const char* name);
// Check if a texture is loaded. 0: no, 1: operation in progress, 2: yes

int graphics_GetTextureDimensions(const char* name, unsigned int* width, unsigned int* height);
// 1 on success, 0 on error

int graphics_GetWindowDimensions(unsigned int* width, unsigned int* height);
// 1 on success, 0 on error (window not opened most likely)

const char* graphics_GetWindowTitle();
// Return the current title of the window

void graphics_CheckEvents(void (*quitevent)(void), void (*mousebuttonevent)(int button, int release, int x, int y), void (*mousemoveevent)(int x, int y), void (*keyboardevent)(const char* button, int release), void (*textevent)(const char* text), void (*putinbackground)(int background));
// Check for events and return info about them through the provided callbacks

void graphics_TransferTexturesFromHW();
// Transfer textures from SDL, e.g. if app is in background

int graphics_TransferTexturesToHW();
// Transfer textures back to SDL

#ifdef ANDROID
void graphics_ReopenForAndroid();
// Reopen graphics and reupload textures. Required when coming back into foreground
#endif

void graphics_TextureFromHW(struct graphicstexture* gt);
// Push texture onto 3d accelerated hardware

int graphics_TextureToHW(struct graphicstexture* gt);
// Pull texture from 3d accelerated hardware

void graphics_DestroyHWTexture(struct graphicstexture* gt);
// Destroy the 3d texture (e.g. in preparation for free'ing the texture)

int graphics_FreeTexture(struct graphicstexture* gt, struct graphicstexture* prev);
// Free a texture

int graphics_HaveValidWindow();
// Returns 1 if a window is open, otherwise 0

#ifdef __cplusplus
}
#endif

#else // ifdef USE_GRAPHICS

#define compiled_without_graphics "No graphics available - this binary was compiled with graphics (including null device) disabled"

#endif // ifdef USE_GRAPHICS

