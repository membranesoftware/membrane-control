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
// UI that invokes commands on and shows status information for camera servers

#ifndef CAMERA_UI_H
#define CAMERA_UI_H

#include <map>
#include "StdString.h"
#include "WidgetHandle.h"
#include "Button.h"
#include "HelpWindow.h"
#include "CapturePlaylistWindow.h"
#include "Ui.h"

class CameraUi : public Ui {
public:
	// Sprite indexes
	enum {
		ConfigureTimelapseButtonSprite = 0,
		ImageQualityIconSprite = 1,
		CapturePeriodIconSprite = 2,
		BreadcrumbIconSprite = 3,
		ClearTimelapseButtonSprite = 4,
		PlayCameraStreamButtonSprite = 5,
		ShowCameraImageButtonSprite = 6,
		TimelineRefreshOnButtonSprite = 7,
		TimelineRefreshOffButtonSprite = 8,
		TimelapseIconSprite = 9,
		ShowCapturePlaylistButtonSprite = 10,
		CreatePlaylistButtonSprite = 11,
		ShuffleIconSprite = 12,
		SpeedIconSprite = 13,
		AddPlaylistItemButtonSprite = 14,
		FlipIconSprite = 15,
		ClearDisplayButtonSprite = 16
	};

	// Card view row numbers
	enum {
		AgentToggleRow = 0,
		UnexpandedAgentRow = 1,
		ExpandedAgentRow = 2,
		PlaylistToggleRow = 3,
		UnexpandedPlaylistRow = 4,
		ExpandedPlaylistRow = 5,
		CaptureRow = 6
	};

	// Toolbar modes
	enum {
		CameraMode = 0,
		MonitorMode = 1,
		ModeCount = 2
	};

	// Prefs keys
	static const char *SelectedAgentsKey;
	static const char *ExpandedAgentsKey;
	static const char *SelectedCapturesKey;
	static const char *ImageSizeKey;
	static const char *AutoReloadKey;
	static const char *ToolbarModeKey;
	static const char *PlaylistsKey;

	CameraUi ();
	~CameraUi ();

	// Set fields in the provided HelpWindow widget as appropriate for the UI's help content
	void setHelpWindowContent (HelpWindow *helpWindow);

	// Execute operations appropriate when an agent control link client becomes connected
	void handleLinkClientConnect (const StdString &agentId);

	// Return the string description of the provided image quality constant
	static StdString getImageQualityDescription (int imageQuality);

protected:
	// Return a resource path containing images to be loaded into the sprites object, or an empty string to disable sprite loading
	StdString getSpritePath ();

	// Return a newly created widget for use as a main toolbar breadcrumb item
	Widget *createBreadcrumbWidget ();

	// Load subclass-specific resources and return a result value
	int doLoad ();

	// Unload subclass-specific resources
	void doUnload ();

	// Add subclass-specific items to the provided main toolbar object
	void doAddMainToolbarItems (Toolbar *toolbar);

	// Add subclass-specific items to the provided secondary toolbar object
	void doAddSecondaryToolbarItems (Toolbar *toolbar);

	// Remove and destroy any subclass-specific popup widgets that have been created by the UI
	void doClearPopupWidgets ();

	// Update subclass-specific interface state as appropriate when the Ui becomes inactive
	void doPause ();

	// Update subclass-specific interface state as appropriate when the Ui becomes active
	void doResume ();

	// Update subclass-specific interface state as appropriate for an elapsed millisecond time period
	void doUpdate (int msElapsed);

	// Execute subclass-specific actions appropriate for a received keypress event and return a boolean value indicating if the event was consumed and should no longer be processed
	bool doProcessKeyEvent (SDL_Keycode keycode, bool isShiftDown, bool isControlDown);

