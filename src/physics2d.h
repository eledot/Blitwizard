
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

#ifdef USE_PHYSICS2D

#ifdef __cplusplus
extern "C" {
#endif

// Create and destroy worlds:
struct physicsworld2d;
struct physicsobject2d;
struct physicsworld2d* physics2d_CreateWorld(void);
void physics2d_DestroyWorld(struct physicsworld2d* world);
void physics2d_Step(struct physicsworld2d* world);
int physics2d_GetStepSize(struct physicsworld2d* world);

// Set a collision callback:
void physics2d_SetCollisionCallback(struct physicsworld2d* world, int (*callback)(void* userdata, struct physicsobject2d* a, struct physicsobject2d* b, double x, double y, double normalx, double normaly, double force), void* userdata);
// The callback receives your specified userdata, the collidion objects,
// the position in the center of the collision/overlap,
// the collision penetration normal pointing from object b to object a,
// and the impact force's strength.
// If the callback returns 0, the collision will be ignored.
// If it returns 1, the collision will be processed.

// Create and destroy objects:
struct physicsobject2d* physics2d_CreateObjectRectangle(struct physicsworld2d* world, void* userdata, int movable, double friction, double width, double height);
struct physicsobject2d* physics2d_CreateObjectOval(struct physicsworld2d* world, void* userdata, int movable, double friction, double width, double height);
void physics2d_DestroyObject(struct physicsobject2d* object);
void* physics2d_GetObjectUserdata(struct physicsobject2d* object);

// Create an edge list object:
struct physicsobject2dedgecontext;
struct physicsobject2dedgecontext* physics2d_CreateObjectEdges_Begin(struct physicsworld2d* world, void* userdata, int movable, double friction);
void physics2d_CreateObjectEdges_Do(struct physicsobject2dedgecontext* context, double x1, double y1, double x2, double y2);
struct physicsobject2d* physics2d_CreateObjectEdges_End(struct physicsobject2dedgecontext* context);

// Get/set various properties
void physics2d_SetMass(struct physicsobject2d* obj, double mass);
double physics2d_GetMass(struct physicsobject2d* obj);
void physics2d_SetMassCenterOffset(struct physicsobject2d* obj, double offsetx, double offsety);
void physics2d_GetMassCenterOffset(struct physicsobject2d* obj, double* offsetx, double* offsety);
void physics2d_SetGravity(struct physicsobject2d* obj, float x, float y);
void physics2d_UnsetGravity(struct physicsobject2d* obj);
void physics2d_SetRotationRestriction(struct physicsobject2d* obj, int restricted);
void physics2d_SetFriction(struct physicsobject2d* obj, double friction);
void physics2d_SetAngularDamping(struct physicsobject2d* obj, double damping);
void physics2d_SetLinearDamping(struct physicsobject2d* obj, double damping);
void physics2d_SetRestitution(struct physicsobject2d* obj, double restitution);

// Change and get position/rotation and apply impulses
void physics2d_GetPosition(struct physicsobject2d* obj, double* x, double* y);
void physics2d_GetRotation(struct physicsobject2d* obj, double* angle);
void physics2d_Warp(struct physicsobject2d* obj, double x, double y, double angle);
void physics2d_ApplyImpulse(struct physicsobject2d* obj, double forcex, double forcey, double sourcex, double sourcey);

// Collision test ray
int physics2d_Ray(struct physicsworld2d* world, double startx, double starty, double targetx, double targety, double* hitpointx, double* hitpointy, struct physicsobject2d** hitobject, double* hitnormalx, double* hitnormaly); // returns 1 when something is hit, otherwise 0  -- XXX: not thread-safe!

#ifdef __cplusplus
}
#endif

#endif

