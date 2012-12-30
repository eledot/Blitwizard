
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

#if (defined(USE_PHYSICS2D) || defined(USE_PHYSICS3D))

#ifdef __cplusplus
extern "C" {
#endif

// Create and destroy worlds:
struct physicsworld;
struct physicsobject;
struct physicsworld* physics_CreateWorld(int use3dphysics);
void physics_DestroyWorld(struct physicsworld* world);
void physics_Step(struct physicsworld* world);
int physics2d_GetStepSize(struct physicsworld* world);

// Set a collision callback:
#ifdef USE_PHYSICS2D
void physics_Set2dCollisionCallback(struct physicsworld* world, int (*callback)(void* userdata, struct physicsobject* a, struct physicsobject* b, double x, double y, double normalx, double normaly, double force), void* userdata);
#endif
#ifdef USE_PHYSICS3D
void physics_Set3dCollisionCallback(struct physicsworld* world, int (*callback)(void* userdata, struct physicsobject* a, struct physicsobject* b, double x, double y, double z, double normalx, double normaly, double normalz, double force), void* userdata);
#endif
// The callback receives your specified userdata, the collidion objects,
// the position in the center of the collision/overlap,
// the collision penetration normal pointing from object b to object a,
// and the impact force's strength.
// If the callback returns 0, the collision will be ignored.
// If it returns 1, the collision will be processed.
//
// The callback will be called during physics_Step.
// All operations in the callback are supported including deleting the object
// for which the callback was called.

// Obtain shape info struct (for use in object creation):
struct physicsobjectshape;
struct physicsobjectshape* physics_CreateEmptyShapes(int count);
#ifdef USE_PHYSICS2D
void physics_Set2dShapeRectangle(struct physicsobjectshape* shape, double width, double height);
void physics_Set2dShapeOval(struct physicsobjectshape* shape, double width, double height);
void physics_Set2dShapeCircle(struct physicsobjectshape* shape, double diameter);
void physics_Set2dShapeOffsetRotation(struct physicsobjectshape* shape, double xoffset, double yoffset, double rotation);
void physics_Get2dShapeOffsetRotation(struct physicsobjectshape* shape, double* xoffset, double* yoffset, double* rotation);
#endif
#ifdef USE_PHYSICS3D

#endif
void physics_DestroyShape(struct physicsobjectshape* shape);

// Create an object with a given pointer list to shape structs (ended with a NULL pointer):
struct physicsobject* physics_CreateObject(struct physicsworld* world, void* userdata, int movable, double friction, struct physicsobjectshape* shapelist);

// Destroy objects or obtain their userdata:
void physics_DestroyObject(struct physicsobject* object);
void* physics_GetObjectUserdata(struct physicsobject* object);

// Create an edge list object (complex 2d geometry):
#ifdef USE_PHYSICS2D
struct physicsobjectedgecontext;
struct physicsobjectedgecontext* physics_Create2dObjectEdges_Begin(struct physicsworld* world, void* userdata, int movable, double friction);
void physics2d_Create2dObjectEdges_Do(struct physicsobjectedgecontext* context, double x1, double y1, double x2, double y2);
struct physicsobject* physics2d_Create2dObjectEdges_End(struct physicsobjectedgecontext* context);
#endif

// Create a triangle list object (static complex 3d geometry):
#ifdef USE_PHYSICS3D
struct physicsobjecttrianglecontext;
struct physicsobjecttrianglecontext* physics_Create3dObjectTriangles_Begin(struct physicsworld* world, void* userdata, int movable, double friction);
void physics2d_Create3dObjectTriangles_Do(struct physicsobjecttrianglecontext* context, double x1, double y1, double x2, double y2);
struct physicsobject* physics2d_Create3dObjectTriangles_End(struct physicsobjecttrianglecontext* context);
#endif

// Get/set various properties
void physics_SetMass(struct physicsobject* obj, double mass);
double physics_GetMass(struct physicsobject* obj);
#ifdef USE_PHYSICS2D
void physics_Set2dMassCenterOffset(struct physicsobject* obj, double offsetx, double offsety);
void physics_Get2dMassCenterOffset(struct physicsobject* obj, double* offsetx, double* offsety);
#endif
#ifdef USE_PHYSICS3D
void physics_Set3dMassCenterOffset(struct physicsobject* obj, double offsetx, double offsety, double offsetz);
void physics_Get3dMassCenterOffset(struct physicsobject* obj, double* offsetx, double* offsety, double* offsetz);
#endif
#ifdef USE_PHYSICS2D
void physics_Set2dGravity(struct physicsobject* obj, float x, float y);
#endif
#ifdef USE_PHYSICS3D
void physics_Set3dGravity(struct physicsobject* obj, float x, float y);
#endif
void physics_UnsetGravity(struct physicsobject* obj);
#ifdef USE_PHYSICS2D
void physics_Set2dRotationRestriction(struct physicsobject* obj, int restricted);
#endif
#ifdef USE_PHYSICS3D
void physics_Set3dRotationRestrictionAroundAxis(struct physicsobject* obj, int restrictedaxisx, int restrictedaxisy, int restrictedaxisz);
void physics_Set3dRotationRestrictionAllAXis(struct physicsobject* obj, int restrictedtotally);
#endif
void physics_SetFriction(struct physicsobject* obj, double friction);
void physics_SetAngularDamping(struct physicsobject* obj, double damping);
void physics_SetLinearDamping(struct physicsobject* obj, double damping);
void physics_SetRestitution(struct physicsobject* obj, double restitution);

// Change and get position/rotation and apply impulses
#ifdef USE_PHYSICS2D
void physics_Get2dPosition(struct physicsobject* obj, double* x, double* y);
void physics_Get2dRotation(struct physicsobject* obj, double* angle);
void physics_Warp2d(struct physicsobject* obj, double x, double y, double angle);
void physics_Apply2dImpulse(struct physicsobject* obj, double forcex, double forcey, double sourcex, double sourcey);
#endif
#ifdef USE_PHYSICS3D
void physics_Get3dPosition(struct physicsobject* obj, double* x, double* y, double* z);
void physics_Get3dRotationQuaternion(struct physicsobject* obj, double* qx, double* qy, double* qz, double* qrot);
void physics_Warp3d(struct physicsobject* obj, double x, double y, double qx, double qy, double qz, double qrot);
void physics_Apply3dImpulse(struct physicsobject* obj, double forcex, double forcey, double sourcex, double sourcey);
#endif

// Collision test ray
#ifdef USE_PHYSICS2D
int physics_Ray2d(struct physicsworld* world, double startx, double starty, double targetx, double targety, double* hitpointx, double* hitpointy, struct physicsobject** objecthit, double* hitnormalx, double* hitnormaly); // returns 1 when something is hit, otherwise 0  -- XXX: not thread-safe!
#endif
#ifdef USE_PHYSICS3D
int physics_Ray3d(struct physicsworld* world, double startx, double starty, double startz, double targetx, double targety, double targetz, double* hitpointx, double* hitpointy, double* hitpointz, struct physicsobject** objecthit, double* hitnormalx, double* hitnormaly, double* hitnormalz); // returns 1 when something is hit, otherwise 0  -- XXX: not thread-safe!
#endif

#ifdef __cplusplus
}
#endif

#endif

