
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

#include "lua.h"
#include "lauxlib.h"

#include "luafuncs.h"
#include "graphics.h"
#include "timefuncs.h"
#include "luastate.h"
#include "audio.h"
#include "audiomixer.h"
#include "file.h"
#include "main.h"
#include "win32console.h"

int drawingallowed = 0;

int luafuncs_loadfile(lua_State* l) {
	const char* p = lua_tostring(l,1);
	if (!p) {
		lua_pushstring(l, "First argument is not a file name string");
		return lua_error(l);
	}
	int r = luaL_loadfile(l, p);
	if (r != 0) {
		char errormsg[512];
		if (r == LUA_ERRFILE) {
			lua_pushstring(l, "Cannot open file");
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
}

int luafuncs_dofile(lua_State* l) {
	const char* p = lua_tostring(l,1);
	if (!p) {
		lua_pushstring(l,"First argument is not a file name string");
		return lua_error(l);
	}
	int i = lua_gettop(l);
	while (i > 1) {
		lua_pop(l,1);
		i--;
	}
	lua_pushstring(l, "loadfile");
	lua_gettable(l, LUA_GLOBALSINDEX); //first, push function
	lua_pushvalue(l, 1); //then push file name as argument
	lua_call(l, 1, 1); //call loadfile
	int previouscount = lua_gettop(l)-1; //minus the function on the stack which lua_pcall() removes
	int ret = lua_pcall(l, 0, LUA_MULTRET, 0); //call returned function by loadfile
	if (ret != 0) {
		return lua_error(l);
	}
	return lua_gettop(l)-previouscount;
}

int luafuncs_setWindow(lua_State* l) {
	if (lua_gettop(l) <= 0) {
		graphics_Quit();
		return 0;
	}
	int x = lua_tonumber(l,1);
	if (x <= 0) {
		lua_pushstring(l,"First argument is not a valid resolution width");
		return lua_error(l);
	}
	int y = lua_tonumber(l,2);
	if (y <= 0) {
		lua_pushstring(l,"Second argument is not a valid resolution height");
		return lua_error(l);
	}
	
	char defaulttitle[] = "blitwizard";
	const char* title = lua_tostring(l,3);
	if (!title) {
		title = defaulttitle;
	}
	
	int fullscreen = 0;
	if (lua_type(l,4) == LUA_TBOOLEAN) {
		fullscreen = lua_toboolean(l,4);
	}else{
		if (lua_gettop(l) >= 4) {
			lua_pushstring(l,"Fourth argument is not a valid fullscreen boolean");
			return lua_error(l);
		}
	}
	
	const char* renderer = lua_tostring(l,5);
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
		lua_pushstring(l, "Eighth parameter is not a valid alpha number");
		alpha = lua_tonumber(l, 5);
		if (alpha < 0) {alpha = 0;}
		if (alpha > 1) {alpha = 1;}
	}

	graphics_DrawRectangle(x, y, width, height, r, g, b, alpha);
	return 0;
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
        cuty = (int)((float)lua_tonumber(l, 5)+0.5f);
    }
    if (lua_gettop(l) >= 7 && lua_type(l, 7) != LUA_TNIL) {
        if (lua_type(l, 8) != LUA_TNUMBER) {
            lua_pushstring(l, "Seventh parameter is not a valid cutwidth number");
            return lua_error(l);
        }
        cutwidth = (int)((float)lua_tonumber(l, 5)+0.5f);
		if (cutwidth < 0) {cutwidth = 0;}
    }
    if (lua_gettop(l) >= 8 && lua_type(l, 8) != LUA_TNIL) {
        if (lua_type(l, 8) != LUA_TNUMBER) {
            lua_pushstring(l, "Eighth parameter is not a valid cutheight number");
            return lua_error(l);
        }
        cutheight = (int)((float)lua_tonumber(l, 5)+0.5f);
		if (cutheight < 0) {cutheight = 0;}
    }

	//proceess negative cut positions and adjust output position accordingly
	if (cutx < 0) {
		if (cutwidth > 0) {
			cutwidth += cutx;
			x -= cutx;
			if (cutwidth < 0) {
				cutwidth = 0;
			}
		}
		cutx = 0;
	}
	if (cuty < 0) {
		if (cutheight > 0) {
			cutheight += cuty;
			x -= cuty;
			if (cutheight < 0) {
				cutheight = 0;
			}
		}
		cuty = 0;
	}

	//empty draw calls aren't possible, but we will "emulate" it to provide an error on a missing texture anyway
	if (cutwidth == 0 || cutheight == 0) {
		if (graphics_IsTextureLoaded(p)) {
			lua_pushstring(l, "Requested texture isn't loaded or available");
			return lua_error(l);
		}
	}

	if (cutwidth < 0) {cutwidth = 0;}
	if (cutheight < 0) {cutheight = 0;}

	//draw:
	if (!graphics_DrawCropped(p, x, y, alpha, cutx, cuty, cutwidth, cutheight)) {
		lua_pushstring(l, "Requested texture isn't loaded or available");
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
	if (lua_objlen(l, index) != sizeof(struct luaidref)) {
		return -1;
	}
	struct luaidref* idref = (struct luaidref*)lua_touserdata(l, index);
	if (!idref || idref->magic != IDREF_MAGIC || idref->type != IDREF_SOUND) {
		return -1;
	}
	return idref->id;
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
	const char* p = lua_tostring(l,1);
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
		if (lua_type(l,3) != LUA_TNUMBER) {
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
	iref->id = audiomixer_PlaySoundFromDisk(p, priority, volume, panning, fadein, looping);
	if (iref->id < 0) {
		lua_pop(l,1);
		lua_pushstring(l, "Cannot play sound");
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
	int i = 0;
	while (i < c) {
		int w,h;
		graphics_GetVideoMode(i, &w, &h);
		
		lua_createtable(l, 3, 0);
		lua_pushnumber(l, 1);
		lua_pushnumber(l, w);
		lua_settable(l, -3);
		lua_pushnumber(l, 2);
		lua_pushnumber(l, h);
		lua_settable(l, -3);
		
		lua_pushnumber(l, i);
		lua_settable(l, -3);
		i++;
	}
	return 1;
}

