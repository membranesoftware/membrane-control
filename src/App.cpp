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
#include <math.h>
#if PLATFORM_LINUX || PLATFORM_MACOS
#include <unistd.h>
#endif
#if PLATFORM_MACOS
#include <mach-o/dyld.h>
#include <libgen.h>
#endif
#if PLATFORM_WINDOWS
#include <io.h>
#endif
#include <vector>
#include <stack>
#include <iostream>
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "StdString.h"
#include "Log.h"
#include "TaskGroup.h"
#include "LuaScript.h"
#include "OsUtil.h"
#include "MathUtil.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "SystemInterface.h"
#include "AgentControl.h"
#include "RecordStore.h"
#include "CommandHistory.h"
#include "CommandListener.h"
#include "Input.h"
#include "Resource.h"
#include "Network.h"
#include "Panel.h"
#include "ConsoleWindow.h"
#include "Toolbar.h"
#include "SnackbarWindow.h"
#include "LogoWindow.h"
#include "HyperlinkWindow.h"
#include "MainUi.h"
#include "App.h"

const int App::DefaultMinFrameDelay = 20;
const int App::WindowWidths[] = { 768, 1024, 1280, 1600, 1920 };
const int App::WindowHeights[] = { 432, 576, 720, 900, 1080 };
const int App::WindowSizeCount = 5;
const float App::FontScales[] = { 0.66f, 0.8f, 1.0f, 1.25f, 1.5f };
const int App::FontScaleCount = 5;
const int App::MaxCornerRadius = 16;
const StdString App::ServerUrl = StdString ("https://membranesoftware.com/");

const char *App::NetworkThreadsKey = "NetworkThreads";
const char *App::WindowWidthKey = "WindowWidth";
const char *App::WindowHeightKey = "WindowHeight";
const char *App::FontScaleKey = "FontScale";
const char *App::HttpsKey = "Https";
const char *App::ShowInterfaceAnimationsKey = "ShowInterfaceAnimations";
const char *App::ShowClockKey = "ShowClock";
const char *App::IsFirstLaunchCompleteKey = "IsFirstLaunchComplete";

App *App::instance = NULL;

void App::createInstance (bool shouldSkipInit) {
	if (App::instance) {
		delete (App::instance);
	}
	App::instance = new App ();
	Input::instance = &(App::instance->input);
	Resource::instance = &(App::instance->resource);
	Network::instance = &(App::instance->network);
	UiConfiguration::instance = &(App::instance->uiConfig);
	UiText::instance = &(App::instance->uiText);
	TaskGroup::instance = &(App::instance->taskGroup);
	SystemInterface::instance = &(App::instance->systemInterface);
	AgentControl::instance = &(App::instance->agentControl);
	RecordStore::instance = &(App::instance->recordStore);
	CommandHistory::instance = &(App::instance->commandHistory);
	CommandListener::instance = &(App::instance->commandListener);

	if (! shouldSkipInit) {
		App::instance->init ();
	}
}

void App::freeInstance () {
	if (App::instance) {
		delete (App::instance);
		App::instance = NULL;
		Input::instance = NULL;
		Resource::instance = NULL;
		Network::instance = NULL;
		UiConfiguration::instance = NULL;
		UiText::instance = NULL;
		TaskGroup::instance = NULL;
		SystemInterface::instance = NULL;
		AgentControl::instance = NULL;
		RecordStore::instance = NULL;
		CommandHistory::instance = NULL;
		CommandListener::instance = NULL;
		IMG_Quit ();
		SDL_Quit ();
	}
}

App::App ()
: nextFontScale (1.0f)
, nextWindowWidth (0)
, nextWindowHeight (0)
, isConsole (false)
, shouldRefreshUi (false)
, isInterfaceAnimationEnabled (false)
, shouldSyncRecordStore (false)
, isMainToolbarClockEnabled (false)
, isNetworkActive (false)
, isShuttingDown (false)
, isShutdown (false)
, startTime (0)
, isHttpsEnabled (false)
, window (NULL)
, render (NULL)
, isTextureRenderEnabled (false)
, rootPanel (NULL)
, displayDdpi (0.0f)
, displayHdpi (0.0f)
, displayVdpi (0.0f)
, windowWidth (0)
, windowHeight (0)
, minDrawFrameDelay (0)
, minUpdateFrameDelay (0)
, fontScale (1.0f)
, imageScale (0)
, drawCount (0)
, updateCount (0)
, isPrefsWriteDisabled (false)
, updateThread (NULL)
, uniqueIdMutex (NULL)
, nextUniqueId (1)
, prefsMapMutex (NULL)
, roundedCornerSprite (NULL)
, renderTaskMutex (NULL)
, isSuspendingUpdate (false)
, updateMutex (NULL)
, updateCond (NULL)
, consoleWindow (NULL)
, consoleWindowMutex (NULL)
, backgroundMutex (NULL)
, backgroundTexture (NULL)
, backgroundTextureWidth (0)
, backgroundTextureHeight (0)
, nextBackgroundTexture (NULL)
, nextBackgroundTextureWidth (0)
, nextBackgroundTextureHeight (0)
, backgroundCrossFadeAlpha (0.0f)
{
	uniqueIdMutex = SDL_CreateMutex ();
	prefsMapMutex = SDL_CreateMutex ();
	renderTaskMutex = SDL_CreateMutex ();
	updateMutex = SDL_CreateMutex ();
	updateCond = SDL_CreateCond ();
	consoleWindowMutex = SDL_CreateMutex ();
	backgroundMutex = SDL_CreateMutex ();
}

App::~App () {
	if (rootPanel) {
		rootPanel->release ();
		rootPanel = NULL;
	}
	if (roundedCornerSprite) {
		delete (roundedCornerSprite);
		roundedCornerSprite = NULL;
	}
	if (uniqueIdMutex) {
		SDL_DestroyMutex (uniqueIdMutex);
		uniqueIdMutex = NULL;
	}
	if (prefsMapMutex) {
		SDL_DestroyMutex (prefsMapMutex);
		prefsMapMutex = NULL;
	}
	if (renderTaskMutex) {
		SDL_DestroyMutex (renderTaskMutex);
		renderTaskMutex = NULL;
	}
	if (updateMutex) {
		SDL_DestroyMutex (updateMutex);
		updateMutex = NULL;
	}
	if (updateCond) {
		SDL_DestroyCond (updateCond);
		updateCond = NULL;
	}
	if (consoleWindow) {
		consoleWindow->release ();
		consoleWindow = NULL;
	}
	if (consoleWindowMutex) {
		SDL_DestroyMutex (consoleWindowMutex);
		consoleWindowMutex = NULL;
	}
	if (backgroundMutex) {
		SDL_DestroyMutex (backgroundMutex);
		backgroundMutex = NULL;
	}
}

