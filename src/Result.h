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
		Success = 0,
		InvalidParamError = -1,
		FileOpenFailedError = -2,
		ThreadCreateFailedError = -3,
		MalformedResponseError = -4,
		FileOperationFailedError = -5,
		MalformedDataError = -6,
		OutOfMemoryError = -7,
		FreetypeOperationFailedError = -8,
		SocketOperationFailedError = -9,
		SocketNotConnectedError = -10,
		HttpOperationFailedError = -11,
		MoreDataRequiredError = -12,
		JsonParseFailedError = -13,
		SystemOperationFailedError = -14,
		KeyNotFoundError = -15,
		MismatchedTypeError = -16,
		SdlOperationFailedError = -17,
		LibmicrohttpdOperationFailedError = -18,
		ArrayIndexOutOfBoundsError = -19,
		HttpRequestFailedError = -20,
		DuplicateIdError = -21,
		InvalidConfigurationError = -22,
		UnknownHostnameError = -23,
		NotImplementedError = -24,
		AlreadyLoadedError = -25,
		InternalApplicationFailureError = -26,
		UnknownProtocolError = -27,
		LibcurlOperationFailedError = -28,
		UnknownMethodError = -29,
		ApplicationNotInstalledError = -30,
		ProgramNotFoundError = -31,
		UnauthorizedError = -32
	};
};

#endif
