
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "lua.h"
#include "lauxlib.h"

#include "luafuncs.h"
#include "graphics.h"
#include "timefuncs.h"
#include "luastate.h"
#include "audio.h"
#include "audiomixer.h"
#include "file.h"
#include "filelist.h"
#include "main.h"
#include "win32console.h"
#include "osinfo.h"
#include "logging.h"

#if defined(ANDROID) || defined(__ANDROID__)
//required for RWops file loading for Android
#include "SDL.h"
#endif

int drawingallowed = 0;

#if defined(ANDROID) || defined(__ANDROID__)
//the lua chunk reader for Android
SDL_RWops* loadfilerwops = NULL;
struct luachunkreaderinfo {
	SDL_RWops* rwops;
	char buffer[512];
};
static const char* luastringchunkreader(lua_State *l, void *data, size_t *size) {
	struct luachunkreaderinfo* info = (struct luachunkreaderinfo*)data;
	printinfo("b");
	int i = info->rwops->read(info->rwops, info->buffer, 1, sizeof(*info->buffer));
	printinfo("c");
	if (i > 0) {
		*size = (size_t)i;
		return info->buffer;
	}
	return NULL;
}

#endif

int luafuncs_loadfile(lua_State* l) {
	printinfo("a1");
	const char* p = lua_tostring(l,1);
	if (!p) {
		lua_pushstring(l, "First argument is not a file name string");
		return lua_error(l);
	}
	printinfo("a2");
#if defined(ANDROID) || defined(__ANDROID__)
	//special Android file loading
	struct luachunkreaderinfo* info = malloc(sizeof(*info));
	if (!info) {
		lua_pushstring(l, "malloc failed");
		return lua_error(l);
	}
	printinfo("a3");
	char errormsg[512];
	memset(info, 0, sizeof(*info));
	info->rwops = SDL_RWFromFile(p, "r");
	printinfo("a3: %s", p);
	if (!info->rwops) {
		free(info);
		snprintf(errormsg, sizeof(errormsg), "Cannot open file \"%s\"", p);
        errormsg[sizeof(errormsg)-1] = 0;
        lua_pushstring(l, errormsg);
        return lua_error(l);
	}
	printinfo("a4");
	int r = lua_load(l, &luastringchunkreader, info, p, NULL);
	printinfo("a5");
	SDL_FreeRW(info->rwops);
	printinfo("a6");
    free(info);
	if (r != 0) {
        if (r == LUA_ERRSYNTAX) {
            snprintf(errormsg,sizeof(errormsg),"Syntax error: %s",lua_tostring(l,-1));
            lua_pop(l, 1);
            lua_pushstring(l, errormsg);
            return lua_error(l);
        }
		return lua_error(l);
	}
	return 1;
#else
	//regular file loading done by Lua
	int r = luaL_loadfile(l, p);
	if (r != 0) {
		char errormsg[512];
		if (r == LUA_ERRFILE) {
			snprintf(errormsg, sizeof(errormsg), "Cannot open file \"%s\"", p);
			errormsg[sizeof(errormsg)-1] = 0;
			lua_pushstring(l, errormsg);
			return lua_error(l);
		}
		if (r == LUA_ERRSYNTAX) {
			snprintf(errormsg,sizeof(errormsg),"Syntax error: %s",lua_tostring(l,-1));
			lua_pop(l, 1);
			lua_pushstring(l, errormsg);
			return lua_error(l);
		}
		return lua_error(l);
	}
	return 1;
#endif
}

