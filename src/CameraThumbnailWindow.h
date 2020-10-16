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
// Panel that shows a thumbnail image from a camera's capture store

#ifndef CAMERA_THUMBNAIL_WINDOW_H
#define CAMERA_THUMBNAIL_WINDOW_H

#include "StdString.h"
#include "ImageWindow.h"
#include "LabelWindow.h"
#include "Panel.h"

class CameraThumbnailWindow : public Panel {
public:
	CameraThumbnailWindow (const StdString &agentId, const StdString &captureImagePath, int64_t thumbnailTimestamp);
	virtual ~CameraThumbnailWindow ();

	// Read-write data members
	Widget::EventCallbackContext loadCallback;

	// Read-only data members
	StdString agentId;
	StdString captureImagePath;
	int64_t thumbnailTimestamp;
	bool isHighlighted;

	// Set the layout type that should be used to arrange the panel's widgets, as specified by a CardView detail constant
	void setLayout (int layoutType, float maxPanelWidth);

	// Set the image timestamp that should be displayed by the window
	void setThumbnailTimestamp (int64_t timestamp);

	// Set the window's highlighted state
	void setHighlighted (bool highlighted);

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static CameraThumbnailWindow *castWidget (Widget *widget);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Execute operations appropriate when the widget receives new mouse state and return a boolean value indicating if mouse wheel events were consumed and should no longer be processed
	virtual bool doProcessMouseState (const Widget::MouseState &mouseState);

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Callback functions
	static void thumbnailImageLoaded (void *windowPtr, Widget *widgetPtr);
	static void thumbnailImageLongPressed (void *windowPtr, Widget *widgetPtr);

	float windowWidth;
	ImageWindow *thumbnailImage;
	LabelWindow *timestampLabel;
};

#endif
