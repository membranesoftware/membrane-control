/*
* Copyright 2018 Membrane Software <author@membranesoftware.com>
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
#include <stdio.h>
#include "App.h"
#include "Result.h"
#include "Log.h"
#include "Position.h"

Position::Position (float x, float y)
: x (x)
, y (y)
, translateTargetX (0.0f)
, translateTargetY (0.0f)
, translateDx (0.0f)
, translateDy (0.0f)
, isTranslating (false)
{
}

Position::~Position () {

}

void Position::update (int msElapsed) {
	float dx, dy;

	if (! isTranslating) {
		return;
	}

	dx = translateTargetX - x;
	dy = translateTargetY - y;
	if ((fabs (dx) <= CONFIG_FLOAT_EPSILON) && (fabs (dy) <= CONFIG_FLOAT_EPSILON)) {
		isTranslating = false;
		return;
	}

	dx = translateDx * msElapsed;
	x += dx;
	if (translateDx < 0.0f) {
		if (x < translateTargetX) {
			x = translateTargetX;
		}
	}
	else {
		if (x > translateTargetX) {
			x = translateTargetX;
		}
	}

	dy = translateDy * msElapsed;
	y += dy;
	if (translateDy < 0.0f) {
		if (y < translateTargetY) {
			y = translateTargetY;
		}
	}
	else {
		if (y > translateTargetY) {
			y = translateTargetY;
		}
	}
}

void Position::assign (float positionX, float positionY) {
	x = positionX;
	y = positionY;
	isTranslating = false;
}

void Position::assign (const Position &otherPosition) {
	x = otherPosition.x;
	y = otherPosition.y;
	isTranslating = false;
}

void Position::assign (const Position &otherPosition, float dx, float dy) {
	x = otherPosition.x + dx;
	y = otherPosition.y + dy;
	isTranslating = false;
}

void Position::assignX (float positionX) {
	x = positionX;
	isTranslating = false;
}

void Position::assignY (float positionY) {
	y = positionY;
	isTranslating = false;
}

void Position::assignBounded (float positionX, float positionY, float minX, float minY, float maxX, float maxY) {
	if (positionX < minX) {
		positionX = minX;
	}
	if (positionX > maxX) {
		positionX = maxX;
	}
	if (positionY < minY) {
		positionY = minY;
	}
	if (positionY > maxY) {
		positionY = maxY;
	}
	assign (positionX, positionY);
}

bool Position::equals (float positionX, float positionY) const {
	return (FLOAT_EQUALS (x, positionX) && FLOAT_EQUALS (y, positionY));
}

bool Position::equals (const Position &otherPosition) const {
	return (FLOAT_EQUALS (x, otherPosition.x) && FLOAT_EQUALS (y, otherPosition.y));
}

void Position::move (float dx, float dy) {
	x += dx;
	y += dy;
	isTranslating = false;
}

void Position::translate (float targetX, float targetY, int durationMs) {
	float dx, dy;

	if (durationMs <= 0) {
		return;
	}

	dx = targetX - x;
	dy = targetY - y;
	if ((fabs (dx) <= CONFIG_FLOAT_EPSILON) && (fabs (dy) <= CONFIG_FLOAT_EPSILON)) {
		return;
	}

	isTranslating = true;
	translateTargetX = targetX;
	translateTargetY = targetY;
	translateDx = dx / durationMs;
	translateDy = dy / durationMs;
}
