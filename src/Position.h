/*
* Copyright 2018-2021 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
// Object that manages a set of position coordinates and executes translation operations against them

#ifndef POSITION_H
#define POSITION_H

#include <queue>

class Position {
public:
	Position (float x = 0.0f, float y = 0.0f);
	~Position ();

	// Read-only data members
	float x, y;
	bool isTranslating;
	float translateTargetX, translateTargetY;
	float translateDx, translateDy;
	int translateDuration;
	int translateClock;

	// Update state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

	// Set the position's coordinate values
	void assign (float positionX, float positionY);
	void assign (const Position &otherPosition);
	void assign (const Position &otherPosition, float dx, float dy);

	// Set the position's x coordinate value
	void assignX (float positionX);

	// Set the position's y coordinate value
	void assignY (float positionY);

	// Set the position's coordinate values, clipped to fit within the specified range
	void assignBounded (float positionX, float positionY, float minX, float minY, float maxX, float maxY);

	// Change the position's coordinate values by adding the specified deltas
	void move (float dx, float dy);

	// Begin a translation operation using the provided parameters
	void translate (float targetX, float targetY, int durationMs);
	void translate (const Position &targetPosition, int durationMs);
	void translate (float startX, float startY, float targetX, float targetY, int durationMs);
	void translate (const Position &startPosition, const Position &targetPosition, int durationMs);

	// Begin a translation operation for the position's x coordinate value
	void translateX (float targetX, int durationMs);
	void translateX (float startX, float targetX, int durationMs);

	// Begin a translation operation for the position's y coordinate value
	void translateY (float targetY, int durationMs);
	void translateY (float startY, float targetY, int durationMs);

	// Add a translation that should be executed as part of a sequence
	void plot (float deltaX, float deltaY, int durationMs);

	// Add a translation for the position's x coordinate value that should be executed as part of a sequence
	void plotX (float deltaX, int durationMs);

	// Add a translation for the position's y coordinate value that should be executed as part of a sequence
	void plotY (float deltaY, int durationMs);

	// Return a boolean value indicating if the position is equivalent to the provided one
	bool equals (float positionX, float positionY) const;
	bool equals (const Position &otherPosition) const;

private:
	struct Translation {
		float deltaX, deltaY;
		int duration;
		Translation (): deltaX (0.0f), deltaY (0.0f), duration (0) { }
	};
	std::queue<Position::Translation> translationQueue;
};

#endif
