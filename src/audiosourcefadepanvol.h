
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

// Get a fadepanvol audio source:
struct audiosource* audiosourcefadepanvol_Create(struct audiosource* source);

// Change settings on the fadepanvol audio source:
void audiosourcefadepanvol_SetPanVol(
struct audiosource* source,
float volume, float pan, int noamplify);
// volume: volume at which sound plays from 0 to 1.5. 1 is regular volume
// pan: panning from 1 (left) to -1 (right). 0 is center.
// noamplify: if set to 1, panning and volume > 1 is disabled,
//  but the audio is of higher quality especially if loud.
//  For regular panning support, specify 0.

// Start a fade to a given volume level:
void audiosourcefadepanvol_StartFade(struct audiosource* source, float seconds, float targetvol, int terminate);