static char printlinebuf[2048] = "";
static int luafuncs_printline(lua_State* l) {
	//print a line from the printlinebuf
	int len = strlen(printlinebuf);
	if (len <= 0) {
		return 0;
	}
	int i = 0;
	while (i < len) {
		if (printlinebuf[i] == '\n') {
			break;
		}
		i++;
	}
	if (i >= len-1 && printlinebuf[len-1] != '\n') {
		return 0;
	}
	printlinebuf[i] = 0;
	printinfo(printlinebuf);
	memmove(printlinebuf, printlinebuf+(i+1), sizeof(printlinebuf)-(i+1));
	return 1;
}
int luafuncs_print(lua_State* l) { //not threadsafe
    int args = lua_gettop(l);
	int i = 1;
	while (i <= args) {
        switch (lua_type(l, i)) {
			case LUA_TSTRING: {
				//add a space char first
				if (strlen(printlinebuf) > 0) {
					if (strlen(printlinebuf) < sizeof(printlinebuf)-1) {
						strcat(printlinebuf, " ");
					}
				}

				//add string
				unsigned int plen = strlen(printlinebuf);
				unsigned int len = (sizeof(printlinebuf)-1) - plen;
				const char* p = lua_tostring(l, i);
				if (len > strlen(p)) {
					len = strlen(p);
				}
				if (len > 0) {
					memcpy(printlinebuf + plen, p, len);
					printlinebuf[plen + len] = 0;
				}
				break;
			}
			case LUA_TNUMBER: {
				//add a space char first
                if (strlen(printlinebuf) > 0) {
                    if (strlen(printlinebuf) < sizeof(printlinebuf)-1) {
                        strcat(printlinebuf, " ");
                    }
                }

				//add number
				unsigned int plen = strlen(printlinebuf);
				unsigned int len = (sizeof(printlinebuf)-1) - plen;
				char number[50];
				snprintf(number, sizeof(number)-1, "%d", lua_tonumber(l, i));
				number[sizeof(number)-1] = 0;
				if (len >= strlen(number)) {
					len = strlen(number);
				}
				if (len > 0) {
					memcpy(printlinebuf + plen, number, len);
					printlinebuf[plen + len] = 0;
				}
				break;
			}
			default:
				break;	
        }
		while (luafuncs_printline(l)) { }
		i++;
    }
	while (luafuncs_printline(l)) { }
    return 0;
}


int luafuncs_sysname(lua_State* l) {
	lua_pushstring(l, osinfo_GetSystemName());
	return 1;
}

int luafuncs_sysversion(lua_State* l) {
    lua_pushstring(l, osinfo_GetSystemVersion());
    return 1;
}

int luafuncs_dofile(lua_State* l) {
	//obtain function name argument
	const char* p = lua_tostring(l,1);
	if (!p) {
		lua_pushstring(l,"First argument is not a file name string");
		return lua_error(l);
	}

	//pop all additional arguments we might have received
	int i = lua_gettop(l);
	while (i > 1) {
		lua_pop(l,1);
		i--;
	}

	//load function and call it
	lua_getglobal(l, "loadfile"); //first, push function
	lua_pushvalue(l, 1); //then push given file name as argument
	lua_call(l, 1, 1); //call loadfile
	int previoustop = lua_gettop(l)-1; //minus the function on the stack which lua_pcall() removes
	lua_call(l, 0, LUA_MULTRET); //call returned function by loadfile
	
	//return all values the function has left for us on the stack
	return lua_gettop(l)-previoustop;
}

int luafuncs_exists(lua_State* l) {
	const char* p = lua_tostring(l, 1);
    if (!p) {
        lua_pushstring(l, "First argument is not a valid path string");
        return lua_error(l);
    }
    if (file_DoesFileExist(p)) {
		lua_pushboolean(l, 1);
	}else{
		lua_pushboolean(l, 0);
	}
	return 1;
}

int luafuncs_isdir(lua_State* l) {
    const char* p = lua_tostring(l, 1);
    if (!p) {
        lua_pushstring(l, "First argument is not a valid path string");
        return lua_error(l);
    }
	if (!file_DoesFileExist(p)) {
		char errmsg[500];
		snprintf(errmsg, sizeof(errmsg), "No such file or directory: %s\n", p);
		errmsg[sizeof(errmsg)-1] = 0;
		lua_pushstring(l, errmsg);
		return lua_error(l);
	}
	if (file_IsDirectory(p)) {
		lua_pushboolean(l, 1);
	}else{
		lua_pushboolean(l, 0);
	}
	return 1;
}

