
/* blitwizard game engine - source code file

  Copyright (C) 2013 Jonas Thiem

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

// Zip decryption api.
// This api is implemented by zip decryption variants and
// decrypts the encrypted zip resource files.
// It implements read-only decrypted access.

#ifndef BLITWIZARD_ZIPDECRYPTION_H_
#define BLITWIZARD_ZIPDECRYPTION_H_

// the zip decrypt file access functions which you pass
// to the zip decrypter. the userdata will be passed through
// to you by the zip decrypter:
struct zipdecryptionfileaccess {
    // read from the archive file:
    size_t (*read)(void* userdata, char* buffer, size_t bytes);
    // must return amount of bytes read, or 0 in case of end of file or error

    // check for end of file state:
    int (*eof)(void* userdata);
    // should return 1 for EOF, or 0 if not.
    // in case of EOF, it should be possible to undo
    // the EOF state  by seeking to a non-EOF position.

    // peek at length from archive file:
    size_t (*length)(void* userdata);
    // must return 0 in case of an error

    // seek to a specific position:
    int (*seek)(void* userdata, size_t pos);
    // return 1 if the seek succeeded, otherwise 0

    // tell the current position in the file:
    size_t (*tell)(void* userdata);

    // close the file:
    void (*close)(void* userdata);
};

// the zip decrypt functions to read the final result:
// (you get this struct from the decrypter)
struct zipdecryptionresult {
    // read from the decrypted file:
    size_t (*read)(struct zipdecryption* d, char* buffer, size_t bytes);
    // returns amount of bytes read, or 0 if EOF or error

    // check if EOF state is reached:
    int (*eof)(struct zipdecryption* d);
    // returns 1 if yes or otherwise 0

    // peek at length of decrypted file:
    size_t (*length)(struct zipdecryption* d);
    // returns 0 in case of an error

    // seek to a given position:
    int (*seek)(struct zipdecryption* d);
    // returns 1 on success, 0 on error

    // tell current position in the file:
    size_t (*tell)(struct zipdecryption* d);

    // close the decrypted file and the decrypter:
    void (*close)(struct zipdecryption* d);
};

#endif  // BLITWIZARD_ZIPDECRYPTION_H_

