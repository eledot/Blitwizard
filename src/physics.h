
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

#ifdef USE_PHYSICS

#ifdef __cplusplus
extern "C" {
#endif

//create and destroy worlds;
struct physicsworld;
struct physicsobject;
struct physicsworld* physics_CreateWorld();
void physics_DestroyWorld(struct physicsworld* world);
void physics_Step(struct physicsworld* world);
int physics_GetStepSize(struct physicsworld* world);

//Set a collision callback:
void physics_SetCollisionCallback(struct physicsworld* world, void (*callback)(void* userdata, struct physicsobject* a, struct physicsobject* b, double x, double y, double normalx, double normaly, double force), void* userdata);
//The callback receives your specified userdata, the collidion objects,
//the position in the center of the collision/overlap,
//the collision penetration normal pointing from object b to object a,
//and the impact force's strength.

//create and destroy objects;
struct physicsobject* physics_CreateObjectRectangle(struct physicsworld* world, void* userdata, int movable, double friction, double width, double height);
struct physicsobject* physics_CreateObjectOval(struct physicsworld* world, void* userdata, int movable, double friction, double width, double height);
void physics_DestroyObject(struct physicsobject* object);
void* physics_GetObjectUserdata(struct physicsobject* object);

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
void physics_UnsetGravity(struct physicsobject* obj);
void physics_SetRotationRestriction(struct physicsobject* obj, int restricted);
void physics_SetFriction(struct physicsobject* obj, double friction);
void physics_SetAngularDamping(struct physicsobject* obj, double damping);
void physics_SetLinearDamping(struct physicsobject* obj, double damping);
void physics_SetRestitution(struct physicsobject* obj, double restitution);

//change and get position/rotation and apply impulses
void physics_GetPosition(struct physicsobject* obj, double* x, double* y);
void physics_GetRotation(struct physicsobject* obj, double* angle);
void physics_Warp(struct physicsobject* obj, double x, double y, double angle);
void physics_ApplyImpulse(struct physicsobject* obj, double forcex, double forcey, double sourcex, double sourcey);

//collision test ray
int physics_Ray(struct physicsworld* world, double startx, double starty, double targetx, double targety, double* hitpointx, double* hitpointy, struct physicsobject** hitobject, double* hitnormalx, double* hitnormaly); //returns 1 when something is hit, otherwise 0  -- XXX: not thread-safe!

#ifdef __cplusplus
}
#endif

#endif
