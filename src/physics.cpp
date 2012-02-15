
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
#define _USE_MATH_DEFINES
#include <cmath>
#include <stdio.h>

#define EPSILON 0.0001

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
	int gravityset;
	double gravityx,gravityy;
	void* userdata;
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
	int i = 0;
	while (i < 2) {
		double forcefactor = (1.0/100.0)*8;
		b2Body* b = world->w->GetBodyList();
		while (b) {
			struct physicsobject* obj = (struct physicsobject*)b->GetUserData();
			if (obj) {
				if (obj->gravityset) {
					b->ApplyLinearImpulse(b2Vec2(obj->gravityx * forcefactor, obj->gravityy * forcefactor), b2Vec2(b->GetPosition().x, b->GetPosition().y));
				}else{
					b->ApplyLinearImpulse(b2Vec2(world->gravityx * forcefactor, world->gravityy * forcefactor), b2Vec2(b->GetPosition().x, b->GetPosition().y));
				}
			}
			b = b->GetNext();
		}
		world->w->Step(1.0 / 100, 10, 7);
		i++;
	}
}

class mycallback : public b2RayCastCallback {
public:
	b2Body* closestcollidedbody;
	b2Vec2 closestcollidedposition;
	b2Vec2 closestcollidednormal;

	mycallback() {
		closestcollidedbody = NULL;
	}

	virtual float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction) {
		closestcollidedbody = fixture->GetBody();
		closestcollidedposition = point;
		closestcollidednormal = normal;
		return fraction;
	}
};

int physics_Ray(struct physicsworld* world, double startx, double starty, double targetx, double targety, double* hitpointx, double* hitpointy, struct physicsobject** hitobject, double* hitnormalx, double* hitnormaly) {
	mycallback* callbackobj = new mycallback();
	world->w->RayCast(callbackobj, b2Vec2(startx, starty), b2Vec2(targetx, targety));
	if (callbackobj->closestcollidedbody) {
		*hitpointx = callbackobj->closestcollidedposition.x;
		*hitpointy = callbackobj->closestcollidedposition.y;
		*hitobject = ((struct physicsobject*)callbackobj->closestcollidedbody->GetUserData());
		*hitnormalx = callbackobj->closestcollidednormal.x;
		*hitnormaly = callbackobj->closestcollidednormal.y;
		delete callbackobj;
		return 1;
	}
	delete callbackobj;
	return 0;
}

void physics_ApplyImpulse(struct physicsobject* obj, double forcex, double forcey, double sourcex, double sourcey) {
	if (!obj->body || !obj->movable) {return;}
	obj->body->ApplyLinearImpulse(b2Vec2(forcex, forcey), b2Vec2(sourcex, sourcey));

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
	bodyDef.userData = (void*)object;
	object->userdata = userdata;
	object->body = world->w->CreateBody(&bodyDef);
	object->body->SetFixedRotation(false);
	object->world = world->w;
	if (!object->body) {free(object);return NULL;}
	return object;
}

struct physicsobject* physics_CreateObjectRectangle(struct physicsworld* world, void* userdata, int movable, double friction, double width, double height) {
	struct physicsobject* obj = createobj(world, userdata, movable);
	if (!obj) {return NULL;}
	b2PolygonShape box;
	box.SetAsBox((width/2) - box.m_radius*2, (height/2) - box.m_radius*2);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &box;
	fixtureDef.friction = friction;
	fixtureDef.density = 1;
	obj->body->SetFixedRotation(false);
	obj->body->CreateFixture(&fixtureDef);
	physics_SetMass(obj, 0);
	return obj;
}

static struct physicsobject* physics_CreateObjectCircle(struct physicsworld* world, void* userdata, int movable, double friction, double radius) {
	struct physicsobject* obj = createobj(world, userdata, movable);
    if (!obj) {return NULL;}
	b2CircleShape circle;
	circle.m_radius = radius - 0.01;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circle;
	fixtureDef.friction = friction;
	fixtureDef.density = 1;
	obj->body->SetFixedRotation(false);
	obj->body->CreateFixture(&fixtureDef);
	physics_SetMass(obj, 0);
	return obj;
}

#define OVALVERTICES 16
#define OVALVERTICESQUARTER 4

struct physicsobject* physics_CreateObjectOval(struct physicsworld* world, void* userdata, int movable, double friction, double width, double height) {
  	if (fabs(width - height) < EPSILON) {
		return physics_CreateObjectCircle(world, userdata, movable, friction, width);
	}

	//construct oval shape
	b2PolygonShape shape;
    b2Vec2 vertices[OVALVERTICES];
	int i = 0;
	int sideplace = 0;
	int sideplacedir = 1;
	double angle = 0;
	while (angle < 2*M_PI && i < OVALVERTICES) {
		if (sideplacedir > 0) {
			if (sideplace < OVALVERTICESQUARTER) {
				sideplace++;
			}else{
				sideplace++;
				sideplacedir = -1;
			}
		}else{
			if (sideplace > 2) {
				sideplace--;
			}else{
				sideplace--;
				sideplacedir = 1;
			}
		}
		double ovalsidepercentage = ((double)sideplace-1.0)/((double)OVALVERTICESQUARTER);
		printf("oval side percentage: %f\n", ovalsidepercentage);
		angle += (2*M_PI)/((double)OVALVERTICES);
		i++;
	}
	shape.Set(vertices, OVALVERTICES);

