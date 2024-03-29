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
// UI that shows controls for management and playback of a monitor agent's stream cache

#ifndef MONITOR_CACHE_UI_H
#define MONITOR_CACHE_UI_H

#include "StdString.h"
#include "Json.h"
#include "WidgetHandle.h"
#include "Toolbar.h"
#include "HelpWindow.h"
#include "StreamWindow.h"
#include "Ui.h"

class MonitorCacheUi : public Ui {
public:
	// Sprite indexes
	enum {
		BreadcrumbIconSprite = 0,
		PlayButtonSprite = 1,
		StopButtonSprite = 2,
		WritePlaylistButtonSprite = 3,
		PauseButtonSprite = 4
	};

	// Card view row numbers
	enum {
		AgentRow = 0,
		StreamRow = 1,
		EmptyStreamRow = 2
	};

	// Prefs keys
	static const char *ImageSizeKey;
	static const char *ExpandedAgentKey;
	static const char *StartPositionKey;
	static const char *PlayDurationKey;

	static const float BottomPaddingHeightScale;

	MonitorCacheUi (const StdString &agentId, const StdString &agentName);
	~MonitorCacheUi ();

	// Read-only data members
	StdString agentId;
	StdString agentName;

	// Set fields in the provided HelpWindow widget as appropriate for the UI's help content
	void setHelpWindowContent (HelpWindow *helpWindow);

	// Execute operations appropriate when an agent control link client becomes connected
	void handleLinkClientConnect (const StdString &agentId);

protected:
	// Return a resource path containing images to be loaded into the sprites object, or an empty string to disable sprite loading
	StdString getSpritePath ();

	// Return a newly created widget for use as a main toolbar breadcrumb item
	Widget *createBreadcrumbWidget ();

	// Load subclass-specific resources and return a result value
	OsUtil::Result doLoad ();

	// Unload subclass-specific resources
	void doUnload ();

	// Add subclass-specific items to the provided main toolbar object
	void doAddMainToolbarItems (Toolbar *toolbar);

	// Add subclass-specific items to the provided secondary toolbar object
	void doAddSecondaryToolbarItems (Toolbar *toolbar);

	// Remove and destroy any subclass-specific popup widgets that have been created by the UI
	void doClearPopupWidgets ();

	// Update subclass-specific interface state as appropriate when the Ui becomes active
	void doResume ();

	// Update subclass-specific interface state as appropriate when the Ui becomes inactive
	void doPause ();

	// Update subclass-specific interface state as appropriate for an elapsed millisecond time period
	void doUpdate (int msElapsed);

	// Execute subclass-specific operations to sync state with records present in the application's RecordStore object, which has been locked prior to invocation
	void doSyncRecordStore ();
	static void doSyncRecordStore_processStreamItem (void *uiPtr, Json *record, const StdString &recordId);

private:
	// Callback functions
	static void imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void reloadButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void monitorScreenshotLoaded (void *uiPtr, Widget *widgetPtr);
	static void monitorExpandStateChanged (void *uiPtr, Widget *widgetPtr);
	static void streamWindowImageClicked (void *uiPtr, Widget *widgetPtr);
	static void streamWindowViewButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void streamWindowRemoveButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void streamWindowRemoveActionClosed (void *uiPtr, Widget *widgetPtr);
	static void streamItemUiThumbnailClicked (void *uiPtr, Widget *widgetPtr);
	static void unselectStreamWindow (void *uiPtr, Widget *widgetPtr);
	static void commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr);
	static void commandButtonMouseExited (void *uiPtr, Widget *widgetPtr);
	static void stopButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void pauseButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void playButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void playAllButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void playAllActionClosed (void *uiPtr, Widget *widgetPtr);
	static void removeStreamComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess);
	static void receiveFindStreamItemsResult (void *uiPtr, const StdString &agentId, Json *command);
	static void receiveStreamItem (void *uiPtr, const StdString &agentId, Json *command);
	static void invokeMonitorCommandComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess);

	// Set the interface's selected StreamWindow item
	void setSelectedStream (StreamWindow *streamWindow);

	// Set the time of the next record sync if it isn't already assigned
	void resetNextRecordSyncTime ();

	Button *stopButton;
	Button *pauseButton;
	Button *playButton;
	Button *playAllButton;
	WidgetHandle emptyStreamWindow;
	WidgetHandle targetStreamWindow;
	WidgetHandle selectedStreamWindow;
	WidgetHandle commandPopup;
	WidgetHandle commandButton;
	int cardDetail;
	int streamCount;
	int streamSetSize;
	int recordReceiveCount;
	int64_t nextRecordSyncTime;
};

#endif
