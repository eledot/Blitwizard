
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

float getangle(float x, float y, float x2, float y2);
float getdist(float x, float y, float x2, float y2);
void pointonline(float x1, float y1, float x2, float y2, float px, float py, float* linepointx, float* linepointy, float* relativepos);
void rotatevec(float x, float y, float rotation, float* x2, float* y2);
float normalizeangle(float angle);

