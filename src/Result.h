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
// Constants to use for result values

#ifndef RESULT_H
#define RESULT_H

class Result {
public:
	enum {
		SUCCESS = 0,
		ERROR_INVALID_PARAM = -1,
		ERROR_FILE_OPEN_FAILED = -2,
		ERROR_THREAD_CREATE_FAILED = -3,
		ERROR_MALFORMED_RESPONSE = -4,
		ERROR_FILE_OPERATION_FAILED = -5,
		ERROR_MALFORMED_DATA = -6,
		ERROR_OUT_OF_MEMORY = -7,
		ERROR_FREETYPE_OPERATION_FAILED = -8,
		ERROR_SOCKET_OPERATION_FAILED = -9,
		ERROR_SOCKET_NOT_CONNECTED = -10,
		ERROR_HTTP_OPERATION_FAILED = -11,
		ERROR_MORE_DATA_REQUIRED = -12,
		ERROR_JSON_PARSE_FAILED = -13,
		ERROR_SYSTEM_OPERATION_FAILED = -14,
		ERROR_KEY_NOT_FOUND = -15,
		ERROR_MISMATCHED_TYPE = -16,
		ERROR_SDL_OPERATION_FAILED = -17,
		ERROR_LIBMICROHTTPD_OPERATION_FAILED = -18,
		ERROR_ARRAY_INDEX_OUT_OF_BOUNDS = -19,
		ERROR_HTTP_REQUEST_FAILED = -20,
		ERROR_DUPLICATE_ID = -21,
		ERROR_INVALID_CONFIGURATION = -22,
		ERROR_UNKNOWN_HOSTNAME = -23,
		ERROR_NOT_IMPLEMENTED = -24,
		ERROR_ALREADY_LOADED = -25,
		ERROR_INTERNAL_APPLICATION_FAILURE = -26,
		ERROR_UNKNOWN_PROTOCOL = -27,
		ERROR_LIBCURL_OPERATION_FAILED = -28,
		ERROR_UNKNOWN_METHOD = -29,
		ERROR_APPLICATION_NOT_INSTALLED = -30,
		ERROR_PROGRAM_NOT_FOUND = -31,
		ERROR_UNAUTHORIZED = -32
	};
};

#endif