void App::init () {
	StdString path;
	OsUtil::Result result;
#if PLATFORM_MACOS
	char exepath[4096], dirpath[4096];
	uint32_t sz;
#endif

	log.setLevelByName (OsUtil::getEnvValue ("LOG_LEVEL", "ERR"));
	if (OsUtil::getEnvValue ("LOG_CONSOLE", false)) {
		log.isStdoutWriteEnabled = true;
	}
	path = OsUtil::getEnvValue ("LOG_FILENAME", "");
	if (! path.empty ()) {
		log.openLogFile (path);
	}

#if PLATFORM_MACOS
	memset (exepath, 0, sizeof (exepath));
	sz = sizeof (exepath);
	if (_NSGetExecutablePath (exepath, &sz) != 0) {
		memset (exepath, 0, sizeof (exepath));
		Log::warning ("Failed to determine executable path");
	}
	else {
		memset (dirpath, 0, sizeof (dirpath));
		if (chdir (dirname_r (exepath, dirpath)) != 0) {
			Log::warning ("Failed to change working directory");
		}
	}
#endif

	path = OsUtil::getEnvValue ("RESOURCE_PATH", "");
	if (path.empty ()) {
		path.sprintf ("%s.dat", APPLICATION_PACKAGE_NAME);
#if PLATFORM_MACOS
		if (exepath[0] != '\0') {
			memset (dirpath, 0, sizeof (dirpath));
			path.sprintf ("%s/%s.dat", dirname_r (exepath, dirpath), APPLICATION_PACKAGE_NAME);
		}
#endif
	}
	resource.setSource (path);

	path = OsUtil::getEnvValue ("APPDATA_PATH", "");
	if (path.empty ()) {
		path = OsUtil::getUserDataPath ();
		if (! path.empty ()) {
			result = OsUtil::createDirectory (path);
			if (result != OsUtil::Success) {
				Log::warning ("Application data cannot be saved (failed to create directory); path=\"%s\" err=%i", path.c_str (), result);
			}
		}
	}
	if (path.empty ()) {
		Log::warning ("Application data cannot be saved (set APPDATA_PATH to specify destination directory)");
	}
	else {
		prefsPath.assign (OsUtil::getAppendPath (path, StdString::createSprintf ("%s.conf", APPLICATION_PACKAGE_NAME)));
		if (! log.isFileWriteEnabled) {
			log.openLogFile (OsUtil::getAppendPath (path, StdString::createSprintf ("%s.log", APPLICATION_PACKAGE_NAME)));
		}
	}

	isConsole = OsUtil::getEnvValue ("CONSOLE", false);
	minDrawFrameDelay = OsUtil::getEnvValue ("MIN_DRAW_FRAME_DELAY", 0);
	minUpdateFrameDelay = OsUtil::getEnvValue ("MIN_UPDATE_FRAME_DELAY", 0);
	windowWidth = OsUtil::getEnvValue ("WINDOW_WIDTH", 0);
	windowHeight = OsUtil::getEnvValue ("WINDOW_HEIGHT", 0);
}

int App::getImageScale (int w, int h) {
	int i, result;

	if ((w <= 0) || (h <= 0)) {
		return (-1);
	}
	result = 0;
	for (i = 0; i < App::WindowSizeCount; ++i) {
		if (w >= App::WindowWidths[i]) {
			result = i;
		}
	}
	return (result);
}

int App::run () {
	int result;

	startTime = OsUtil::getTime ();
	if (minDrawFrameDelay <= 0) {
		minDrawFrameDelay = App::DefaultMinFrameDelay;
	}
	if (minUpdateFrameDelay <= 0) {
		minUpdateFrameDelay = App::DefaultMinFrameDelay;
	}
	prng.seed ((uint32_t) (OsUtil::getTime () & 0xFFFFFFFF));
	prefsMap.clear ();
	if (prefsPath.empty ()) {
		isPrefsWriteDisabled = true;
	}
	else {
		result = prefsMap.read (prefsPath, true);
		if (result != OsUtil::Success) {
			Log::debug ("Failed to read preferences file; prefsPath=\"%s\" err=%i", prefsPath.c_str (), result);
			prefsMap.clear ();
		}
	}
	isHttpsEnabled = prefsMap.find (App::HttpsKey, true);

	result = resource.open ();
	if (result != OsUtil::Success) {
		Log::err ("Failed to open application resources; err=%i", result);
		return (result);
	}
	result = uiText.load (OsUtil::getEnvLanguage (UiText::DefaultLanguage));
	if (result != OsUtil::Success) {
		Log::err ("Failed to load text resources; err=%i", result);
		return (result);
	}
	network.maxRequestThreads = prefsMap.find (App::NetworkThreadsKey, Network::DefaultMaxRequestThreads);
	network.httpUserAgent.sprintf ("Membrane Control/%s_%s", BUILD_ID, PLATFORM_ID);
	network.datagramCallback = Network::DatagramCallbackContext (App::datagramReceived, NULL);
	network.enableDatagramSocket = true;
	result = network.start ();
	if (result != OsUtil::Success) {
		Log::err ("Failed to acquire application network resources; err=%i", result);
		return (result);
	}

	agentControl.urlHostname = network.getPrimaryInterfaceAddress ();
	if (agentControl.urlHostname.empty ()) {
		Log::warning ("Failed to determine local hostname, network services may not be available");
	}
	result = agentControl.start ();
	if (result != OsUtil::Success) {
		Log::err ("Failed to start agent control processes; err=%i", result);
		return (result);
	}

	commandHistory.readPrefs ();

	if (isConsole) {
		result = runConsole ();
	}
	else {
		result = runWindow ();
	}

	writePrefs ();
	return (result);
}

