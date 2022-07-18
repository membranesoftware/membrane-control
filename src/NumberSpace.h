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
// Class that tracks state for numeric values in a bounded range

#ifndef NUMBER_SPACE_H
#define NUMBER_SPACE_H

#include <set>
#include <list>

class NumberSpace {
public:
	NumberSpace ();
	~NumberSpace ();

	static const int64_t MinValue;
	static const int64_t MaxValue;

	struct Span {
		int64_t start;
		int64_t end;
		Span (): start (0), end (0) { }
		Span (int64_t start, int64_t end): start (start), end (end) { }
	};

	// Read-only data members
	int64_t minBound, maxBound;

	// Remove all number data and reset bounds to defaults
	void clear ();

	// Set the lower and upper bounds for numbers that can be covered
	void setBounds (int64_t min, int64_t max);

	// Insert value as a point in the number space
	void insert (int64_t value);

	// Return a boolean value indicating if value exists as a point in the number space
	bool exists (int64_t value) const;

	enum FindDirection {
		Increasing = 1,
		Decreasing = 2,
		Bidirectional = 3
	};
	// Return the inserted value nearest to targetValue, or defaultResult if the value was not found. If maxDistance is zero or greater, return found values only if within that distance from targetValue.
	int64_t findNearest (int64_t targetValue, int64_t maxDistance = -1, NumberSpace::FindDirection direction = NumberSpace::Bidirectional, int64_t defaultResult = 0) const;

	// Mark the specified span of numbers as covered within the space
	void cover (int64_t spanStart, int64_t spanEnd);

	// Return a boolean value indicating if targetValue is covered with the space
	bool isCovered (int64_t targetValue) const;

	// Return a boolean value indicating if the full range of bounded values is covered within the space
	bool isFullyCovered () const;

	// Return the lowest uncovered number within the space
	int64_t getUncoveredMin () const;

	// Return the highest uncovered number within the space
	int64_t getUncoveredMax () const;

	// Clear destList and insert values from stored cover spans
	void getCoverSpans (std::list<NumberSpace::Span> *destList) const;

private:
	static bool compareSpans (const NumberSpace::Span &a, const NumberSpace::Span &b);

	// Reset cover data as needed after a change in spans
	void refreshCover ();

	std::set<int64_t> values;
	std::list<NumberSpace::Span> coverSpans;
};

#endif
