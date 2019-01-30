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
// Class that runs the application and holds global state

#ifndef APP_H
#define APP_H

#include <vector>
#include <stack>
#include <queue>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "Buffer.h"
#include "Input.h"
#include "Resource.h"
#include "Network.h"
#include "HashMap.h"
#include "Json.h"
#include "Prng.h"
#include "Ui.h"
#include "UiText.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "Panel.h"
#include "Toolbar.h"
#include "NewsWindow.h"
#include "SnackbarWindow.h"
#include "AgentControl.h"
#ifndef ENABLE_TEST_KEYS
#define ENABLE_TEST_KEYS 0
#endif

class App {
public:
	App ();
	~App ();

	// Return the singleton instance of this class, creating it if necessary
	static App *getInstance ();

	// Destroy any previously created singleton instance of this class
	static void freeInstance ();

	static const int defaultMinFrameDelay;
	static const int windowWidths[];
	static const int windowHeights[];
	static const int numWindowSizes;
	static const float fontScales[];
	static const int numFontScales;
	static const StdString prefsFilename;

	// Constants to use as key values in the prefs map
	static const StdString prefsIsFirstLaunchComplete;
	static const StdString prefsNetworkThreads;
	static const StdString prefsAgentStatus;
	static const StdString prefsWindowWidth;
	static const StdString prefsWindowHeight;
	static const StdString prefsFontScale;
	static const StdString prefsShowClock;
	static const StdString prefsMediaImageSize;
	static const StdString prefsMonitorImageSize;
	static const StdString prefsMediaItemImageSize;
	static const StdString prefsStreamItemImageSize;
	static const StdString prefsServerAdminSecrets;
	static const StdString prefsServerTimeout; // seconds
	static const int64_t defaultServerTimeout;
	static const StdString prefsServerPath;
	static const StdString prefsApplicationName;
	static const StdString prefsHttps;
	static const StdString prefsWebKioskUiSelectedAgents;
	static const StdString prefsWebKioskUiExpandedAgents;
	static const StdString prefsWebKioskUiPlaylists;
	static const StdString prefsMediaUiExpandedAgents;
	static const StdString prefsMonitorUiPlaylists;
	static const StdString prefsMonitorUiSelectedAgents;
	static const StdString prefsMonitorUiExpandedAgents;
	static const StdString prefsMainUiShowAllEnabled;

	// Read-write data members
	bool shouldSyncRecordStore;
	bool shouldRefreshUi;
	float nextFontScale;
	int nextWindowWidth;
	int nextWindowHeight;
#if ENABLE_TEST_KEYS
	bool isUiPaused;
	bool isUiPauseKeyPressed;
#endif
	StdString prefsPath;
	Prng prng;
	Input input;
	UiText uiText;
	UiConfiguration uiConfig;
	Resource resource;
	Network network;
	SystemInterface systemInterface;
	AgentControl agentControl;
	HashMap prefsMap;

	// Read-only data members
	bool isShuttingDown;
	bool isShutdown;
	int64_t startTime;
	bool isHttpsEnabled;
	SDL_Window *window;
	SDL_Renderer *render; // The renderer must be accessed only from the application's main thread
	Panel *rootPanel;
	Toolbar *mainToolbar;
	Toolbar *secondaryToolbar;
	NewsWindow *newsWindow;
	SnackbarWindow *snackbarWindow;
	float displayDdpi;
	float displayHdpi;
	float displayVdpi;
	int windowWidth;
	int windowHeight;
	int minDrawFrameDelay; // milliseconds
	int minUpdateFrameDelay; // milliseconds
	float topBarHeight;
	float bottomBarHeight;
	float rightBarWidth;
	float fontScale;
	int imageScale;
	int64_t drawCount;
	int64_t updateCount;
	bool isPrefsWriteDisabled;

	// Run the application, returning only after the application exits
	int run ();

	// Begin the application's shutdown process and halt execution when complete
	void shutdown ();

	// Return an int64_t value for use as a unique ID
	int64_t getUniqueId ();

	// Remove all Ui objects and add the provided one as the top item in the Ui stack
	void setUi (Ui *ui);

	// Push the provided Ui object to the top of the stack
	void pushUi (Ui *ui);

	// Remove and unload the top item in the Ui stack
	void popUi ();

	// Return the topmost item in the Ui stack, or NULL if no such item was found. If a Ui object is returned by this method, it has been retained and must be released by the caller when no longer needed.
	Ui *getCurrentUi ();

	// Toggle the visible state of the news window, shown as a right nav
	void toggleNewsWindow ();

