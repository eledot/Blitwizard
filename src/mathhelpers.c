
/* blitwizard 2d engine - source code file

  Copyright (C) 2012 Jonas Thiem

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

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "mathhelpers.h"

static float distfromzero(float x, float y) {
    return sqrt(x*x + y*y);
}

float getdist(float x, float y, float x2, float y2) {
    x2 -= x;
    y2 -= y;
    return distfromzero(x2,y2);
}

static float anglefromzero(float x, float y) {
    int substract180 = 0;
    if (x < 0) {x = -x;substract180 = 1;}
    if (x < 0.0001 && x > -0.0001) {
        if (y < 0) {
            return -90;
        }
        return 90;
    }
    float w = (float)atan((float)((float)y/(float)x));
    float angle = (w/3.141)*180;
    if (substract180 == 1) {
        if (angle > 0) {
            angle = 180-angle;
        }else{
            angle = -(180+angle);
        }
    }
    return angle;
}

float getangle(float x, float y, float x2, float y2) {
    x2 -= x;y2 -= y;
    return anglefromzero(x2,y2);
}

void rotatevec(float x, float y, float rotation, float* x2, float* y2) {
    float angle = anglefromzero(x,y);
    float dist = distfromzero(x,y);
    *x2 = cos((angle+rotation)/180*3.1415)*dist;
    *y2 = sin((angle+rotation)/180*3.1415)*dist;
}

float normalizeangle(float angle) {
    while (angle > 180) {angle -= 360;}
    while (angle < -180) {angle += 360;}
    return angle;
}

static float pointonline_relativepos(float x1, float y1, float x2, float y2, float px, float py) {
    float p2x_p1x = x2 - x1;
    float p2y_p1y = y2 - y1;
    return (((px - x1)*(x2 - x1) + (py - y1)*(y2 - y1))) / (p2x_p1x*p2x_p1x + p2y_p1y*p2y_p1y);
}

void pointonline(float x1, float y1, float x2, float y2, float px, float py, float* linepointx, float* linepointy, float* relativepos) {
    float relpos = pointonline_relativepos(x1,y1,x2,y2,px,py);
    if (relativepos) {
        *relativepos = relpos;
    }
    if (linepointx) {
        *linepointx = x1 + (x2-x1)*relpos;
    }
    if (linepointy) {
        *linepointy = y1 + (y2-y1)*relpos;
    }
}
