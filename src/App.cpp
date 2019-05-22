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
#include "Config.h"
#include <stdlib.h>
#include <math.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "Result.h"
#include "StdString.h"
#include "Log.h"
#include "OsUtil.h"
#include "Input.h"
#include "Network.h"
#include "Panel.h"
#include "Toolbar.h"
#include "SnackbarWindow.h"
#include "LogoWindow.h"
#include "MainUi.h"
#include "App.h"

const int App::defaultMinFrameDelay = 20;
const int App::windowWidths[] = { 768, 1024, 1280, 1600, 1920 };
const int App::windowHeights[] = { 432, 576, 720, 900, 1080 };
const int App::windowSizeCount = 5;
const float App::fontScales[] = { 0.66f, 0.8f, 1.0f, 1.25f, 1.5f };
const int App::fontScaleCount = 5;
const StdString App::serverUrl = StdString ("https://membranesoftware.com/");
const int64_t App::defaultServerTimeout = 180;

const char *App::NetworkThreadsKey = "NetworkThreads";
const char *App::WindowWidthKey = "WindowWidth";
const char *App::WindowHeightKey = "WindowHeight";
const char *App::FontScaleKey = "FontScale";
const char *App::HttpsKey = "Https";
const char *App::ShowInterfaceAnimationsKey = "ShowInterfaceAnimations";
const char *App::ShowClockKey = "ShowClock";
const char *App::IsFirstLaunchCompleteKey = "IsFirstLaunchComplete";
const char *App::AgentStatusKey = "AgentStatus";
const char *App::MediaImageSizeKey = "Media_ImageSize";
const char *App::MonitorImageSizeKey = "Monitor_ImageSize";
const char *App::MediaItemImageSizeKey = "MediaItem_ImageSize";
const char *App::StreamItemImageSizeKey = "StreamItem_ImageSize";
const char *App::ServerAdminSecretsKey = "ServerAdminSecrets";
const char *App::ServerTimeoutKey = "ServerTimeout";
const char *App::WebKioskUiSelectedAgentsKey = "WebKiosk_SelectedAgents";
const char *App::WebKioskUiExpandedAgentsKey = "WebKiosk_ExpandedAgents";
const char *App::WebKioskUiPlaylistsKey = "WebKiosk_Playlists";
const char *App::WebKioskUiToolbarModeKey = "WebKiosk_ToolbarMode";
const char *App::MediaUiSelectedAgentsKey = "Media_SelectedAgents";
const char *App::MediaUiExpandedAgentsKey = "Media_ExpandedAgents";
const char *App::MediaUiPlaylistsKey = "Media_Playlists";
const char *App::MediaUiToolbarModeKey = "Media_ToolbarMode";
const char *App::MainUiShowAllEnabledKey = "Main_ShowAllEnabled";
const char *App::MainUiApplicationNewsItemsKey = "Main_ApplicationNewsItems";
const char *App::MediaItemUiVideoQualityKey = "MediaItem_VideoQuality";
const char *App::MediaUiVideoQualityKey = "Media_VideoQuality";
const char *App::MediaUiVisibilityKey = "Media_Visibility";

App *App::instance = NULL;

void App::createInstance () {
	if (App::instance) {
		delete (App::instance);
	}
	App::instance = new App ();
}

void App::freeInstance () {
	if (App::instance) {
		delete (App::instance);
		App::instance = NULL;
		IMG_Quit ();
		SDL_Quit ();
	}
}

