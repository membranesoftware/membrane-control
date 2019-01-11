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
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "SDL2/SDL_video.h"
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "Color.h"

Color::Color (float r, float g, float b)
: r (r)
, g (g)
, b (b)
, isRotating (false)
, rotateDuration (0)
, targetR (0.0f)
, targetG (0.0f)
, targetB (0.0f)
, deltaR (0.0f)
, deltaG (0.0f)
, deltaB (0.0f)
{
	normalize ();
}

Color::~Color () {

}

void Color::normalize () {
	if (r < 0.0f) {
		r = 0.0f;
	}
	else if (r > 1.0f) {
		r = 1.0f;
	}
	if (g < 0.0f) {
		g = 0.0f;
	}
	else if (g > 1.0f) {
		g = 1.0f;
	}
	if (b < 0.0f) {
		b = 0.0f;
	}
	else if (b > 1.0f) {
		b = 1.0f;
	}

	rByte = (uint8_t) (r * 255.0f);
	gByte = (uint8_t) (g * 255.0f);
	bByte = (uint8_t) (b * 255.0f);
}

StdString Color::toString () {
	return (StdString::createSprintf ("{color: r=%.2f g=%.2f b=%.2f rByte=%i gByte=%i bByte=%i isRotating=%s deltaR=%.2f deltaG=%.2f deltaB=%.2f}", r, g, b, rByte, gByte, bByte, BOOL_STRING (isRotating), deltaR, deltaG, deltaB));
}

void Color::assign (float r, float g, float b) {
	this->r = r;
	this->g = g;
	this->b = b;
	isRotating = false;
	normalize ();
}

void Color::assign (const Color &sourceColor) {
	assign (sourceColor.r, sourceColor.g, sourceColor.b);
}

void Color::blend (float r, float g, float b, float alpha) {
	float rval, gval, bval, aval;

	if (r < 0.0f) {
		r = 0.0f;
	}
	else if (r > 1.0f) {
		r = 1.0f;
	}
	if (g < 0.0f) {
		g = 0.0f;
	}
	else if (g > 1.0f) {
		g = 1.0f;
	}
	if (b < 0.0f) {
		b = 0.0f;
	}
	else if (b > 1.0f) {
		b = 1.0f;
	}
	if (alpha < 0.0f) {
		alpha = 0.0f;
	}
	else if (alpha > 1.0f) {
		alpha = 1.0f;
	}

	aval = 1.0f - alpha;
	rval = (this->r * aval) + (r * alpha);
	gval = (this->g * aval) + (g * alpha);
	bval = (this->b * aval) + (b * alpha);
	assign (rval, gval, bval);
}

void Color::blend (const Color &sourceColor, float alpha) {
	blend (sourceColor.r, sourceColor.g, sourceColor.b, alpha);
}

void Color::update (int msElapsed) {
	int matchcount;

	if (isRotating) {
		matchcount = 0;

		r += (deltaR * (float) msElapsed);
		if (deltaR < 0.0f) {
			if (r <= targetR) {
				r = targetR;
				++matchcount;
			}
		}
		else {
			if (r >= targetR) {
				r = targetR;
				++matchcount;
			}
		}

		g += (deltaG * (float) msElapsed);
		if (deltaG < 0.0f) {
			if (g <= targetG) {
				g = targetG;
				++matchcount;
			}
		}
		else {
			if (g >= targetG) {
				g = targetG;
				++matchcount;
			}
		}

		b += (deltaB * (float) msElapsed);
		if (deltaB < 0.0f) {
			if (b <= targetB) {
				b = targetB;
				++matchcount;
			}
		}
		else {
			if (b >= targetB) {
				b = targetB;
				++matchcount;
			}
		}

		normalize ();
		if (matchcount >= 3) {
			isRotating = false;
		}
	}
}

void Color::rotate (float rotateTargetR, float rotateTargetG, float rotateTargetB, int durationMs) {
	float dr, dg, db;

	if (durationMs <= 0) {
		assign (rotateTargetR, rotateTargetG, rotateTargetB);
		return;
	}

	dr = rotateTargetR - r;
	dg = rotateTargetG - g;
	db = rotateTargetB - b;
	if ((fabs (dr) < CONFIG_FLOAT_EPSILON) && (fabs (dg) < CONFIG_FLOAT_EPSILON) && (fabs (db) < CONFIG_FLOAT_EPSILON)) {
		isRotating = false;
		return;
	}

	if (isRotating && FLOAT_EQUALS (rotateTargetR, targetR) && FLOAT_EQUALS (rotateTargetG, targetG) && FLOAT_EQUALS (rotateTargetB, targetB) && (rotateDuration == durationMs)) {
		return;
	}

	isRotating = true;
	rotateDuration = durationMs;
	targetR = rotateTargetR;
	targetG = rotateTargetG;
	targetB = rotateTargetB;
	deltaR = dr / ((float) durationMs);
	deltaG = dg / ((float) durationMs);
	deltaB = db / ((float) durationMs);
}

void Color::rotate (const Color &targetColor, int durationMs) {
	rotate (targetColor.r, targetColor.g, targetColor.b, durationMs);
}

void Color::rotate (const Color &startColor, const Color &targetColor, int durationMs) {
	assign (startColor);
	rotate (targetColor.r, targetColor.g, targetColor.b, durationMs);
}

bool Color::equals (const Color &other) const {
	return ((rByte == other.rByte) && (gByte == other.gByte) && (bByte == other.bByte));
}

Color Color::fromByteValues (uint8_t r, uint8_t g, uint8_t b) {
	return (Color (Color::getByteValue (r), Color::getByteValue (g), Color::getByteValue (b)));
}

float Color::getByteValue (uint8_t byte) {
	return (((float) byte) / 255.0f);
}
