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
// UI that browses images from a camera server's capture store

#ifndef CAMERA_TIMELINE_UI_H
#define CAMERA_TIMELINE_UI_H

#include "StdString.h"
#include "Button.h"
#include "NumberSpace.h"
#include "HelpWindow.h"
#include "CameraWindow.h"
#include "Ui.h"

class CameraTimelineUi : public Ui {
public:
	// Sprite indexes
	enum {
		TimeIconSprite = 0,
		SelectedTimespanIconSprite = 1,
		OpenImageButtonSprite = 2,
		OpenVideoButtonSprite = 3,
		NoTimelineImageIconSprite = 4
	};

	// Card view row numbers
	enum {
		InfoRow = 0,
		ImageRow = 1
	};

	// Prefs keys
	static const char *ImageSizeKey;

	CameraTimelineUi (CameraWindow *cameraWindow);
	~CameraTimelineUi ();

	// Read-write data members
	Widget::EventCallbackContext thumbnailClickCallback;

	// Read-only data members
	StdString agentId;
	int sensor;
	StdString agentName;
	StdString captureImagePath;
	StdString captureVideoPath;

	// Set fields in the provided HelpWindow widget as appropriate for the UI's help content
	void setHelpWindowContent (HelpWindow *helpWindow);

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
	static void timelineHoverImageLoaded (void *uiPtr, Widget *widgetPtr);
	static void sortToggleStateChanged (void *uiPtr, Widget *widgetPtr);
	static void receiveFindCaptureImagesResult (void *uiPtr, const StdString &agentId, Json *command);
	static void selectModeButtonClicked (void *uiPtr, Widget *widgetPtr);

	// Invoke FindCaptureImages as a link command to the camera agent
	void findCaptureImages (int64_t captureTime = 0, bool isDescending = true, bool isDisplayFind = true);

	// Set state of bottom toolbar select mode buttons as needed for a click of clickedButton
	void setSelectMode (Button *clickedButton);

	static const float TimelineWindowScale;
	static const int DisplayPageSize;
	static const int TimelinePageSize;
	static const int CapturePlayPeriod;
	static const float TimelinePopupWidthMultiplier;

	int cardDetail;
	bool isSortDescending;
	WidgetHandle sortToggle;
	WidgetHandle timelineWindow;
	WidgetHandle cameraDetailWindow;
	WidgetHandle emptyStateWindow;
	WidgetHandle timelinePopup;
	WidgetHandle commandCaption;
	WidgetHandle openImageButton;
	WidgetHandle openVideoButton;
	WidgetHandle targetImageButton;
	WidgetHandle activeSelectButton;
	bool isFindingCaptureImages;
	bool isFindingDisplayCaptureImages;
	int64_t findCaptureImagesTime;
	NumberSpace captureTimes;
	int64_t captureStartTime;
	int64_t captureEndTime;
	int64_t timelineHoverPosition;
	int64_t timelinePopupPositionStartTime;
	int64_t timelinePopupPosition;
	int64_t selectedTime;
	int64_t displayStartTime;
	int64_t displayEndTime;
	bool isSelectingCaptureImage;
};

#endif
