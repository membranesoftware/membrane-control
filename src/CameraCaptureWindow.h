/*
* Copyright 2018-2020 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
// Panel that contains elements representing a camera's capture store on a card view

#ifndef CAMERA_CAPTURE_WINDOW_H
#define CAMERA_CAPTURE_WINDOW_H

#include "StdString.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "Widget.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Image.h"
#include "ImageWindow.h"
#include "Button.h"
#include "Toggle.h"
#include "Panel.h"

class CameraCaptureWindow : public Panel {
public:
	CameraCaptureWindow (Json *agentStatus, int captureId, SpriteGroup *cameraUiSpriteGroup);
	virtual ~CameraCaptureWindow ();

	// Read-write data members
	StdString itemId;
	Widget::EventCallbackContext selectStateChangeCallback;
	Widget::EventCallbackContext viewButtonClickCallback;

	// Read-only data members
	StdString agentId;
	StdString agentName;
	int captureId;
	StdString captureName;
	StdString captureImagePath;
	int lastCaptureWidth, lastCaptureHeight;
	int64_t lastCaptureTime;
	int displayCaptureWidth, displayCaptureHeight;
	int64_t displayCaptureTime;
	int64_t selectedTime;
	bool isSelected;

	// Set the layout type that should be used to arrange the panel's widgets, as specified by a CardView detail constant
	void setLayout (int layoutType, float maxPanelWidth);

	// Set the window's selected state, then execute any select state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setSelected (bool selected, bool shouldSkipStateChangeCallback = false);

	// Set the image timestamp that should be displayed by the capture window. A negative timestamp indicates that the window should show the latest available image.
	void setSelectedTimestamp (int64_t timestamp);

	// Update widget state as appropriate for records present in the application's RecordStore object, which has been locked prior to invocation
	void syncRecordStore ();

	// Return a boolean value indicating if a capture image is available from the window's targeted agent
	bool hasCaptureImage ();

	// Reload the window's capture image content
	void reloadCaptureImage ();

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static CameraCaptureWindow *castWidget (Widget *widget);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Callback functions
	static void viewButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void thumbnailImageLongPressed (void *windowPtr, Widget *widgetPtr);
	static void thumbnailImageLoaded (void *windowPtr, Widget *widgetPtr);
	static void selectToggleStateChanged (void *windowPtr, Widget *widgetPtr);

	SpriteGroup *sprites;
	Image *iconImage;
	Label *nameLabel;
	Label *timeLabel;
	ImageWindow *thumbnailImage;
	Toggle *selectToggle;
	Button *viewButton;
};

#endif