int luafuncs_ls(lua_State* l) {
	const char* p = lua_tostring(l, 1);
	if (!p) {
		lua_pushstring(l, "First argument is not a valid path string");
		return lua_error(l);
	}
	struct filelistcontext* ctx = filelist_Create(p);
	if (!ctx) {
		char errmsg[500];
		snprintf(errmsg, sizeof(errmsg), "Failed to ls folder: %s", p);
		errmsg[sizeof(errmsg)-1] = 0;
		lua_pushstring(l, errmsg);
		return lua_error(l);
	}

	//create file listing table
	lua_newtable(l);

	//add all files/folders to file listing table
	char filenamebuf[500];
	int isdir;
	int returnvalue;
	int i = 0;
	while ((returnvalue = filelist_GetNextFile(ctx, filenamebuf, sizeof(filenamebuf), &isdir)) == 1) {
		i++;
		lua_pushinteger(l, i);
		lua_pushstring(l, filenamebuf);
		lua_settable(l, -3);
	}

	//free file list
	filelist_Free(ctx);

	//process error during listing
	if (returnvalue < 0) {
		lua_pop(l, 1); //remove file listing table

		char errmsg[500];
		snprintf(errmsg, sizeof(errmsg), "Error while processing ls in folder: %s", p);
		errmsg[sizeof(errmsg)-1] = 0;
		lua_pushstring(l, errmsg);
		return lua_error(l);
	}

	//return file list
	return 1;
}

int luafuncs_setWindow(lua_State* l) {
	if (lua_gettop(l) <= 0) {
		graphics_Quit();
		return 0;
	}
	int x = lua_tonumber(l, 1);
	if (x <= 0) {
		lua_pushstring(l, "First argument is not a valid resolution width");
		return lua_error(l);
	}
	int y = lua_tonumber(l, 2);
	if (y <= 0) {
		lua_pushstring(l, "Second argument is not a valid resolution height");
		return lua_error(l);
	}
	
	char defaulttitle[] = "blitwizard";
	const char* title = lua_tostring(l, 3);
	if (!title) {
		title = defaulttitle;
	}
	
	int fullscreen = 0;
	if (lua_type(l,4) == LUA_TBOOLEAN) {
		fullscreen = lua_toboolean(l, 4);
	}else{
		if (lua_gettop(l) >= 4) {
			lua_pushstring(l, "Fourth argument is not a valid fullscreen boolean");
			return lua_error(l);
		}
	}
	
	const char* renderer = lua_tostring(l, 5);
	char* error;
	if (!graphics_SetMode(x, y, fullscreen, 0, title, renderer, &error)) {
		if (error) {
			lua_pushstring(l, error);
			free(error);
			return lua_error(l);
		}
		lua_pushstring(l, "Unknown error on setting mode");
		return lua_error(l);
	}
	return 0;
}

int luafuncs_loadImage(lua_State* l) {
    const char* p = lua_tostring(l,1);
    if (!p) {
        lua_pushstring(l, "First parameter is not a valid image name string");
        return lua_error(l);
    }
	int i = graphics_IsTextureLoaded(p);
	if (i > 0) {
		lua_pushstring(l, "Image is either already loaded or currently being asynchronously loaded");
		return lua_error(l);
	} 
    i = graphics_LoadTextureInstantly(p);
    if (i == 0) {
        lua_pushstring(l, "Failed to load image");
        return lua_error(l);
    }
    return 0;
}

int luafuncs_loadImageAsync(lua_State* l) {
	const char* p = lua_tostring(l,1);
	if (!p) {
		lua_pushstring(l, "First parameter is not a valid image name string");
		return lua_error(l);
	}
	int i = graphics_PromptTextureLoading(p);
	if (i == 0) {
		lua_pushstring(l, "Failed to load image due to fatal error: Out of memory?");
		return lua_error(l);
	}
	if (i == 2) {
		lua_pushstring(l, "Image is already loaded");
		return lua_error(l);
	}
	return 0;
}

