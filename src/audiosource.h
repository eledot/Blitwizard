
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

struct audiosource {
	//Read data:
	int (*read)(struct audiosource* source, unsigned int bytes, char* buffer);
	//Writes 0 to bytes to the buffer pointer. Returns the amount of bytes written.
	//When -1 is returned, an error occured (the buffer is left untouched).
	//When 0 is returned, the end of the source is reached.
	//In case of 0/-1, you should close the audio resource.
	
	//Close audio source:
	void (*close)(struct audiosource* source);
	//Closes the audio source and frees this struct and all data.
	
	//Audio sample rate:
	unsigned int samplerate;
	//This is set to the audio sample rate. Format is always S16 stereo int if not
	//specified.
	//In case sample rate is set to 0, this is not a raw audio source. It might
	//contain e.g. ogg-encoded data!
	
	void* internaldata; //DON'T TOUCH, used for internal purposes.
};
