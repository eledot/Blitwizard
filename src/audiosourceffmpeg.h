
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

int audiosourceffmpeg_LoadFFmpeg(void);
// Returns 1 when FFmpeg is available, otherwise 0

void audiosourceffmpeg_DisableFFmpeg(void);
// Call this before audiosourceffmpeg_LoadFFmpeg() is ever called, and FFmpeg
// won't be loaded and supported (audiosourceffmpeg_LoadFFmpeg() will
// return 0).

struct audiosource* audiosourceffmpeg_Create(struct audiosource* filesource);
// Take an audio source that returns encoded binary data (usually
// audiosourcefile) and attempt to decode the data as any audio format
// which FFmpeg understands.
// Will attempt to load FFmpeg at runtime.
// If the file isn't understood by FFmpeg or FFmpeg failed to load,
// this creation function will simply return NULL.
