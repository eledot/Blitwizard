
/* blitwizard 2d engine - source code file

  Copyright (C) 2011-2012 Jonas Thiem

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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "lua.h"

#include "luastate.h"
#include "luafuncs_physics.h"
#include "physics.h"
#include "main.h"

struct luaphysicsobj {
	int movable;
	struct physicsobject* object;
	double friction;
};

static struct luaphysicsobj* toluaphysicsobj(lua_State* l, int index) {
	char invalid[] = "Not a valid physics object reference";
	if (lua_type(l, index) != LUA_TTABLE) {
		lua_pushstring(l, invalid);
		lua_error(l);
		return NULL;
	}
	lua_pushnumber(l, 1);
	if (index < 0) {index--;}
	lua_gettable(l, index);
	if (lua_type(l, -1) != LUA_TUSERDATA) {
		lua_pop(l, 1);
		lua_pushstring(l, invalid);
		lua_error(l);
		return NULL;
	}
	if (lua_rawlen(l, -1) != sizeof(struct luaidref)) {
        lua_pushstring(l, invalid);
        lua_error(l);
        return NULL;
    }
    struct luaidref* idref = (struct luaidref*)lua_touserdata(l, -1);
    if (!idref || idref->magic != IDREF_MAGIC || idref->type != IDREF_PHYSICS) {
		lua_pushstring(l, invalid);
        lua_error(l);
		return NULL;
    }
	struct luaphysicsobj* obj = idref->ref.ptr;
	lua_pop(l, 1);
	return obj;
}

static struct luaidref* createphysicsobj(lua_State* l) {
	struct luaidref* ref = lua_newuserdata(l, sizeof(*ref));
	struct luaphysicsobj* obj = malloc(sizeof(*obj));
	if (!obj) {
		lua_pop(l, 1);
		lua_pushstring(l, "Failed to allocate physics object wrap struct");
		lua_error(l);
		return NULL;
	}
	memset(obj, 0, sizeof(*obj));
	memset(ref, 0, sizeof(*ref));
	ref->magic = IDREF_MAGIC;
	ref->type = IDREF_PHYSICS;
	ref->ref.ptr = obj;
	return ref;
}

int luafuncs_createMovableObject(lua_State* l) {
	lua_newtable(l);
	lua_pushnumber(l, 1);
	struct luaidref* ref = createphysicsobj(l);
	((struct luaphysicsobj*)ref->ref.ptr)->movable = 1;
	lua_settable(l, -3);
	return 1;
}

int luafuncs_createStaticObject(lua_State* l) {
    lua_newtable(l);
    lua_pushnumber(l, 1);
    struct luaidref* ref = createphysicsobj(l);
    lua_settable(l, -3);
    return 1;
}

int luafuncs_setMass(lua_State* l) {
    struct luaphysicsobj* obj = toluaphysicsobj(l, 1);
    if (!obj->movable) {
		lua_pushstring(l, "Mass can be only set on movable objects");
		return lua_error(l);
	}
	if (lua_gettop(l) < 2 || lua_type(l, 2) != LUA_TNUMBER || lua_tonumber(l, 2) <= 0) {
        lua_pushstring(l, "Second parameter is not a valid mass number");
        return lua_error(l);
    }
	double centerx = 0;
	double centery = 0;
	double mass = lua_tonumber(l, 2);
	if (lua_gettop(l) >= 3 && lua_type(l, 3) != LUA_TNIL) {
		if (lua_type(l, 3) != LUA_TNUMBER) {
			lua_pushstring(l, "Third parameter is not a valid x center offset number");
			return lua_error(l);
		}
		centerx = lua_tonumber(l, 3);
	}
	if (lua_gettop(l) >= 4 && lua_type(l, 4) != LUA_TNIL) {
		if (lua_type(l, 4) != LUA_TNUMBER) {
			lua_pushstring(l, "Fourth parameter is not a valid y center offset number");
			return lua_error(l);
		}
		centery = lua_tonumber(l, 4);
	}
	if (!obj->object) {
		lua_pushstring(l, "Physics object has no shape");
		return lua_error(l);
	}
	physics_SetMass(obj->object, mass);
	physics_SetMassCenterOffset(obj->object, centerx, centery);
	return 0;
}

void transferbodysettings(struct physicsobject* oldbody, struct physicsobject* newbody) {
	double mass = physics_GetMass(oldbody);
	double massx,massy;
	physics_GetMassCenterOffset(oldbody, &massx, &massy);
	physics_SetMass(newbody, mass);
	physics_SetMassCenterOffset(newbody, massx, massy);
}

int luafuncs_warp(lua_State* l) {
	struct luaphysicsobj* obj = toluaphysicsobj(l, 1);
	if (!obj->object) {
		lua_pushstring(l, "Physics object doesn't have a shape");
		return lua_error(l);
	}
	double warpx,warpy,warpangle;
	physics_GetPosition(obj->object, &warpx, &warpy);
	physics_GetRotation(obj->object, &warpangle);
	if (lua_gettop(l) >= 2 && lua_type(l, 2) != LUA_TNIL) {
		if (lua_type(l, 2) != LUA_TNUMBER) {
       		lua_pushstring(l, "Second parameter not a valid warp x position number");
       		return lua_error(l);
    	}
		warpx = lua_tonumber(l, 2);
	}
	if (lua_gettop(l) >= 3 && lua_type(l, 3) != LUA_TNIL) {
        if (lua_type(l, 3) != LUA_TNUMBER) {
            lua_pushstring(l, "Third parameter not a valid warp y position number");
            return lua_error(l);
        }
		warpy = lua_tonumber(l, 3);
    }
	if (lua_gettop(l) >= 4 && lua_type(l, 4) != LUA_TNIL) {
        if (lua_type(l, 4) != LUA_TNUMBER) { 
            lua_pushstring(l, "Fourth parameter not a valid warp angle number");
            return lua_error(l);
        }
		warpangle = lua_tonumber(l, 4);
    }
	physics_Warp(obj->object, warpx, warpy, warpangle);
	return 0;
}

int luafuncs_getPosition(lua_State* l) {
	struct luaphysicsobj* obj = toluaphysicsobj(l, 1);
	if (!obj->object) {
        lua_pushstring(l, "Physics object doesn't have a shape");
        return lua_error(l);
    }
	double x,y;
	physics_GetPosition(obj->object, &x, &y);
	lua_pushnumber(l, x);
	lua_pushnumber(l, y);
	return 2;
}

int luafuncs_getRotation(lua_State* l) {
    struct luaphysicsobj* obj = toluaphysicsobj(l, 1);
    if (!obj->object) {
        lua_pushstring(l, "Physics object doesn't have a shape");
        return lua_error(l);
    }
    double angle;
    physics_GetRotation(obj->object, &angle);
    lua_pushnumber(l, angle);
    return angle;
}

int luafuncs_setShapeEdges(lua_State* l) {
	struct luaphysicsobj* obj = toluaphysicsobj(l, 1);
    if (lua_gettop(l) < 2 || lua_type(l, 2) != LUA_TTABLE) {
        lua_pushstring(l, "Second parameter is not a valid edge list");
        return lua_error(l);
    }
	if (obj->movable) {
		lua_pushstring(l, "This shape is not allowed for movable objects");
		return lua_error(l);
	}

	struct physicsobjectedgecontext* context = physics_CreateObjectEdges_Begin(main_DefaultPhysicsPtr(), obj, 0, obj->friction);

	int haveedge = 0;
	double d = 0;
	while (1) {
		lua_pushnumber(l, d);
		lua_gettable(l, 2);
		if (lua_type(l, -1) != LUA_TTABLE) {
			if (lua_type(l, -1) == LUA_TNIL && haveedge) {
				break;
			}
			lua_pushstring(l, "Second parameter is not a valid edge list");
			physics_DestroyObject(physics_CreateObjectEdges_End(context));
			return lua_error(l);
		}
		haveedge = 1;

		double x1,y1,x2,y2;
		lua_pushnumber(l, 1);
		lua_gettable(l, -1);
		x1 = lua_tonumber(l, 1);
		lua_pushnumber(l, 1); 
        lua_gettable(l, -1);
        y1 = lua_tonumber(l, 1);
		lua_pushnumber(l, 1); 
        lua_gettable(l, -1);
        x2 = lua_tonumber(l, 1);
		lua_pushnumber(l, 1); 
        lua_gettable(l, -1);
        y2 = lua_tonumber(l, 1);

		physics_CreateObjectEdges_Do(context, x1, y1, x2, y2);
		d++;
    }
	
	struct physicsobject* oldobject = obj->object;

	obj->object = physics_CreateObjectEdges_End(context);
	if (!obj->object) {
		lua_pushstring(l, "Creation of the edges shape failed");
		return lua_error(l);
	}
	
	if (oldobject) {
        transferbodysettings(oldobject, obj->object);
        physics_DestroyObject(oldobject);
    }
	return 0;
}



int luafuncs_setShapeRectangle(lua_State* l) {
	struct luaphysicsobj* obj = toluaphysicsobj(l, 1);
	if (lua_gettop(l) < 2 || lua_type(l, 2) != LUA_TNUMBER || lua_tonumber(l, 2) <= 0) {
		lua_pushstring(l, "Not a valid rectangle width number");
		return lua_error(l);
	}
	if (lua_gettop(l) < 3 || lua_type(l, 3) != LUA_TNUMBER || lua_tonumber(l, 3) <= 0) {
        lua_pushstring(l, "Not a valid rectangle height number");
        return lua_error(l);
    }
	double width = lua_tonumber(l, 2);
	double height = lua_tonumber(l, 3);
	struct physicsobject* oldobject = obj->object;
	obj->object = physics_CreateObjectRectangle(main_DefaultPhysicsPtr(), obj, obj->movable, obj->friction, width, height);
	if (!obj->object) {
		lua_pushstring(l, "Failed to allocate shape");
		return lua_error(l);
	}

	if (oldobject) {
		transferbodysettings(oldobject, obj->object);
		physics_DestroyObject(oldobject);
	}

	return 0;
}

