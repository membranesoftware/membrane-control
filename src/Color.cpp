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
#include "StdString.h"
#include "Color.h"

Color::Color (float r, float g, float b, float a)
: r (r)
, g (g)
, b (b)
, a (a)
, isTranslating (false)
, isAnimating (false)
, translateDuration (0)
, animateDuration (0)
, animateRepeatDelay (0)
, animateStage (0)
, animateClock (0)
, targetR (0.0f)
, targetG (0.0f)
, targetB (0.0f)
, targetA (0.0f)
, deltaR (0.0f)
, deltaG (0.0f)
, deltaB (0.0f)
, deltaA (0.0f)
, animateColor1R (0.0f)
, animateColor1G (0.0f)
, animateColor1B (0.0f)
, animateColor1A (0.0f)
, animateColor2R (0.0f)
, animateColor2G (0.0f)
, animateColor2B (0.0f)
, animateColor2A (0.0f)
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
	if (a < 0.0f) {
		a = 0.0f;
	}
	else if (a > 1.0f) {
		a = 1.0f;
	}

	rByte = (uint8_t) (r * 255.0f);
	gByte = (uint8_t) (g * 255.0f);
	bByte = (uint8_t) (b * 255.0f);
	aByte = (uint8_t) (a * 255.0f);
}

StdString Color::toString () const {
	return (StdString::createSprintf ("{color: r=%.2f g=%.2f b=%.2f a=%.2f rByte=%i gByte=%i bByte=%i aByte=%i isTranslating=%s deltaR=%.2f deltaG=%.2f deltaB=%.2f}", r, g, b, a, rByte, gByte, bByte, aByte, BOOL_STRING (isTranslating), deltaR, deltaG, deltaB));
}

Color Color::copy (float aValue) {
	return (Color (r, g, b, (aValue >= 0.0f) ? aValue : a));
}

void Color::assign (float rValue, float gValue, float bValue) {
	r = rValue;
	g = gValue;
	b = bValue;
	isTranslating = false;
	normalize ();
}

void Color::assign (float rValue, float gValue, float bValue, float aValue) {
	r = rValue;
	g = gValue;
	b = bValue;
	a = aValue;
	isTranslating = false;
	normalize ();
}

void Color::assign (const Color &sourceColor) {
	assign (sourceColor.r, sourceColor.g, sourceColor.b, sourceColor.a);
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

	if (isTranslating) {
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

		a += (deltaA * (float) msElapsed);
		if (deltaA < 0.0f) {
			if (a <= targetA) {
				a = targetA;
				++matchcount;
			}
		}
		else {
			if (a >= targetA) {
				a = targetA;
				++matchcount;
			}
		}

		normalize ();
		if (matchcount >= 4) {
			isTranslating = false;
		}
	}

	if (isAnimating) {
		switch (animateStage) {
			case 0: {
				if (! isTranslating) {
					translate (animateColor2R, animateColor2G, animateColor2B, animateColor2A, animateDuration / 2);
					animateStage = 1;
				}
				break;
			}
			case 1: {
				if (! isTranslating) {
					translate (animateColor1R, animateColor1G, animateColor1B, animateColor1A, animateDuration / 2);
					animateStage = 2;
					animateClock = animateRepeatDelay;
				}
				break;
			}
			case 2: {
				if (! isTranslating) {
					animateClock -= msElapsed;
					if (animateClock <= 0) {
						animateStage = 0;
					}
				}
				break;
			}
		}
	}
}

void Color::translate (float translateTargetR, float translateTargetG, float translateTargetB, int durationMs) {
	float dr, dg, db;

	if (translateTargetR < 0.0f) {
		translateTargetR = 0.0f;
	}
	else if (translateTargetR > 1.0f) {
		translateTargetR = 1.0f;
	}
	if (translateTargetG < 0.0f) {
		translateTargetG = 0.0f;
	}
	else if (translateTargetG > 1.0f) {
		translateTargetG = 1.0f;
	}
	if (translateTargetB < 0.0f) {
		translateTargetB = 0.0f;
	}
	else if (translateTargetB > 1.0f) {
		translateTargetB = 1.0f;
	}

	if (durationMs <= 0) {
		assign (translateTargetR, translateTargetG, translateTargetB);
		return;
	}

	dr = translateTargetR - r;
	dg = translateTargetG - g;
	db = translateTargetB - b;
	if ((fabs (dr) < CONFIG_FLOAT_EPSILON) && (fabs (dg) < CONFIG_FLOAT_EPSILON) && (fabs (db) < CONFIG_FLOAT_EPSILON)) {
		isTranslating = false;
		return;
	}

	if (isTranslating && FLOAT_EQUALS (translateTargetR, targetR) && FLOAT_EQUALS (translateTargetG, targetG) && FLOAT_EQUALS (translateTargetB, targetB) && (translateDuration == durationMs)) {
		return;
	}

	isTranslating = true;
	translateDuration = durationMs;
	targetR = translateTargetR;
	targetG = translateTargetG;
	targetB = translateTargetB;
	targetA = a;
	deltaR = dr / ((float) durationMs);
	deltaG = dg / ((float) durationMs);
	deltaB = db / ((float) durationMs);
	deltaA = 0.0f;
}

