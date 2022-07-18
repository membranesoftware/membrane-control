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
// Object that represents an RGBA color value

#ifndef COLOR_H
#define COLOR_H

#include "StdString.h"

class Color {
public:
	Color (float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
	~Color ();

	// Read-only data members
	float r, g, b, a;
	uint8_t rByte, gByte, bByte, aByte;
	bool isTranslating;
	bool isAnimating;

	// Return a string description of the color
	StdString toString () const;

	// Return a Color object that has been assigned to the specified byte values
	static Color fromByteValues (uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

	// Return the decimal color value associated with the specified byte
	static float getByteValue (uint8_t byte);

	// Return a Color object with values copied from this one. If aValue is zero or greater, assign that value to the returned object's alpha component.
	Color copy (float aValue = -1.0f);

	// Return a boolean value indicating if the color holds values equal to another
	bool equals (const Color &other) const;

	// Assign the color's values
	void assign (float rValue, float gValue, float bValue);
	void assign (float rValue, float gValue, float bValue, float aValue);
	void assign (const Color &sourceColor);

	// Modify the color's values by applying the specified blend operation
	void blend (float r, float g, float b, float alpha);
	void blend (const Color &sourceColor, float alpha);

	// Execute operations to update object state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

	// Begin an operation to change the color's value over time
	void translate (float translateTargetR, float translateTargetG, float translateTargetB, int durationMs);
	void translate (float translateTargetR, float translateTargetG, float translateTargetB, float translateTargetA, int durationMs);
	void translate (const Color &targetColor, int durationMs);
	void translate (const Color &startColor, const Color &targetColor, int durationMs);

	// Begin an operation to animate the color's value over time
	void animate (const Color &color1, const Color &color2, int durationMs, int repeatDelayMs = 0);

private:
	// Clip the r, g, and b data members to valid ranges, and reset dependent data members
	void normalize ();

	int translateDuration;
	int animateDuration;
	int animateRepeatDelay;
	int animateStage;
	int animateClock;
	float targetR, targetG, targetB, targetA;
	float deltaR, deltaG, deltaB, deltaA;
	float animateColor1R, animateColor1G, animateColor1B, animateColor1A;
	float animateColor2R, animateColor2G, animateColor2B, animateColor2A;
};

#endif
