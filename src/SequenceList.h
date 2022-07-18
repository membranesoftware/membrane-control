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
// List template providing functions for repeated traversal in natural or randomized order (extends std::vector<T>)

#ifndef SEQUENCE_LIST_H
#define SEQUENCE_LIST_H

#include <vector>
#include "Prng.h"

template<class T> class SequenceList : public std::vector<T> {
public:
	SequenceList<T> (): std::vector<T> (), nextItemIndex (-1), sequenceSize (0), prng (NULL), lastItemIndex (-1) { }
	~SequenceList<T> () { }

	// Read-write data members
	int nextItemIndex;

	// Set the list to return items in a randomized order, as generated using the provided Prng
	void randomizeOrder (Prng *p) {
		prng = p;
	}

	// Get the next item in the list's sequence and write it to the provided pointer. Returns a boolean value indicating if an item was found.
	bool next (T *destItem) {
		int i, count, pos, itemindex;

		count = (int) std::vector<T>::size ();
		if (count > sequenceSize) {
			for (i = sequenceSize; i < count; ++i) {
				if (prng) {
					sequenceList.insert (sequenceList.begin () + prng->getRandomValue (0, (int) sequenceList.size ()), i);
				}
				else {
					sequenceList.push_back (i);
				}
			}
			sequenceSize = count;
		}
		else if (count < sequenceSize) {
			sequenceSize = count;
			i = 0;
			while (i < (int) sequenceList.size ()) {
				if (sequenceList.at (i) >= sequenceSize) {
					sequenceList.erase (sequenceList.begin () + i);
				}
				else {
					++i;
				}
			}
		}

		if (sequenceList.empty ()) {
			if (count <= 0) {
				return (false);
			}

			for (i = 0; i < count; ++i) {
				if (prng) {
					sequenceList.insert (sequenceList.begin () + prng->getRandomValue (0, i), i);
				}
				else {
					sequenceList.push_back (i);
				}
			}

			if (prng && (count > 1) && (lastItemIndex >= 0)) {
				i = sequenceList.at (0);
				if (i == lastItemIndex) {
					pos = prng->getRandomValue (1, ((int) sequenceList.size ()) - 1);
					sequenceList.at (0) = sequenceList.at (pos);
					sequenceList.at (pos) = i;
				}
			}
		}

		if ((nextItemIndex >= 0) && (nextItemIndex < count)) {
			itemindex = nextItemIndex;
			i = 0;
			while (i < (int) sequenceList.size ()) {
				if (sequenceList.at (i) == itemindex) {
					sequenceList.erase (sequenceList.begin () + i);
					break;
				}
				++i;
			}
		}
		else {
			itemindex = sequenceList.at (0);
			sequenceList.erase (sequenceList.begin ());
		}
		nextItemIndex = -1;

		if (destItem) {
			*destItem = std::vector<T>::at (itemindex);
		}
		lastItemIndex = itemindex;
		return (true);
	}

private:
	std::vector<int> sequenceList;
	int sequenceSize;
	Prng *prng;
	int lastItemIndex;
};

#endif