int luafuncs_getTime(lua_State* l) {
	lua_pushnumber(l, time_GetMilliSeconds());
	return 1;
}

int luafuncs_getImageSize(lua_State* l) {
	const char* p = lua_tostring(l,1);
	if (!p) {
		lua_pushstring(l, "First parameter is not a valid image name string");
		return lua_error(l);
	}
	unsigned int w,h;
	if (!graphics_GetTextureDimensions(p, &w,&h)) {
		lua_pushstring(l, "Failed to get image size");
		return lua_error(l);
	}
	lua_pushnumber(l, w);
	lua_pushnumber(l, h);
	return 2;
}

int luafuncs_getWindowSize(lua_State* l) {
	unsigned int w,h;
	if (!graphics_GetWindowDimensions(&w,&h)) {
		lua_pushstring(l, "Failed to get window size");
		return lua_error(l);
	}
	lua_pushnumber(l, w);
	lua_pushnumber(l, h);
	return 2;
}

int luafuncs_drawRectangle(lua_State* l) {
    //rectangle position
	int x,y;
    float alpha = 1;
    if (lua_type(l, 1) != LUA_TNUMBER) {
        lua_pushstring(l, "First parameter is not a valid x position number");
        return lua_error(l);
    }
    if (lua_type(l, 2) != LUA_TNUMBER) {
        lua_pushstring(l, "Second parameter is not a valid y position number");
        return lua_error(l);
    }
    x = (int)((float)lua_tonumber(l, 1)+0.5f);
    y = (int)((float)lua_tonumber(l, 2)+0.5f);

	//rectangle widths
	int width,height;
    if (lua_type(l, 3) != LUA_TNUMBER) {
        lua_pushstring(l, "Third parameter is not a valid width number");
        return lua_error(l);
    }
    if (lua_type(l, 4) != LUA_TNUMBER) {
        lua_pushstring(l, "Fourth parameter is not a valid height number");
        return lua_error(l);
    }
    width = (int)((float)lua_tonumber(l, 3)+0.5f);
    height = (int)((float)lua_tonumber(l, 4)+0.5f);

	//see if we are on screen anyway
	if (width <= 0 || height <= 0) {return 0;}
	if (x + width < 0 || y + height < 0) {return 0;}

	//read rectangle colors
	float r,g,b;
	if (lua_type(l, 5) != LUA_TNUMBER) {
		lua_pushstring(l, "Fifth parameter is not a valid red color number");
		return lua_error(l);
	}
	if (lua_type(l, 6) != LUA_TNUMBER) {
        lua_pushstring(l, "Sixth parameter is not a valid red color number");
        return lua_error(l);
    }
	if (lua_type(l, 7) != LUA_TNUMBER) {
        lua_pushstring(l, "Seventh parameter is not a valid red color number");
        return lua_error(l);
    }
	r = lua_tonumber(l, 5);
	g = lua_tonumber(l, 6);
	b = lua_tonumber(l, 7);
	if (r < 0) {r = 0;}
	if (r > 1) {r = 1;}
	if (g < 0) {g = 0;}
	if (g > 1) {g = 1;}
	if (b < 0) {b = 0;}
	if (b > 1) {b = 1;}

	//obtain alpha if given
	if (lua_gettop(l) >= 8) {
		if (lua_type(l, 8) != LUA_TNUMBER) {
			lua_pushstring(l, "Eighth parameter is not a valid alpha number");
			return lua_error(l);
		}
		alpha = lua_tonumber(l, 8);
		if (alpha < 0) {alpha = 0;}
		if (alpha > 1) {alpha = 1;}
	}

	graphics_DrawRectangle(x, y, width, height, r, g, b, alpha);
	return 0;
}

