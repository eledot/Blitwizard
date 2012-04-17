
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


extern int s16mixmode; // 1: output s16 samples, 0: output float32 samples (default)
void* audiomixer_GetBuffer(unsigned int len);
void audiomixer_Init();

int audiomixer_PlaySoundFromDisk(const char* path, int priority, float volume, float panning, float fadeinseconds, int loop);
void audiomixer_StopSound(int id);
void audiomixer_AdjustSound(int id, float volume, float panning);
int audiomixer_IsSoundPlaying(int id);
int audiomixer_NoSoundsPlaying();


