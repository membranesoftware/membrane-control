/*
* Copyright 2018-2021 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "TaskGroup.h"
#include "Resource.h"
#include "Network.h"
#include "Json.h"
#include "HashMap.h"
#include "Prng.h"
#include "UiStack.h"
#include "UiText.h"
#include "UiConfiguration.h"
#include "Sprite.h"
#include "Widget.h"
#include "Panel.h"
#include "ConsoleWindow.h"
#include "SystemInterface.h"
#include "AgentControl.h"
#include "RecordStore.h"
#include "CommandListener.h"

class App {
public:
	App ();
	~App ();
	static App *instance;

	// Populate the App instance pointer with a newly created object
	static void createInstance (bool shouldSkipInit = false);

	// Destroy and clear the App instance pointer
	static void freeInstance ();

	static const int DefaultMinFrameDelay;
	static const int WindowWidths[];
	static const int WindowHeights[];
	static const int WindowSizeCount;
	static const float FontScales[];
	static const int FontScaleCount;
	static const int MaxCornerRadius;
	static const StdString ServerUrl;

	// Key values for the prefs map
	static const char *NetworkThreadsKey;
	static const char *WindowWidthKey;
	static const char *WindowHeightKey;
	static const char *FontScaleKey;
	static const char *HttpsKey;
	static const char *ShowInterfaceAnimationsKey;
	static const char *ShowClockKey;
	static const char *IsFirstLaunchCompleteKey;

	// Read-write data members
	Log log;
	Prng prng;
	Input input;
	TaskGroup taskGroup;
	UiStack uiStack;
	UiText uiText;
	UiConfiguration uiConfig;
	Resource resource;
	Network network;
	SystemInterface systemInterface;
	AgentControl agentControl;
	RecordStore recordStore;
	CommandListener commandListener;
	bool isConsole;
	bool shouldRefreshUi;
	bool shouldSyncRecordStore;
	bool isInterfaceAnimationEnabled;
	bool isMainToolbarClockEnabled;
	float nextFontScale;
	int nextWindowWidth;
	int nextWindowHeight;
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
	SDL_Rect clipRect;
	bool isPrefsWriteDisabled;

	// Run the application, returning only after the application exits
	int run ();

	// Begin the application's shutdown process and halt execution when complete
	void shutdown ();

	// Return an int64_t value for use as a unique ID
	int64_t getUniqueId ();

	// Lock the application prefs map and return a pointer to its HashMap object
	HashMap *lockPrefs ();

	// Unlock the application prefs map
	void unlockPrefs ();

	// Suspend the application's update thread and block until the current update cycle completes
	void suspendUpdate ();

	// Unsuspend the application's update thread after a previous call to suspendUpdate
	void unsuspendUpdate ();

	// Push the provided rectangle onto the clip stack and apply it to the application's renderer. Apply the new clip rectangle as an intersection of any existing clip rectangle unless disableIntersection is true.
	void pushClipRect (const SDL_Rect *rect, bool disableIntersection = false);

	// Pop the clip stack
	void popClipRect ();

	// Disable any active clip rectangle, to be restored by a call to unsuspendClipRect
	void suspendClipRect ();

	// Restore a previously suspended clip rectangle
	void unsuspendClipRect ();

	// Set a resource path that should be used to load a background texture and render it during draw operations
	void setNextBackgroundTexturePath (const StdString &path);
	void setNextBackgroundTexturePath (const char *path);

	// Return a texture containing a rounded corner of the specified radius, or NULL if no such texture is available. If a texture is found and width and height pointers are provided, those values are filled in with texture attributes.
	SDL_Texture *getRoundedCornerTexture (int radius, int *textureWidth = NULL, int *textureHeight = NULL);

	typedef void (*RenderTaskFunction) (void *fnData);
	struct RenderTaskContext {
		RenderTaskFunction fn;
		void *fnData;
		RenderTaskContext (): fn (NULL), fnData (NULL) { }
	};
	// Schedule a task function to execute at the top of the next render loop
	void addRenderTask (RenderTaskFunction fn, void *fnData);

	// Set a ConsoleWindow widget that should receive output from the Log::printf method until destroyed
	void setConsoleWindow (ConsoleWindow *window);

	// Append text to any previously set console window
	void writeConsoleOutput (const StdString &text);

	// Execute actions appropriate for a connect event from an agent control link client
	void handleLinkClientConnect (const StdString &agentId);

	// Execute actions appropriate for a disconnect event from an agent control link client
	void handleLinkClientDisconnect (const StdString &agentId, const StdString &errorDescription);

	// Return a SystemInterface::Prefix struct containing command prefix fields
	SystemInterface::Prefix createCommandPrefix ();

	// Return a newly created Json object containing the specified command and the default prefix, or NULL if the command could not be created. commandParams can be NULL if not needed, causing the returned command to use empty parameter fields. If a commandParams object is provided, this method becomes responsible for deleting it when it's no longer needed.
	Json *createCommand (const char *commandName, Json *commandParams = NULL);

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

	// Return a boolean value indicating if the provided string contains an application update URL
	static bool isUpdateUrl (const StdString &url);

	// Return the URL that should be used for retrieving news related to the specified application ID, or the current application if no ID value is provided
	static StdString getApplicationNewsUrl (const StdString &applicationId = StdString (""));

	// Return the URL that should be used for the Membrane Software donation page
	static StdString getDonateUrl ();

	// Find and remove items from the specified legacy StringList prefs key, adding any items found to the provided list. Returns a boolean value indicating if any items were found.
	static bool transferJsonListPrefs (HashMap *prefs, const char *prefsKey, JsonList *items);

	// Callback functions
	static bool keyEvent (void *ptr, SDL_Keycode keycode, bool isShiftDown, bool isControlDown);
	static void hyperlinkOpened (void *ptr, Widget *widgetPtr);

private:
	// Read environment settings and configure the app
	void init ();

	// Run the application in window mode
	int runWindow ();

	// Run the application in console mode
	int runConsole ();

	// Create the root panel and other top-level widgets
	void populateWidgets ();

	// Create roundedCornerSprite and load it with textures
	void populateRoundedCornerSprite ();

	// Execute draw operations to update the application window
	void draw ();

	// Execute all operations in renderTaskList
	void executeRenderTasks ();

	// Execute operations to update application state as appropriate for an elapsed millisecond time period
	void update (int msElapsed);
	void updateConsole (int msElapsed);

	// Close the application window and reopen it at the size indicated by nextWindowWidth and nextWindowHeight
	void resizeWindow ();

	// Return the image scale index associated with the specified window size, or a negative value if no such scale is known
	int getImageScale (int w, int h);

	// Run the application's state update thread
	static int runUpdateThread (void *appPtr);
	static int runConsoleUpdateThread (void *appPtr);

	// Callback function for use with Network
	static void datagramReceived (void *callbackData, const char *messageData, int messageLength, const char *sourceAddress, int sourcePort);

	// Write the prefs file if any prefsMap keys have changed since the last write
	void writePrefs ();

	SDL_Thread *updateThread;
	SDL_mutex *uniqueIdMutex;
	int64_t nextUniqueId;
	HashMap prefsMap;
	SDL_mutex *prefsMapMutex;
	std::vector<SDL_Keycode> keyPressList;
	std::stack<SDL_Rect> clipRectStack;
	Sprite *roundedCornerSprite;
	SDL_mutex *renderTaskMutex;
	std::vector<App::RenderTaskContext> renderTaskList;
	std::vector<App::RenderTaskContext> renderTaskAddList;
	bool isSuspendingUpdate;
	SDL_mutex *updateMutex;
	SDL_cond *updateCond;
	ConsoleWindow *consoleWindow;
	SDL_mutex *consoleWindowMutex;
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
