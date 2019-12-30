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
// Panel that contains elements representing a playlist of stream items

#ifndef STREAM_PLAYLIST_WINDOW_H
#define STREAM_PLAYLIST_WINDOW_H

#include "StdString.h"
#include "Json.h"
#include "SpriteGroup.h"
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

class StreamPlaylistWindow : public Panel {
public:
	StreamPlaylistWindow (SpriteGroup *mediaUiSpriteGroup);
	virtual ~StreamPlaylistWindow ();

	static const float windowWidthMultiplier;

	// Constants to use for slider options
	enum {
		ZeroStartPosition = 0,
		NearBeginningStartPosition = 1,
		MiddleStartPosition = 2,
		NearEndStartPosition = 3,
		FullRangeStartPosition = 4,
		StartPositionCount = 5
	};
	enum {
		VeryShortPlayDuration = 0,
		ShortPlayDuration = 1,
		MediumPlayDuration = 2,
		LongPlayDuration = 3,
		VeryLongPlayDuration = 4,
		FullPlayDuration = 5,
		PlayDurationCount = 6
	};

	// Read-write data members
	StdString itemId;
	Widget::EventCallbackContext selectStateChangeCallback;
	Widget::EventCallbackContext expandStateChangeCallback;
	Widget::EventCallbackContext renameClickCallback;
	Widget::EventCallbackContext listChangeCallback;
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

	// Set the playlist name shown by the window
	void setPlaylistName (const StdString &name);

	// Set the window's select state, then execute any select state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setSelected (bool selected, bool shouldSkipStateChangeCallback = false);

	// Set the window's expand state, then execute any expand state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setExpanded (bool expanded, bool shouldSkipStateChangeCallback = false);

	// Return the number of items in the window's playlist
	int getItemCount ();

	// Add an item to the window's playlist
	void addItem (const StdString &streamUrl, const StdString &streamId, const StdString &mediaName, float startPosition, const StdString &thumbnailUrl, int thumbnailIndex);

	// Return a newly created Json object suitable for storing the window's state
	Json *getState ();

	// Reset the window's state using data from the provided Json object, as previously obtained from the getState method
	void setState (Json *stateObject);

	// Return a newly created CreateMediaDisplayIntent command containing the window's data fields, or NULL if the command could not be created
	Json *getCreateCommand ();

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static StreamPlaylistWindow *castWidget (Widget *widget);

	// Set minStartPositionDelta and maxStartPositionDelta fields in the provided params object
	static void setStartPositionDelta (int startPosition, Json *createMediaDisplayIntentParams);

	// Set minItemDisplayDuration and maxItemDisplayDuration fields in the provided params object
	static void setItemDisplayDuration (int playDuration, Json *createMediaDisplayIntentParams);

	// Callback functions
	static void freeItem (void *itemPtr);
	static void nameLabelClicked (void *windowPtr, Widget *widgetPtr);
	static void selectToggleStateChanged (void *windowPtr, Widget *widgetPtr);
	static void expandToggleStateChanged (void *windowPtr, Widget *widgetPtr);
	static void itemListChanged (void *windowPtr, Widget *widgetPtr);
	static void removeButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void addItemButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void addItemButtonMouseEntered (void *windowPtr, Widget *widgetPtr);
	static void addItemButtonMouseExited (void *windowPtr, Widget *widgetPtr);
	static StdString startPositionSliderValueName (float sliderValue);
	static StdString playDurationSliderValueName (float sliderValue);

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
	static const char *PlayDurationKey;
	static const char *ItemListKey;
	static const char *IsShuffleKey;
	static const char *StreamUrlKey;
	static const char *StreamIdKey;
	static const char *MediaNameKey;
	static const char *StartPositionKey;
	static const char *ThumbnailUrlKey;
	static const char *ThumbnailIndexKey;

	struct Item {
		StdString streamUrl;
		StdString streamId;
		StdString mediaName;
		float startPosition;
		StdString thumbnailUrl;
		int thumbnailIndex;
		Item (): startPosition (0.0f), thumbnailIndex (0) { }
	};

	// Reset the text shown by the name label, truncating it as needed to fit in its available space
	void resetNameLabel ();

	SpriteGroup *sprites;
	float windowWidth;
	Image *iconImage;
	LabelWindow *nameLabel;
	IconLabelWindow *itemCountLabel;
	Toggle *selectToggle;
	Toggle *expandToggle;
	ToggleWindow *shuffleToggle;
	SliderWindow *startPositionSlider;
	SliderWindow *playDurationSlider;
	ListView *itemListView;
	Button *removeButton;
	Button *addItemButton;
};

#endif
