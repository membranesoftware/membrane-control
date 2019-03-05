/*
* Copyright 2019 Membrane Software <author@membranesoftware.com>
*                 https://membranesoftware.com
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
#include "Config.h"
#include <stdlib.h>
#include <math.h>
#include "MathUtil.h"

float MathUtil::normalizeDirection (float direction) {
	while (direction < 0.0f) {
		direction += 360.0f;
	}
	while (direction >= 360.0f) {
		direction -= 360.0f;
	}

	return (direction);
}

float MathUtil::getDistance (float x1, float y1, float x2, float y2) {
	float dx, dy;

	dx = x2 - x1;
	dy = y2 - y1;
	return ((float) sqrt ((dx * dx) + (dy * dy)));
}

float MathUtil::getDistance (float dx, float dy) {
	return ((float) sqrt ((dx * dx) + (dy * dy)));
}

void MathUtil::getDirectionVector (float direction, float scale, float *dx, float *dy) {
	float rad;

	rad = direction * ((double) (3.14159265f / 180.0f));
	if (dx) {
		*dx = scale * cos (rad);
	}
	if (dy) {
		*dy = -1.0f * scale * sin (rad);
	}
}

float MathUtil::getVectorDirection (float dx, float dy) {
	double dist, theta;

	dist = (double) ((dx * dx) + (dy * dy));
	if (fabs (dist) < CONFIG_FLOAT_EPSILON) {
		return (0);
	}

	dist = sqrt (dist);
	theta = acos (((double) fabs (dx)) / dist);
	theta *= (double) (180.0f / 3.14159265f);

	if (dx < 0) {
		if (dy < 0) {
			theta = 180 - theta;
		}
		else {
			theta += 180;
		}
	}
	else {
		if (dy > 0) {
			theta = 360 - theta;
		}
	}

	return ((float) theta);
}
