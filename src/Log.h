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
// Class that handles logging functions

#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include "SDL2/SDL.h"
#include "StdString.h"

class Log {
public:
	// Constants to use for log levels
	enum {
		ERR = 0,
		WARNING = 1,
		NOTICE = 2,
		INFO = 3,
		DEBUG = 4,
		DEBUG1 = 5,
		DEBUG2 = 6,
		DEBUG3 = 7,
		DEBUG4 = 8,
		NUM_LEVELS = 9,
		NO_LEVEL = 255
	};

	Log ();
	~Log ();

	// Read-only data members
	int writeLevel;
	StdString outputFilename;

	static const char *levelNames[];

	// Return the singleton instance of this class, creating it if necessary
	static Log *getInstance ();

	// Free the singleton instance of this class
	static void freeInstance ();

	// Set the log's level, causing it to write messages of that level and below
	void setLevel (int level);

	// Set the log's level according to the value of the provided string. Returns Result::SUCCESS or an error value.
	int setLevelByName (const char *name);
	int setLevelByName (const StdString &name);

	// Set the log's stderr output option. If enabled, log output is written to stderr.
	void setStderrOutput (bool enable);

	// Set the log's file output option. If enabled, log output is written to the specified filename. Returns Result::SUCCESS or an error value.
	int setFileOutput (bool enable, const char *filename);
	int setFileOutput (bool enable, const StdString &filename);

	// Write a message to the default log instance using the specified parameters
	static void write (int level, const char *str, ...) __attribute__((format(printf, 2, 3)));
	static void write (int level, const char *str, va_list args);

	// Write a message to the default log instance using the default level
	static void printf (const char *str, ...) __attribute__((format(printf, 1, 2)));

	// Write a message to the log using the provided va_list and args
	void voutput (int level, const char *str, va_list args);

protected:
	bool isStderrOutputEnabled;
	bool isFileOutputEnabled;
	bool isFileErrorLogged;
	SDL_mutex *mutex;
};

#endif