	// Execute subclass-specific operations to sync state with records present in the application's RecordStore object, which has been locked prior to invocation
	void doSyncRecordStore ();
	static void doSyncRecordStore_processCameraAgent (void *uiPtr, Json *record, const StdString &recordId);
	static void doSyncRecordStore_processMonitorAgent (void *uiPtr, Json *record, const StdString &recordId);

private:
	// Callback functions
	static void modeButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void cameraModeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void monitorModeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void cameraSelectStateChanged (void *uiPtr, Widget *widgetPtr);
	static void cameraExpandStateChanged (void *uiPtr, Widget *widgetPtr);
	static void monitorSelectStateChanged (void *uiPtr, Widget *widgetPtr);
	static void monitorExpandStateChanged (void *uiPtr, Widget *widgetPtr);
	static void monitorScreenshotLoaded (void *uiPtr, Widget *widgetPtr);
	static void imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void expandAgentsToggleStateChanged (void *uiPtr, Widget *widgetPtr);
	static void expandPlaylistsToggleStateChanged (void *uiPtr, Widget *widgetPtr);
	static void reloadButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void reloadAgent (void *uiPtr, Widget *widgetPtr);
	static void autoReloadToggleStateChanged (void *uiPtr, Widget *widgetPtr);
	static void playlistSelectStateChanged (void *uiPtr, Widget *widgetPtr);
	static void playlistExpandStateChanged (void *uiPtr, Widget *widgetPtr);
	static void playlistNameClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistNameEdited (void *uiPtr, Widget *widgetPtr);
	static void playlistNameEditEnterButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistItemListChanged (void *uiPtr, Widget *widgetPtr);
	static void playlistAddItemActionClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistRemoveActionClicked (void *uiPtr, Widget *widgetPtr);
	static void playlistRemoveActionClosed (void *uiPtr, Widget *widgetPtr);
	static void playlistAddItemMouseEntered (void *uiPtr, Widget *widgetPtr);
	static void playlistAddItemMouseExited (void *uiPtr, Widget *widgetPtr);
	static void cameraTimelineThumbnailClicked (void *uiPtr, Widget *widgetPtr);
	static void configureCameraButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void configureCameraActionOptionChanged (void *uiPtr, Widget *widgetPtr);
	static void configureCameraActionClosed (void *uiPtr, Widget *widgetPtr);
	static void clearTimelapseButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void clearTimelapseActionClosed (void *uiPtr, Widget *widgetPtr);
	static void showCameraImageButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void showCapturePlaylistButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void playCameraStreamButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void clearDisplayButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void createPlaylistButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr);
	static void commandButtonMouseExited (void *uiPtr, Widget *widgetPtr);
	static void captureViewButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void captureSelectStateChanged (void *uiPtr, Widget *widgetPtr);
	static StdString capturePeriodValueName (float sliderValue);

	static const int CapturePeriods[];
	static const int DefaultCapturePeriodIndex;
	static const int MinAutoReloadDelay;

	struct AutoReloadInfo {
		int64_t lastSendTime;
		int64_t lastReceiveTime;
		int64_t lastStatusCreateTime;
		bool isCapturing;
		int capturePeriod;
		AutoReloadInfo (): lastSendTime (0), lastReceiveTime (0), lastStatusCreateTime (0), isCapturing (false), capturePeriod (0) { }
	};

	// Return an autoReloadMap iterator positioned at the specified entry, creating it if it doesn't already exist
	std::map<StdString, CameraUi::AutoReloadInfo>::iterator getAutoReloadInfo (const StdString &agentId);

	// Set the control mode for the secondary toolbar, optionally forcing the reset even if the requested mode matches the mode already active
	void setToolbarMode (int mode, bool forceReset = false);

	// Return the provided base value, after appending suffixes as needed to generate an unused playlist name
	StdString getAvailablePlaylistName (const StdString &baseName = StdString (""));

	// Return a newly created CapturePlaylistWindow object that has been initialized for use with the UI
	CapturePlaylistWindow *createCapturePlaylistWindow ();

	// Return a string containing the set of selected camera agent names, appropriate for use in a command popup and truncated to fit within the specified maximum width, or an empty string if no camera agents are selected
	StdString getSelectedCameraNames (float maxWidth);

	// Return a string containing the set of selected monitor agent names, appropriate for use in a command popup and truncated to fit within the specified maximum width, or an empty string if no monitor agents are selected
	StdString getSelectedMonitorNames (float maxWidth);

	// Return a string containing the set of selected capture names, appropriate for use in a command popup and truncated to fit within the specified maximum width, or an empty string if no captures are selected
	StdString getSelectedCaptureNames (float maxWidth);

	// Reset checked states for row expand toggles, as appropriate for item expand state
	void resetExpandToggles ();

	int toolbarMode;
	Button *configureCameraButton;
	Button *clearTimelapseButton;
	Button *showCameraImageButton;
	Button *showCapturePlaylistButton;
	Button *playCameraStreamButton;
	Button *clearDisplayButton;
	int cameraCount;
	int cardDetail;
	bool isAutoReloading;
	WidgetHandle autoReloadToggle;
	WidgetHandle emptyStateWindow;
	WidgetHandle commandPopup;
	WidgetHandle commandPopupSource;
	WidgetHandle expandAgentsToggle;
	WidgetHandle expandPlaylistsToggle;
	WidgetHandle targetCapture;
	HashMap selectedCameraMap;
	HashMap selectedMonitorMap;
	HashMap selectedCaptureMap;
	StdString selectedPlaylistId;
	std::map<StdString, CameraUi::AutoReloadInfo> autoReloadMap;
};

#endif