void luafuncs_pushnosuchtex(lua_State* l, const char* tex) {
	char errmsg[512];
    snprintf(errmsg,sizeof(errmsg), "Requested texture \"%s\" isn't loaded or available", tex);
    errmsg[sizeof(errmsg)-1] = 0;
    lua_pushstring(l, errmsg);
}

int luafuncs_drawImage(lua_State* l) {
	const char* p = lua_tostring(l,1);
	if (!p) {
		lua_pushstring(l, "First parameter is not a valid image name string");
		return lua_error(l);
	}
	
	//get position parameters
	int x,y;
	float alpha = 1;
	if (lua_type(l, 2) != LUA_TNUMBER) {
		lua_pushstring(l, "Second parameter is not a valid x position number");
		return lua_error(l);
	}
	if (lua_type(l, 3) != LUA_TNUMBER) {
		lua_pushstring(l, "Third parameter is not a valid y position number");
		return lua_error(l);
	}
	x = (int)((float)lua_tonumber(l,2)+0.5f);
	y = (int)((float)lua_tonumber(l,3)+0.5f);

	//get alpha parameter
	if (lua_gettop(l) >= 4 && lua_type(l, 4) != LUA_TNIL) {
		if (lua_type(l, 4) != LUA_TNUMBER) {
			lua_pushstring(l,"Fourth parameter is not a valid alpha number");
			return lua_error(l);
		}
		alpha = lua_tonumber(l,4);
		if (alpha < 0) {alpha = 0;}
		if (alpha > 1) {alpha = 1;}
	}
	
	//read cut rectangle parameters
	int cutx = 0;
	int cuty = 0;
	int cutwidth = -1;
	int cutheight = -1;
	if (lua_gettop(l) >= 5 && lua_type(l, 5) != LUA_TNIL) {
		if (lua_type(l, 5) != LUA_TNUMBER) {
			lua_pushstring(l, "Fifth parameter is not a valid cutx number");
			return lua_error(l);
		}
		cutx = (int)((float)lua_tonumber(l, 5)+0.5f);
	}
    if (lua_gettop(l) >= 6 && lua_type(l, 6) != LUA_TNIL) {
        if (lua_type(l, 6) != LUA_TNUMBER) {
            lua_pushstring(l, "Sixth parameter is not a valid cutx number");
            return lua_error(l);
        }
        cuty = (int)((float)lua_tonumber(l, 6)+0.5f);
    }
    if (lua_gettop(l) >= 7 && lua_type(l, 7) != LUA_TNIL) {
        if (lua_type(l, 7) != LUA_TNUMBER) {
            lua_pushstring(l, "Seventh parameter is not a valid cutwidth number");
            return lua_error(l);
        }
        cutwidth = (int)((float)lua_tonumber(l, 7)+0.5f);
		if (cutwidth < 0) {cutwidth = 0;}
    }
    if (lua_gettop(l) >= 8 && lua_type(l, 8) != LUA_TNIL) {
        if (lua_type(l, 8) != LUA_TNUMBER) {
            lua_pushstring(l, "Eighth parameter is not a valid cutheight number");
            return lua_error(l);
        }
        cutheight = (int)((float)lua_tonumber(l, 8)+0.5f);
		if (cutheight < 0) {cutheight = 0;}
    }
    
    //obtain scale parameters
    float scalex = 1;
    float scaley = 1;
    if (lua_gettop(l) >= 9 && lua_type(l, 9) != LUA_TNIL) {
        if (lua_type(l, 9) != LUA_TNUMBER) {
            lua_pushstring(l, "Ninth parameter is not a valid x scale number");
            return lua_error(l);
        }
        scalex = lua_tonumber(l, 9);
    }
    if (lua_gettop(l) >= 10 && lua_type(l, 10) != LUA_TNIL) {
        if (lua_type(l, 10) != LUA_TNUMBER) {
            lua_pushstring(l, "Tenth parameter is not a valid y scale number");
            return lua_error(l);
        }
        scaley = lua_tonumber(l, 10);
    }

	//obtain rotation parameters
	double rotationangle = 0;
	int rotationcenterx = 0;
	int rotationcentery = 0;
	//make rotation center default to image center
	unsigned int imgw,imgh;
	if (graphics_GetTextureDimensions(p, &imgw, &imgh)) {
		rotationcenterx = imgw/2;
		rotationcentery = imgh/2;
	}
	//get supplied rotation info
	if (lua_gettop(l) >= 11 && lua_type(l, 11) != LUA_TNIL) {
		if (lua_type(l, 11) != LUA_TNUMBER) {
			lua_pushstring(l, "Eleventh parameter is not a valid rotation angle number");
			return lua_error(l);
		}
		rotationangle = lua_tonumber(l, 11);
	}
	if (lua_gettop(l) >= 12 && lua_type(l, 12) != LUA_TNIL) {
		if (lua_type(l, 12) != LUA_TNUMBER) {
			lua_pushstring(l, "Twelfth parameter is not a valid rotation center x position number");
			return lua_error(l);
		}
		rotationcenterx = (int)(lua_tonumber(l, 12) + 0.5f);
	}
	if (lua_gettop(l) >= 13 && lua_type(l, 13) != LUA_TNIL) {
        if (lua_type(l, 13) != LUA_TNUMBER) {
            lua_pushstring(l, "Thirteenth parameter is not a valid rotation center y position number");
            return lua_error(l);
        }
        rotationcentery = (int)(lua_tonumber(l, 13) + 0.5f);
    }

	//process negative cut positions and adjust output position accordingly
	if (cutx < 0) {
		if (cutwidth > 0) { //decrease draw width accordingly
			cutwidth += cutx;
			if (cutwidth < 0) {
				cutwidth = 0;
			}
		}
		//move position to the right
		cutx = 0;
		x -= cutx;
	}
	if (cuty < 0) {
		if (cutheight < 0) { //decrease draw height accordingly
			cutheight += cuty;
			if (cutheight < 0) {
				cutheight = 0;
			}
		}
		cuty = 0;
		x -= cuty;
	}

	//empty draw calls aren't possible, but we will "emulate" it to provide an error on a missing texture anyway
	if (scalex <= 0 || scaley <= 0 || cutwidth == 0 || cutheight == 0) {
		if (!graphics_IsTextureLoaded(p)) {
			luafuncs_pushnosuchtex(l, p);
			return lua_error(l);
		}
	}

	if (cutwidth < 0) {cutwidth = 0;}
	if (cutheight < 0) {cutheight = 0;}

	int imgdraww = cutwidth;
	int imgdrawh = cutheight;

	if (imgdraww == 0 || imgdrawh == 0) {
		unsigned int w,h;
		if (graphics_GetTextureDimensions(p, &w, &h)) {
			if (imgdraww == 0) {imgdraww = w;}
			if (imgdrawh == 0) {imgdrawh = h;}
		}
	}	
	unsigned int drawwidth = (unsigned int)((float)(imgdraww) * scalex + 0.5f);
	unsigned int drawheight = (unsigned int)((float)(imgdrawh) * scaley + 0.5f);

	//draw:
	if (!graphics_DrawCropped(p, x, y, alpha, cutx, cuty, cutwidth, cutheight, drawwidth, drawheight, rotationcenterx, rotationcentery, rotationangle)) {
		luafuncs_pushnosuchtex(l, p);
		return lua_error(l);
	}
	return 0;
}

