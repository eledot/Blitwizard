
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

// Read-only zip archive api.
// Supports reading .zip files which can be arbitrary parts inside
// a larger file (e.g. appended to an .exe file) if you specify
// the bounds accordingly.
// Can be used from multiple threads if each thread has a separate
// struct zipfile* pointer opened.

#ifndef BLITWIZARD_ZIPFILE_H_
#define BLITWIZARD_ZIPFILE_H_

#ifdef USE_PHYSFS

// open a zip archive file:
struct zipfile;
struct zipfile* zipfile_Open(const char* file, size_t offsetinfile,
size_t sizeinfile);

// iterate over all files in a directory the given zip archive
// (start with "" to get a listing of the archive's root directory)
struct zipfileiter;
struct zipfileiter* zipfile_Iterate(struct zipfile* f, const char* dir);
// Returns the next file or directory, or NULL if you reached the end:
const char* zipfile_NextFile(struct zipfileiter* iter);
// Clear your zipfile archive iterator again:
void zipfile_FinishIteration(struct zipfileiter* iter);

// check if a given path is a directory or a file:
int zipfile_IsDirectory(struct zipfile* f, const char* path);

// do things with files inside the archive:
// get length of a file in bytes:
int zipfile_FileGetLength(struct zipfile* f, const char* path);
// read from a file:
struct zipfilereader;
struct zipfilereader* zipfile_FileOpen(struct zipfile* f, const char* path);
size_t zipfile_FileRead(struct zipfilereader* reader, char* buffer,
size_t bytes);  // returns amount of bytes read, or 0 on end of file/error
void zipfile_FileClose(struct zipfilereader* reader);

// Close the archive again (do not use if you still got open zipfilereader handles!):
void zipfile_Close(struct zipfile* f);

#endif  // USE_PHYSFS

#endif  // BLITWIZARD_ZIPFILE_H_

