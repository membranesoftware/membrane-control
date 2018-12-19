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
#include <time.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "Result.h"
#include "StdString.h"
#include "StringList.h"
#include "Log.h"
#include "Util.h"
#include "Input.h"
#include "HashMap.h"
#include "Sprite.h"
#include "Resource.h"
#include "Input.h"
#include "Network.h"
#include "Json.h"
#include "UiConfiguration.h"
#include "Panel.h"
#include "Toolbar.h"
#include "NewsWindow.h"
#include "SnackbarWindow.h"
#include "LogoWindow.h"
#include "MainUi.h"
#include "App.h"

static App *appInstance = NULL;

const int App::defaultMinFrameDelay = 20;
const int App::windowWidths[] = { 768, 1024, 1280, 1600, 1920 };
const int App::windowHeights[] = { 432, 576, 720, 900, 1080 };
const int App::numWindowSizes = 5;
const float App::fontScales[] = { 0.66f, 0.8f, 1.0f, 1.25f, 1.5f };
const int App::numFontScales = 5;
const StdString App::prefsIsFirstLaunchComplete = StdString ("IsFirstLaunchComplete");
const StdString App::prefsNetworkThreads = StdString ("NetworkThreads");
const StdString App::prefsAgentStatus = StdString ("AgentStatus");
const StdString App::prefsWindowWidth = StdString ("WindowWidth");
const StdString App::prefsWindowHeight = StdString ("WindowHeight");
const StdString App::prefsFontScale = StdString ("FontScale");
const StdString App::prefsShowClock = StdString ("ShowClock");
const StdString App::prefsMediaImageSize = StdString ("Media_ImageSize");
const StdString App::prefsMonitorImageSize = StdString ("Monitor_ImageSize");
const StdString App::prefsMediaItemImageSize = StdString ("MediaItem_ImageSize");
const StdString App::prefsStreamItemImageSize = StdString ("StreamItem_ImageSize");
const StdString App::prefsServerTimeout = StdString ("ServerTimeout");
const int64_t App::defaultServerTimeout = 180;
const StdString App::prefsServerPath = StdString ("ServerPath");
const StdString App::prefsApplicationName = StdString ("ApplicationName");
const StdString App::prefsHttps = StdString ("Https");
const StdString App::prefsWebKioskUiPlaylists = StdString ("WebKiosk_Playlists");
const StdString App::prefsMonitorUiPlaylists = StdString ("Monitor_Playlists");
const StdString App::prefsMainUiShowAllEnabled = StdString ("Main_ShowAllEnabled");

App::App ()
: shouldSyncRecordStore (false)
, shouldRefreshUi (false)
, nextFontScale (1.0f)
, nextWindowWidth (0)
, nextWindowHeight (0)
#if ENABLE_TEST_KEYS
, isUiPaused (false)
, isUiPauseKeyPressed (false)
#endif
, isShuttingDown (false)
, isShutdown (false)
, startTime (0)
, isHttpsEnabled (false)
, window (NULL)
, render (NULL)
, rootPanel (NULL)
, mainToolbar (NULL)
, secondaryToolbar (NULL)
, newsWindow (NULL)
, snackbarWindow (NULL)
, displayDdpi (0.0f)
, displayHdpi (0.0f)
, displayVdpi (0.0f)
, windowWidth (0)
, windowHeight (0)
, minDrawFrameDelay (App::defaultMinFrameDelay)
, minUpdateFrameDelay (App::defaultMinFrameDelay)
, topBarHeight (0.0f)
, bottomBarHeight (0.0f)
, rightBarWidth (0.0f)
, fontScale (1.0f)
, imageScale (0)
, drawCount (0)
, updateCount (0)
, isPrefsWriteDisabled (false)
, updateThread (NULL)
, uniqueIdMutex (NULL)
, nextUniqueId (1)
, uiMutex (NULL)
, currentUi (NULL)
, renderTaskMutex (NULL)
, isSuspendingUpdate (false)
, updateMutex (NULL)
, updateCond (NULL)
, backgroundMutex (NULL)
, backgroundTexture (NULL)
, backgroundTextureWidth (0)
, backgroundTextureHeight (0)
, nextBackgroundTexture (NULL)
, nextBackgroundTextureWidth (0)
, nextBackgroundTextureHeight (0)
, backgroundTransitionAlpha (0.0f)
{
	uniqueIdMutex = SDL_CreateMutex ();
	uiMutex = SDL_CreateMutex ();
	renderTaskMutex = SDL_CreateMutex ();
	updateMutex = SDL_CreateMutex ();
	updateCond = SDL_CreateCond ();
	backgroundMutex = SDL_CreateMutex ();
}

