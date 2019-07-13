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
// UI that invokes commands on and shows status information for camera servers

#ifndef CAMERA_UI_H
#define CAMERA_UI_H

#include <map>
#include "StdString.h"
#include "CardView.h"
#include "Button.h"
#include "HelpWindow.h"
#include "Ui.h"

class CameraUi : public Ui {
public:
	// Constants to use for sprite indexes
	enum {
		ConfigureTimelapseButtonSprite = 0,
		ImageQualityIconSprite = 1,
		CapturePeriodIconSprite = 2,
		BreadcrumbIconSprite = 3,
		ClearTimelapseButtonSprite = 4
	};

	// Constants to use for card view row numbers
	enum {
		AgentRow = 0,
		CaptureRow = 1
	};

	CameraUi ();
	~CameraUi ();

	// Set fields in the provided HelpWindow widget as appropriate for the UI's help content
	void setHelpWindowContent (HelpWindow *helpWindow);

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

	// Execute subclass-specific operations to refresh the interface's layout as appropriate for the current set of UiConfiguration values
	void doRefresh ();

	// Update subclass-specific interface state as appropriate when the Ui becomes inactive
	void doPause ();

	// Update subclass-specific interface state as appropriate when the Ui becomes active
	void doResume ();

	// Update subclass-specific interface state as appropriate for an elapsed millisecond time period
	void doUpdate (int msElapsed);

	// Reload subclass-specific interface resources as needed to account for a new application window size
	void doResize ();

	// Execute subclass-specific operations to sync state with records present in the application's RecordStore object, which has been locked prior to invocation
	void doSyncRecordStore ();

	// Callback functions
	static void processCameraAgent (void *uiPtr, Json *record, const StdString &recordId);
	static void appendSelectedAgentId (void *stringListPtr, Widget *widgetPtr);
	static void appendExpandedAgentId (void *stringListPtr, Widget *widgetPtr);
	static void cardExpandStateChanged (void *uiPtr, Widget *widgetPtr);
	static void imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void reloadButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void reloadAgent (void *uiPtr, Widget *widgetPtr);
	static void autoReloadToggleStateChanged (void *uiPtr, Widget *widgetPtr);
	static void configureTimelapseButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void configureTimelapseActionOptionChanged (void *uiPtr, Widget *widgetPtr);
	static void configureTimelapseActionClosed (void *uiPtr, Widget *widgetPtr);
	static void clearTimelapseButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void cameraSelectStateChanged (void *uiPtr, Widget *widgetPtr);
	static void commandButtonMouseEntered (void *uiPtr, Widget *widgetPtr);
	static void commandButtonMouseExited (void *uiPtr, Widget *widgetPtr);
	static void captureViewButtonClicked (void *uiPtr, Widget *widgetPtr);
	static StdString capturePeriodValueName (float sliderValue);

private:
	static const int capturePeriods[];
	static const int defaultCapturePeriodIndex;
	static const int minAutoReloadDelay;

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

  // Return a string containing the set of selected camera agent names, appropriate for use in a command popup and truncated to fit within the specified maximum width, or an empty string if no camera agents are selected
	StdString getSelectedCameraNames (float maxWidth);

	CardView *cardView;
	Button *configureTimelapseButton;
	Button *clearTimelapseButton;
	int cameraCount;
	int cardDetail;
	bool isAutoReloading;
	WidgetHandle autoReloadToggle;
	WidgetHandle emptyStateWindow;
	WidgetHandle commandPopup;
	WidgetHandle commandPopupSource;
	HashMap selectedCameraMap;
	std::map<StdString, CameraUi::AutoReloadInfo> autoReloadMap;
};

#endif
