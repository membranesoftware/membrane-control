/*
* Copyright 2018-2020 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <map>
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "OsUtil.h"
#include "App.h"

// Execute operations appropriate when the process is about to exit
static void cleanup ();

#if PLATFORM_LINUX || PLATFORM_MACOS
// Handle a signal by halting the application
static void sighandleExit (int signum);

// Handle a signal by taking no action
static void sighandleDiscard (int signum);
#endif

#if PLATFORM_WINDOWS
int CALLBACK WinMain (_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
#else
int main (int argc, char **argv)
#endif
{
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct sigaction action;
#endif
	StdString logfilename, path;
	int result, exitstatus;
	bool shouldcreate;

	atexit (cleanup);
	App::createInstance ();

	App::instance->log.setLevelByName (OsUtil::getEnvValue ("LOG_LEVEL", "ERR"));
	if (OsUtil::getEnvValue ("LOG_STDERR", false)) {
		App::instance->log.setStderrOutput (true);
	}
	logfilename = OsUtil::getEnvValue ("LOG_FILENAME", "");
	if (! logfilename.empty ()) {
		App::instance->log.setFileOutput (true, logfilename);
	}

	shouldcreate = false;
	path = OsUtil::getEnvValue ("APPDATA_PATH", "");
	if (path.empty ()) {
		shouldcreate = true;
#if PLATFORM_LINUX
		path = OsUtil::getEnvValue ("HOME", "");
		if (! path.empty ()) {
			path = OsUtil::getAppendPath (path, CONFIG_APPDATA_DIRNAME);
		}
#endif
#if PLATFORM_MACOS
		path = OsUtil::getEnvValue ("HOME", "");
		if (! path.empty ()) {
			path = OsUtil::getAppendPath (path, "Library");
			path = OsUtil::getAppendPath (path, "Application Support");
			path = OsUtil::getAppendPath (path, CONFIG_APPDATA_DIRNAME);
		}
#endif
#if PLATFORM_WINDOWS
		path = OsUtil::getEnvValue ("LOCALAPPDATA", "");
		if (! path.empty ()) {
			path = OsUtil::getAppendPath (path, CONFIG_APPDATA_DIRNAME);
		}
#endif
	}
	if (path.empty ()) {
		Log::warning ("Application data cannot be saved (set APPDATA_PATH to specify destination directory)");
	}
	else {
		if (shouldcreate) {
			result = OsUtil::createDirectory (path);
			if (result != Result::Success) {
				Log::warning ("Application data cannot be saved (failed to create directory); path=\"%s\" err=%i", path.c_str (), result);
			}
		}
		App::instance->prefsPath.assign (OsUtil::getAppendPath (path, CONFIG_PREFS_FILENAME));
		if (logfilename.empty ()) {
			App::instance->log.setFileOutput (true, OsUtil::getAppendPath (path, CONFIG_LOG_FILENAME));
		}
	}

#if PLATFORM_LINUX || PLATFORM_MACOS
	memset (&action, 0, sizeof (action));
	action.sa_handler = sighandleExit;
	sigemptyset (&(action.sa_mask));
	action.sa_flags = 0;
	sigaction (SIGINT, &action, NULL);

	memset (&action, 0, sizeof (action));
	action.sa_handler = sighandleExit;
	sigemptyset (&(action.sa_mask));
	action.sa_flags = 0;
	sigaction (SIGQUIT, &action, NULL);

	memset (&action, 0, sizeof (action));
	action.sa_handler = sighandleExit;
	sigemptyset (&(action.sa_mask));
	action.sa_flags = 0;
	sigaction (SIGTERM, &action, NULL);

	memset (&action, 0, sizeof (action));
	action.sa_handler = sighandleDiscard;
	sigemptyset (&(action.sa_mask));
	action.sa_flags = SA_RESTART;
	sigaction (SIGPIPE, &action, NULL);
#endif

	path = OsUtil::getEnvValue ("RESOURCE_PATH", CONFIG_DEFAULT_RESOURCE_PATH);
	if (path.empty ()) {
		Log::err ("Application resources not found (set RESOURCE_PATH to specify source path)");
		exit (1);
	}
	App::instance->resource.setSource (path);

	App::instance->minDrawFrameDelay = OsUtil::getEnvValue ("MIN_DRAW_FRAME_DELAY", 0);
	App::instance->minUpdateFrameDelay = OsUtil::getEnvValue ("MIN_UPDATE_FRAME_DELAY", 0);
	App::instance->windowWidth = OsUtil::getEnvValue ("WINDOW_WIDTH", 0);
	App::instance->windowHeight = OsUtil::getEnvValue ("WINDOW_HEIGHT", 0);

	exitstatus = 0;
	result = App::instance->run ();
	if (result != Result::Success) {
		printf ("Failed to execute application. For errors, see log file: %s\n", App::instance->log.outputFilename.c_str ());
		exitstatus = 1;
	}
	exit (exitstatus);
}

void cleanup () {
	App::freeInstance ();
}

#if PLATFORM_LINUX || PLATFORM_MACOS
void sighandleExit (int signum) {
	App::instance->shutdown ();
}

void sighandleDiscard (int signum) {

}
#endif