App::~App () {
	clearRenderTaskQueue ();
	clearUiStack ();

	if (rootPanel) {
		rootPanel->release ();
		rootPanel = NULL;
	}

	if (uniqueIdMutex) {
		SDL_DestroyMutex (uniqueIdMutex);
		uniqueIdMutex = NULL;
	}

	if (uiMutex) {
		SDL_DestroyMutex (uiMutex);
		uiMutex = NULL;
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

	if (backgroundMutex) {
		SDL_DestroyMutex (backgroundMutex);
		backgroundMutex = NULL;
	}
}

App *App::getInstance () {
	if (! appInstance) {
		appInstance = new App ();
	}
	return (appInstance);
}

void App::freeInstance () {
	if (appInstance) {
		delete (appInstance);
		appInstance = NULL;
		IMG_Quit ();
		SDL_Quit ();
	}
}

void App::clearUiStack () {
	Ui *ui;

	SDL_LockMutex (uiMutex);
	while (! uiStack.empty ()) {
		ui = uiStack.back ();
		ui->pause ();
		ui->unload ();
		ui->release ();

		uiStack.pop_back ();
	}
	if (currentUi) {
		currentUi->release ();
		currentUi = NULL;
	}
	SDL_UnlockMutex (uiMutex);
}

void App::clearRenderTaskQueue () {
	SDL_LockMutex (renderTaskMutex);
	while (! renderTaskQueue.empty ()) {
		renderTaskQueue.pop ();
	}
	SDL_UnlockMutex (renderTaskMutex);
}

int App::getImageScale (int w, int h) {
	int i;

	for (i = 0; i < App::numWindowSizes; ++i) {
		if ((w == App::windowWidths[i]) && (h == App::windowHeights[i])) {
			return (i);
		}
	}

	return (-1);
}

int App::run () {
	SDL_version version1, version2;
	SDL_Rect rect;
	int result, delay, i;
	int64_t endtime, elapsed, t1, t2;
	double fps;
	Ui *ui;

	prng.seed ((uint32_t) time (NULL));
	prefsMap.clear ();
	if (prefsPath.empty ()) {
		isPrefsWriteDisabled = true;
	}
	else {
		result = prefsMap.read (prefsPath, true);
		if (result != Result::SUCCESS) {
			Log::write (Log::DEBUG, "Failed to read preferences file; prefsPath=\"%s\" err=%i", prefsPath.c_str (), result);
			prefsMap.clear ();
		}
	}

	isHttpsEnabled = prefsMap.find (App::prefsHttps, true);
	windowWidth = prefsMap.find (App::prefsWindowWidth, 0);
	windowHeight = prefsMap.find (App::prefsWindowHeight, 0);
	imageScale = getImageScale (windowWidth, windowHeight);
	i = prefsMap.find (App::prefsFontScale, App::numFontScales / 2);
	if ((i >= 0) && (i < App::numFontScales)) {
		fontScale = App::fontScales[i];
		nextFontScale = fontScale;
	}

	clipRect.x = 0;
	clipRect.y = 0;
	clipRect.w = windowWidth;
	clipRect.h = windowHeight;
	nextWindowWidth = windowWidth;
	nextWindowHeight = windowHeight;

	if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		Log::write (Log::ERR, "Failed to start SDL: %s", SDL_GetError ());
		return (Result::ERROR_SDL_OPERATION_FAILED);
	}

	if (IMG_Init (IMG_INIT_JPG | IMG_INIT_PNG) != (IMG_INIT_JPG | IMG_INIT_PNG)) {
		Log::write (Log::ERR, "Failed to start SDL_image: %s", IMG_GetError ());
		return (Result::ERROR_SDL_OPERATION_FAILED);
	}

	result = resource.open ();
	if (result != Result::SUCCESS) {
		Log::write (Log::ERR, "Failed to open application resources; err=%i", result);
		return (result);
	}

	// TODO: Load UI text for another language if appropriate (currently defaulting to en)
	result = uiText.load ();
	if (result != Result::SUCCESS) {
		Log::write (Log::ERR, "Failed to load text resources; err=%i", result);
		return (result);
	}

	result = input.start ();
	if (result != Result::SUCCESS) {
		Log::write (Log::ERR, "Failed to acquire application input devices; err=%i", result);
		return (result);
	}

	result = network.start (prefsMap.find (App::prefsNetworkThreads, Network::defaultRequestThreadCount));
	if (result != Result::SUCCESS) {
		Log::write (Log::ERR, "Failed to acquire application network resources; err=%i", result);
		return (result);
	}
	network.addDatagramCallback (App::datagramReceived, NULL);

	agentControl.urlHostname = network.getPrimaryInterfaceAddress ();
	if (agentControl.urlHostname.empty ()) {
		Log::write (Log::WARNING, "Failed to determine local hostname, network services may not be available");
	}
	result = agentControl.start ();
	if (result != Result::SUCCESS) {
		Log::write (Log::ERR, "Failed to start agent control processes; err=%i", result);
		return (result);
	}

	result = SDL_GetDisplayDPI (0, &displayDdpi, &displayHdpi, &displayVdpi);
	if (result != 0) {
		Log::write (Log::WARNING, "Failed to determine display DPI: %s", SDL_GetError ());
		displayHdpi = 72.0f;
		displayVdpi = 72.0f;
	}

	if (imageScale < 0) {
		imageScale = 0;
		windowWidth = App::windowWidths[0];
		windowHeight = App::windowHeights[0];
		result = SDL_GetDisplayUsableBounds (0, &rect);
		if (result != 0) {
			Log::write (Log::WARNING, "Failed to determine display usable bounds: %s", SDL_GetError ());
		}
		else {
			for (i = 0; i < App::numWindowSizes; ++i) {
				if ((rect.w >= App::windowWidths[i]) && (rect.h >= App::windowHeights[i])) {
					imageScale = i;
					windowWidth = App::windowWidths[i];
					windowHeight = App::windowHeights[i];
				}
			}
			Log::write (Log::DEBUG, "Set window size from display usable bounds; boundsRect=x%i,y%i,w%i,h%i windowWidth=%i windowHeight=%i imageScale=%i", rect.x, rect.y, rect.w, rect.h, windowWidth, windowHeight, imageScale);
		}
	}

	result = SDL_CreateWindowAndRenderer (windowWidth, windowHeight, 0, &window, &render);
	if (result != 0) {
		Log::write (Log::ERR, "Failed to create application window: %s", SDL_GetError ());
		return (Result::ERROR_SDL_OPERATION_FAILED);
	}
	SDL_SetWindowTitle (window, uiText.getText (UiTextString::windowTitle).c_str ());

	startTime = Util::getTime ();
	uiConfig.resetScale ();
	result = uiConfig.load (fontScale);
	if (result != Result::SUCCESS) {
		Log::write (Log::ERR, "Failed to load application resources; err=%i", result);
		return (result);
	}
	populateWidgets ();

	ui = new MainUi ();
	setUi (ui);

	updateThread = SDL_CreateThread (App::runUpdateThread, "runUpdateThread", (void *) this);

	SDL_VERSION (&version1);
	SDL_GetVersion (&version2);
	Log::write (Log::INFO, "Application started; buildId=\"%s\" sdlBuildVersion=%i.%i.%i sdlLinkVersion=%i.%i.%i windowSize=%ix%i windowFlags=0x%x diagonalDpi=%.2f horizontalDpi=%.2f verticalDpi=%.2f imageScale=%i minDrawFrameDelay=%i minUpdateFrameDelay=%i", BUILD_ID, version1.major, version1.minor, version1.patch, version2.major, version2.minor, version2.patch, windowWidth, windowHeight, (unsigned int) SDL_GetWindowFlags (window), displayDdpi, displayHdpi, displayVdpi, imageScale, minDrawFrameDelay, minUpdateFrameDelay);

	while (true) {
		if (isShutdown) {
			break;
		}

		t1 = Util::getTime ();
		input.pollEvents ();
#if ENABLE_TEST_KEYS
		if (! isUiPauseKeyPressed) {
			if (input.isKeyDown (SDLK_p) && input.isControlDown ()) {
				isUiPaused = (! isUiPaused);
				isUiPauseKeyPressed = true;
				Log::write (Log::INFO, "Application pause key pressed; isUiPaused=%s", BOOL_STRING (isUiPaused));
			}
		}
		else {
			if (!(input.isKeyDown (SDLK_p) && input.isControlDown ())) {
				isUiPauseKeyPressed = false;
			}
		}
#endif

		if (! FLOAT_EQUALS (fontScale, nextFontScale)) {
			if (uiConfig.reloadFonts (nextFontScale) != Result::SUCCESS) {
				nextFontScale = fontScale;
			}
			else {
				shouldRefreshUi = true;
				fontScale = nextFontScale;
				for (i = 0; i < App::numFontScales; ++i) {
					if (FLOAT_EQUALS (fontScale, App::fontScales[i])) {
						prefsMap.insert (App::prefsFontScale, i);
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
		resource.compact ();

		t2 = Util::getTime ();
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
	clearUiStack ();
	if (rootPanel) {
		rootPanel->release ();
		rootPanel = NULL;
	}
	uiConfig.unload ();
	writePrefsMap ();
	agentControl.stop ();
	resource.compact ();
	resource.close ();

	SDL_DestroyRenderer (render);
	render = NULL;
	SDL_DestroyWindow (window);
	window = NULL;

	endtime = Util::getTime ();
	elapsed = endtime - startTime;
	fps = (double) drawCount;
	if (elapsed > 1000) {
		fps /= ((double) elapsed) / 1000.0f;
	}
	Log::write (Log::INFO, "Application ended; updateCount=%lli drawCount=%lli runtime=%.3fs FPS=%f", (long long) updateCount, (long long) drawCount, ((double) elapsed) / 1000.0f, fps);

	return (Result::SUCCESS);
}

void App::populateWidgets () {
	if (! rootPanel) {
		rootPanel = new Panel ();
		rootPanel->retain ();
		rootPanel->id = getUniqueId ();
		rootPanel->setFixedSize (true, windowWidth, windowHeight);
	}

	if (! mainToolbar) {
		mainToolbar = (Toolbar *) rootPanel->addWidget (new Toolbar (windowWidth));
		mainToolbar->zLevel = 1;
		mainToolbar->setLeftCorner (new LogoWindow ());
	}
	topBarHeight = mainToolbar->position.y + mainToolbar->height;

	if (! secondaryToolbar) {
		secondaryToolbar = (Toolbar *) rootPanel->addWidget (new Toolbar (windowWidth));
		secondaryToolbar->zLevel = 1;
		secondaryToolbar->isVisible = false;
		secondaryToolbar->position.assign (0.0f, windowHeight - secondaryToolbar->height);
	}
	bottomBarHeight = secondaryToolbar->isVisible ? secondaryToolbar->height : 0.0f;

	if (! newsWindow) {
		newsWindow = (NewsWindow *) rootPanel->addWidget (new NewsWindow (windowWidth * uiConfig.rightNavWidthPercent, windowHeight - topBarHeight - bottomBarHeight));
		newsWindow->zLevel = 1;
		newsWindow->isVisible = false;
		newsWindow->position.assign (0.0f, windowWidth - newsWindow->width);
	}
	rightBarWidth = newsWindow->isVisible ? newsWindow->width : 0.0f;

	if (! snackbarWindow) {
		snackbarWindow = (SnackbarWindow *) rootPanel->addWidget (new SnackbarWindow (windowWidth - (uiConfig.paddingSize * 2.0f)));
		snackbarWindow->isVisible = false;
	}
}

void App::shutdown () {
	if (isShuttingDown) {
		return;
	}

	isShuttingDown = true;
	if (currentUi) {
		currentUi->showShutdownWindow ();
	}
	agentControl.stop ();
	network.stop ();
}

void App::datagramReceived (void *callbackData, const char *messageData, int messageLength) {
	App *app;

	app = App::getInstance ();
	app->agentControl.receiveMessage (messageData, messageLength);
}

void App::executeRenderTasks () {
	App::RenderTaskContext ctx;

	SDL_LockMutex (renderTaskMutex);
	while (! renderTaskQueue.empty ()) {
		ctx = renderTaskQueue.front ();
		ctx.callback (ctx.callbackData);
		renderTaskQueue.pop ();
	}
	SDL_UnlockMutex (renderTaskMutex);
}

void App::draw () {
	Ui *ui;
	std::vector<App::UiChange>::iterator i, end;
	SDL_Rect rect;
	int result;

	SDL_LockMutex (uiMutex);
	if (! uiChangeList.empty ()) {
		i = uiChangeList.begin ();
		end = uiChangeList.end ();
		while (i != end) {
			switch (i->type) {
				case App::UI_CHANGE_PUSH: {
					result = i->ui->load ();
					if (result != Result::SUCCESS) {
						Log::write (Log::ERR, "Failed to load UI resources; err=%i", result);
						delete (i->ui);
					}
					else {
						i->ui->retain ();
						uiStack.push_back (i->ui);
					}
					break;
				}
				case App::UI_CHANGE_POP: {
					if (! uiStack.empty ()) {
						ui = uiStack.back ();
						ui->pause ();
						ui->unload ();
						ui->release ();
						ui = NULL;

						uiStack.pop_back ();
					}
					break;
				}
				case App::UI_CHANGE_SET: {
					result = i->ui->load ();
					if (result != Result::SUCCESS) {
						Log::write (Log::ERR, "Failed to load UI resources; err=%i", result);
						delete (i->ui);
					}
					else {
						clearUiStack ();
						i->ui->retain ();
						uiStack.push_back (i->ui);
					}
					break;
				}
			}
			++i;
		}

		uiChangeList.clear ();
	}

	ui = NULL;
	if (! uiStack.empty ()) {
		ui = uiStack.back ();
		ui->retain ();
	}
	SDL_UnlockMutex (uiMutex);

	SDL_LockMutex (backgroundMutex);
	if (! nextBackgroundTexturePath.empty ()) {
		if (! nextBackgroundTexture) {
			nextBackgroundTextureWidth = 0;
			nextBackgroundTextureHeight = 0;
			nextBackgroundTexture = resource.loadTexture (nextBackgroundTexturePath);
			if (! nextBackgroundTexture) {
				Log::write (Log::ERR, "Failed to load background texture; path=%s", nextBackgroundTexturePath.c_str ());
				nextBackgroundTexturePath.assign ("");
			}
			else {
				SDL_QueryTexture (nextBackgroundTexture, NULL, NULL, &nextBackgroundTextureWidth, &nextBackgroundTextureHeight);
				backgroundTransitionAlpha = 0.0f;
			}
		}
	}

	SDL_RenderClear (render);
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
		SDL_SetTextureAlphaMod (nextBackgroundTexture, (Uint8) (backgroundTransitionAlpha * 255.0f));
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
	int64_t t1, t2, last;
	int delay;

	app = (App *) appPtr;
	last = Util::getTime ();
	while (true) {
		if (app->isShutdown) {
			break;
		}

		t1 = Util::getTime ();
		app->update ((int) (t1 - last));
		t2 = Util::getTime ();
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
	RecordStore *store;

	agentControl.update (msElapsed);

	ui = NULL;
	SDL_LockMutex (uiMutex);
	if (! uiStack.empty ()) {
		ui = uiStack.back ();
		if (ui != currentUi) {
			if (currentUi) {
				currentUi->pause ();
				currentUi->release ();
			}
			currentUi = ui;
			currentUi->retain ();

			rootPanel->resetInputState ();
			currentUi->resetMainToolbar (mainToolbar);
			currentUi->resetSecondaryToolbar (secondaryToolbar);
			topBarHeight = mainToolbar->position.y + mainToolbar->height;
			bottomBarHeight = secondaryToolbar->isVisible ? secondaryToolbar->height : 0.0f;

			if (newsWindow->isVisible) {
				newsWindow->setViewSize (windowWidth * uiConfig.rightNavWidthPercent, windowHeight - topBarHeight - bottomBarHeight);
				rightBarWidth = newsWindow->width;
				newsWindow->position.assign (windowWidth - newsWindow->width, topBarHeight);
			}
			else {
				rightBarWidth = 0.0f;
			}
			secondaryToolbar->position.assign (0.0f, windowHeight - secondaryToolbar->height);
			snackbarWindow->isVisible = false;

			currentUi->resume ();

			shouldSyncRecordStore = true;
		}
		ui->retain ();
	}

	if (shouldRefreshUi) {
		refreshUi ();
		shouldRefreshUi = false;
	}
	SDL_UnlockMutex (uiMutex);

	if (shouldSyncRecordStore) {
		store = &(agentControl.recordStore);
		store->lock ();
		rootPanel->syncRecordStore (store);
		if (ui) {
			ui->syncRecordStore (store);
		}
		store->compact ();
		store->unlock ();
		shouldSyncRecordStore = false;
	}

	SDL_LockMutex (backgroundMutex);
	if (nextBackgroundTexture) {
		if (uiConfig.backgroundTransitionDuration <= 0) {
			backgroundTransitionAlpha = 1.0f;
		}
		else {
			backgroundTransitionAlpha += (1.0f / ((float) uiConfig.backgroundTransitionDuration)) * (float) msElapsed;
		}

		if (backgroundTransitionAlpha >= 1.0f) {
			backgroundTransitionAlpha = 1.0f;
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

	writePrefsMap ();
	++updateCount;

	SDL_LockMutex (updateMutex);
	while (isSuspendingUpdate) {
		SDL_CondBroadcast (updateCond);
		SDL_CondWait (updateCond, updateMutex);
	}
	SDL_UnlockMutex (updateMutex);

	if (isShuttingDown) {
		if (network.isStopComplete ()) {
			network.waitThreads ();
			isShutdown = true;
		}
	}
}

void App::refreshUi () {
	std::vector<Ui *>::iterator i, end;


	rootPanel->refresh ();
	topBarHeight = mainToolbar->position.y + mainToolbar->height;
	bottomBarHeight = secondaryToolbar->isVisible ? secondaryToolbar->height : 0.0f;
	i = uiStack.begin ();
	end = uiStack.end ();
	while (i != end) {
		(*i)->refresh ();
		++i;
	}

	secondaryToolbar->position.assign (0.0f, windowHeight - secondaryToolbar->height);
	newsWindow->position.assign (windowWidth - newsWindow->width, topBarHeight);
	newsWindow->setViewSize (windowWidth * uiConfig.rightNavWidthPercent, windowHeight - topBarHeight - bottomBarHeight);
	snackbarWindow->position.assign (windowWidth - rightBarWidth - snackbarWindow->width, topBarHeight);
}

int64_t App::getUniqueId () {
	int64_t id;

	SDL_LockMutex (uniqueIdMutex);
	id = nextUniqueId;
	++nextUniqueId;
	SDL_UnlockMutex (uniqueIdMutex);

	return (id);
}

void App::setUi (Ui *ui) {
	SDL_LockMutex (uiMutex);
	uiChangeList.push_back (App::UiChange (ui, App::UI_CHANGE_SET));
	SDL_UnlockMutex (uiMutex);
}

void App::pushUi (Ui *ui) {
	SDL_LockMutex (uiMutex);
	uiChangeList.push_back (App::UiChange (ui, App::UI_CHANGE_PUSH));
	SDL_UnlockMutex (uiMutex);
}

void App::popUi () {
	SDL_LockMutex (uiMutex);
	uiChangeList.push_back (App::UiChange (NULL, App::UI_CHANGE_POP));
	SDL_UnlockMutex (uiMutex);
}

Ui *App::getCurrentUi () {
	Ui *ui;

	ui = NULL;
	SDL_LockMutex (uiMutex);
	if (currentUi) {
		ui = currentUi;
		ui->retain ();
	}
	SDL_UnlockMutex (uiMutex);

	return (ui);
}

void App::toggleNewsWindow () {
	if (newsWindow->isVisible) {
		newsWindow->isVisible = false;
		rightBarWidth = 0.0f;
	}
	else {
		newsWindow->setViewSize (windowWidth * uiConfig.rightNavWidthPercent, windowHeight - topBarHeight - bottomBarHeight);
		newsWindow->position.assign (windowWidth - newsWindow->width, topBarHeight);
		rightBarWidth = newsWindow->width;
		newsWindow->isVisible = true;
	}
	shouldRefreshUi = true;
}

void App::showSnackbar (const StdString &messageText, const StdString &actionButtonText, Widget::EventCallback actionButtonClickCallback, void *actionButtonClickCallbackData) {
	float y;

	snackbarWindow->setMaxWidth (windowWidth - (uiConfig.paddingSize * 2.0f) - rightBarWidth);
	snackbarWindow->setMessageText (messageText);
	if ((! actionButtonText.empty ()) && actionButtonClickCallback) {
		snackbarWindow->setActionButtonEnabled (true, actionButtonText, actionButtonClickCallback, actionButtonClickCallbackData);
	}
	else {
		snackbarWindow->setActionButtonEnabled (false);
	}

	if (currentUi && currentUi->isSideWindowOpen ()) {
		y = 0.0f;
	}
	else {
		y = topBarHeight;
	}
	snackbarWindow->position.assign (windowWidth - rightBarWidth - snackbarWindow->width, y);
	snackbarWindow->startTimeout (uiConfig.snackbarTimeout);
	snackbarWindow->startScroll (uiConfig.snackbarScrollDuration);
	if (snackbarWindow->zLevel < rootPanel->maxWidgetZLevel) {
		snackbarWindow->zLevel = rootPanel->maxWidgetZLevel + 1;
	}
	snackbarWindow->isVisible = true;
}

void App::handleLinkClientCommand (const StdString &agentId, Json *command) {
	Ui *ui;
	int commandid;

	commandid = systemInterface.getCommandId (command);
	switch (commandid) {
		case SystemInterface::Command_TaskItem: {
			agentControl.recordStore.addRecord (command);
			shouldSyncRecordStore = true;
			break;
		}
	}

	ui = getCurrentUi ();
	if (ui) {
		ui->handleLinkClientCommand (agentId, commandid, command);
		ui->release ();
	}
}

void App::handleLinkClientConnect (const StdString &agentId) {
	std::vector<Ui *>::iterator i, end;

	SDL_LockMutex (uiMutex);
	i = uiStack.begin ();
	end = uiStack.end ();
	while (i != end) {
		(*i)->handleLinkClientConnect (agentId);
		++i;
	}
	SDL_UnlockMutex (uiMutex);

	shouldRefreshUi = true;
}

void App::handleLinkClientDisconnect (const StdString &agentId, const StdString &errorDescription) {
	std::vector<Ui *>::iterator i, end;

	SDL_LockMutex (uiMutex);
	i = uiStack.begin ();
	end = uiStack.end ();
	while (i != end) {
		(*i)->handleLinkClientDisconnect (agentId, errorDescription);
		++i;
	}
	SDL_UnlockMutex (uiMutex);

	shouldRefreshUi = true;
}

void App::pushClipRect (const SDL_Rect *rect) {
	int x, y, w, h, diff;

	if (clipRectStack.empty ()) {
		clipRect = *rect;
	}
	else {
		x = rect->x;
		y = rect->y;
		w = rect->w;
		h = rect->h;

		diff = x - clipRect.x;
		if (diff < 0) {
			w -= diff;
			x = clipRect.x;
		}
		diff = y - clipRect.y;
		if (diff < 0) {
			h -= diff;
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

		clipRect.x = x;
		clipRect.y = y;
		clipRect.w = w;
		clipRect.h = h;
	}
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

Json *App::createCommand (const char *commandName, int commandType, Json *commandParams) {
	Json *cmd;
	SystemInterface::Prefix prefix;

	prefix = createCommandPrefix ();
	cmd = systemInterface.createCommand (prefix, commandName, commandType, commandParams);
	if (! cmd) {
		Log::write (Log::ERR, "Failed to create SystemInterface command; commandName=\"%s\" commandType=%i err=\"%s\"", commandName, commandType, systemInterface.lastError.c_str ());
	}
	return (cmd);
}

StdString App::createCommandJson (const char *commandName, int commandType, Json *commandParams) {
	Json *cmd;
	StdString json;

	cmd = createCommand (commandName, commandType, commandParams);
	if (! cmd) {
		return (StdString (""));
	}
	json = cmd->toString ();
	delete (cmd);

	return (json);
}

SystemInterface::Prefix App::createCommandPrefix () {
	SystemInterface::Prefix prefix;

	// TODO: Possibly set different fields here
	prefix.agentId.assign (agentControl.agentId);
	prefix.userId.assign ("");
	prefix.priority = 0;
	prefix.startTime = Util::getTime ();
	prefix.duration = 0;

	return (prefix);
}

void App::addRenderTask (RenderTaskFunction fn, void *fnData) {
	App::RenderTaskContext ctx;

	if (! fn) {
		return;
	}

	ctx.callback = fn;
	ctx.callbackData = fnData;
	SDL_LockMutex (renderTaskMutex);
	renderTaskQueue.push (ctx);
	SDL_UnlockMutex (renderTaskMutex);
}

void App::writePrefsMap () {
	int result;

	if (isPrefsWriteDisabled) {
		return;
	}

	if (prefsMap.isWriteDirty) {
		result = prefsMap.write (prefsPath);
		if (result != Result::SUCCESS) {
			Log::write (Log::ERR, "Failed to write prefs file; prefsPath=\"%s\" err=%i", prefsPath.c_str (), result);
			isPrefsWriteDisabled = true;
		}
	}
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

	SDL_LockMutex (updateMutex);
	isSuspendingUpdate = true;
	SDL_CondWait (updateCond, updateMutex);

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
	mainToolbar->setWidth (windowWidth);
	secondaryToolbar->setWidth (windowWidth);
	newsWindow->setViewSize (windowWidth * uiConfig.rightNavWidthPercent, windowHeight - topBarHeight - bottomBarHeight);

	uiConfig.coreSprites.resize ();
	result = uiConfig.reloadFonts (fontScale);
	if (result != Result::SUCCESS) {
		Log::write (Log::ERR, "Failed to reload fonts; fontScale=%.2f err=%i", fontScale, result);
	}
	if (! backgroundTextureBasePath.empty ()) {
		setNextBackgroundTexturePath (backgroundTextureBasePath);
	}

	SDL_LockMutex (uiMutex);
	i = uiStack.begin ();
	end = uiStack.end ();
	while (i != end) {
		(*i)->resize ();
		++i;
	}
	SDL_UnlockMutex (uiMutex);

	secondaryToolbar->position.assign (0.0f, windowHeight - secondaryToolbar->height);
	newsWindow->position.assign (windowWidth - newsWindow->width, topBarHeight);
	rootPanel->resetInputState ();

	if (currentUi) {
		currentUi->resetMainToolbar (mainToolbar);
		currentUi->resetSecondaryToolbar (secondaryToolbar);
	}

	shouldRefreshUi = true;

	isSuspendingUpdate = false;
	SDL_CondBroadcast (updateCond);
	SDL_UnlockMutex (updateMutex);

	prefsMap.insert (App::prefsWindowWidth, windowWidth);
	prefsMap.insert (App::prefsWindowHeight, windowHeight);
}

void App::setNextBackgroundTexturePath (const StdString &path) {
	StdString filepath;

	backgroundTextureBasePath.assign (path);
	filepath = getBackgroundTexturePath (backgroundTextureBasePath);
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

StdString App::getBackgroundTexturePath (const StdString &basePath) {
	return (StdString::createSprintf ("%s/%i.png", basePath.c_str (), imageScale));
}
