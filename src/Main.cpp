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
#include <stdio.h>
#include <signal.h>
#include <string.h>
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
	int result, exitstatus;

	atexit (cleanup);

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

	exitstatus = 0;
	App::createInstance ();
	result = App::instance->run ();
	if (result != OsUtil::Success) {
		if (App::instance->log.isFileWriteEnabled) {
			printf ("Failed to execute application. Visit membranesoftware.com for help or an updated version. Errors written to log file: %s\n", App::instance->log.writeFilename.c_str ());
		}
		else {
			printf ("Failed to execute application. Visit membranesoftware.com for help or an updated version. error=%i\n", result);
		}
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
