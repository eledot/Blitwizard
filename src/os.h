
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

#ifndef _HAVE_BLITWIZARD_OS_H

#define _HAVE_BLITWIZARD_OS_H

// Detect operating system:

#if defined(__CYGWIN__)
#error "You should compile blitwizard natively for Windows, not using Cygwin."
#endif

#if defined(__ANDROID__) && !defined(ANDROID)
#define ANDROID
#endif

#if defined(linux__) || defined(__linux) || defined(__linux__) || defined(linux)
#define LINUX
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define MAC
#endif

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(_MSC_VER)
#define WINDOWS
#define _WIN32_WINNT=0x0501
#if defined __MINGW_H
#define _WIN32_IE 0x0400
#endif
#endif

// Set SDLRW for Android:

#ifdef ANDROID
#define SDLRW
#define NOTHREADEDSDLRW
#endif

#endif


