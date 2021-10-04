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
// Class for parsing and manipulating IPv4 addresses

#ifndef IPV4_ADDRESS_H
#define IPV4_ADDRESS_H

#include <stdint.h>
#include "StdString.h"

class Ipv4Address {
public:
	Ipv4Address ();
	~Ipv4Address ();

	// Read-only data members
	bool isValid;
	int octets[4];
	int netmaskOctets[4];
	int netmaskLength;

	// Return a string representation of the stored address, or an empty string if the address does not contain valid data
	StdString toString () const;

	// Return a string representation of the broadcast address associated with stored data, or an empty string if the broadcast address could not be determined
	StdString getBroadcastAddress () const;

	// Return a string representation of the netmask address associated with stored data, or an empty string if the netmask could not be determined
	StdString getNetmask () const;

	// Parse the specified text string and store the resulting address values, while also setting the isValid data member. Returns a boolean value indicating if the parse was successful.
	bool parse (const StdString &text);
	bool parse (const char *text);

	// Set the address held by this object, provided in network byte order
	void setAddress (uint32_t address);

	// Set the netmask value associated with the address, provided in network byte order
	void setNetmask (uint32_t netmask);

	// Parse the specified text string and set the netmask to the resulting address value. Returns a boolean value indicating if the parse was successful.
	bool parseNetmask (const StdString &text);
	bool parseNetmask (const char *text);

	// Return a boolean value indicating if the address holds a localhost value
	bool isLocalhost () const;
};

#endif
