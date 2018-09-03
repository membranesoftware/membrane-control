/*
* Copyright 2018 Membrane Software <author@membranesoftware.com>
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
#include "Result.h"
#include "StdString.h"
#include "Util.h"
#include "Log.h"

static Log *logInstance = NULL;
const char *Log::levelNames[Log::NUM_LEVELS] = { "ERR", "WARNING", "NOTICE", "INFO", "DEBUG", "DEBUG1", "DEBUG2", "DEBUG3", "DEBUG4" };

Log::Log ()
: writeLevel (Log::ERR)
, outputFilename ("")
, isStderrOutputEnabled (false)
, isFileOutputEnabled (false)
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

Log *Log::getInstance () {
	if (! logInstance) {
		logInstance = new Log ();
	}

	return (logInstance);
}

void Log::freeInstance () {
	if (logInstance) {
		delete (logInstance);
		logInstance = NULL;
	}
}

void Log::setLevel (int level) {
	if ((level >= 0) && (level < Log::NUM_LEVELS)) {
		writeLevel = level;
	}
}

int Log::setLevelByName (const char *name) {
	int rval, i;

	rval = Result::ERROR_INVALID_PARAM;
	for (i = 0; i < Log::NUM_LEVELS; ++i) {
		if (! strcmp (name, Log::levelNames[i])) {
			setLevel (i);
			rval = Result::SUCCESS;
			break;
		}
	}

	return (rval);
}

int Log::setLevelByName (const StdString &name) {
	return (setLevelByName (name.c_str ()));
}

void Log::setStderrOutput (bool enable) {
	isStderrOutputEnabled = enable;
}

int Log::setFileOutput (bool enable, const char *filename) {
	StdString fname;
	int fd;
#if PLATFORM_LINUX || PLATFORM_MACOS
	char *c, *cwd;
	int sz;
#endif

	if (! enable) {
		isFileOutputEnabled = false;
		outputFilename.assign ("");
		return (Result::SUCCESS);
	}

	fname.assign (filename);
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
				fprintf (stderr, "Failed to open log file %s - %s\n", filename, strerror (errno));
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
			return (Result::ERROR_FILE_OPEN_FAILED);
		}
	}
#endif

#if PLATFORM_WINDOWS
	fd = open (fname.c_str (), O_APPEND | OPEN_CREATE_FLAGS);
#else
	fd = open (fname.c_str (), O_APPEND | OPEN_CREATE_FLAGS, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
#endif
	if (fd < 0) {
		fprintf (stderr, "Failed to open log file %s - %s\n", fname.c_str (), strerror (errno));
		return (Result::ERROR_FILE_OPEN_FAILED);
	}
	close (fd);

	isFileOutputEnabled = true;
	outputFilename.assign (fname);

	return (Result::SUCCESS);
}

int Log::setFileOutput (bool enable, const StdString &filename) {
	return (setFileOutput (enable, filename.c_str ()));
}

void Log::write (int level, const char *str, ...) {
	Log *log;
	va_list ap;

	log = Log::getInstance ();
	va_start (ap, str);
	log->voutput (level, str, ap);
	va_end (ap);
}

void Log::write (int level, const char *str, va_list args) {
	Log *log;

	log = Log::getInstance ();
	log->voutput (level, str, args);
}

void Log::voutput (int level, const char *str, va_list args) {
	FILE *fp;
	int64_t now;
	va_list argscopy;
	StdString text;

	if (!(isStderrOutputEnabled || isFileOutputEnabled)) {
		return;
	}

	if ((level < 0) || (level >= Log::NUM_LEVELS)) {
		level = Log::NO_LEVEL;
	}

	if (level != Log::NO_LEVEL) {
		if (level > writeLevel) {
			return;
		}
	}

	now = Util::getTime ();

	text.appendSprintf ("[%s]", Util::getTimestampString (now, true).c_str ());
	if (level != Log::NO_LEVEL) {
		text.appendSprintf ("[%s]", Log::levelNames[level]);
	}
	text.append (" ");

	va_copy (argscopy, args);
	text.appendVsprintf (str, argscopy);
	va_end (argscopy);

	SDL_LockMutex (mutex);
	if (isStderrOutputEnabled) {
		fprintf (stderr, "%s%s", text.c_str (), CONFIG_NEWLINE);
	}

	if (isFileOutputEnabled) {
		fp = fopen (outputFilename.c_str (), "ab");
		if (! fp) {
			if (! isFileErrorLogged) {
				fprintf (stderr, "Warning: could not open log file %s for writing - %s\n", outputFilename.c_str (), strerror (errno));
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

void Log::printf (const char *str, ...) {
	Log *log;
	va_list ap;

	log = Log::getInstance ();
	va_start (ap, str);
	log->voutput (Log::NO_LEVEL, str, ap);
	va_end (ap);
}
