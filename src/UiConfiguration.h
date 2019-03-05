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
// Class that holds configuration values for UI elements

#ifndef UI_CONFIGURATION_H
#define UI_CONFIGURATION_H

#include "SDL2/SDL.h"
#include "StdString.h"
#include "Color.h"
#include "Font.h"
#include "SpriteGroup.h"
#include "Widget.h"

class UiConfiguration {
public:
	// Constants to use for indexes in the coreSprites group
	enum {
		OK_BUTTON = 0,
		CANCEL_BUTTON = 1,
		ENTER_TEXT_BUTTON = 2,
		APP_LOGO = 3,
		SETTINGS_BUTTON = 4,
		EXIT_BUTTON = 5,
		MAIN_MENU_BUTTON = 6,
		BACK_BUTTON = 7,
		NEWS_BUTTON = 8,
		DELETE_BUTTON = 9,
		HELP_BUTTON = 10,
		PASTE_BUTTON = 11,
		INFO_NEWS_ICON = 12,
		NETWORK_CONNECT_ICON = 13,
		NETWORK_DISCONNECT_ICON = 14,
		CHECKMARK = 15,
		SCROLL_UP_ARROW = 16,
		SCROLL_DOWN_ARROW = 17,
		TASK_IN_PROGRESS_ICON = 18,
		TASK_COMPLETE_ICON = 19,
		INFO_ICON = 20,
		WEB_LINK_ICON = 21,
		TOGGLE_CHECKBOX = 22,
		TOGGLE_CHECKBOX_OUTLINE = 23,
		TOGGLE_CHECKBOX_INDETERMINATE = 24,
		UPDATE_BUTTON = 25,
		FEEDBACK_BUTTON = 26,
		STAR_BUTTON = 27,
		STAR_OUTLINE_BUTTON = 28,
		STAR_HALF_BUTTON = 29,
		CLEAR_BUTTON = 30,
		RELOAD_BUTTON = 31,
		RENAME_BUTTON = 32,
		ERROR_ICON = 33,
		ABOUT_BUTTON = 34,
		AGENT_ADMIN_BUTTON = 35,
		SERVER_ICON = 36,
		DISPLAY_ICON = 37,
		LARGE_MEDIA_ICON = 38,
		LARGE_STREAM_ICON = 39,
		PROGRAM_ICON = 40,
		EXPAND_LESS_BUTTON = 41,
		EXPAND_MORE_BUTTON = 42,
		CONFIGURATION_ICON = 43,
		LOADING_IMAGE_ICON = 44,
		IMAGE_BUTTON = 45,
		ACTIVITY_STATE_ICON = 46,
		STORAGE_ICON = 47,
		SMALL_STREAM_ICON = 48,
		TASK_COUNT_ICON = 49,
		SMALL_MEDIA_ICON = 50,
		KEY_ICON = 51,
		ADD_BUTTON = 52,
		VISIBILITY_ON_BUTTON = 53,
		VISIBILITY_OFF_BUTTON = 54,
		RANDOMIZE_BUTTON = 55
	};

	// Constants to use for font types
	enum {
		CAPTION = 0,
		BODY = 1,
		BUTTON = 2,
		TITLE = 3,
		HEADLINE = 4,
		NUM_FONT_TYPES = 5
	};

	// Constants to use for sprite frame indexes
	enum {
		INACTIVE_STATE_ICON_FRAME = 0,
		ACTIVE_STATE_ICON_FRAME = 1,
		WHITE_BUTTON_FRAME = 0,
		WHITE_LARGE_BUTTON_FRAME = 1,
		BLACK_BUTTON_FRAME = 2,
		BLACK_LARGE_BUTTON_FRAME = 3,
		CHIP_ICON_FRAME = 0
	};

	UiConfiguration ();
	~UiConfiguration ();

	// Load resources referenced by the UiConfiguration and return a result value
	int load (float fontScale = 1.0f);

	// Free resources allocated by any previous load operation
	void unload ();

	// Reset configuration values as appropriate for the current application image scale
	void resetScale ();

	// Free any loaded font resources and replace them with new ones at the specified scale
	int reloadFonts (float fontScale);

	// Read-only data members; read-write access is permissible by the Ui class and its subclasses
	float paddingSize;
	float marginSize;
	int shortColorTranslateDuration; // ms
	int longColorTranslateDuration; // ms
	int mouseHoverThreshold;
	int blinkDuration; // ms
	int backgroundCrossFadeDuration; // ms
	StdString fontNames[UiConfiguration::NUM_FONT_TYPES];
	int fontBaseSizes[UiConfiguration::NUM_FONT_TYPES];
	int fontSizes[UiConfiguration::NUM_FONT_TYPES];
	Font *fonts[UiConfiguration::NUM_FONT_TYPES];
	Color lightPrimaryColor;
	Color mediumPrimaryColor;
	Color darkPrimaryColor;
	Color lightSecondaryColor;
	Color mediumSecondaryColor;
	Color darkSecondaryColor;
	Color lightBackgroundColor;
	Color mediumBackgroundColor;
	Color darkBackgroundColor;
	Color lightInverseBackgroundColor;
	Color mediumInverseBackgroundColor;
	Color darkInverseBackgroundColor;
	Color primaryTextColor;
	Color lightPrimaryTextColor;
	Color inverseTextColor;
	Color darkInverseTextColor;
	Color flatButtonTextColor;
	Color linkTextColor;
	Color errorTextColor;
	Color raisedButtonTextColor;
	Color raisedButtonInverseTextColor;
	Color raisedButtonBackgroundColor;
	float buttonFocusedShadeAlpha;
	float buttonPressedShadeAlpha;
	float buttonDisabledShadeAlpha;
	Color mouseoverBgColor;
	float mouseoverBgAlpha;
	float activeFocusedIconAlpha;
	float activeUnfocusedIconAlpha;
	float inactiveIconAlpha;
	float scrimBackgroundAlpha;
	float overlayWindowAlpha;
	float waitingShadeAlpha;
	float progressBarHeight;
	float mouseWheelScrollSpeed; // portion of view height per wheel scroll event, from 0.0f to 1.0f
	float textLineHeightMargin;
	float textUnderlineMargin;
	float menuDividerLineWidth;
	float sliderThumbSize;
	float sliderTrackWidth;
	float sliderTrackHeight;
	int textAreaShortLineLength;
	int textAreaMediumLineLength;
	int textAreaLongLineLength;
	int textFieldShortLineLength;
	int textFieldMediumLineLength;
	int textFieldLongLineLength;
	float timelineMarkerWidth;
	float rightNavWidthScale; // portion of total window width, from 0.0f to 1.0f
	int snackbarTimeout; // ms
	int snackbarScrollDuration; // ms
	int recordSyncDelayDuration; // ms
	float smallThumbnailImageScale; // portion of total window width, from 0.0f to 1.0f
	float mediumThumbnailImageScale; // portion of total window width, from 0.0f to 1.0f
	float largeThumbnailImageScale; // portion of total window width, from 0.0f to 1.0f
	StdString coreSpritesPath;
	bool isLoaded;
	SpriteGroup coreSprites;
};

#endif