int App::runWindow () {
	SDL_version version1, version2;
	SDL_RendererInfo renderinfo;
	StdString text;
	int result, delay, i;
	int64_t endtime, elapsed, t1, t2;
	Uint32 windowflags;
	double fps;
	Ui *ui;
	SDL_Rect rect;

	if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		Log::err ("Failed to start SDL: %s", SDL_GetError ());
		return (OsUtil::SdlOperationFailedError);
	}
	if (IMG_Init (IMG_INIT_JPG | IMG_INIT_PNG) != (IMG_INIT_JPG | IMG_INIT_PNG)) {
		Log::err ("Failed to start SDL_image: %s", IMG_GetError ());
		return (OsUtil::SdlOperationFailedError);
	}

	result = input.start ();
	if (result != OsUtil::Success) {
		Log::err ("Failed to acquire application input devices; err=%i", result);
		return (result);
	}

	result = SDL_GetDisplayDPI (0, &displayDdpi, &displayHdpi, &displayVdpi);
	if (result != 0) {
		Log::warning ("Failed to determine display DPI: %s", SDL_GetError ());
		displayHdpi = 72.0f;
		displayVdpi = 72.0f;
	}

	if ((windowWidth <= 0) || (windowHeight <= 0)) {
		windowWidth = prefsMap.find (App::WindowWidthKey, 0);
		windowHeight = prefsMap.find (App::WindowHeightKey, 0);
	}
	imageScale = getImageScale (windowWidth, windowHeight);
	if (imageScale < 0) {
		imageScale = 0;
		windowWidth = App::WindowWidths[0];
		windowHeight = App::WindowHeights[0];
		result = SDL_GetDisplayUsableBounds (0, &rect);
		if (result != 0) {
			Log::warning ("Failed to determine display usable bounds: %s", SDL_GetError ());
		}
		else {
			for (i = 0; i < App::WindowSizeCount; ++i) {
				if ((rect.w >= App::WindowWidths[i]) && (rect.h >= App::WindowHeights[i])) {
					windowWidth = App::WindowWidths[i];
					windowHeight = App::WindowHeights[i];
				}
			}
			imageScale = getImageScale (windowWidth, windowHeight);
			Log::debug ("Set window size from display usable bounds; boundsRect=x%i,y%i,w%i,h%i windowWidth=%i windowHeight=%i imageScale=%i", rect.x, rect.y, rect.w, rect.h, windowWidth, windowHeight, imageScale);
		}
	}

	result = SDL_CreateWindowAndRenderer (windowWidth, windowHeight, 0, &window, &render);
	if (result != 0) {
		Log::err ("Failed to create application window: %s", SDL_GetError ());
		return (OsUtil::SdlOperationFailedError);
	}
	result = SDL_GetRendererInfo (render, &renderinfo);
	if (result != 0) {
		Log::err ("Failed to create application renderer: %s", SDL_GetError ());
		return (OsUtil::SdlOperationFailedError);
	}
	if ((renderinfo.flags & (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE)) == (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE)) {
		isTextureRenderEnabled = true;
	}
	if (isTextureRenderEnabled) {
		isInterfaceAnimationEnabled = prefsMap.find (App::ShowInterfaceAnimationsKey, true);
	}
	isMainToolbarClockEnabled = prefsMap.find (App::ShowClockKey, false);

	clipRect.x = 0;
	clipRect.y = 0;
	clipRect.w = windowWidth;
	clipRect.h = windowHeight;
	nextWindowWidth = windowWidth;
	nextWindowHeight = windowHeight;

	i = prefsMap.find (App::FontScaleKey, App::FontScaleCount / 2);
	if ((i >= 0) && (i < App::FontScaleCount)) {
		fontScale = App::FontScales[i];
		nextFontScale = fontScale;
	}

	SDL_SetWindowTitle (window, APPLICATION_NAME);
	uiConfig.resetScale ();
	result = uiConfig.load (fontScale);
	if (result != OsUtil::Success) {
		Log::err ("Failed to load application resources; err=%i", result);
		return (result);
	}
	populateRoundedCornerSprite ();
	populateWidgets ();

	ui = new MainUi ();
	uiStack.setUi (ui);

	updateThread = SDL_CreateThread (App::runUpdateThread, "runUpdateThread", (void *) this);

	Log::info ("Application started; buildId=%s windowSize=%ix%i renderName=%s lang=%s pid=%i", BUILD_ID, windowWidth, windowHeight, renderinfo.name, OsUtil::getEnvLanguage ("").c_str (), OsUtil::getProcessId ());
	windowflags = SDL_GetWindowFlags (window);
	SDL_VERSION (&version1);
	SDL_GetVersion (&version2);
	Log::debug ("* sdlBuildVersion=%i.%i.%i sdlLinkVersion=%i.%i.%i windowFlags=0x%x renderName=%s renderFlags=0x%x isTextureRenderEnabled=%s diagonalDpi=%.2f horizontalDpi=%.2f verticalDpi=%.2f imageScale=%i minDrawFrameDelay=%i minUpdateFrameDelay=%i", version1.major, version1.minor, version1.patch, version2.major, version2.minor, version2.patch, (unsigned int) windowflags, renderinfo.name, (unsigned int) renderinfo.flags, BOOL_STRING (isTextureRenderEnabled), displayDdpi, displayHdpi, displayVdpi, imageScale, minDrawFrameDelay, minUpdateFrameDelay);

	text.assign ("");
	if (windowflags & SDL_WINDOW_FULLSCREEN) {
		text.append (" FULLSCREEN");
	}
	if (windowflags & SDL_WINDOW_OPENGL) {
		text.append (" OPENGL");
	}
	if (windowflags & SDL_WINDOW_SHOWN) {
		text.append (" SHOWN");
	}
	if (windowflags & SDL_WINDOW_HIDDEN) {
		text.append (" HIDDEN");
	}
	if (windowflags & SDL_WINDOW_BORDERLESS) {
		text.append (" BORDERLESS");
	}
	if (windowflags & SDL_WINDOW_RESIZABLE) {
		text.append (" RESIZABLE");
	}
	if (windowflags & SDL_WINDOW_MINIMIZED) {
		text.append (" MINIMIZED");
	}
	if (windowflags & SDL_WINDOW_MAXIMIZED) {
		text.append (" MAXIMIZED");
	}
	if (windowflags & SDL_WINDOW_INPUT_GRABBED) {
		text.append (" INPUT_GRABBED");
	}
	if (windowflags & SDL_WINDOW_INPUT_FOCUS) {
		text.append (" INPUT_FOCUS");
	}
	if (windowflags & SDL_WINDOW_MOUSE_FOCUS) {
		text.append (" MOUSE_FOCUS");
	}
	if (windowflags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
		text.append (" FULLSCREEN_DESKTOP");
	}
	if (windowflags & SDL_WINDOW_FOREIGN) {
		text.append (" FOREIGN");
	}
	if (windowflags & SDL_WINDOW_ALLOW_HIGHDPI) {
		text.append (" ALLOW_HIGHDPI");
	}
	if (windowflags & SDL_WINDOW_MOUSE_CAPTURE) {
		text.append (" MOUSE_CAPTURE");
	}
	if (windowflags & SDL_WINDOW_ALWAYS_ON_TOP) {
		text.append (" ALWAYS_ON_TOP");
	}
	if (windowflags & SDL_WINDOW_SKIP_TASKBAR) {
		text.append (" SKIP_TASKBAR");
	}
	if (windowflags & SDL_WINDOW_UTILITY) {
		text.append (" UTILITY");
	}
	if (windowflags & SDL_WINDOW_TOOLTIP) {
		text.append (" TOOLTIP");
	}
	if (windowflags & SDL_WINDOW_POPUP_MENU) {
		text.append (" POPUP_MENU");
	}
	if (windowflags & SDL_WINDOW_VULKAN) {
		text.append (" VULKAN");
	}
	if (windowflags & SDL_WINDOW_METAL) {
		text.append (" METAL");
	}
	Log::debug3 ("* Window flags:%s", text.c_str ());

	text.assign ("");
	if (renderinfo.flags & SDL_RENDERER_SOFTWARE) {
		text.append (" SOFTWARE");
	}
	if (renderinfo.flags & SDL_RENDERER_ACCELERATED) {
		text.append (" ACCELERATED");
	}
	if (renderinfo.flags & SDL_RENDERER_PRESENTVSYNC) {
		text.append (" PRESENTVSYNC");
	}
	if (renderinfo.flags & SDL_RENDERER_TARGETTEXTURE) {
		text.append (" TARGETTEXTURE");
	}
	Log::debug3 ("* Render flags:%s", text.c_str ());
	text.assign ("");

	while (true) {
		if (isShutdown) {
			break;
		}

		t1 = OsUtil::getTime ();
		input.pollEvents ();
		if (! FLOAT_EQUALS (fontScale, nextFontScale)) {
			if (uiConfig.reloadFonts (nextFontScale) != OsUtil::Success) {
				nextFontScale = fontScale;
			}
			else {
				shouldRefreshUi = true;
				fontScale = nextFontScale;
				for (i = 0; i < App::FontScaleCount; ++i) {
					if (FLOAT_EQUALS (fontScale, App::FontScales[i])) {
						SDL_LockMutex (prefsMapMutex);
						prefsMap.insert (App::FontScaleKey, i);
						SDL_UnlockMutex (prefsMapMutex);
						break;
					}
				}
			}
		}

		executeRenderTasks ();
		draw ();
		if ((windowWidth != nextWindowWidth) || (windowHeight != nextWindowHeight)) {
			resizeWindow ();
		}
		uiStack.executeStackCommands ();
		resource.compact ();

		t2 = OsUtil::getTime ();
		delay = (int) (minDrawFrameDelay - (t2 - t1));
		if (delay < 1) {
			delay = 1;
		}
		SDL_Delay (delay);
	}
	SDL_WaitThread (updateThread, &result);

	if (backgroundTexture) {
		backgroundTexture = NULL;
		resource.unloadTexture (backgroundTexturePath);
		backgroundTexturePath.assign ("");
	}
	if (nextBackgroundTexture) {
		nextBackgroundTexture = NULL;
		resource.unloadTexture (nextBackgroundTexturePath);
		nextBackgroundTexturePath.assign ("");
	}

	uiStack.clear ();
	if (rootPanel) {
		rootPanel->release ();
		rootPanel = NULL;
	}
	uiConfig.unload ();
	agentControl.stop ();
	if (roundedCornerSprite) {
		roundedCornerSprite->unload ();
	}
	resource.compact ();
	resource.close ();

	SDL_DestroyRenderer (render);
	render = NULL;
	SDL_DestroyWindow (window);
	window = NULL;

	endtime = OsUtil::getTime ();
	elapsed = endtime - startTime;
	fps = (double) drawCount;
	if (elapsed > 1000) {
		fps /= ((double) elapsed) / 1000.0f;
	}
	Log::info ("Application ended; updateCount=%lli drawCount=%lli runtime=%.3fs FPS=%f pid=%i", (long long) updateCount, (long long) drawCount, ((double) elapsed) / 1000.0f, fps, OsUtil::getProcessId ());

	return (OsUtil::Success);
}

