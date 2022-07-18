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
// Class that handles logging functions

#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "OsUtil.h"

class Log {
public:
	enum LogLevel {
		ErrLevel = 0,
		WarningLevel = 1,
		NoticeLevel = 2,
		InfoLevel = 3,
		DebugLevel = 4,
		Debug1Level = 5,
		Debug2Level = 6,
		Debug3Level = 7,
		Debug4Level = 8,
		NoLevel = 255
	};
	enum {
		LevelCount = 9
	};

	Log ();
	~Log ();

	// Read-write data members
	bool isStdoutWriteEnabled;

	// Read-only data members
	bool isFileWriteEnabled;
	StdString writeFilename;
	int writeLevel;

	static const char *LevelNames[];

	// Set the log's level, causing it to write messages of that level and below
	void setLevel (Log::LogLevel level);

	// Set the log's level according to the value of the provided string. Returns a Result value.
	OsUtil::Result setLevelByName (const char *name);
	OsUtil::Result setLevelByName (const StdString &name);

	// Open the specified file for log message writing. If filename is empty, instead close any previously opened log file. Returns a Result value.
	OsUtil::Result openLogFile (const char *filename);
	OsUtil::Result openLogFile (const StdString &filename);

	// Write a message to the log using the provided va_list and args
	void voutput (Log::LogLevel level, const char *str, va_list args);

	// Write a message to the default log instance using the specified parameters
	static void write (Log::LogLevel level, const char *str, ...) __attribute__((format(printf, 2, 3)));
	static void write (Log::LogLevel level, const char *str, va_list args);

	// Write a message to the default log instance without specifying a level
	static void printf (const char *str, ...) __attribute__((format(printf, 1, 2)));

	// Write a message to the default log instance at the ERR level
	static void err (const char *str, ...) __attribute__((format(printf, 1, 2)));

	// Write a message to the default log instance at the WARNING level
	static void warning (const char *str, ...) __attribute__((format(printf, 1, 2)));

	// Write a message to the default log instance at the NOTICE level
	static void notice (const char *str, ...) __attribute__((format(printf, 1, 2)));

	// Write a message to the default log instance at the INFO level
	static void info (const char *str, ...) __attribute__((format(printf, 1, 2)));

	// Write a message to the default log instance at the DEBUG level
	static void debug (const char *str, ...) __attribute__((format(printf, 1, 2)));

	// Write a message to the default log instance at the DEBUG1 level
	static void debug1 (const char *str, ...) __attribute__((format(printf, 1, 2)));

	// Write a message to the default log instance at the DEBUG2 level
	static void debug2 (const char *str, ...) __attribute__((format(printf, 1, 2)));

	// Write a message to the default log instance at the DEBUG3 level
	static void debug3 (const char *str, ...) __attribute__((format(printf, 1, 2)));

	// Write a message to the default log instance at the DEBUG4 level
	static void debug4 (const char *str, ...) __attribute__((format(printf, 1, 2)));

protected:
	bool isFileErrorLogged;
	SDL_mutex *mutex;
};

#endif
