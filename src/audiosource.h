
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

#include "os.h"

#define AUDIOSOURCEFORMAT_UNKNOWN 0
#define AUDIOSOURCEFORMAT_U8 1
#define AUDIOSOURCEFORMAT_S16LE 2
#define AUDIOSOURCEFORMAT_S24LE 3
#define AUDIOSOURCEFORMAT_F32LE 4
#define AUDIOSOURCEFORMAT_S32LE 5

struct audiosource {
    // Read data:
    int (*read)(struct audiosource* source, char* buffer, unsigned int bytes);
    // Writes 0 to bytes to the buffer pointer.
    // Returns the amount of bytes written.
    // When -1 is returned, an error occured (the buffer is left untouched).
    // When 0 is returned, the end of the source is reached.
    // In case of 0/-1, you should close the audio resource.

    // Seek:
    int (*seek)(struct audiosource* source, size_t pos);
    // Seek to the given sample position.
    // Returns 0 when seeking fails, 1 when seeking succeeds.

    // Pos:
    size_t (*position)(struct audiosource* source);
    // Get the current playback position in the stream in samples.

    // Rewind:
    void (*rewind)(struct audiosource* source);
    // Rewind and start over from the beginning.
    // This can be used any time as long as read has never returned
    // an error (-1).
    // Some streams will support rewind, whereas seeking is not supported.

    // Query stream length:
    size_t (*length)(struct audiosource* source);
    // Query the total song length. Returns length in total samples
    // (it doesn't count one sample per channel but one for all, so
    // 48kHz audio will always have 48000 samples per second, no matter if
    // mono or stereo or something else)
    // If you need the length in seconds, divide this through the samplerate.
    // Returns 0 if peeking at length is not supported or length is unknown.

    // Close audio source:
    void (*close)(struct audiosource* source);
    // Closes the audio source and frees this struct and all data.

    // Audio sample rate:
    unsigned int samplerate;
    // This is set to the audio sample rate.
    // Format is always 32bit signed float stereo if not specified.
    // Set to 0 for unknown or encoded (ogg) formats

    // Channels:
    unsigned int channels;
    // This is set to the audio channels.
    // Set to 0 for unknown or encoded (ogg) formats

    // Audio sample format:
    unsigned int format;
    // This is set to the sample format.

    // Supports seeking:
    int seekable;
    // Set to 1 if seeking support is available, otherwise 0.

    void* internaldata; // DON'T TOUCH, used for internal purposes.
};