int App::runConsole () {
	int64_t endtime, elapsed;
	int result;
	char c;
	StdString line;

	if (SDL_Init (SDL_INIT_TIMER) != 0) {
		Log::printf ("Failed to start SDL: %s", SDL_GetError ());
		return (OsUtil::SdlOperationFailedError);
	}

	updateThread = SDL_CreateThread (App::runConsoleUpdateThread, "runConsoleUpdateThread", (void *) this);
	Log::printf ("%s; buildId=%s lang=%s pid=%i", UiText::instance->getText (UiTextString::ConsoleStartText1).c_str (), BUILD_ID, OsUtil::getEnvLanguage ("").c_str (), OsUtil::getProcessId ());
	Log::printf ("* %s / v%s.%s.%s. %s", UiText::instance->getText (UiTextString::ConsoleStartText2).c_str (), LUA_VERSION_MAJOR, LUA_VERSION_MINOR, LUA_VERSION_RELEASE, UiText::instance->getText (UiTextString::ConsoleStartText3).c_str ());
	Log::printf ("* %s", UiText::instance->getText (UiTextString::ConsoleStartText4).c_str ());

	line = OsUtil::getEnvValue ("RUN_SCRIPT", "");
	if (! line.empty ()) {
		LuaScript::run (new LuaScript (line));
		line.assign ("");
	}

	printf ("> ");
	while (true) {
		if (isShutdown) {
			break;
		}

		c = (char) std::cin.get ();
		if (c == EOF) {
			isShutdown = true;
			break;
		}
		if (c == '\n') {
			LuaScript::run (new LuaScript (line));
			line.assign ("");
			printf ("\n> ");
		}
		else {
			line.append (1, c);
		}
	}

	resource.compact ();
	resource.close ();
	taskGroup.stop ();
	network.stop ();
	network.waitThreads ();
	taskGroup.waitThreads ();
	SDL_WaitThread (updateThread, &result);

	endtime = OsUtil::getTime ();
	elapsed = endtime - startTime;
	Log::info ("Application ended; runtime=%.3fs pid=%i", ((double) elapsed) / 1000.0f, OsUtil::getProcessId ());
	return (OsUtil::Success);
}

