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
#include <stdarg.h>
#include <stdio.h>
#if PLATFORM_LINUX || PLATFORM_MACOS
#include <errno.h>
#include <unistd.h>
#endif
#if PLATFORM_WINDOWS
#include <io.h>
#include <direct.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "SDL2/SDL.h"
#include "App.h"
#include "StdString.h"
#include "OsUtil.h"
#include "Log.h"

const char *Log::LevelNames[Log::LevelCount] = {
	"ERR",
	"WARNING",
	"NOTICE",
	"INFO",
	"DEBUG",
	"DEBUG1",
	"DEBUG2",
	"DEBUG3",
	"DEBUG4"
};

Log::Log ()
: isStdoutWriteEnabled (false)
, isFileWriteEnabled (false)
, writeLevel (Log::ErrLevel)
, isFileErrorLogged (false)
, mutex (NULL)
{
	mutex = SDL_CreateMutex ();
}

Log::~Log () {
	if (mutex) {
		SDL_DestroyMutex (mutex);
		mutex = NULL;
	}
}

void Log::setLevel (Log::LogLevel level) {
	if ((level >= 0) && ((int) level < Log::LevelCount)) {
		writeLevel = level;
	}
}

OsUtil::Result Log::setLevelByName (const char *name) {
	StdString s;
	int i;
	OsUtil::Result result;

	result = OsUtil::InvalidParamError;
	s.assign (StdString (name).uppercased ());
	for (i = 0; i < Log::LevelCount; ++i) {
		if (s.equals (Log::LevelNames[i])) {
			setLevel ((Log::LogLevel) i);
			result = OsUtil::Success;
			break;
		}
	}
	return (result);
}

OsUtil::Result Log::setLevelByName (const StdString &name) {
	return (setLevelByName (name.c_str ()));
}

OsUtil::Result Log::openLogFile (const char *filename) {
	StdString fname;
	int fd;
#if PLATFORM_LINUX || PLATFORM_MACOS
	char *c, *cwd;
	int sz;
#endif

	fname.assign (filename);
	if (fname.empty ()) {
		isFileWriteEnabled = false;
		writeFilename.assign ("");
		return (OsUtil::Success);
	}
#if PLATFORM_LINUX || PLATFORM_MACOS
	if (filename[0] != '/') {
		c = NULL;
		cwd = NULL;
		sz = 0;
		do {
			sz += 1024;
			cwd = (char *) realloc (cwd, sz);
			c = getcwd (cwd, sz);
			if (c) {
				break;
			}
			if (errno != ERANGE) {
				::printf ("Failed to open log file %s - %s\n", filename, strerror (errno));
				break;
			}
		} while (sz < 8192);
		if (c && cwd) {
			fname.sprintf ("%s/%s", cwd, filename);
		}

		if (cwd) {
			free (cwd);
		}
		if (! c) {
			return (OsUtil::FileOpenFailedError);
		}
	}
#endif
#if PLATFORM_WINDOWS
	fd = _open (fname.c_str (), _O_APPEND | _O_WRONLY | _O_CREAT | _O_BINARY, _S_IREAD | _S_IWRITE);
#else
	fd = open (fname.c_str (), O_APPEND | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
#endif
	if (fd < 0) {
		::printf ("Failed to open log file %s - %s\n", fname.c_str (), strerror (errno));
		return (OsUtil::FileOpenFailedError);
	}
	close (fd);

	isFileWriteEnabled = true;
	writeFilename.assign (fname);
	return (OsUtil::Success);
}

OsUtil::Result Log::openLogFile (const StdString &filename) {
	return (openLogFile (filename.c_str ()));
}

void Log::voutput (Log::LogLevel level, const char *str, va_list args) {
	FILE *fp;
	int64_t now;
	va_list argscopy;
	StdString text;

	if (!(isStdoutWriteEnabled || isFileWriteEnabled)) {
		return;
	}

	if ((level < 0) || ((int) level >= Log::LevelCount)) {
		level = Log::NoLevel;
	}
	if (level != Log::NoLevel) {
		if (level > writeLevel) {
			return;
		}
	}

	now = OsUtil::getTime ();
	text.appendSprintf ("[%s]", OsUtil::getTimestampString (now, true).c_str ());
	if (level != Log::NoLevel) {
		text.appendSprintf ("[%s]", Log::LevelNames[level]);
	}
	text.append (" ");

	va_copy (argscopy, args);
	text.appendVsprintf (str, argscopy);
	va_end (argscopy);

	SDL_LockMutex (mutex);
	if (isStdoutWriteEnabled) {
		::printf ("%s%s", text.c_str (), CONFIG_NEWLINE);
	}
	if (isFileWriteEnabled) {
		fp = fopen (writeFilename.c_str (), "ab");
		if (! fp) {
			if (! isFileErrorLogged) {
				::printf ("Warning: could not open log file %s for writing - %s\n", writeFilename.c_str (), strerror (errno));
				isFileErrorLogged = true;
			}
		}
		else {
			fprintf (fp, "%s%s", text.c_str (), CONFIG_NEWLINE);
			fclose (fp);
		}
	}
	SDL_UnlockMutex (mutex);
}

void Log::write (Log::LogLevel level, const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (level, str, ap);
	va_end (ap);
}

void Log::write (Log::LogLevel level, const char *str, va_list args) {
	App::instance->log.voutput (level, str, args);
}

void Log::printf (const char *str, ...) {
	va_list ap;
	StdString text;

	va_start (ap, str);
	text.vsprintf (str, ap);
	va_end (ap);
	::printf ("[%s] %s\n", OsUtil::getTimestampString (OsUtil::getTime (), true).c_str (), text.c_str ());
	fflush (stdout);
	App::instance->writeConsoleOutput (text);
}

void Log::err (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::ErrLevel, str, ap);
	va_end (ap);
}

void Log::warning (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::WarningLevel, str, ap);
	va_end (ap);
}

void Log::notice (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::NoticeLevel, str, ap);
	va_end (ap);
}

void Log::info (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::InfoLevel, str, ap);
	va_end (ap);
}

void Log::debug (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::DebugLevel, str, ap);
	va_end (ap);
}

void Log::debug1 (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::Debug1Level, str, ap);
	va_end (ap);
}

void Log::debug2 (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::Debug2Level, str, ap);
	va_end (ap);
}

void Log::debug3 (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::Debug3Level, str, ap);
	va_end (ap);
}

void Log::debug4 (const char *str, ...) {
	va_list ap;

	va_start (ap, str);
	App::instance->log.voutput (Log::Debug4Level, str, ap);
	va_end (ap);
}