	//get physics object
    struct physicsobject* obj = createobj(world, userdata, movable);
    if (!obj) {return NULL;}

	//construct fixture def from shape
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.friction = friction;
    fixtureDef.density = 1;

	//set fixture to body and finish
    obj->body->SetFixedRotation(false);
    obj->body->CreateFixture(&fixtureDef);
	physics_SetMass(obj, 0);
    return obj;
}

void physics_SetMass(struct physicsobject* obj, double mass) {
	if (!obj->movable) {return;}
	if (!obj->body) {return;}
	if (mass > 0) {
		if (obj->body->GetType() == b2_staticBody) {
			obj->body->SetType(b2_dynamicBody);
		}
	}else{
		mass = 0;
		if (obj->body->GetType() == b2_dynamicBody) {
			obj->body->SetType(b2_staticBody);
		}
	}
	b2MassData mdata;
	obj->body->GetMassData(&mdata);
	mdata.mass = mass;
	obj->body->SetMassData(&mdata);
}

void physics_SetRotationRestriction(struct physicsobject* obj, int restricted) {
	if (!obj->body) {return;}
	if (restricted) {
		obj->body->SetFixedRotation(true);
	}else{
		obj->body->SetFixedRotation(false);
	}
}

void physics_SetLinearDamping(struct physicsobject* obj, double damping) {
    if (!obj->body) {return;}
    obj->body->SetLinearDamping(damping);
}


void physics_SetAngularDamping(struct physicsobject* obj, double damping) {
	if (!obj->body) {return;}
	obj->body->SetAngularDamping(damping);
}

void physics_SetFriction(struct physicsobject* obj, double friction) {
	if (!obj->body) {return;}
	b2Fixture* f = obj->body->GetFixtureList();
	while (f) {
		f->SetFriction(friction);
		f = f->GetNext();
	}
	b2ContactEdge* e = obj->body->GetContactList();
    while (e) {
		e->contact->ResetFriction();
		e = e->next;
	}
}

