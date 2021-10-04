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
// Panel that contains elements representing a capture playlist item on a card view

#ifndef CAPTURE_PLAYLIST_WINDOW_H
#define CAPTURE_PLAYLIST_WINDOW_H

#include "StdString.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "Image.h"
#include "Label.h"
#include "LabelWindow.h"
#include "ListView.h"
#include "Button.h"
#include "Toggle.h"
#include "ToggleWindow.h"
#include "SliderWindow.h"
#include "IconLabelWindow.h"
#include "Panel.h"

class CapturePlaylistWindow : public Panel {
public:
	CapturePlaylistWindow (SpriteGroup *cameraUiSpriteGroup);
	virtual ~CapturePlaylistWindow ();

	// State field names
	static const char *PlaylistNameKey;
	static const char *IsSelectedKey;
	static const char *IsExpandedKey;
	static const char *ItemListKey;
	static const char *IsShuffleKey;
	static const char *ItemDisplayDurationKey;
	static const char *HostnameKey;
	static const char *AuthorizePathKey;
	static const char *AuthorizeSecretKey;
	static const char *AuthorizeTokenKey;
	static const char *CaptureIdKey;
	static const char *CaptureNameKey;

	static const float WindowWidthMultiplier;
	static const int ItemDisplayDurations[]; // seconds

	struct Item {
		StdString hostname;
		StdString authorizePath;
		StdString authorizeSecret;
		StdString authorizeToken;
		int captureId;
		StdString captureName;
		Item (): captureId (0) { }
	};

	// Read-write data members
	StdString itemId;
	Widget::EventCallbackContext selectStateChangeCallback;
	Widget::EventCallbackContext expandStateChangeCallback;
	Widget::EventCallbackContext nameClickCallback;
	Widget::EventCallbackContext itemListChangeCallback;
	Widget::EventCallbackContext removeClickCallback;
	Widget::EventCallbackContext addItemClickCallback;
	Widget::EventCallbackContext addItemMouseEnterCallback;
	Widget::EventCallbackContext addItemMouseExitCallback;

	// Read-only data members
	bool isSelected;
	bool isExpanded;
	float windowWidth;
	StdString playlistName;

	// Set the window's selected state, then execute any select state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setSelected (bool selected, bool shouldSkipStateChangeCallback = false);

	// Set the playlist name shown by the window
	void setPlaylistName (const StdString &name);

	// Set the window's expanded state, then execute any expand state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setExpanded (bool expanded, bool shouldSkipStateChangeCallback = false);

	// Return the number of items in the window's URL list
	int getItemCount ();

	// Add an item to the window's list
	void addItem (const CapturePlaylistWindow::Item &item);
	void addItem (const StdString &hostname, const StdString &authorizePath, const StdString &authorizeSecret, const StdString &authorizeToken, int captureId, const StdString &captureName);

	// Return the screen extent rectangle for the window's add item button
	Widget::Rectangle getAddItemButtonScreenRect ();

	// Return the screen extent rectangle for the window's remove button
	Widget::Rectangle getRemoveButtonScreenRect ();

	// Return a newly created CreateCameraDisplayIntent command containing the window's data fields, or NULL if the command could not be created
	Json *createCommand ();

	// Return a newly created Json object suitable for storing the window's state
	Json *createState ();

	// Reset the window's fields using state data from the provided Json object
	void readState (Json *state);

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static CapturePlaylistWindow *castWidget (Widget *widget);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Execute subclass-specific operations to refresh the widget's layout as appropriate for the current set of UiConfiguration values
	void doRefresh ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Callback functions
	static void freeItem (void *itemPtr);
	static void nameLabelClicked (void *windowPtr, Widget *widgetPtr);
	static void selectToggleStateChanged (void *windowPtr, Widget *widgetPtr);
	static void expandToggleStateChanged (void *windowPtr, Widget *widgetPtr);
	static void itemListChanged (void *windowPtr, Widget *widgetPtr);
	static StdString itemDisplayDurationSliderValueName (float sliderValue);
	static void removeButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void addItemButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void addItemButtonMouseEntered (void *windowPtr, Widget *widgetPtr);
	static void addItemButtonMouseExited (void *windowPtr, Widget *widgetPtr);

	// Reset the text shown by the name label, truncating it as needed to fit in its available space
	void resetNameLabel ();

	// Return the item display duration value indicated by the associated slider control, in seconds
	int getItemDisplayDuration ();

	SpriteGroup *sprites;
	Image *iconImage;
	LabelWindow *nameLabel;
	Panel *dividerPanel;
	IconLabelWindow *itemCountLabel;
	ToggleWindow *shuffleToggle;
	SliderWindow *itemDisplayDurationSlider;
	ListView *itemListView;
	Toggle *selectToggle;
	Toggle *expandToggle;
	Button *removeButton;
	Button *addItemButton;
};

#endif
