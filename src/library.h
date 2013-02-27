
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

void* library_Load(const char* name);
// Load dynamic library at runtime.
//  Returns library pointer or NULL in case of failure.
//  Platform dependent library extension will be appended
//  (.dll for Windows, .so for Linux, .dylib for Mac OS X)

void* library_LoadSearch(const char* name);
// Same as library_Load, but on Linux/BSD, this will do a more
// intensive search also in /usr/lib and other places
// for .so files with versions appended, e.g. .so.5.4 or
// something like that.
// Behaves exactly like library_Load() for other systems.
// Use this when you are desparate to get the lib :-)

void* library_GetSymbol(void* ptr, const char* name);
// Get symbol from dynamically loaded library
//  Returns symbol pointer or NULL in case of failure.

void library_Close(void* ptr);
// Close dynamically loaded library again

