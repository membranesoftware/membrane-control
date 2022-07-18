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
#include <stdint.h>
#include <string.h>
#include "Prng.h"

// Return a uint32_t resulting from performing a left circular shift on the provided value
static uint32_t lshift (uint32_t value, int shift);

// Return a uint32_t resulting from performing a right circular shift on the provided value
static uint32_t rshift (uint32_t value, int shift);

Prng::Prng ()
: sequenceIndex (0)
{
	memset (&MT, 0, sizeof (MT));
}

Prng::~Prng () {

}

void Prng::seed (uint32_t seedValue) {
	int i;
	uint32_t *a;
	uint64_t val;

	a = MT;
	a[0] = seedValue;
	for (i = 1; i < Prng::MersenneTwisterN; ++i) {
		val = 0x6C078965;
		val *= (a[i - 1] ^ rshift (a[i - 1], 30));
		val += i;
		a[i] = (uint32_t) (val & 0xFFFFFFFF);
	}
}

int Prng::getRandomValue (int i1, int i2) {
	int64_t diff, i;

	diff = (int64_t) i2;
	diff -= i1;
	diff += 1;
	i = i1;
	if (diff > 1) {
		i += llabs (extract ()) % diff;
	}
	return ((int) i);
}

float Prng::getRandomValue (float f1, float f2) {
	float diff, fraction;

	diff = f2 - f1;
	if (diff < CONFIG_FLOAT_EPSILON) {
		return (f1);
	}

	fraction = (float) getRandomValue (0, RAND_MAX - 1);
	fraction /= (float) (RAND_MAX - 1);
	return (f1 + (diff * fraction));
}

int Prng::extract () {
	uint32_t *a, y;
	int i;

	a = MT;
	if (sequenceIndex == 0) {
		for (i = 0; i < Prng::MersenneTwisterN; ++i) {
			y = a[i] & 0x80000000;
			y += (a[(i + 1) % Prng::MersenneTwisterN] & 0x7FFFFFFF);
			a[i] = a[(i + 397) % Prng::MersenneTwisterN] ^ (rshift (y, 1));
			if (y % 2) {
				a[i] ^= 0x9908B0DF;
			}
		}
	}

	y = a[sequenceIndex];
	y ^= rshift (y, 11);
	y ^= (lshift (y, 7)) & 0x9D2C5680;
	y ^= (lshift (y, 15)) & 0xEFC60000;
	y ^= rshift (y, 18);

	++sequenceIndex;
	if (sequenceIndex > (Prng::MersenneTwisterN - 1)) {
		sequenceIndex = 0;
	}

	return ((int) y);
}

uint32_t lshift (uint32_t value, int shift) {
	shift &= 0x1F;
	if (! shift) {
		return (value);
	}
	return ((value << shift) | (value >> (32 - shift)));
}

uint32_t rshift (uint32_t value, int shift) {
	shift &= 0x1F;
	if (! shift) {
		return (value);
	}
	return ((value >> shift) | (value << (32 - shift)));
}
