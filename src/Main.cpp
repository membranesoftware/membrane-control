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
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <map>
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "Resource.h"
#include "Util.h"
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
	Log *log;
	App *app;
	StdString val;
	int result, exitval;

	atexit (cleanup);

	log = Log::getInstance ();
	app = App::getInstance ();

	log->setLevelByName (Util::getEnvValue ("LOG_LEVEL", "ERR"));
	if (Util::getEnvValue ("LOG_STDERR", false)) {
		log->setStderrOutput (true);
	}
	val = Util::getEnvValue ("LOG_FILENAME", "");
	if (! val.empty ()) {
		log->setFileOutput (true, val);
	}

	val = Util::getEnvValue ("APPDATA_PATH", "");
	if (! val.empty ()) {
		app->prefsPath.assign (Util::getAppendPath (val, "membranecontrol.conf"));
		log->setFileOutput (true, Util::getAppendPath (val, "membranecontrol.log"));
	}
	else {
		val.assign ("");
#if PLATFORM_LINUX
		val = Util::getEnvValue ("HOME", "");
		if (! val.empty ()) {
			val = Util::getAppendPath (val, ".membrane");
		}
#endif
#if PLATFORM_MACOS
		val = Util::getEnvValue ("HOME", "");
		if (! val.empty ()) {
			val = Util::getAppendPath (val, "Library");
			val = Util::getAppendPath (val, "Application Support");
			val = Util::getAppendPath (val, "Membrane Control");
		}
#endif
#if PLATFORM_WINDOWS
		val = Util::getEnvValue ("LOCALAPPDATA", "");
		if (! val.empty ()) {
			val = Util::getAppendPath (val, "Membrane Control");
		}
#endif
		if (! val.empty ()) {
			result = Util::createDirectory (val);
			if (result != Result::SUCCESS) {
				Log::write (Log::WARNING, "Application data cannot be saved (failed to create directory); path=\"%s\" err=%i", val.c_str (), result);
			}
			else {
				app->prefsPath.assign (Util::getAppendPath (val, "membranecontrol.conf"));
				if (log->outputFilename.empty ()) {
					log->setFileOutput (true, Util::getAppendPath (val, "membranecontrol.log"));
				}
			}
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

	val = Util::getEnvValue ("RESOURCE_PATH", CONFIG_DEFAULT_RESOURCE_PATH);
	if (val.empty ()) {
		Log::write (Log::ERR, "Launch error: no resource path specified");
		exit (1);
	}
	app->resource.setSource (val);

	val = Util::getEnvValue ("MIN_DRAW_FRAME_DELAY", "");
	if ((! val.empty ()) && StdString::parseInt (val.c_str (), &result) && (result > 0)) {
		app->minDrawFrameDelay = result;
	}

	val = Util::getEnvValue ("MIN_UPDATE_FRAME_DELAY", "");
	if ((! val.empty ()) && StdString::parseInt (val.c_str (), &result) && (result > 0)) {
		app->minUpdateFrameDelay = result;
	}

	exitval = 0;
	result = app->run ();
	if (result != Result::SUCCESS) {
		printf ("Failed to execute application. For errors, see log file: %s\n", log->outputFilename.c_str ());
		exitval = 1;
	}

	exit (exitval);
}

void cleanup () {
	App::freeInstance ();
	Log::freeInstance ();
}

#if PLATFORM_LINUX || PLATFORM_MACOS
void sighandleExit (int signum) {
	App::getInstance ()->shutdown ();
}

void sighandleDiscard (int signum) {

}
#endif