int App::runConsoleUpdateThread (void *appPtr) {
	App *app;
	int64_t t1, t2, last;
	int delay;

	app = (App *) appPtr;
	last = OsUtil::getTime ();
	while (true) {
		if (app->isShutdown) {
			break;
		}

		t1 = OsUtil::getTime ();
		app->updateConsole ((int) (t1 - last));
		t2 = OsUtil::getTime ();
		last = t1;

		delay = (int) (app->minUpdateFrameDelay - (t2 - t1));
		if (delay < 1) {
			delay = 1;
		}
		SDL_Delay (delay);
	}

	return (0);
}

void App::updateConsole (int msElapsed) {
	agentControl.update (msElapsed);
	writePrefs ();
	++updateCount;
}

void App::populateWidgets () {
	if (! rootPanel) {
		rootPanel = new Panel ();
		rootPanel->retain ();
		rootPanel->id = getUniqueId ();
		rootPanel->setFixedSize (true, windowWidth, windowHeight);
		rootPanel->keyEventCallback = Widget::KeyEventCallbackContext (App::keyEvent, NULL);
	}
	uiStack.populateWidgets ();
}

void App::populateRoundedCornerSprite () {
	Uint32 *pixels, *dest, color, rmask, gmask, bmask, amask;
	SDL_Surface *surface;
	SDL_Texture *texture;
	StdString path;
	float dist, targetalpha, minalpha, opacity;
	int radius, x, y, w, h;
	uint8_t alpha;

	if (roundedCornerSprite) {
		delete (roundedCornerSprite);
	}
	roundedCornerSprite = new Sprite ();
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xFF000000;
	gmask = 0x00FF0000;
	bmask = 0x0000FF00;
	amask = 0x000000FF;
#else
	rmask = 0x000000FF;
	gmask = 0x0000FF00;
	bmask = 0x00FF0000;
	amask = 0xFF000000;
#endif
	minalpha = 0.1f;
	opacity = 8.0f;
	radius = 1;
	while (radius <= App::MaxCornerRadius) {
		w = (radius * 2) + 1;
		h = (radius * 2) + 1;
		pixels = (Uint32 *) malloc (w * h * sizeof (Uint32));
		if (! pixels) {
			Log::warning ("Failed to create texture; err=\"Out of memory, dimensions %ix%i\"", radius, radius);
			delete (roundedCornerSprite);
			roundedCornerSprite = NULL;
			return;
		}
		dest = pixels;
		y = 0;
		while (y < h) {
			x = 0;
			while (x < w) {
				dist = MathUtil::getDistance (((float) x) + 0.5f, ((float) y) + 0.5f, (float) radius + 0.5f, (float) radius + 0.5f);
				targetalpha = 1.0f - ((1.0f - minalpha) * (dist / (float) radius));
				if (targetalpha <= 0.0f) {
					targetalpha = 0.0f;
				}
				else {
					targetalpha *= opacity;
					if (targetalpha > 1.0f) {
						targetalpha = 1.0f;
					}
				}
				alpha = (uint8_t) (targetalpha * 255.0f);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				color = 0xFFFFFF00 | (alpha & 0xFF);
#else
				color = 0x00FFFFFF | (((Uint32) (alpha & 0xFF)) << 24);
#endif
				*dest = color;
				++dest;
				++x;
			}
			++y;
		}

		surface = SDL_CreateRGBSurfaceFrom (pixels, w, h, 32, w * sizeof (Uint32), rmask, gmask, bmask, amask);
		if (! surface) {
			free (pixels);
			Log::warning ("Failed to create texture; err=\"SDL_CreateRGBSurfaceFrom, %s\"", SDL_GetError ());
			delete (roundedCornerSprite);
			roundedCornerSprite = NULL;
			return;
		}
		path.sprintf ("*_App::roundedCornerSprite_%llx", (long long int) App::instance->getUniqueId ());
		texture = Resource::instance->createTexture (path, surface);
		SDL_FreeSurface (surface);
		free (pixels);
		if (! texture) {
			delete (roundedCornerSprite);
			roundedCornerSprite = NULL;
			return;
		}
		roundedCornerSprite->addTexture (texture, path);
		++radius;
	}
}

SDL_Texture *App::getRoundedCornerTexture (int radius, int *textureWidth, int *textureHeight) {
	if ((radius <= 0) || (radius > App::MaxCornerRadius)) {
		return (NULL);
	}
	return (roundedCornerSprite->getTexture ((radius - 1), textureWidth, textureHeight));
}

void App::shutdown () {
	Ui *ui;

	if (isConsole) {
		isShutdown = true;
		::close (0);
		return;
	}

	if (isShuttingDown) {
		return;
	}
	isShuttingDown = true;
	ui = uiStack.getActiveUi ();
	if (ui) {
		ui->showShutdownWindow ();
		ui->release ();
	}
	agentControl.stop ();
	taskGroup.stop ();
	network.stop ();
	input.stop ();
}

HashMap *App::lockPrefs () {
	SDL_LockMutex (prefsMapMutex);
	return (&prefsMap);
}

void App::unlockPrefs () {
	SDL_UnlockMutex (prefsMapMutex);
}

void App::datagramReceived (void *callbackData, const char *messageData, int messageLength, const char *sourceAddress, int sourcePort) {
	AgentControl::instance->receiveMessage (messageData, messageLength, sourceAddress, sourcePort);
}

void App::executeRenderTasks () {
	std::vector<App::RenderTaskContext>::iterator i, end;

	renderTaskList.clear ();
	SDL_LockMutex (renderTaskMutex);
	renderTaskList.swap (renderTaskAddList);
	SDL_UnlockMutex (renderTaskMutex);

	i = renderTaskList.begin ();
	end = renderTaskList.end ();
	while (i != end) {
		i->fn (i->fnData);
		++i;
	}
	renderTaskList.clear ();
}