int luafuncs_chdir(lua_State* l) {
	const char* p = lua_tostring(l,1);
	if (!p) {
		lua_pushstring(l, "First parameter is not a directory string");
		return lua_error(l);
	}
	if (!file_Cwd(p)) {
		char errmsg[512];
		snprintf(errmsg,sizeof(errmsg)-1,"Failed to change directory to: %s",p);
		errmsg[sizeof(errmsg)-1] = 0;
		lua_pushstring(l, errmsg);
		return lua_error(l);
	}
	return 0;
}

int luafuncs_getRendererName(lua_State* l) {
	const char* p = graphics_GetCurrentRendererName();
	if (!p) {
		lua_pushnil(l);
		return 1;
	}else{
		lua_pushstring(l, p);
		return 1;
	}
}

int luafuncs_getBackendName(lua_State* l) {
	main_InitAudio();
	const char* p = audio_GetCurrentBackendName();
	if (p) {
		lua_pushstring(l, p);
	}else{
		lua_pushstring(l, "null driver");
	}
	return 1;
}

int luafuncs_openConsole(lua_State* intentionally_unused) {
	win32console_Launch();
	return 0;
}

static int soundfromstack(lua_State* l, int index) {
	if (lua_type(l, index) != LUA_TUSERDATA) {
		return -1;
	}
	if (lua_rawlen(l, index) != sizeof(struct luaidref)) {
		return -1;
	}
	struct luaidref* idref = (struct luaidref*)lua_touserdata(l, index);
	if (!idref || idref->magic != IDREF_MAGIC || idref->type != IDREF_SOUND) {
		return -1;
	}
	return idref->ref.id;
}

