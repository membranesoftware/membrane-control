/*
* Copyright 2018-2019 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
// Panel that contains elements representing a web playlist item on a card view

#ifndef WEB_PLAYLIST_WINDOW_H
#define WEB_PLAYLIST_WINDOW_H

#include "StdString.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "Image.h"
#include "Label.h"
#include "LabelWindow.h"
#include "TextArea.h"
#include "ListView.h"
#include "Button.h"
#include "Toggle.h"
#include "ToggleWindow.h"
#include "SliderWindow.h"
#include "IconLabelWindow.h"
#include "Panel.h"

class WebPlaylistWindow : public Panel {
public:
	WebPlaylistWindow (SpriteGroup *webKioskUiSpriteGroup);
	virtual ~WebPlaylistWindow ();

	static const float windowWidthMultiplier;
	static const int itemDisplayDurations[]; // seconds

	// Read-write data members
	StdString itemId;
	Widget::EventCallbackContext selectStateChangeCallback;
	Widget::EventCallbackContext expandStateChangeCallback;
	Widget::EventCallbackContext nameClickCallback;
	Widget::EventCallbackContext urlListChangeCallback;
	Widget::EventCallbackContext removeClickCallback;
	Widget::EventCallbackContext addItemClickCallback;
	Widget::EventCallbackContext addItemMouseEnterCallback;
	Widget::EventCallbackContext addItemMouseExitCallback;

	// Read-only data members
	bool isSelected;
	bool isExpanded;
	StdString playlistName;
	float addItemButtonX1;
	float addItemButtonX2;
	float addItemButtonY;

	// Set the window's selected state, then execute any select state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setSelected (bool selected, bool shouldSkipStateChangeCallback = false);

	// Set the playlist name shown by the window
	void setPlaylistName (const StdString &name);

	// Set the window's expanded state, then execute any expand state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setExpanded (bool expanded, bool shouldSkipStateChangeCallback = false);

	// Return the number of items in the window's URL list
	int getItemCount ();

	// Add an item to the window's URL list
	void addUrl (const StdString &url);

	// Return a newly created CreateWebDisplayIntent command containing the window's data fields, or NULL if the command could not be created
	Json *getCreateCommand ();

	// Return a newly created Json object suitable for storing the window's state
	Json *getState ();

	// Reset the window's state using data from the provided Json object, as previously obtained from the getState method
	void setState (Json *stateObject);

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static WebPlaylistWindow *castWidget (Widget *widget);

	// Callback functions
	static void nameLabelClicked (void *windowPtr, Widget *widgetPtr);
	static void selectToggleStateChanged (void *windowPtr, Widget *widgetPtr);
	static void expandToggleStateChanged (void *windowPtr, Widget *widgetPtr);
	static void urlListChanged (void *windowPtr, Widget *widgetPtr);
	static StdString itemDisplayDurationSliderValueName (float sliderValue);
	static void removeButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void addItemButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void addItemButtonMouseEntered (void *windowPtr, Widget *widgetPtr);
	static void addItemButtonMouseExited (void *windowPtr, Widget *widgetPtr);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Execute subclass-specific operations to refresh the widget's layout as appropriate for the current set of UiConfiguration values
	void doRefresh ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Constants to use as object field names
	static const char *PlaylistNameKey;
	static const char *IsSelectedKey;
	static const char *IsExpandedKey;
	static const char *UrlListKey;
	static const char *IsShuffleKey;
	static const char *ItemDisplayDurationKey;

	// Reset the text shown by the name label, truncating it as needed to fit in its available space
	void resetNameLabel ();

	// Return the item display duration value indicated by the associated slider control, in seconds
	int getItemDisplayDuration ();

	SpriteGroup *sprites;
	float windowWidth;
	Image *iconImage;
	LabelWindow *nameLabel;
	IconLabelWindow *urlCountLabel;
	ToggleWindow *shuffleToggle;
	Image *itemDisplayDurationIcon;
	SliderWindow *itemDisplayDurationSlider;
	ListView *urlListView;
	Toggle *selectToggle;
	Toggle *expandToggle;
	Button *removeButton;
	Button *addItemButton;
};

#endif