void App::draw () {
	Ui *ui;
	SDL_Rect rect;

	SDL_RenderClear (render);

	SDL_LockMutex (backgroundMutex);
	if (! nextBackgroundTexturePath.empty ()) {
		if (! nextBackgroundTexture) {
			nextBackgroundTextureWidth = 0;
			nextBackgroundTextureHeight = 0;
			nextBackgroundTexture = resource.loadTexture (nextBackgroundTexturePath);
			if (! nextBackgroundTexture) {
				Log::err ("Failed to load background texture; path=%s", nextBackgroundTexturePath.c_str ());
				nextBackgroundTexturePath.assign ("");
			}
			else {
				SDL_QueryTexture (nextBackgroundTexture, NULL, NULL, &nextBackgroundTextureWidth, &nextBackgroundTextureHeight);
				backgroundCrossFadeAlpha = 0.0f;
			}
		}
	}

	if (backgroundTexture && nextBackgroundTexture) {
		rect.x = 0;
		rect.y = 0;
		rect.w = backgroundTextureWidth;
		rect.h = backgroundTextureHeight;
		SDL_SetTextureBlendMode (backgroundTexture, SDL_BLENDMODE_NONE);
		SDL_RenderCopy (render, backgroundTexture, NULL, &rect);

		rect.x = 0;
		rect.y = 0;
		rect.w = nextBackgroundTextureWidth;
		rect.h = nextBackgroundTextureHeight;
		SDL_SetTextureBlendMode (nextBackgroundTexture, SDL_BLENDMODE_BLEND);
		SDL_SetTextureAlphaMod (nextBackgroundTexture, (Uint8) (backgroundCrossFadeAlpha * 255.0f));
		SDL_RenderCopy (render, nextBackgroundTexture, NULL, &rect);
	}
	else if (nextBackgroundTexture) {
		rect.x = 0;
		rect.y = 0;
		rect.w = nextBackgroundTextureWidth;
		rect.h = nextBackgroundTextureHeight;
		SDL_SetTextureBlendMode (nextBackgroundTexture, SDL_BLENDMODE_NONE);
		SDL_RenderCopy (render, nextBackgroundTexture, NULL, &rect);
	}
	else if (backgroundTexture) {
		rect.x = 0;
		rect.y = 0;
		rect.w = backgroundTextureWidth;
		rect.h = backgroundTextureHeight;
		SDL_SetTextureBlendMode (backgroundTexture, SDL_BLENDMODE_NONE);
		SDL_RenderCopy (render, backgroundTexture, NULL, &rect);
	}
	SDL_UnlockMutex (backgroundMutex);

	ui = uiStack.getActiveUi ();
	if (ui) {
		ui->draw ();
		rootPanel->draw ();
		ui->release ();
	}
	SDL_RenderPresent (render);
	++drawCount;
}

int App::runUpdateThread (void *appPtr) {
	App *app;
	StdString line;
	int64_t t1, t2, last;
	int delay;

	app = (App *) appPtr;

	line = OsUtil::getEnvValue ("RUN_SCRIPT", "");
	if (! line.empty ()) {
		app->taskGroup.run (TaskGroup::RunContext (LuaScript::run, new LuaScript (line)));
		line.assign ("");
	}

	last = OsUtil::getTime ();
	while (true) {
		if (app->isShutdown) {
			break;
		}

		t1 = OsUtil::getTime ();
		app->update ((int) (t1 - last));
		t2 = OsUtil::getTime ();
		last = t1;

		delay = (int) (app->minUpdateFrameDelay - (t2 - t1));
		if (delay < 1) {
			delay = 1;
		}
		SDL_Delay (delay);
	}

	return (0);
}

void App::update (int msElapsed) {
	Ui *ui;

	agentControl.update (msElapsed);
	taskGroup.update (msElapsed);
	uiStack.update (msElapsed);
	if (shouldRefreshUi) {
		uiStack.refresh ();
		rootPanel->refresh ();
		shouldRefreshUi = false;
	}
	ui = uiStack.getActiveUi ();

	if (shouldSyncRecordStore) {
		recordStore.lock ();
		rootPanel->syncRecordStore ();
		if (ui) {
			ui->syncRecordStore ();
		}
		recordStore.unlock ();
		shouldSyncRecordStore = false;
	}

	SDL_LockMutex (backgroundMutex);
	if (nextBackgroundTexture) {
		if (uiConfig.backgroundCrossFadeDuration <= 0) {
			backgroundCrossFadeAlpha = 1.0f;
		}
		else {
			backgroundCrossFadeAlpha += (1.0f / ((float) uiConfig.backgroundCrossFadeDuration)) * (float) msElapsed;
		}

		if (backgroundCrossFadeAlpha >= 1.0f) {
			backgroundCrossFadeAlpha = 1.0f;
			if (backgroundTexture) {
				resource.unloadTexture (backgroundTexturePath);
			}
			backgroundTexture = nextBackgroundTexture;
			backgroundTexturePath.assign (nextBackgroundTexturePath);
			backgroundTextureWidth = nextBackgroundTextureWidth;
			backgroundTextureHeight = nextBackgroundTextureHeight;
			nextBackgroundTexture = NULL;
			nextBackgroundTexturePath.assign ("");
			nextBackgroundTextureWidth = 0;
			nextBackgroundTextureHeight = 0;
		}
	}
	SDL_UnlockMutex (backgroundMutex);

	rootPanel->processInput ();
	if (ui) {
		ui->update (msElapsed);
		ui->release ();
	}
	rootPanel->update (msElapsed, 0.0f, 0.0f);

	writePrefs ();
	++updateCount;

	SDL_LockMutex (updateMutex);
	while (isSuspendingUpdate) {
		SDL_CondBroadcast (updateCond);
		SDL_CondWait (updateCond, updateMutex);
	}
	SDL_UnlockMutex (updateMutex);

	if (isShuttingDown) {
		if (network.isStopComplete () && taskGroup.isStopComplete ()) {
			network.waitThreads ();
			taskGroup.waitThreads ();
			isShutdown = true;
		}
	}
}

bool App::keyEvent (void *ptr, SDL_Keycode keycode, bool isShiftDown, bool isControlDown) {
	if (isControlDown) {
		switch (keycode) {
			case SDLK_q: {
				Input::instance->windowClose ();
				return (true);
			}
			case SDLK_l: {
				App::instance->uiStack.toggleHistoryWindow ();
				return (true);
			}
			case SDLK_s: {
				App::instance->uiStack.toggleSettingsWindow ();
				return (true);
			}
			case SDLK_h: {
				App::instance->uiStack.toggleHelpWindow ();
				return (true);
			}
			case SDLK_o: {
				App::instance->uiStack.toggleConsoleWindow ();
				return (true);
			}
		}
	}

	if (App::instance->uiStack.processKeyEvent (keycode, isShiftDown, isControlDown)) {
		return (true);
	}
	return (false);
}

