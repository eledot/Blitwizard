
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

//Run script:

int luastate_DoInitialFile(const char* file, char** error);


//Call functions:

//Arguments:
int luastate_PushFunctionArgumentToMainstate_Bool(int yesno); //1: success, 0: failure (out of memory)
int luastate_PushFunctionArgumentToMainstate_String(const char* string); //1: success, 0: failure (out of memory)
//Function call:
int luastate_CallFunctionInMainstate(const char* function, int args, char** error); //1: success, 0: failure (*error will be either changed to NULL or to an error string - NULL means most likely out of memory)


