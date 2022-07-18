/*
* Copyright 2018-2022 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include <queue>
#include "Position.h"

Position::Position (float x, float y)
: x (x)
, y (y)
, isTranslating (false)
, translateTargetX (0.0f)
, translateTargetY (0.0f)
, translateDx (0.0f)
, translateDy (0.0f)
, translateDuration (0)
, translateClock (0)
{
}

Position::~Position () {

}

void Position::update (int msElapsed) {
	Position::Translation t;
	float dx, dy;
	int ms, dt;

	ms = msElapsed;
	while (isTranslating && (ms > 0)) {
		dt = ms;
		if (dt > translateClock) {
			dt = translateClock;
		}
		translateClock -= dt;
		ms -= dt;

		dx = translateDx * (float) dt;
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

		dy = translateDy * (float) dt;
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

		if (translateClock <= 0) {
			x = translateTargetX;
			y = translateTargetY;
			if (translationQueue.empty ()) {
				isTranslating = false;
			}
			else {
				t = translationQueue.front ();
				translationQueue.pop ();
				translate (x + t.deltaX, y + t.deltaY, t.duration);
			}
		}
	}
}

void Position::assign (float positionX, float positionY) {
	x = positionX;
	y = positionY;
	isTranslating = false;
	while (! translationQueue.empty ()) {
		translationQueue.pop ();
	}
}

void Position::assign (const Position &otherPosition) {
	assign (otherPosition.x, otherPosition.y);
}

void Position::assign (const Position &otherPosition, float dx, float dy) {
	assign (otherPosition.x + dx, otherPosition.y + dy);
}

void Position::assignX (float positionX) {
	assign (positionX, y);
}

void Position::assignY (float positionY) {
	assign (x, positionY);
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
	assign (x + dx, y + dy);
}

void Position::translate (float targetX, float targetY, int durationMs) {
	float dx, dy;

	if (durationMs <= 0) {
		x = targetX;
		y = targetY;
		translateTargetX = targetX;
		translateTargetY = targetY;
		translateDuration = 0;
		translateClock = 0;
		translateDx = 0.0f;
		translateDy = 0.0f;
		if (translationQueue.empty ()) {
			isTranslating = false;
		}
		return;
	}

	dx = targetX - x;
	dy = targetY - y;
	if ((fabs (dx) <= CONFIG_FLOAT_EPSILON) && (fabs (dy) <= CONFIG_FLOAT_EPSILON) && translationQueue.empty ()) {
		isTranslating = false;
		return;
	}

	if (isTranslating && FLOAT_EQUALS (translateTargetX, targetX) && FLOAT_EQUALS (translateTargetY, targetY) && (translateDuration == durationMs)) {
		return;
	}

	isTranslating = true;
	translateTargetX = targetX;
	translateTargetY = targetY;
	translateDuration = durationMs;
	translateClock = durationMs;
	translateDx = dx / (float) durationMs;
	translateDy = dy / (float) durationMs;
}

void Position::translate (const Position &targetPosition, int durationMs) {
	translate (targetPosition.x, targetPosition.y, durationMs);
}

void Position::translate (float startX, float startY, float targetX, float targetY, int durationMs) {
	assign (startX, startY);
	translate (targetX, targetY, durationMs);
}

void Position::translate (const Position &startPosition, const Position &targetPosition, int durationMs) {
	assign (startPosition);
	translate (targetPosition.x, targetPosition.y, durationMs);
}

void Position::translateX (float targetX, int durationMs) {
	translate (targetX, y, durationMs);
}

void Position::translateX (float startX, float targetX, int durationMs) {
	assignX (startX);
	translate (targetX, y, durationMs);
}

void Position::translateY (float targetY, int durationMs) {
	translate (x, targetY, durationMs);
}

void Position::translateY (float startY, float targetY, int durationMs) {
	assignY (startY);
	translate (x, targetY, durationMs);
}

void Position::plot (float deltaX, float deltaY, int durationMs) {
	Position::Translation t;

	if (! isTranslating) {
		while (! translationQueue.empty ()) {
			translationQueue.pop ();
		}
		translate (x + deltaX, y + deltaY, durationMs);
		return;
	}

	t.deltaX = deltaX;
	t.deltaY = deltaY;
	t.duration = durationMs;
	translationQueue.push (t);
}

void Position::plotX (float deltaX, int durationMs) {
	plot (deltaX, 0.0f, durationMs);
}

void Position::plotY (float deltaY, int durationMs) {
	plot (0.0f, deltaY, durationMs);
}
