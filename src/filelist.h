
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

//This API allows cross-platform querying of files and sub directories inside a directory.
//Please note the output is generally unsorted.

struct filelistcontext;

//Create a new file list context from a directory. Returns NULL in case of error (e.g. no such directory)
struct filelistcontext* filelist_Create(const char* path);

//Get the next file in the given file list context.
int filelist_GetNextFile(struct filelistcontext* listcontext, char* namebuf, size_t namebufsize, int* isdirectory);
//Returns 0 if no additional file has been found in the context (you should free it with filelist_Free then!).
//Returns 1 when another has been found and copies the name into namebuf and, if isdirectory is not NULL, sets
// *isdirectory to 1 for directories and 0 for other file types.
//Returns <0 on error. You must not use the file list any further and you should remove it with filelist_Free.

//Free a file list context:
void filelist_Free(struct filelistcontext* listcontext);
