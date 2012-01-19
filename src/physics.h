
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

#ifdef __cplusplus
extern "C" {
#endif

//create and destroy worlds;
struct physicsworld;
struct physicsworld* physics_CreateWorld();
void physics_DestroyWorld(struct physicsworld* world);
void physics_Step(struct physicsworld* world);
int physics_GetStepSize(struct physicsworld* world);

//create and destroy objects;
struct physicsobject;
struct physicsobject* physics_CreateObjectRectangle(struct physicsworld* world, void* userdata, int movable, double friction, double width, double height);
struct physicsobject* physics_CreateObjectOval(struct physicsworld* world, void* userdata, int movable, double friction, double width, double height);
void physics_DestroyObject(struct physicsobject* object);

//create an edge list object:
struct physicsobjectedgecontext;
struct physicsobjectedgecontext* physics_CreateObjectEdges_Begin(struct physicsworld* world, void* userdata, int movable, double friction);
void physics_CreateObjectEdges_Do(struct physicsobjectedgecontext* context, double x1, double y1, double x2, double y2);
struct physicsobject* physics_CreateObjectEdges_End(struct physicsobjectedgecontext* context);

//get/set various properties
void physics_SetMass(struct physicsobject* obj, double mass);
double physics_GetMass(struct physicsobject* obj);
void physics_SetMassCenterOffset(struct physicsobject* obj, double offsetx, double offsety);
void physics_GetMassCenterOffset(struct physicsobject* obj, double* offsetx, double* offsety);
void physics_SetGravity(struct physicsobject* obj, float x, float y);
void physics_SetRotationRestriction(struct physicsobject* obj, int restricted);
void physics_SetFriction(struct physicsobject* obj, double friction);
void physics_SetAngularDamping(struct physicsobject* obj, double damping);

//change and get position/rotation
void physics_GetPosition(struct physicsobject* obj, double* x, double* y);
void physics_GetRotation(struct physicsobject* obj, double* angle);
void physics_Warp(struct physicsobject* obj, double x, double y, double angle);

#ifdef __cplusplus
}
#endif


