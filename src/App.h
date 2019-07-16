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
#include "SDL2/SDL.h"
#include "StdString.h"
#include "Log.h"
#include "Input.h"
#include "Resource.h"
#include "Network.h"
#include "HashMap.h"
#include "Prng.h"
#include "UiStack.h"
#include "UiText.h"
#include "UiConfiguration.h"
#include "Widget.h"
#include "Panel.h"
#include "SystemInterface.h"
#include "AgentControl.h"
#ifndef ENABLE_TEST_KEYS
#define ENABLE_TEST_KEYS 0
#endif

class App {
public:
	App ();
	~App ();

	static App *instance;

	// Populate the App instance pointer with a newly created object
	static void createInstance ();

	// Destroy and clear the App instance pointer
	static void freeInstance ();

	static const int defaultMinFrameDelay;
	static const int windowWidths[];
	static const int windowHeights[];
	static const int windowSizeCount;
	static const float fontScales[];
	static const int fontScaleCount;
	static const StdString serverUrl;
	static const int64_t defaultServerTimeout;

	// Constants to use as key values in the prefs map
	static const char *NetworkThreadsKey;
	static const char *WindowWidthKey;
	static const char *WindowHeightKey;
	static const char *FontScaleKey;
	static const char *HttpsKey;
	static const char *ShowInterfaceAnimationsKey;
	static const char *ShowClockKey;
	static const char *IsFirstLaunchCompleteKey;
	static const char *AgentStatusKey;
	static const char *MonitorImageSizeKey;
	static const char *ServerAdminSecretsKey;
	static const char *ServerTimeoutKey;
	static const char *WebKioskUiSelectedAgentsKey;
	static const char *WebKioskUiExpandedAgentsKey;
	static const char *WebKioskUiPlaylistsKey;
	static const char *WebKioskUiToolbarModeKey;
	static const char *MainUiShowAllEnabledKey;
	static const char *MainUiApplicationNewsItemsKey;
	static const char *MediaUiImageSizeKey;
	static const char *MediaUiSortOrderKey;
	static const char *MediaUiSelectedAgentsKey;
	static const char *MediaUiExpandedAgentsKey;
	static const char *MediaUiPlaylistsKey;
	static const char *MediaUiToolbarModeKey;
	static const char *MediaItemUiVideoQualityKey;
	static const char *MediaUiVideoQualityKey;
	static const char *MediaUiShowMediaWithoutStreamsKey;
	static const char *MediaItemUiImageSizeKey;
	static const char *StreamItemUiImageSizeKey;
	static const char *MonitorCacheUiImageSizeKey;
	static const char *CameraUiSelectedAgentsKey;
	static const char *CameraUiExpandedAgentsKey;
	static const char *CameraUiImageSizeKey;
	static const char *CameraUiAutoReloadKey;
	static const char *CameraTimelineUiImageSizeKey;

	// Read-write data members
	Log log;
	Prng prng;
	Input input;
	UiStack uiStack;
	UiText uiText;
	UiConfiguration uiConfig;
	Resource resource;
	Network network;
	SystemInterface systemInterface;
	AgentControl agentControl;
	HashMap prefsMap;
	bool shouldRefreshUi;
	bool shouldSyncRecordStore;
	bool isInterfaceAnimationEnabled;
	float nextFontScale;
	int nextWindowWidth;
	int nextWindowHeight;
#if ENABLE_TEST_KEYS
	bool isUiPaused;
	bool isUiPauseKeyPressed;
#endif
	StdString prefsPath;

	// Read-only data members
	bool isShuttingDown;
	bool isShutdown;
	int64_t startTime;
	bool isHttpsEnabled;
	SDL_Window *window;
	SDL_Renderer *render; // The renderer must be accessed only from the application's main thread
	bool isTextureRenderEnabled;
	Panel *rootPanel;
	float displayDdpi;
	float displayHdpi;
	float displayVdpi;
	int windowWidth;
	int windowHeight;
	int minDrawFrameDelay; // milliseconds
	int minUpdateFrameDelay; // milliseconds
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

