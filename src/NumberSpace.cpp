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
#include <set>
#include <list>
#include <climits>
#include "NumberSpace.h"

const int64_t NumberSpace::MinValue = LLONG_MIN;
const int64_t NumberSpace::MaxValue = LLONG_MAX;

NumberSpace::NumberSpace ()
: minBound (NumberSpace::MinValue)
, maxBound (NumberSpace::MaxValue)
{
}

NumberSpace::~NumberSpace () {
}

void NumberSpace::clear () {
	minBound = NumberSpace::MinValue;
	maxBound = NumberSpace::MaxValue;
	values.clear ();
	coverSpans.clear ();
}

void NumberSpace::setBounds (int64_t min, int64_t max) {
	std::list<NumberSpace::Span>::iterator i, end;

	if (min > max) {
		min = max;
	}
	minBound = min;
	maxBound = max;

	i = coverSpans.begin ();
	end = coverSpans.end ();
	while (i != end) {
		if (i->start < minBound) {
			i->start = minBound;
		}
		if (i->start > maxBound) {
			i->start = maxBound;
		}
		if (i->end < minBound) {
			i->end = minBound;
		}
		if (i->end > maxBound) {
			i->end = maxBound;
		}
		++i;
	}

	refreshCover ();
}

void NumberSpace::insert (int64_t value) {
	if ((value < minBound) || (value > maxBound)) {
		return;
	}
	values.insert (value);
}

bool NumberSpace::exists (int64_t value) const {
	if ((value < minBound) || (value > maxBound)) {
		return (false);
	}
	return (values.count (value) > 0);
}

int64_t NumberSpace::findNearest (int64_t targetValue, int64_t maxDistance, NumberSpace::FindDirection direction, int64_t defaultResult) const {
	std::set<int64_t>::iterator lower, start, i;
	std::set<int64_t>::const_reverse_iterator last;
	int64_t result, dist, mindist;

	if (values.empty ()) {
		return (defaultResult);
	}
	lower = values.lower_bound (targetValue);
	if ((lower != values.end ()) && (*lower == targetValue)) {
		return (targetValue);
	}
	if (maxDistance < 0) {
		maxDistance = NumberSpace::MaxValue;
	}
	result = defaultResult;
	mindist = -1;

	if ((direction == NumberSpace::Increasing) || (direction == NumberSpace::Bidirectional)) {
		if (lower != values.end ()) {
			dist = llabs (*lower - targetValue);
			if ((dist <= maxDistance) && ((mindist < 0) || (dist < mindist))) {
				result = *lower;
				mindist = dist;
			}
		}
	}
	if ((direction == NumberSpace::Decreasing) || (direction == NumberSpace::Bidirectional)) {
		if (lower != values.end ()) {
			start = values.begin ();
			if (*start < targetValue) {
				dist = llabs (*start - targetValue);
				if ((dist <= maxDistance) && ((mindist < 0) || (dist < mindist))) {
					result = *start;
					mindist = dist;
				}

				i = lower;
				while (i != start) {
					if (*i < targetValue) {
						dist = llabs (*i - targetValue);
						if (dist > maxDistance) {
							break;
						}
						if ((mindist < 0) || (dist < mindist)) {
							result = *i;
							mindist = dist;
						}
					}
					--i;
				}
			}
		}
		else {
			last = values.crbegin ();
			dist = llabs (*last - targetValue);
			if ((dist <= maxDistance) && ((mindist < 0) || (dist < mindist))) {
				result = *last;
				mindist = dist;
			}
		}
	}
	return (result);
}

void NumberSpace::cover (int64_t spanStart, int64_t spanEnd) {
	if (spanStart < minBound) {
		spanStart = minBound;
	}
	if (spanStart > maxBound) {
		spanStart = maxBound;
	}
	if (spanEnd < minBound) {
		spanEnd = minBound;
	}
	if (spanEnd > maxBound) {
		spanEnd = maxBound;
	}
	if (spanStart > spanEnd) {
		return;
	}
	coverSpans.push_back (NumberSpace::Span (spanStart, spanEnd));
	refreshCover ();
}

bool NumberSpace::compareSpans (const NumberSpace::Span &a, const NumberSpace::Span &b) {
	if (a.start < b.start) {
		return (true);
	}
	if (a.start > b.start) {
		return (false);
	}
	if (a.end < b.end) {
		return (true);
	}
	return (false);
}

void NumberSpace::refreshCover () {
	std::list<NumberSpace::Span> spans;
	std::list<NumberSpace::Span>::iterator i, end;
	int64_t curstart, curend;
	bool curspan;

	if (coverSpans.size () <= 0) {
		return;
	}
	coverSpans.sort (NumberSpace::compareSpans);
	curspan = false;
	curstart = minBound;
	curend = minBound;
	i = coverSpans.begin ();
	end = coverSpans.end ();
	while (i != end) {
		if (! curspan) {
			curstart = i->start;
			curend = i->end;
			curspan = true;
		}
		else {
			if (i->start > (curend + 1)) {
				spans.push_back (NumberSpace::Span (curstart, curend));
				curstart = i->start;
				curend = i->end;
			}
			else if (i->end > curend) {
				curend = i->end;
			}
		}
		++i;
	}
	if (curspan) {
		spans.push_back (NumberSpace::Span (curstart, curend));
	}

	coverSpans.clear ();
	coverSpans.swap (spans);
}

bool NumberSpace::isCovered (int64_t targetValue) const {
	std::list<NumberSpace::Span>::const_iterator i, end;

	i = coverSpans.cbegin ();
	end = coverSpans.cend ();
	while (i != end) {
		if ((targetValue >= i->start) && (targetValue <= i->end)) {
			return (true);
		}
		++i;
	}
	return (false);
}

bool NumberSpace::isFullyCovered () const {
	std::list<NumberSpace::Span>::const_iterator i;

	if (coverSpans.size () == 1) {
		i = coverSpans.cbegin ();
		if ((i->start <= minBound) && (i->end >= maxBound)) {
			return (true);
		}
	}
	return (false);
}

int64_t NumberSpace::getUncoveredMin () const {
	std::list<NumberSpace::Span>::const_iterator i;
	int64_t result;

	result = minBound;
	if (coverSpans.size () > 0) {
		i = coverSpans.cbegin ();
		if (i->start <= minBound) {
			result = i->end + 1;
			if (result > maxBound) {
				result = maxBound;
			}
		}
	}
	return (result);
}

int64_t NumberSpace::getUncoveredMax () const {
	std::list<NumberSpace::Span>::const_reverse_iterator i;
	int64_t result;

	result = maxBound;
	if (coverSpans.size () > 0) {
		i = coverSpans.crbegin ();
		if (i->end >= maxBound) {
			result = i->start - 1;
			if (result < minBound) {
				result = minBound;
			}
		}
	}
	return (result);
}

void NumberSpace::getCoverSpans (std::list<NumberSpace::Span> *destList) const {
	std::list<NumberSpace::Span>::const_iterator i, end;

	destList->clear ();
	i = coverSpans.cbegin ();
	end = coverSpans.cend ();
	while (i != end) {
		destList->push_back (*i);
		++i;
	}
}