int luafuncs_stop(lua_State* l) {
	main_InitAudio();
	int id = soundfromstack(l, 1);
	if (id < 0) {
		lua_pushstring(l, "First parameter is not a valid sound handle");
		return lua_error(l);
	}
	audiomixer_StopSound(id);
	return 0;
}

int luafuncs_playing(lua_State* l) {
	main_InitAudio();
    int id = soundfromstack(l, 1);
    if (id < 0) {
        lua_pushstring(l, "First parameter is not a valid sound handle");
        return lua_error(l);
    }
	if (audiomixer_IsSoundPlaying(id)) {
		lua_pushboolean(l, 1);
	}else{
		lua_pushboolean(l, 0);
	}
	return 1;
}

int luafuncs_adjust(lua_State* l) {
	main_InitAudio();
	int id = soundfromstack(l, 1);
	if (id < 0) {
        lua_pushstring(l, "First parameter is not a valid sound handle");
        return lua_error(l);
    }
	if (lua_type(l, 2) != LUA_TNUMBER) {
		lua_pushstring(l, "Second parameter is not a valid volume number");
		return lua_error(l);
	}
	float volume = lua_tonumber(l, 2);
	if (volume < 0) {volume = 0;}
	if (volume > 1) {volume = 1;}
	if (lua_type(l, 3) != LUA_TNUMBER) {
		lua_pushstring(l, "Third parameter is not a valid panning number");
		return lua_error(l);
	}
	float panning = lua_tonumber(l, 3);
	if (panning < -1) {panning = -1;}
	if (panning > 1) {panning = 1;}

	audiomixer_AdjustSound(id, volume, panning);

	return 0;
}

