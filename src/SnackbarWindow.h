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
// Panel that contains a notification message

#ifndef SNACKBAR_WINDOW_H
#define SNACKBAR_WINDOW_H

#include "StdString.h"
#include "Label.h"
#include "Button.h"
#include "Panel.h"

class SnackbarWindow : public Panel {
public:
	SnackbarWindow (float maxWidth);
	virtual ~SnackbarWindow ();

	// Set the snackbar's maximum width
	void setMaxWidth (float maxWidthValue);

	// Set the text displayed in the snackbar
	void setMessageText (const StdString &messageText);

	// Set the state of the snackbar's action button
	void setActionButtonEnabled (bool enable, const StdString &buttonText = StdString (""), Widget::EventCallbackContext buttonClickCallback = Widget::EventCallbackContext ());

	// Start the snackbar's scroll animation and run it over the specified number of milliseconds
	void startScroll (int duration);

	// Start the snackbar's timeout, causing it to hide itself when the specified number of milliseconds elapses
	void startTimeout (int timeout);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Execute operations to update object state as appropriate for an elapsed millisecond time period
	void doUpdate (int msElapsed);

	// Execute operations appropriate when the widget receives new mouse state and return a boolean value indicating if mouse wheel events were consumed and should no longer be processed
	virtual bool doProcessMouseState (const Widget::MouseState &mouseState);

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Callback functions
	static void backgroundPanelClicked (void *windowPtr, Widget *widgetPtr);

	Panel *backgroundPanel;
	Label *messageLabel;
	Button *actionButton;
	float maxWidth;
	int timeoutClock;
	bool isTimeoutSuspended;
	bool isTimeoutDismissed;
	int scrollDuration;
};

#endif
