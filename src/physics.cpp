
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

#include "Box2D/Box2D.h"
#include <stdint.h>
#include <math.h>
#include <stdio.h>

#include "physics.h"

extern "C" {

struct physicsworld {
	b2World* w;
	double gravityx,gravityy;
};

struct physicsobject {
	int movable;
	b2World* world;
	b2Body* body;
};

struct physicsworld* physics_CreateWorld() {
	struct physicsworld* world = (struct physicsworld*)malloc(sizeof(*world));
	if (!world) {
		return NULL;
	}
	memset(world, 0, sizeof(*world));
	b2Vec2 gravity(0.0f, 0.0f);
	world->w = new b2World(gravity);
	world->w->SetAllowSleeping(true);
	world->gravityx = 0;
	world->gravityy = 10;
	return world;
}

void physics_DestroyWorld(struct physicsworld* world) {
	delete world->w;
	free(world);
}

int physics_GetStepSize(struct physicsworld* world) {
	return (1000/50);
}

void physics_Step(struct physicsworld* world) {
	double forcefactor = (1.0/50.0)*1000;
	b2Body* b = world->w->GetBodyList();
	while (b) {
		b->SetAwake(true);
		b->ApplyForceToCenter(b2Vec2(world->gravityx * forcefactor, world->gravityy * forcefactor));
		b = b->GetNext();
	}
	world->w->Step(1.0 / 50, 8, 3);
}

static struct physicsobject* createobj(struct physicsworld* world, void* userdata, int movable) {
	struct physicsobject* object = (struct physicsobject*)malloc(sizeof(*object));
	if (!object) {return NULL;}
	memset(object, 0, sizeof(*object));
	b2BodyDef bodyDef;
	if (movable) {
		bodyDef.type = b2_dynamicBody;
	}
	object->movable = movable;
	bodyDef.userData = userdata;
	object->body = world->w->CreateBody(&bodyDef);
	object->world = world->w;
	if (!object->body) {free(object);return NULL;}
	return object;
}

struct physicsobject* physics_CreateObjectRectangle(struct physicsworld* world, void* userdata, int movable, double friction, double width, double height) {
	struct physicsobject* obj = createobj(world, userdata, movable);
	if (!obj) {return NULL;}
	b2PolygonShape box;
	box.SetAsBox(width/2, height/2);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &box;
	fixtureDef.friction = friction;
	obj->body->CreateFixture(&fixtureDef);
	return obj;
}

void physics_SetMass(struct physicsobject* obj, double mass) {
	if (!obj->movable) {return;}
	b2MassData mdata;
	obj->body->GetMassData(&mdata);
	mdata.mass = mass;
	obj->body->SetMassData(&mdata);
}

void physics_SetFriction(struct physicsobject* obj, double friction) {
	if (!obj->body) {return;}
	obj->body->GetFixtureList()->SetFriction(friction);
}

double physics_GetMass(struct physicsobject* obj) {
	b2MassData mdata;
    obj->body->GetMassData(&mdata);
    return mdata.mass;
}
void physics_SetMassCenterOffset(struct physicsobject* obj, double offsetx, double offsety) {
b2MassData mdata;
    obj->body->GetMassData(&mdata);
    mdata.center = b2Vec2(offsetx, offsety);
    obj->body->SetMassData(&mdata);
}
void physics_GetMassCenterOffset(struct physicsobject* obj, double* offsetx, double* offsety) {
	b2MassData mdata;
    obj->body->GetMassData(&mdata);
	*offsetx = mdata.center.x;
	*offsety = mdata.center.y;
}

void physics_DestroyObject(struct physicsobject* obj) {
	if (obj->body) {
		obj->world->DestroyBody(obj->body);
	}
	free(obj);
}

void physics_Warp(struct physicsobject* obj, double x, double y, double angle) {
	obj->body->SetTransform(b2Vec2(x, y), angle * M_PI / 180); 	
}

void physics_GetPosition(struct physicsobject* obj, double* x, double* y) {
	b2Vec2 pos = obj->body->GetPosition();
	*x = pos.x;
	*y = pos.y;
}

void physics_GetRotation(struct physicsobject* obj, double* angle) {
	*angle = (obj->body->GetAngle() * 180)/M_PI;
}

} //extern "C"