void physics_SetRestitution(struct physicsobject* obj, double restitution) {
	if (restitution > 1) {restitution = 1;}
	if (restitution < 0) {restitution = 0;}
    if (!obj->body) {return;}
    b2Fixture* f = obj->body->GetFixtureList();
    while (f) {
        f->SetRestitution(restitution);
        f = f->GetNext();
    }
    b2ContactEdge* e = obj->body->GetContactList();
    while (e) {
        e->contact->SetRestitution(restitution);
        e = e->next;
    }
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
	if (!obj) {return;}
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

struct edge {
	int inaloop;
	int processed;
	int adjacentcount;
	double x1,y1,x2,y2;
	struct edge* next;
	struct edge* adjacent1, *adjacent2;
};

struct physicsobjectedgecontext {
	struct physicsobject* obj;
	double friction;
	struct edge* edgelist;
};

struct physicsobjectedgecontext* physics_CreateObjectEdges_Begin(struct physicsworld* world, void* userdata, int movable, double friction) {
	struct physicsobjectedgecontext* context = (struct physicsobjectedgecontext*)malloc(sizeof(*context));
	if (!context) {
		return NULL;
	}
	memset(context, 0, sizeof(*context));
	context->obj = createobj(world, userdata, movable);
	if (!context->obj) {
		free(context);
		return NULL;
	}
	context->friction = friction;
	return context;
}

int physics_CheckEdgeLoop(struct edge* edge, struct edge* target) {
	struct edge* e = edge;
	struct edge* eprev = NULL;
	while (e) {
		if (e == target) {return 1;}
		struct edge* nextprev = e;
		if (e->adjacent1 && e->adjacent1 != eprev) {
			e = e->adjacent1;
		}else{
			if (e->adjacent2 && e->adjacent2 != eprev) {
				e = e->adjacent2;
			}else{
				e = NULL;
			}
		}
		eprev = nextprev;
	}
	return 0;
}

void physics_CreateObjectEdges_Do(struct physicsobjectedgecontext* context, double x1, double y1, double x2, double y2) {
	struct edge* newedge = (struct edge*)malloc(sizeof(*newedge));
	if (!newedge) {return;}
	memset(newedge, 0, sizeof(*newedge));
	newedge->x1 = x1;
	newedge->y1 = y1;
	newedge->x2 = x2;
	newedge->y2 = y2;
	newedge->adjacentcount = 1;
	
	//search for adjacent edges
	struct edge* e = context->edgelist;
	while (e) {
		if (!newedge->adjacent1) {
			if (fabs(e->x1 - newedge->x1) < EPSILON && fabs(e->y1 - newedge->y1) < EPSILON && e->adjacent1 == NULL) {
				if (physics_CheckEdgeLoop(e, newedge)) {
					newedge->inaloop = 1;
				}else{
					e->adjacentcount += newedge->adjacentcount;
                	newedge->adjacentcount = e->adjacentcount;
				}
				newedge->adjacent1 = e;
				e->adjacent1 = newedge;
                e = e->next;
                continue;
			}
			if (fabs(e->x2 - newedge->x1) < EPSILON && fabs(e->y2 - newedge->y1) < EPSILON && e->adjacent2 == NULL) {
				if (physics_CheckEdgeLoop(e, newedge)) {
					newedge->inaloop = 1;
				}else{
					e->adjacentcount += newedge->adjacentcount;
            	    newedge->adjacentcount = e->adjacentcount;
				}
            	newedge->adjacent1 = e;
            	e->adjacent2 = newedge;
                e = e->next;
                continue;
        	}
		}
		if (!newedge->adjacent2) {
			if (fabs(e->x1 - newedge->x2) < EPSILON && fabs(e->y1 - newedge->y2) < EPSILON && e->adjacent1 == NULL) {
				if (physics_CheckEdgeLoop(e, newedge)) {
					newedge->inaloop = 1;
				}else{
					e->adjacentcount += newedge->adjacentcount;
                	newedge->adjacentcount = e->adjacentcount;
				}
            	newedge->adjacent2 = e;
            	e->adjacent1 = newedge;
                e = e->next;
                continue;
        	}
			if (fabs(e->x2 - newedge->x2) < EPSILON && fabs(e->y2 - newedge->y2) < EPSILON && e->adjacent2 == NULL) {
				if (physics_CheckEdgeLoop(e, newedge)) {
					newedge->inaloop = 1;
				}else{
					e->adjacentcount += newedge->adjacentcount;
                	newedge->adjacentcount = e->adjacentcount;
				}
           	 	newedge->adjacent2 = e;
           		e->adjacent2 = newedge;
				e = e->next;
				continue;
			}
		}
		e = e->next;
	}

	//add us to the unsorted linear list
	newedge->next = context->edgelist;
	context->edgelist = newedge;
}

struct physicsobject* physics_CreateObjectEdges_End(struct physicsobjectedgecontext* context) {
	if (!context->edgelist) {
		physics_DestroyObject(context->obj);
		free(context);
		return NULL;
	}

	struct edge* e = context->edgelist;
	while (e) {
		//skip edges we already processed
		if (e->processed) {
			e = e->next;
			continue;
		}

		//only process edges which are start of an adjacent chain, in a loop or lonely
		if (e->adjacent1 && e->adjacent2 && !e->inaloop) {
			e = e->next;
			continue;
		}

		int varraysize = e->adjacentcount+1;
		b2Vec2 varray[varraysize];
		b2ChainShape chain;
		e->processed = 1;

		//see into which direction we want to go
		struct edge* eprev = e;
		struct edge* e2;
		if (e->adjacent1) {
			varray[0] = b2Vec2(e->x2, e->y2);
			varray[1] = b2Vec2(e->x1, e->y1);
			e2 = e->adjacent1;
		}else{
            varray[0] = b2Vec2(e->x1, e->y1);
            varray[1] = b2Vec2(e->x2, e->y2);
			e2 = e->adjacent2;
		}

		//ok let's take a walk:
		int i = 2;
		while (e2) {
			if (e2->processed) {break;}
			e2->processed = 1;
			struct edge* enextprev = e2;

			//Check which vertex we want to add
			if (e2->adjacent1 == eprev) {
				varray[i] = b2Vec2(e2->x2, e2->y2);
			}else{
				varray[i] = b2Vec2(e2->x1, e2->y1);
			}

			//advance to next edge
			if (e2->adjacent1 && e2->adjacent1 != eprev) {
				e2 = e2->adjacent1;
			}else{
				if (e2->adjacent2 && e2->adjacent2 != eprev) {
					e2 = e2->adjacent2;
				}else{
					e2 = NULL;
				}
			}
			eprev = enextprev;
			i++;
		}

		//debug print values
		/*int u = 0;
		while (u < e->adjacentcount + 1 - (1 * e->inaloop)) {
			printf("Chain vertex: %f, %f\n", varray[u].x, varray[u].y);
			u++;
		}*/
	
		//construct an edge shape from this
		if (e->inaloop) {
			chain.CreateLoop(varray, e->adjacentcount);
		}else{
			chain.CreateChain(varray, e->adjacentcount+1);
		}

		//add it to our body
		b2FixtureDef fixtureDef;
    	fixtureDef.shape = &chain;
    	fixtureDef.friction = context->friction;
		fixtureDef.density = 0;
    	context->obj->body->CreateFixture(&fixtureDef);
	}
	struct physicsobject* obj = context->obj;
	
	//free all edges
	e = context->edgelist;
	while (e) {
		struct edge* enext = e->next;
		free(e);
		e = enext;
	}

	free(context);

	physics_SetMass(obj, 0);
	return obj;
}

} //extern "C"