int luafuncs_play(lua_State* l) {
	main_InitAudio();
	const char* p = lua_tostring(l, 1);
	if (!p) {
		lua_pushstring(l, "First parameter is not a valid sound name string");
		return lua_error(l);
	}
	float volume = 1;
	float panning = 0;
	int priority = -1;
	int looping = 0;
	float fadein = -1;
	if (lua_gettop(l) >= 2 && lua_type(l, 2) != LUA_TNIL) {
		if (lua_type(l,2) != LUA_TNUMBER) {
			lua_pushstring(l, "Second parameter is not a valid volume number");
			return lua_error(l);
		}
		volume = lua_tonumber(l, 2);
		if (volume < 0) {volume = 0;}
		if (volume > 1) {volume = 1;}
	}
	if (lua_gettop(l) >= 3 && lua_type(l, 3) != LUA_TNIL) {
		printf("Type: %d\n", lua_type(l, 3));
		if (lua_type(l, 3) != LUA_TNUMBER) {
			lua_pushstring(l, "Third parameter is not a valid panning number");
			return lua_error(l);
		}
		panning = lua_tonumber(l, 3);
		if (panning < -1) {panning = -1;}
		if (panning > 1) {panning = 1;}
	}
	if (lua_gettop(l) >= 4 && lua_type(l, 4) != LUA_TNIL) {
		if (lua_type(l, 4) != LUA_TBOOLEAN) {
			lua_pushstring(l,"Fourth parameter is not a valid loop boolean");
			return lua_error(l);
		}
		if (lua_toboolean(l, 4)) {
			looping = 1;
		}
	}
	if (lua_gettop(l) >= 5 && lua_type(l, 5) != LUA_TNIL) {
		if (lua_type(l,5) != LUA_TNUMBER) {
			lua_pushstring(l, "Fifth parameter is not a valid priority index number");
			return lua_error(l);
		}
		priority = lua_tointeger(l, 5);
		if (priority < 0) {priority = 0;}
	}
	if (lua_gettop(l) >= 6 && lua_type(l, 6) != LUA_TNIL) {
		if (lua_type(l,6) != LUA_TNUMBER) {
			lua_pushstring(l, "Sixth parameter is not a valid fade-in seconds number");
			return lua_error(l);
		}
		fadein = lua_tonumber(l,6);
		if (fadein <= 0) {
			fadein = -1;
		}
	}
	struct luaidref* iref = lua_newuserdata(l, sizeof(*iref));
	memset(iref,0,sizeof(*iref));
	iref->magic = IDREF_MAGIC;
	iref->type = IDREF_SOUND;
	iref->ref.id = audiomixer_PlaySoundFromDisk(p, priority, volume, panning, fadein, looping);
	if (iref->ref.id < 0) {
		char errormsg[512];
		snprintf(errormsg, sizeof(errormsg), "Cannot play sound \"%s\"", p);
		errormsg[sizeof(errormsg)-1] = 0;
		lua_pop(l,1);
		lua_pushstring(l, errormsg);
		return lua_error(l);
	}
	return 1;
}

int luafuncs_getDesktopDisplayMode(lua_State* l) {
	int w,h;
	graphics_GetDesktopVideoMode(&w, &h);
	lua_pushnumber(l, w);
	lua_pushnumber(l, h);
	return 2;
}

int luafuncs_getDisplayModes(lua_State* l) {
	int c = graphics_GetNumberOfVideoModes();
	lua_createtable(l, 5, 0);

	//first, add desktop mode
	int desktopw,desktoph;
	graphics_GetDesktopVideoMode(&desktopw, &desktoph);

	//resolution table with desktop width, height
	lua_createtable(l, 2, 0);
	lua_pushnumber(l, 1);
	lua_pushnumber(l, desktopw);
	lua_settable(l, -3);
	lua_pushnumber(l, 2);
	lua_pushnumber(l, desktoph);
	lua_settable(l, -3);

	//add table into our list
	lua_pushnumber(l, 2);
	lua_settable(l, -3);

	int i = 1;
	int index = 2;
	while (i <= c) {
		//add all supported video modes...
		int w,h;
		graphics_GetVideoMode(i, &w, &h);
		
		//...but not the desktop mode twice
		if (w == desktopw && h == desktoph) {
			i++;
			continue;
		}

		//table containing the resolution width, height
		lua_createtable(l, 2, 0);
		lua_pushnumber(l, 1);
		lua_pushnumber(l, w);
		lua_settable(l, -3);
		lua_pushnumber(l, 2);
		lua_pushnumber(l, h);
		lua_settable(l, -3);
		
		//add the table into our list
		lua_pushnumber(l, index);
		lua_settable(l, -3);
		index++;
		i++;
	}
	return 1;
}

int luafuncs_exit(lua_State* l) {
	int exitcode = lua_tonumber(l,1);
	main_Quit(exitcode);
	return 0;
}

