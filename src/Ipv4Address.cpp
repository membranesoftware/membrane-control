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
#include <string.h>
#include <ctype.h>
#include "StdString.h"
#include "Ipv4Address.h"

Ipv4Address::Ipv4Address ()
: isValid (false)
, netmaskLength (-1)
{
	memset (octets, 0, sizeof (octets));
	memset (netmaskOctets, 0, sizeof (netmaskOctets));
}

Ipv4Address::~Ipv4Address () {

}

StdString Ipv4Address::toString () const {
	if (! isValid) {
		return (StdString (""));
	}
	return (StdString::createSprintf ("%i.%i.%i.%i", octets[0] & 0xFF, octets[1] & 0xFF, octets[2] & 0xFF, octets[3] & 0xFF));
}

bool Ipv4Address::parse (const StdString &text) {
	char *ptr, *end, *startpos, c;
	StdString s;
	int i;
	bool result;

	result = false;
	ptr = (char *) text.c_str ();
	end = ptr + text.length ();
	startpos = NULL;
	i = 0;
	while (ptr < end) {
		c = *ptr;
		if (! startpos) {
			if (! isdigit (c)) {
				break;
			}
			startpos = ptr;
		}
		else {
			if (c == '.') {
				s.assign (startpos, ptr - startpos);
				if (! s.parseInt (&octets[i])) {
					break;
				}
				startpos = NULL;
				if ((octets[i] < 0) || (octets[i] > 255)) {
					break;
				}
				++i;
				if (i >= 4) {
					break;
				}
			}
			else if (! isdigit (c)) {
				break;
			}
		}
		++ptr;
	}

	if (startpos && (i == 3)) {
		s.assign (startpos);
		if (s.parseInt (&octets[i])) {
			if ((octets[i] >= 0) && (octets[i] <= 255)) {
				result = true;
			}
		}
	}

	isValid = result;
	return (result);
}

bool Ipv4Address::parse (const char *text) {
	return (parse (StdString (text)));
}

void Ipv4Address::setAddress (uint32_t address) {
	octets[0] = (int) ((address >> 24) & 0xFF);
	octets[1] = (int) ((address >> 16) & 0xFF);
	octets[2] = (int) ((address >> 8) & 0xFF);
	octets[3] = (int) (address & 0xFF);
	isValid = true;
}

void Ipv4Address::setNetmask (uint32_t netmask) {
	int len, i;
	uint32_t mask;

	len = 0;
	mask = 0x80000000;
	while (true) {
		if (! (mask & netmask)) {
			break;
		}
		++len;
		if (mask == 0x00000001) {
			break;
		}
		mask >>= 1;
	}

	netmaskLength = len;
	memset (netmaskOctets, 0, sizeof (netmaskOctets));
	mask = 0x80;
	i = 0;
	while (len > 0) {
		if (i >= 4) {
			break;
		}
		netmaskOctets[i] |= mask;
		if (mask == 0x01) {
			mask = 0x80;
			++i;
		}
		else {
			mask >>= 1;
		}
		--len;
	}
}

StdString Ipv4Address::getNetmask () const {
	if ((! isValid) || (netmaskLength < 0)) {
		return (StdString (""));
	}
	return (StdString::createSprintf ("%i.%i.%i.%i", netmaskOctets[0] & 0xFF, netmaskOctets[1] & 0xFF, netmaskOctets[2] & 0xFF, netmaskOctets[3] & 0xFF));
}

StdString Ipv4Address::getBroadcastAddress () const {
	int result[4], i;

	if ((! isValid) || (netmaskLength < 0)) {
		return (StdString (""));
	}
	for (i = 0; i < 4; ++i) {
		result[i] = octets[i];
		result[i] &= netmaskOctets[i];
		result[i] |= ~(netmaskOctets[i]);
	}
	return (StdString::createSprintf ("%i.%i.%i.%i", result[0] & 0xFF, result[1] & 0xFF, result[2] & 0xFF, result[3] & 0xFF));
}

bool Ipv4Address::parseNetmask (const StdString &text) {
	Ipv4Address mask;
	uint32_t netmask;

	mask.parse (text);
	if (! mask.isValid) {
		return (false);
	}
	netmask = 0;
	netmask |= (mask.octets[0] & 0xFF); netmask <<= 8;
	netmask |= (mask.octets[1] & 0xFF); netmask <<= 8;
	netmask |= (mask.octets[2] & 0xFF); netmask <<= 8;
	netmask |= (mask.octets[3] & 0xFF);
	setNetmask (netmask);
	return (true);
}

bool Ipv4Address::parseNetmask (const char *text) {
	return (parseNetmask (StdString (text)));
}

bool Ipv4Address::isLocalhost () const {
	if (! isValid) {
		return (false);
	}
	return ((octets[0] == 127) && (octets[1] == 0) && (octets[2] == 0) && (octets[3] == 1));
}
