/*
* Copyright 2018 Membrane Software <author@membranesoftware.com>
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
// Panel that contains a help menu with topics available for display

#ifndef HELP_WINDOW_H
#define HELP_WINDOW_H

#include <list>
#include <map>
#include "StdString.h"
#include "Sprite.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Image.h"
#include "ImageWindow.h"
#include "TextArea.h"
#include "Button.h"
#include "Json.h"
#include "HelpActionWindow.h"
#include "HyperlinkWindow.h"
#include "Panel.h"

class HelpWindow : public Panel {
public:
	HelpWindow (float windowWidth, float windowHeight);
	virtual ~HelpWindow ();

	// Set the window's size
	void setWindowSize (float windowWidth, float windowHeight);

	// Set the window's help text
	void setHelpText (const StdString &title, const StdString &text);

	// Add an action item to the window's help content
	void addAction (const StdString &actionText, const StdString &linkText = StdString (""), const StdString &linkUrl = StdString (""));

	// Add a link item to the window's topic list
	void addTopicLink (const StdString &linkText, const StdString &linkUrl);

	// Callback functions
	static void headerImageLoaded (void *windowPtr, Widget *widgetPtr);
	static void closeButtonClicked (void *windowPtr, Widget *widgetPtr);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void resetLayout ();

private:
	ImageWindow *headerImage;
	LabelWindow *titleLabel;
	Button *closeButton;
	Label *helpTitleLabel;
	TextArea *helpText;
	Label *linkTitleLabel;
	Image *linkIconImage;
	std::list<HelpActionWindow *> actionList;
	std::list<HyperlinkWindow *> linkList;
	Label *versionLabel;
};

#endif