void Color::translate (float translateTargetR, float translateTargetG, float translateTargetB, float translateTargetA, int durationMs) {
	float dr, dg, db, da;

	if (translateTargetR < 0.0f) {
		translateTargetR = 0.0f;
	}
	else if (translateTargetR > 1.0f) {
		translateTargetR = 1.0f;
	}
	if (translateTargetG < 0.0f) {
		translateTargetG = 0.0f;
	}
	else if (translateTargetG > 1.0f) {
		translateTargetG = 1.0f;
	}
	if (translateTargetB < 0.0f) {
		translateTargetB = 0.0f;
	}
	else if (translateTargetB > 1.0f) {
		translateTargetB = 1.0f;
	}
	if (translateTargetA < 0.0f) {
		translateTargetA = 0.0f;
	}
	else if (translateTargetA > 1.0f) {
		translateTargetA = 1.0f;
	}

	if (durationMs <= 0) {
		assign (translateTargetR, translateTargetG, translateTargetB, translateTargetA);
		return;
	}

	dr = translateTargetR - r;
	dg = translateTargetG - g;
	db = translateTargetB - b;
	da = translateTargetA - a;
	if ((fabs (dr) < CONFIG_FLOAT_EPSILON) && (fabs (dg) < CONFIG_FLOAT_EPSILON) && (fabs (db) < CONFIG_FLOAT_EPSILON) && (fabs (da) <= CONFIG_FLOAT_EPSILON)) {
		isTranslating = false;
		return;
	}

	if (isTranslating && FLOAT_EQUALS (translateTargetR, targetR) && FLOAT_EQUALS (translateTargetG, targetG) && FLOAT_EQUALS (translateTargetB, targetB) && FLOAT_EQUALS (translateTargetA, targetA) && (translateDuration == durationMs)) {
		return;
	}

	isTranslating = true;
	translateDuration = durationMs;
	targetR = translateTargetR;
	targetG = translateTargetG;
	targetB = translateTargetB;
	targetA = translateTargetA;
	deltaR = dr / ((float) durationMs);
	deltaG = dg / ((float) durationMs);
	deltaB = db / ((float) durationMs);
	deltaA = da / ((float) durationMs);
}

void Color::translate (const Color &targetColor, int durationMs) {
	translate (targetColor.r, targetColor.g, targetColor.b, targetColor.a, durationMs);
}

void Color::translate (const Color &startColor, const Color &targetColor, int durationMs) {
	assign (startColor);
	translate (targetColor.r, targetColor.g, targetColor.b, targetColor.a, durationMs);
}

void Color::animate (const Color &color1, const Color &color2, int durationMs, int repeatDelayMs) {
	assign (color1);
	if (durationMs <= 0) {
		return;
	}
	animateColor1R = color1.r;
	animateColor1G = color1.g;
	animateColor1B = color1.b;
	animateColor1A = color1.a;
	animateColor2R = color2.r;
	animateColor2G = color2.g;
	animateColor2B = color2.b;
	animateColor2A = color2.a;
	animateDuration = durationMs;
	animateRepeatDelay = repeatDelayMs;
	if (animateRepeatDelay < 0) {
		animateRepeatDelay = 0;
	}
	animateStage = 0;
	animateClock = 0;
	isAnimating = true;
}

bool Color::equals (const Color &other) const {
	return ((rByte == other.rByte) && (gByte == other.gByte) && (bByte == other.bByte) && (aByte == other.aByte));
}

Color Color::fromByteValues (uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	return (Color (Color::getByteValue (r), Color::getByteValue (g), Color::getByteValue (b), Color::getByteValue (a)));
}

float Color::getByteValue (uint8_t byte) {
	return (((float) byte) / 255.0f);
}