	// Suspend the application's update thread and block until the current update cycle completes
	void suspendUpdate ();

	// Unsuspend the application's update thread after a previous call to suspendUpdate
	void unsuspendUpdate ();

	// Apply the provided clip rectangle to the application's renderer and push it onto the clip stack
	void pushClipRect (const SDL_Rect *rect);

	// Pop the clip stack
	void popClipRect ();

	// Disable any active clip rectangle, to be restored by a call to unsuspendClipRect
	void suspendClipRect ();

	// Restore a previously suspended clip rectangle
	void unsuspendClipRect ();

	// Set a resource path that should be used to load a background texture and render it during draw operations
	void setNextBackgroundTexturePath (const StdString &path);
	void setNextBackgroundTexturePath (const char *path);

	typedef void (*RenderTaskFunction) (void *fnData);
	struct RenderTaskContext {
		RenderTaskFunction callback;
		void *callbackData;
		RenderTaskContext (): callback (NULL), callbackData (NULL) { }
	};
	// Schedule a task function to execute at the top of the next render loop
	void addRenderTask (RenderTaskFunction fn, void *fnData);

	// Execute actions appropriate for a command received from the agent control link client
	void handleLinkClientCommand (const StdString &agentId, Json *command);

	// Execute actions appropriate for a connect event from an agent control link client
	void handleLinkClientConnect (const StdString &agentId);

	// Execute actions appropriate for a disconnect event from an agent control link client
	void handleLinkClientDisconnect (const StdString &agentId, const StdString &errorDescription);

	// Return a SystemInterface::Prefix struct containing command prefix fields
	SystemInterface::Prefix createCommandPrefix ();

	// Return a newly created Json object containing the specified command and the default prefix, or NULL if the command could not be created. commandParams can be NULL if not needed, causing the returned command to use empty parameter fields. If a commandParams object is provided, this method becomes responsible for deleting it when it's no longer needed.
	Json *createCommand (const char *commandName, int commandType, Json *commandParams = NULL);

	// Return a pseudorandom int value, chosen from within the specified inclusive range
	int getRandomInt (int i1, int i2);

	// Return a pseudorandomly chosen string value of the specified length
	StdString getRandomString (int stringLength);

	// Return the URL that should be used for viewing the specified help topic ID
	static StdString getHelpUrl (const StdString &topicId);
	static StdString getHelpUrl (const char *topicId);

	// Return the URL that should be used for downloading the specified application
	static StdString getApplicationUrl (const StdString &applicationId);

	// Return the URL that should be used for viewing information about the specified feature
	static StdString getFeatureUrl (const StdString &featureId);

	// Return the URL that should be used for application feedback, optionally appending an ID value that specifies the application version
	static StdString getFeedbackUrl (bool shouldIncludeVersion = false);

	// Return the URL that should be used for updates targeting the specified application ID, or the current application if no ID value is provided
	static StdString getUpdateUrl (const StdString &applicationId = StdString (""));

	// Return the URL that should be used for retrieving news related to the current application
	static StdString getApplicationNewsUrl ();

	// Return the URL that should be used for the Membrane Software donation page
	static StdString getDonateUrl ();

private:
	// Create the root panel and other top-level widgets
	void populateWidgets ();

	// Execute draw operations to update the application window
	void draw ();

	// Execute all operations in renderTaskList
	void executeRenderTasks ();

	// Execute operations to update application state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);

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

	SDL_Thread *updateThread;
	SDL_mutex *uniqueIdMutex;
	int64_t nextUniqueId;
	std::vector<SDL_Keycode> keyPressList;
	std::stack<SDL_Rect> clipRectStack;
	SDL_Rect clipRect;
	SDL_mutex *renderTaskMutex;
	std::vector<App::RenderTaskContext> renderTaskList;
	std::vector<App::RenderTaskContext> renderTaskAddList;
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
	float backgroundCrossFadeAlpha;
};

#endif