int64_t App::getUniqueId () {
	int64_t id;

	SDL_LockMutex (uniqueIdMutex);
	id = nextUniqueId;
	++nextUniqueId;
	SDL_UnlockMutex (uniqueIdMutex);

	return (id);
}

void App::suspendUpdate () {
	SDL_LockMutex (updateMutex);
	isSuspendingUpdate = true;
	SDL_CondWait (updateCond, updateMutex);
	SDL_UnlockMutex (updateMutex);
}

void App::unsuspendUpdate () {
	SDL_LockMutex (updateMutex);
	isSuspendingUpdate = false;
	SDL_CondBroadcast (updateCond);
	SDL_UnlockMutex (updateMutex);
}

void App::pushClipRect (const SDL_Rect *rect, bool disableIntersection) {
	int x, y, w, h, diff;

	x = rect->x;
	y = rect->y;
	w = rect->w;
	h = rect->h;
	if ((! clipRectStack.empty ()) && (! disableIntersection)) {
		diff = x - clipRect.x;
		if (diff < 0) {
			w += diff;
			x = clipRect.x;
		}
		diff = y - clipRect.y;
		if (diff < 0) {
			h += diff;
			y = clipRect.y;
		}
		diff = (x + w) - (clipRect.x + clipRect.w);
		if (diff > 0) {
			w -= diff;
		}
		diff = (y + h) - (clipRect.y + clipRect.h);
		if (diff > 0) {
			h -= diff;
		}

		if (w < 0) {
			w = 0;
		}
		if (h < 0) {
			h = 0;
		}
	}

	clipRect.x = x;
	clipRect.y = y;
	clipRect.w = w;
	clipRect.h = h;
	SDL_RenderSetClipRect (render, &clipRect);
	clipRectStack.push (clipRect);
}

void App::popClipRect () {
	if (clipRectStack.empty ()) {
		return;
	}

	clipRectStack.pop ();
	if (clipRectStack.empty ()) {
		clipRect.x = 0;
		clipRect.y = 0;
		clipRect.w = windowWidth;
		clipRect.h = windowHeight;
		SDL_RenderSetClipRect (render, NULL);
	}
	else {
		clipRect = clipRectStack.top ();
		SDL_RenderSetClipRect (render, &clipRect);
	}
}

void App::suspendClipRect () {
	SDL_RenderSetClipRect (render, NULL);
}

void App::unsuspendClipRect () {
	SDL_RenderSetClipRect (render, &clipRect);
}

void App::addRenderTask (RenderTaskFunction fn, void *fnData) {
	App::RenderTaskContext ctx;

	if (! fn) {
		return;
	}
	ctx.fn = fn;
	ctx.fnData = fnData;
	SDL_LockMutex (renderTaskMutex);
	renderTaskAddList.push_back (ctx);
	SDL_UnlockMutex (renderTaskMutex);
}

void App::setConsoleWindow (ConsoleWindow *window) {
	SDL_LockMutex (consoleWindowMutex);
	if (consoleWindow) {
		consoleWindow->release ();
	}
	consoleWindow = window;
	if (consoleWindow) {
		consoleWindow->retain ();
	}
	SDL_UnlockMutex (consoleWindowMutex);
}

void App::writeConsoleOutput (const StdString &text) {
	if (! consoleWindow) {
		return;
	}
	SDL_LockMutex (consoleWindowMutex);
	if (consoleWindow) {
		if (consoleWindow->isDestroyed) {
			consoleWindow->release ();
			consoleWindow = NULL;
		}
		else {
			consoleWindow->appendText (text);
		}
	}
	SDL_UnlockMutex (consoleWindowMutex);
}

void App::writePrefs () {
	int result;

	if (isPrefsWriteDisabled) {
		return;
	}

	SDL_LockMutex (prefsMapMutex);
	if (prefsMap.isWriteDirty) {
		result = prefsMap.write (prefsPath);
		if (result != OsUtil::Success) {
			Log::err ("Failed to write prefs file; prefsPath=\"%s\" err=%i", prefsPath.c_str (), result);
			isPrefsWriteDisabled = true;
		}
	}
	SDL_UnlockMutex (prefsMapMutex);
}

void App::resizeWindow () {
	std::vector<Ui *>::iterator i, end;
	int scale, result;

	scale = getImageScale (nextWindowWidth, nextWindowHeight);
	if (scale < 0) {
		nextWindowWidth = windowWidth;
		nextWindowHeight = windowHeight;
		return;
	}

	suspendUpdate ();
	SDL_SetWindowSize (window, nextWindowWidth, nextWindowHeight);
	windowWidth = nextWindowWidth;
	windowHeight = nextWindowHeight;
	imageScale = scale;
	uiConfig.resetScale ();

	clipRect.x = 0;
	clipRect.y = 0;
	clipRect.w = windowWidth;
	clipRect.h = windowHeight;
	rootPanel->setFixedSize (true, windowWidth, windowHeight);

	uiConfig.coreSprites.resize ();
	result = uiConfig.reloadFonts (fontScale);
	if (result != OsUtil::Success) {
		Log::err ("Failed to reload fonts; fontScale=%.2f err=%i", fontScale, result);
	}
	if (! backgroundTextureBasePath.empty ()) {
		setNextBackgroundTexturePath (backgroundTextureBasePath);
	}

	uiStack.resize ();
	rootPanel->resetInputState ();
	shouldRefreshUi = true;
	unsuspendUpdate ();

	SDL_LockMutex (prefsMapMutex);
	prefsMap.insert (App::WindowWidthKey, windowWidth);
	prefsMap.insert (App::WindowHeightKey, windowHeight);
	SDL_UnlockMutex (prefsMapMutex);
}

int App::getRandomInt (int i1, int i2) {
	return (prng.getRandomValue (i1, i2));
}

