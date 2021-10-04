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
// UI that browses images from a camera server's capture store

#ifndef CAMERA_TIMELINE_UI_H
#define CAMERA_TIMELINE_UI_H

#include <queue>
#include "StdString.h"
#include "Button.h"
#include "HelpWindow.h"
#include "Ui.h"

class CameraTimelineUi : public Ui {
public:
	// Sprite indexes
	enum {
		TimeIconSprite = 0,
		SelectedTimespanIconSprite = 1,
		PlayButtonSprite = 2,
		StopButtonSprite = 3
	};

	// Card view row numbers
	enum {
		InfoRow = 0,
		ImageRow = 1
	};

	// Prefs keys
	static const char *ImageSizeKey;

	CameraTimelineUi (const StdString &agentId, const StdString &agentName);
	~CameraTimelineUi ();

	// Read-write data members
	Widget::EventCallbackContext thumbnailClickCallback;

	// Read-only data members
	StdString agentId;
	StdString agentName;
	StdString captureImagePath;

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
	int doLoad ();

	// Unload subclass-specific resources
	void doUnload ();

	// Add subclass-specific items to the provided main toolbar object
	void doAddMainToolbarItems (Toolbar *toolbar);

	// Add subclass-specific items to the provided secondary toolbar object
	void doAddSecondaryToolbarItems (Toolbar *toolbar);

	// Update subclass-specific interface state as appropriate when the Ui becomes active
	void doResume ();

	// Update subclass-specific interface state as appropriate for an elapsed millisecond time period
	void doUpdate (int msElapsed);

	// Reload subclass-specific interface resources as needed to account for a new application window size
	void doResize ();

	// Execute subclass-specific operations to sync state with records present in the application's RecordStore object, which has been locked prior to invocation
	void doSyncRecordStore ();

private:
	// Callback functions
	static void imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void thumbnailImageLoaded (void *uiPtr, Widget *widgetPtr);
	static void thumbnailImageClicked (void *uiPtr, Widget *widgetPtr);
	static void thumbnailImageMouseEntered (void *uiPtr, Widget *widgetPtr);
	static void thumbnailImageMouseExited (void *uiPtr, Widget *widgetPtr);
	static void timelineWindowPositionHovered (void *uiPtr, Widget *widgetPtr);
	static void timelineWindowPositionClicked (void *uiPtr, Widget *widgetPtr);
	static void sortToggleStateChanged (void *uiPtr, Widget *widgetPtr);
	static void playToggleStateChanged (void *uiPtr, Widget *widgetPtr);
	static void capturePlayThumbnailImageLoaded (void *uiPtr, Widget *widgetPtr);
	static void selectToggleStateChanged (void *uiPtr, Widget *widgetPtr);
	static void receiveFindCaptureImages (void *uiPtr, const StdString &agentId, Json *command);

	static const float TimelineWindowScale;
	static const int PageSize;
	static const int CapturePlayPeriod;

	int cardDetail;
	bool isSortDescending;
	WidgetHandle sortToggle;
	WidgetHandle selectToggle;
	WidgetHandle timelineWindow;
	WidgetHandle cameraDetailWindow;
	WidgetHandle capturePlayWindow;
	WidgetHandle commandCaption;
	bool isFindingCaptureImages;
	bool isPlayingCapture;
	std::queue<int64_t> capturePlayTimes;
	int capturePlayClock;
	int64_t captureStartTime;
	int64_t captureEndTime;
	int64_t lastTimelineHoverTime;
	int64_t selectedTime;
	int64_t displayStartTime;
	int64_t displayEndTime;
	bool isSelectingCaptureImage;
};

#endif