App::App ()
: shouldRefreshUi (false)
, shouldSyncRecordStore (false)
, isInterfaceAnimationEnabled (false)
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
, isTextureRenderEnabled (false)
, rootPanel (NULL)
, displayDdpi (0.0f)
, displayHdpi (0.0f)
, displayVdpi (0.0f)
, windowWidth (0)
, windowHeight (0)
, minDrawFrameDelay (App::defaultMinFrameDelay)
, minUpdateFrameDelay (App::defaultMinFrameDelay)
, fontScale (1.0f)
, imageScale (0)
, drawCount (0)
, updateCount (0)
, isPrefsWriteDisabled (false)
, updateThread (NULL)
, uniqueIdMutex (NULL)
, nextUniqueId (1)
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
, backgroundCrossFadeAlpha (0.0f)
{
	uniqueIdMutex = SDL_CreateMutex ();
	renderTaskMutex = SDL_CreateMutex ();
	updateMutex = SDL_CreateMutex ();
	updateCond = SDL_CreateCond ();
	backgroundMutex = SDL_CreateMutex ();
}

App::~App () {
	clearRenderTaskQueue ();

	if (rootPanel) {
		rootPanel->release ();
		rootPanel = NULL;
	}

	if (uniqueIdMutex) {
		SDL_DestroyMutex (uniqueIdMutex);
		uniqueIdMutex = NULL;
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

void App::clearRenderTaskQueue () {
	SDL_LockMutex (renderTaskMutex);
	while (! renderTaskQueue.empty ()) {
		renderTaskQueue.pop ();
	}
	SDL_UnlockMutex (renderTaskMutex);
}

int App::getImageScale (int w, int h) {
	int i;

	for (i = 0; i < App::windowSizeCount; ++i) {
		if ((w == App::windowWidths[i]) && (h == App::windowHeights[i])) {
			return (i);
		}
	}

	return (-1);
}

int App::run () {
	SDL_version version1, version2;
	SDL_RendererInfo renderinfo;
	SDL_Rect rect;
	int result, delay, i;
	int64_t endtime, elapsed, t1, t2;
	double fps;
	Ui *ui;

	prng.seed ((uint32_t) (OsUtil::getTime () & 0xFFFFFFFF));
	prefsMap.clear ();
	if (prefsPath.empty ()) {
		isPrefsWriteDisabled = true;
	}
	else {
		result = prefsMap.read (prefsPath, true);
		if (result != Result::Success) {
			Log::debug ("Failed to read preferences file; prefsPath=\"%s\" err=%i", prefsPath.c_str (), result);
			prefsMap.clear ();
		}
	}

	isHttpsEnabled = prefsMap.find (App::HttpsKey, true);
	windowWidth = prefsMap.find (App::WindowWidthKey, 0);
	windowHeight = prefsMap.find (App::WindowHeightKey, 0);
	imageScale = getImageScale (windowWidth, windowHeight);
	i = prefsMap.find (App::FontScaleKey, App::fontScaleCount / 2);
	if ((i >= 0) && (i < App::fontScaleCount)) {
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
		Log::err ("Failed to start SDL: %s", SDL_GetError ());
		return (Result::SdlOperationFailedError);
	}

	if (IMG_Init (IMG_INIT_JPG | IMG_INIT_PNG) != (IMG_INIT_JPG | IMG_INIT_PNG)) {
		Log::err ("Failed to start SDL_image: %s", IMG_GetError ());
		return (Result::SdlOperationFailedError);
	}

	result = resource.open ();
	if (result != Result::Success) {
		Log::err ("Failed to open application resources; err=%i", result);
		return (result);
	}

	result = uiText.load (OsUtil::getEnvLanguage (UiText::defaultLanguage));
	if (result != Result::Success) {
		Log::err ("Failed to load text resources; err=%i", result);
		return (result);
	}

	result = input.start ();
	if (result != Result::Success) {
		Log::err ("Failed to acquire application input devices; err=%i", result);
		return (result);
	}

	network.httpUserAgent.sprintf ("membrane-control/%s_%s", BUILD_ID, PLATFORM_ID);
	result = network.start (prefsMap.find (App::NetworkThreadsKey, Network::defaultRequestThreadCount));
	if (result != Result::Success) {
		Log::err ("Failed to acquire application network resources; err=%i", result);
		return (result);
	}
	network.addDatagramCallback (App::datagramReceived, NULL);

	agentControl.urlHostname = network.getPrimaryInterfaceAddress ();
	if (agentControl.urlHostname.empty ()) {
		Log::warning ("Failed to determine local hostname, network services may not be available");
	}
	result = agentControl.start ();
	if (result != Result::Success) {
		Log::err ("Failed to start agent control processes; err=%i", result);
		return (result);
	}

	result = SDL_GetDisplayDPI (0, &displayDdpi, &displayHdpi, &displayVdpi);
	if (result != 0) {
		Log::warning ("Failed to determine display DPI: %s", SDL_GetError ());
		displayHdpi = 72.0f;
		displayVdpi = 72.0f;
	}

	if (imageScale < 0) {
		imageScale = 0;
		windowWidth = App::windowWidths[0];
		windowHeight = App::windowHeights[0];
		result = SDL_GetDisplayUsableBounds (0, &rect);
		if (result != 0) {
			Log::warning ("Failed to determine display usable bounds: %s", SDL_GetError ());
		}
		else {
			for (i = 0; i < App::windowSizeCount; ++i) {
				if ((rect.w >= App::windowWidths[i]) && (rect.h >= App::windowHeights[i])) {
					imageScale = i;
					windowWidth = App::windowWidths[i];
					windowHeight = App::windowHeights[i];
				}
			}
			Log::debug ("Set window size from display usable bounds; boundsRect=x%i,y%i,w%i,h%i windowWidth=%i windowHeight=%i imageScale=%i", rect.x, rect.y, rect.w, rect.h, windowWidth, windowHeight, imageScale);
		}
	}

	result = SDL_CreateWindowAndRenderer (windowWidth, windowHeight, 0, &window, &render);
	if (result != 0) {
		Log::err ("Failed to create application window: %s", SDL_GetError ());
		return (Result::SdlOperationFailedError);
	}
	result = SDL_GetRendererInfo (render, &renderinfo);
	if (result != 0) {
		Log::err ("Failed to create application renderer: %s", SDL_GetError ());
		return (Result::SdlOperationFailedError);
	}
	if ((renderinfo.flags & (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE)) == (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE)) {
		isTextureRenderEnabled = true;
	}
	if (isTextureRenderEnabled) {
		isInterfaceAnimationEnabled = prefsMap.find (App::ShowInterfaceAnimationsKey, true);
	}

	SDL_SetWindowTitle (window, uiText.getText (UiTextString::windowTitle).c_str ());

	startTime = OsUtil::getTime ();
	uiConfig.resetScale ();
	result = uiConfig.load (fontScale);
	if (result != Result::Success) {
		Log::err ("Failed to load application resources; err=%i", result);
		return (result);
	}
	populateWidgets ();

	ui = new MainUi ();
	uiStack.setUi (ui);

	updateThread = SDL_CreateThread (App::runUpdateThread, "runUpdateThread", (void *) this);

	SDL_VERSION (&version1);
	SDL_GetVersion (&version2);
	Log::info ("Application started; buildId=\"%s\" sdlBuildVersion=%i.%i.%i sdlLinkVersion=%i.%i.%i windowSize=%ix%i windowFlags=0x%x renderFlags=0x%x isTextureRenderEnabled=%s diagonalDpi=%.2f horizontalDpi=%.2f verticalDpi=%.2f imageScale=%i minDrawFrameDelay=%i minUpdateFrameDelay=%i lang=%s", BUILD_ID, version1.major, version1.minor, version1.patch, version2.major, version2.minor, version2.patch, windowWidth, windowHeight, (unsigned int) SDL_GetWindowFlags (window), (unsigned int) renderinfo.flags, BOOL_STRING (isTextureRenderEnabled), displayDdpi, displayHdpi, displayVdpi, imageScale, minDrawFrameDelay, minUpdateFrameDelay, OsUtil::getEnvLanguage ("").c_str ());

	while (true) {
		if (isShutdown) {
			break;
		}

		t1 = OsUtil::getTime ();
		input.pollEvents ();
#if ENABLE_TEST_KEYS
		if (! isUiPauseKeyPressed) {
			if (input.isKeyDown (SDLK_p) && input.isControlDown ()) {
				isUiPaused = (! isUiPaused);
				isUiPauseKeyPressed = true;
				Log::info ("Application pause key pressed; isUiPaused=%s", BOOL_STRING (isUiPaused));
			}
		}
		else {
			if (!(input.isKeyDown (SDLK_p) && input.isControlDown ())) {
				isUiPauseKeyPressed = false;
			}
		}
#endif

		if (! FLOAT_EQUALS (fontScale, nextFontScale)) {
			if (uiConfig.reloadFonts (nextFontScale) != Result::Success) {
				nextFontScale = fontScale;
			}
			else {
				shouldRefreshUi = true;
				fontScale = nextFontScale;
				for (i = 0; i < App::fontScaleCount; ++i) {
					if (FLOAT_EQUALS (fontScale, App::fontScales[i])) {
						prefsMap.insert (App::FontScaleKey, i);
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
	writePrefsMap ();
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
	Log::info ("Application ended; updateCount=%lli drawCount=%lli runtime=%.3fs FPS=%f", (long long) updateCount, (long long) drawCount, ((double) elapsed) / 1000.0f, fps);

	return (Result::Success);
}

void App::populateWidgets () {
	if (! rootPanel) {
		rootPanel = new Panel ();
		rootPanel->retain ();
		rootPanel->id = getUniqueId ();
		rootPanel->setFixedSize (true, windowWidth, windowHeight);
	}
	uiStack.populateWidgets ();
}

void App::shutdown () {
	Ui *ui;

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
	network.stop ();
}

void App::datagramReceived (void *callbackData, const char *messageData, int messageLength) {
	App::instance->agentControl.receiveMessage (messageData, messageLength);
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
	int64_t t1, t2, last;
	int delay;

	app = (App *) appPtr;
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
	RecordStore *store;

	agentControl.update (msElapsed);

	uiStack.update (msElapsed);
	if (shouldRefreshUi) {
		uiStack.refresh ();
		rootPanel->refresh ();
		shouldRefreshUi = false;
	}
	ui = uiStack.getActiveUi ();

	if (shouldSyncRecordStore) {
		store = &(agentControl.recordStore);
		store->lock ();
		rootPanel->syncRecordStore ();
		if (ui) {
			ui->syncRecordStore ();
		}
		store->compact ();
		store->unlock ();
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

int64_t App::getUniqueId () {
	int64_t id;

	SDL_LockMutex (uniqueIdMutex);
	id = nextUniqueId;
	++nextUniqueId;
	SDL_UnlockMutex (uniqueIdMutex);

	return (id);
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
		if (result != Result::Success) {
			Log::err ("Failed to write prefs file; prefsPath=\"%s\" err=%i", prefsPath.c_str (), result);
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

	uiConfig.coreSprites.resize ();
	result = uiConfig.reloadFonts (fontScale);
	if (result != Result::Success) {
		Log::err ("Failed to reload fonts; fontScale=%.2f err=%i", fontScale, result);
	}
	if (! backgroundTextureBasePath.empty ()) {
		setNextBackgroundTexturePath (backgroundTextureBasePath);
	}

	uiStack.resize ();
	rootPanel->resetInputState ();
	shouldRefreshUi = true;

	isSuspendingUpdate = false;
	SDL_CondBroadcast (updateCond);
	SDL_UnlockMutex (updateMutex);

	prefsMap.insert (App::WindowWidthKey, windowWidth);
	prefsMap.insert (App::WindowHeightKey, windowHeight);
}

int App::getRandomInt (int i1, int i2) {
	return (prng.getRandomValue (i1, i2));
}

const char RANDOM_STRING_CHARS[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
StdString App::getRandomString (int stringLength) {
	StdString s;
	int i;

	for (i = 0; i < stringLength; ++i) {
		s.append (1, RANDOM_STRING_CHARS[getRandomInt (0, sizeof (RANDOM_STRING_CHARS) - 1)]);
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

void App::handleLinkClientCommand (const StdString &agentId, Json *command) {
	Ui *ui;
	int commandid;

	commandid = systemInterface.getCommandId (command);
	switch (commandid) {
		case SystemInterface::CommandId_TaskItem: {
			agentControl.recordStore.addRecord (command);
			shouldSyncRecordStore = true;
			break;
		}
	}

	ui = uiStack.getActiveUi ();
	if (ui) {
		ui->handleLinkClientCommand (agentId, commandid, command);
		ui->release ();
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

Json *App::createCommand (const char *commandName, int commandType, Json *commandParams) {
	Json *cmd;
	SystemInterface::Prefix prefix;

	prefix = createCommandPrefix ();
	cmd = systemInterface.createCommand (prefix, commandName, commandType, commandParams);
	if (! cmd) {
		Log::err ("Failed to create SystemInterface command; commandName=\"%s\" commandType=%i err=\"%s\"", commandName, commandType, systemInterface.lastError.c_str ());
	}
	return (cmd);
}

SystemInterface::Prefix App::createCommandPrefix () {
	SystemInterface::Prefix prefix;

	prefix.createTime = OsUtil::getTime ();
	prefix.agentId.assign (agentControl.agentId);

	// TODO: Possibly assign values to these fields
	prefix.userId.assign ("");
	prefix.priority = 0;
	prefix.startTime = 0;
	prefix.duration = 0;

	return (prefix);
}

StdString App::getHelpUrl (const StdString &topicId) {
	return (StdString::createSprintf ("%si/%s", App::serverUrl.c_str (), topicId.urlEncoded ().c_str ()));
}

StdString App::getHelpUrl (const char *topicId) {
	return (App::getHelpUrl (StdString (topicId)));
}

StdString App::getApplicationUrl (const StdString &applicationId) {
	return (StdString::createSprintf ("%s%s", App::serverUrl.c_str (), applicationId.urlEncoded ().c_str ()));
}

StdString App::getFeatureUrl (const StdString &featureId) {
	return (StdString::createSprintf ("%sfeature/%s", App::serverUrl.c_str (), featureId.urlEncoded ().c_str ()));
}

StdString App::getFeedbackUrl (bool shouldIncludeVersion) {
	if (! shouldIncludeVersion) {
		return (StdString::createSprintf ("%scontact", App::serverUrl.c_str ()));
	}

	return (StdString::createSprintf ("%scontact/%s", App::serverUrl.c_str (), StdString (BUILD_ID).urlEncoded ().c_str ()));
}

StdString App::getUpdateUrl (const StdString &applicationId) {
	if (! applicationId.empty ()) {
		return (StdString::createSprintf ("%supdate/%s", App::serverUrl.c_str (), applicationId.c_str ()));
	}

	return (StdString::createSprintf ("%supdate/%s_%s", App::serverUrl.c_str (), StdString (BUILD_ID).urlEncoded ().c_str (), StdString (PLATFORM_ID).urlEncoded ().c_str ()));
}

StdString App::getApplicationNewsUrl () {
	return (StdString::createSprintf ("%sapplication-news/%s_%s_%s", App::serverUrl.c_str (), StdString (BUILD_ID).urlEncoded ().c_str (), StdString (PLATFORM_ID).urlEncoded ().c_str (), OsUtil::getEnvLanguage ("en").urlEncoded ().c_str ()));
}

StdString App::getDonateUrl () {
	return (StdString::createSprintf ("%scontribute", App::serverUrl.c_str ()));
}