static const char getRandomString_chars[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
StdString App::getRandomString (int stringLength) {
	StdString s;
	int i;

	for (i = 0; i < stringLength; ++i) {
		s.append (1, getRandomString_chars[getRandomInt (0, sizeof (getRandomString_chars) - 1)]);
	}

	return (s);
}

void App::setNextBackgroundTexturePath (const StdString &path) {
	StdString filepath;

	backgroundTextureBasePath.assign (path);
	filepath.sprintf ("%s/%i.png", backgroundTextureBasePath.c_str (), imageScale);
	SDL_LockMutex (backgroundMutex);
	if (! filepath.equals (nextBackgroundTexturePath)) {
		if (nextBackgroundTexture) {
			nextBackgroundTexture = NULL;
			resource.unloadTexture (nextBackgroundTexturePath);
			nextBackgroundTexturePath.assign ("");
		}
		nextBackgroundTexturePath.assign (filepath);
	}
	SDL_UnlockMutex (backgroundMutex);
}

void App::setNextBackgroundTexturePath (const char *path) {
	setNextBackgroundTexturePath (StdString (path));
}

void App::hyperlinkOpened (void *ptr, Widget *widgetPtr) {
	HyperlinkWindow *window;

	window = (HyperlinkWindow *) widgetPtr;
	if (window->linkOpenResult != OsUtil::Success) {
		App::instance->uiStack.showSnackbar (UiText::instance->getText (UiTextString::OpenHelpUrlError));
	}
	else {
		App::instance->uiStack.showSnackbar (StdString::createSprintf ("%s - %s", UiText::instance->getText (UiTextString::LaunchedWebBrowser).capitalized ().c_str (), window->url.c_str ()));
	}
}

void App::handleLinkClientConnect (const StdString &agentId) {
	Ui *ui;

	ui = uiStack.getActiveUi ();
	if (ui) {
		ui->handleLinkClientConnect (agentId);
		ui->release ();
		shouldRefreshUi = true;
	}
}

void App::handleLinkClientDisconnect (const StdString &agentId, const StdString &errorDescription) {
	Ui *ui;

	ui = uiStack.getActiveUi ();
	if (ui) {
		ui->handleLinkClientDisconnect (agentId, errorDescription);
		ui->release ();
		shouldRefreshUi = true;
	}
}

Json *App::createCommand (const char *commandName, Json *commandParams) {
	Json *cmd;
	SystemInterface::Prefix prefix;

	prefix = createCommandPrefix ();
	cmd = systemInterface.createCommand (prefix, commandName, commandParams);
	if (! cmd) {
		Log::err ("Failed to create SystemInterface command; commandName=\"%s\" err=\"%s\"", commandName, systemInterface.lastError.c_str ());
	}
	return (cmd);
}

Json *App::createCommand (int commandId, Json *commandParams) {
	Json *cmd;
	SystemInterface::Prefix prefix;

	prefix = createCommandPrefix ();
	cmd = systemInterface.createCommand (prefix, commandId, commandParams);
	if (! cmd) {
		Log::err ("Failed to create SystemInterface command; commandId=%i err=\"%s\"", commandId, systemInterface.lastError.c_str ());
	}
	return (cmd);
}

SystemInterface::Prefix App::createCommandPrefix () {
	SystemInterface::Prefix prefix;

	prefix.createTime = OsUtil::getTime ();
	prefix.agentId.assign ("");
	prefix.userId.assign ("");
	prefix.priority = 0;
	prefix.startTime = 0;
	prefix.duration = 0;

	return (prefix);
}

StdString App::getHelpUrl (const StdString &topicId) {
	StdString topic;

	topic.assign (topicId.empty () ? "index" : topicId);
	return (StdString::createSprintf ("%si/%s_%s_%s_%s", App::ServerUrl.c_str (), topic.urlEncoded ().c_str (), StdString (BUILD_ID).urlEncoded ().c_str (), StdString (PLATFORM_ID).urlEncoded ().c_str (), OsUtil::getEnvLanguage ("en").urlEncoded ().c_str ()));
}

StdString App::getHelpUrl (const char *topicId) {
	return (App::getHelpUrl (StdString (topicId)));
}

StdString App::getApplicationUrl (const StdString &applicationId) {
	return (StdString::createSprintf ("%s%s", App::ServerUrl.c_str (), applicationId.urlEncoded ().c_str ()));
}

StdString App::getFeatureUrl (const StdString &featureId) {
	return (StdString::createSprintf ("%sfeature/%s", App::ServerUrl.c_str (), featureId.urlEncoded ().c_str ()));
}

StdString App::getFeedbackUrl (bool shouldIncludeVersion) {
	if (! shouldIncludeVersion) {
		return (StdString::createSprintf ("%scontact", App::ServerUrl.c_str ()));
	}
	return (StdString::createSprintf ("%scontact/%s_%s", App::ServerUrl.c_str (), StdString (BUILD_ID).urlEncoded ().c_str (), StdString (PLATFORM_ID).urlEncoded ().c_str ()));
}

StdString App::getUpdateUrl (const StdString &applicationId) {
	if (! applicationId.empty ()) {
		return (StdString::createSprintf ("%supdate/%s", App::ServerUrl.c_str (), applicationId.urlEncoded ().c_str ()));
	}
	return (StdString::createSprintf ("%supdate/%s_%s", App::ServerUrl.c_str (), StdString (BUILD_ID).urlEncoded ().c_str (), StdString (PLATFORM_ID).urlEncoded ().c_str ()));
}

bool App::isUpdateUrl (const StdString &url) {
	return (url.startsWith (StdString::createSprintf ("%supdate/", App::ServerUrl.c_str ())));
}

StdString App::getApplicationNewsUrl (const StdString &applicationId) {
	if (! applicationId.empty ()) {
		return (StdString::createSprintf ("%sapplication-news/%s", App::ServerUrl.c_str (), applicationId.urlEncoded ().c_str ()));
	}
	return (StdString::createSprintf ("%sapplication-news/%s_%s_%s", App::ServerUrl.c_str (), StdString (BUILD_ID).urlEncoded ().c_str (), StdString (PLATFORM_ID).urlEncoded ().c_str (), OsUtil::getEnvLanguage ("en").urlEncoded ().c_str ()));
}

StdString App::getDonateUrl () {
	return (StdString::createSprintf ("%scontribute", App::ServerUrl.c_str ()));
}

bool App::transferJsonListPrefs (HashMap *prefs, const char *prefsKey, JsonList *items) {
	Json *state;
	StringList s;
	StringList::iterator i, end;
	bool result;

	prefs->find (prefsKey, &s);
	if (s.empty ()) {
		return (false);
	}
	result = false;
	i = s.begin ();
	end = s.end ();
	while (i != end) {
		state = new Json ();
		if (state->parse (*i)) {
			items->push_back (state);
			result = true;
		}
		else {
			delete (state);
		}
		++i;
	}
	prefs->remove (prefsKey);

	return (result);
}
