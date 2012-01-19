
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

int luafuncs_createMovableObject(lua_State* l);
int luafuncs_createStaticObject(lua_State* l);
int luafuncs_setShapeRectangle(lua_State* l);
int luafuncs_setShapeEdges(lua_State* l);
int luafuncs_setShapeCircle(lua_State* l);
int luafuncs_setShapeOval(lua_State* l);
int luafuncs_restrictRotation(lua_State* l);
int luafuncs_setMass(lua_State* l);
int luafuncs_setFriction(lua_State* l);
int luafuncs_setAngularDamping(lua_State* l);
int luafuncs_getRotation(lua_State* l);
int luafuncs_getPosition(lua_State* l);
int luafuncs_warp(lua_State* l);