	// Show the specified message in the application's snackbar window, optionally including an action button
	void showSnackbar (const StdString &messageText, const StdString &actionButtonText = StdString (""), Widget::EventCallback actionButtonClickCallback = NULL, void *actionButtonClickCallbackData = NULL);

	// Execute actions appropriate for a command received from the agent control link client
	void handleLinkClientCommand (const StdString &agentId, Json *command);

	// Execute actions appropriate for a connect event from an agent control link client
	void handleLinkClientConnect (const StdString &agentId);

	// Execute actions appropriate for a disconnect event from an agent control link client
	void handleLinkClientDisconnect (const StdString &agentId, const StdString &errorDescription);

	// Apply the provided clip rectangle to the application's renderer and push it onto the clip stack
	void pushClipRect (const SDL_Rect *rect);

	// Pop the clip stack
	void popClipRect ();

	// Return a SystemInterface::Prefix struct containing command prefix fields
	SystemInterface::Prefix createCommandPrefix ();

	// Return a newly created Json object containing the specified command and the default prefix, or NULL if the command could not be created. commandParams can be NULL if not needed, causing the returned command to use empty parameter fields. If a commandParams object is provided, this method becomes responsible for deleting it when it's no longer needed.
	Json *createCommand (const char *commandName, int commandType, Json *commandParams = NULL);

	typedef void (*RenderTaskFunction) (void *fnData);
	struct RenderTaskContext {
		RenderTaskFunction callback;
		void *callbackData;
		RenderTaskContext (): callback (NULL), callbackData (NULL) { }
	};
	// Schedule a task function to execute at the top of the next render loop
	void addRenderTask (RenderTaskFunction fn, void *fnData);

	// Set a resource path that should be used to load a background texture and render it during draw operations
	void setNextBackgroundTexturePath (const StdString &path);
	void setNextBackgroundTexturePath (const char *path);

	// Return a pseudorandom int value, chosen from within the specified inclusive range
	int getRandomInt (int i1, int i2);

	// Return a pseudorandomly chosen string value of the specified length
	StdString getRandomString (int stringLength);

private:
	// Constants to use for UiChange type values
	enum {
		UI_CHANGE_PUSH = 0,
		UI_CHANGE_POP = 1,
		UI_CHANGE_SET = 2
	};
	struct UiChange {
		Ui *ui;
		int type;
		UiChange (Ui *ui, int type): ui (ui), type (type) { }
	};

	// Create the root panel and other top-level widgets
	void populateWidgets ();

	// Execute draw operations to update the application window
	void draw ();

	// Execute all operations in renderTaskQueue
	void executeRenderTasks ();

	// Execute operations to update application state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

	// Refresh all interface elements in the app and any Ui objects present in the stack. This method must be invoked only while holding a lock on uiMutex.
	void refreshUi ();

	// Close the application window and reopen it at the size indicated by nextWindowWidth and nextWindowHeight
	void resizeWindow ();

	// Return the image scale index associated with the specified window size, or a negative value if no such scale is known
	int getImageScale (int w, int h);

	// Run the application's state update thread
	static int runUpdateThread (void *appPtr);

	// Callback function for use with Network::addDatagramCallback
	static void datagramReceived (void *callbackData, const char *messageData, int messageLength);

	// Write the prefs map file if any of its keys have changed since the last write
	void writePrefsMap ();

	// Remove all items from uiStack
	void clearUiStack ();

	// Remove all items from renderTaskQueue
	void clearRenderTaskQueue ();

	// Return the path to the image file that should be loaded as a background texture from the specified base path, as appropriate for the current image scale
	StdString getBackgroundTexturePath (const StdString &basePath);

	SDL_Thread *updateThread;
	SDL_mutex *uniqueIdMutex;
	int64_t nextUniqueId;
	SDL_mutex *uiMutex;
	Ui *currentUi;
	std::vector<Ui *> uiStack;
	std::vector<App::UiChange> uiChangeList;
	std::vector<SDL_Keycode> keyPressList;
	std::stack<SDL_Rect> clipRectStack;
	SDL_Rect clipRect;
	SDL_mutex *renderTaskMutex;
	std::queue<App::RenderTaskContext> renderTaskQueue;
	bool isSuspendingUpdate;
	SDL_mutex *updateMutex;
	SDL_cond *updateCond;
	SDL_mutex *backgroundMutex;
	StdString backgroundTextureBasePath;
	SDL_Texture *backgroundTexture;
	StdString backgroundTexturePath;
	int backgroundTextureWidth, backgroundTextureHeight;
	SDL_Texture *nextBackgroundTexture;
	StdString nextBackgroundTexturePath;
	int nextBackgroundTextureWidth, nextBackgroundTextureHeight;
	float backgroundTransitionAlpha;
};

#endif
