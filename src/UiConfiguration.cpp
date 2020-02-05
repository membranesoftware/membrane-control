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
#include "Config.h"
#include <stdlib.h>
#include <string.h>
#include "SDL2/SDL.h"
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Resource.h"
#include "Font.h"
#include "UiConfiguration.h"

UiConfiguration::UiConfiguration ()
: paddingSize (12.0f)
, marginSize (12.0f)
, shortColorTranslateDuration (120)
, longColorTranslateDuration (768)
, mouseHoverThreshold (1000)
, blinkDuration (486)
, backgroundCrossFadeDuration (140)
, lightPrimaryColor (Color::getByteValue (0x51), Color::getByteValue (0x4A), Color::getByteValue (0xAC))
, mediumPrimaryColor (Color::getByteValue (0x18), Color::getByteValue (0x22), Color::getByteValue (0x7C))
, darkPrimaryColor (Color::getByteValue (0x00), Color::getByteValue (0x00), Color::getByteValue (0x4F))
, lightSecondaryColor (Color::getByteValue (0xFF), Color::getByteValue (0xFF), Color::getByteValue (0xA8))
, mediumSecondaryColor (Color::getByteValue (0xFF), Color::getByteValue (0xF1), Color::getByteValue (0x76))
, darkSecondaryColor (Color::getByteValue (0xCA), Color::getByteValue (0xBF), Color::getByteValue (0x45))
, lightBackgroundColor (Color::getByteValue (0xFF), Color::getByteValue (0xFF), Color::getByteValue (0xFF))
, mediumBackgroundColor (Color::getByteValue (0xF0), Color::getByteValue (0xF0), Color::getByteValue (0xF0))
, darkBackgroundColor (Color::getByteValue (0xE0), Color::getByteValue (0xE0), Color::getByteValue (0xE0))
, lightInverseBackgroundColor (Color::getByteValue (0x22), Color::getByteValue (0x22), Color::getByteValue (0x22))
, mediumInverseBackgroundColor (Color::getByteValue (0x11), Color::getByteValue (0x11), Color::getByteValue (0x11))
, darkInverseBackgroundColor (Color::getByteValue (0x00), Color::getByteValue (0x00), Color::getByteValue (0x00))
, primaryTextColor (Color::getByteValue (0x00), Color::getByteValue (0x00), Color::getByteValue (0x00))
, lightPrimaryTextColor (Color::getByteValue (0x60), Color::getByteValue (0x60), Color::getByteValue (0x60))
, inverseTextColor (Color::getByteValue (0xFF), Color::getByteValue (0xFF), Color::getByteValue (0xFF))
, darkInverseTextColor (Color::getByteValue (0x98), Color::getByteValue (0x98), Color::getByteValue (0x98))
, flatButtonTextColor (Color::getByteValue (0x18), Color::getByteValue (0x22), Color::getByteValue (0x7C))
, linkTextColor (Color::getByteValue (0x18), Color::getByteValue (0x22), Color::getByteValue (0x7C))
, errorTextColor (Color::getByteValue (0xB0), Color::getByteValue (0x00), Color::getByteValue (0x20))
, statusOkTextColor (Color::getByteValue (0x10), Color::getByteValue (0x8D), Color::getByteValue (0x10))
, raisedButtonTextColor (Color::getByteValue (0x18), Color::getByteValue (0x22), Color::getByteValue (0x7C))
, raisedButtonInverseTextColor (Color::getByteValue (0x51), Color::getByteValue (0x4A), Color::getByteValue (0xAC))
, raisedButtonBackgroundColor (Color::getByteValue (0xD0), Color::getByteValue (0xD0), Color::getByteValue (0xD0))
, dropShadowColor (0.0f, 0.0f, 0.0f, 0.78f)
, dropShadowWidth (2.0f)
, cornerRadius (6)
, buttonFocusedShadeAlpha (0.12f)
, buttonPressedShadeAlpha (0.28f)
, buttonDisabledShadeAlpha (0.58f)
, mouseoverBgColor (Color::getByteValue (0x15), Color::getByteValue (0x9F), Color::getByteValue (0x0E))
, mouseoverBgAlpha (0.53f)
, activeFocusedIconAlpha (1.0f)
, activeUnfocusedIconAlpha (0.74f)
, inactiveIconAlpha (0.38f)
, scrimBackgroundAlpha (0.77f)
, overlayWindowAlpha (0.72f)
, waitingShadeAlpha (0.81f)
, progressBarHeight (8.0f)
, mouseWheelScrollSpeed (0.1f)
, textLineHeightMargin (1.0f)
, textUnderlineMargin (2.0f)
, menuDividerLineWidth (2.0f)
, selectionBorderWidth (5.0f)
, selectionBorderAlpha (0.72f)
, sliderThumbSize (16.0f)
, sliderTrackWidth (200.0f)
, sliderTrackHeight (6.0f)
, textAreaShortLineLength (30)
, textAreaMediumLineLength (60)
, textAreaLongLineLength (75)
, textFieldShortLineLength (16)
, textFieldMediumLineLength (32)
, textFieldLongLineLength (60)
, timelineMarkerWidth (16.0f)
, rightNavWidthScale (0.275f)
, snackbarTimeout (16000)
, snackbarScrollDuration (280)
, recordSyncDelayDuration (700)
, smallThumbnailImageScale (0.123f)
, mediumThumbnailImageScale (0.240f)
, largeThumbnailImageScale (0.450f)
, coreSpritesPath ("ui/CoreSprites")
, isLoaded (false)
{
	memset (fonts, 0, sizeof (fonts));
	memset (fontSizes, 0, sizeof (fontSizes));

	fontNames[UiConfiguration::CaptionFont].assign ("font/Roboto-Regular.ttf");
	fontBaseSizes[UiConfiguration::CaptionFont] = 10;

	fontNames[UiConfiguration::BodyFont].assign ("font/Roboto-Regular.ttf");
	fontBaseSizes[UiConfiguration::BodyFont] = 12;

	fontNames[UiConfiguration::ButtonFont].assign ("font/Roboto-Medium.ttf");
	fontBaseSizes[UiConfiguration::ButtonFont] = 12;

	fontNames[UiConfiguration::TitleFont].assign ("font/Roboto-Medium.ttf");
	fontBaseSizes[UiConfiguration::TitleFont] = 14;

	fontNames[UiConfiguration::HeadlineFont].assign ("font/Roboto-Regular.ttf");
	fontBaseSizes[UiConfiguration::HeadlineFont] = 16;
}

UiConfiguration::~UiConfiguration () {
	unload ();
}

int UiConfiguration::load (float fontScale) {
	Resource *resource;
	Font *font;
	int i, result, sz;

	if (fontScale <= 0.0f) {
		return (Result::InvalidParamError);
	}

	if (isLoaded) {
		return (Result::Success);
	}

	resource = &(App::instance->resource);

	for (i = 0; i < UiConfiguration::FontCount; ++i) {
		sz = (int) (fontScale * (float) fontBaseSizes[i]);
		if (sz < 1) {
			sz = 1;
		}
		font = resource->loadFont (fontNames[i], sz);
		if (! font) {
			return (Result::FreetypeOperationFailedError);
		}
		fonts[i] = font;
		fontSizes[i] = sz;
	}

	if (! coreSpritesPath.empty ()) {
		result = coreSprites.load (coreSpritesPath);
		if (result != Result::Success) {
			return (result);
		}
	}


	isLoaded = true;
	return (Result::Success);
}

void UiConfiguration::unload () {
	Resource *resource;
	int i;

	resource = &(App::instance->resource);
	isLoaded = false;

	for (i = 0; i < UiConfiguration::FontCount; ++i) {
		if (fonts[i]) {
			fonts[i] = NULL;
			resource->unloadFont (fontNames[i], fontSizes[i]);
		}
	}

	coreSprites.unload ();
}

int UiConfiguration::reloadFonts (float fontScale) {
	Resource *resource;
	Font *font;
	int i, sz;

	if (fontScale <= 0.0f) {
		return (Result::InvalidParamError);
	}

	resource = &(App::instance->resource);

	for (i = 0; i < UiConfiguration::FontCount; ++i) {
		sz = (int) (fontScale * (float) fontBaseSizes[i]);
		if (sz < 1) {
			sz = 1;
		}
		font = resource->loadFont (fontNames[i], sz);
		if (! font) {
			return (Result::FreetypeOperationFailedError);
		}
		if (fonts[i]) {
			resource->unloadFont (fontNames[i], fontSizes[i]);
		}
		fonts[i] = font;
		fontSizes[i] = sz;
	}
	Log::debug ("Fonts reloaded; fontScale=%.2f captionFontSize=%i bodyFontSize=%i buttonFontSize=%i titleFontSize=%i headlineFontSize=%i", fontScale, fontSizes[UiConfiguration::CaptionFont], fontSizes[UiConfiguration::BodyFont], fontSizes[UiConfiguration::ButtonFont], fontSizes[UiConfiguration::TitleFont], fontSizes[UiConfiguration::HeadlineFont]);

	return (Result::Success);
}

void UiConfiguration::resetScale () {
	switch (App::instance->imageScale) {
		case 0: {
			paddingSize = 6.0f;
			marginSize = 6.0f;
			cornerRadius = 3;
			sliderTrackWidth = 100.0f;
			sliderTrackHeight = 4.0f;
			timelineMarkerWidth = 12.0f;
			fontBaseSizes[UiConfiguration::CaptionFont] = 8;
			fontBaseSizes[UiConfiguration::BodyFont] = 10;
			fontBaseSizes[UiConfiguration::ButtonFont] = 10;
			fontBaseSizes[UiConfiguration::TitleFont] = 12;
			fontBaseSizes[UiConfiguration::HeadlineFont] = 14;
			break;
		}
		case 1: {
			paddingSize = 12.0f;
			marginSize = 12.0f;
			cornerRadius = 6;
			sliderTrackWidth = 130.0f;
			sliderTrackHeight = 4.0f;
			timelineMarkerWidth = 16.0f;
			fontBaseSizes[UiConfiguration::CaptionFont] = 8;
			fontBaseSizes[UiConfiguration::BodyFont] = 10;
			fontBaseSizes[UiConfiguration::ButtonFont] = 10;
			fontBaseSizes[UiConfiguration::TitleFont] = 12;
			fontBaseSizes[UiConfiguration::HeadlineFont] = 14;
			break;
		}
		case 2: {
			paddingSize = 16.0f;
			marginSize = 16.0f;
			cornerRadius = 8;
			sliderTrackWidth = 180.0f;
			sliderTrackHeight = 5.0f;
			timelineMarkerWidth = 16.0f;
			fontBaseSizes[UiConfiguration::CaptionFont] = 10;
			fontBaseSizes[UiConfiguration::BodyFont] = 12;
			fontBaseSizes[UiConfiguration::ButtonFont] = 12;
			fontBaseSizes[UiConfiguration::TitleFont] = 14;
			fontBaseSizes[UiConfiguration::HeadlineFont] = 16;
			break;
		}
		case 3: {
			paddingSize = 16.0f;
			marginSize = 16.0f;
			cornerRadius = 8;
			sliderTrackWidth = 240.0f;
			sliderTrackHeight = 5.0f;
			timelineMarkerWidth = 20.0f;
			fontBaseSizes[UiConfiguration::CaptionFont] = 10;
			fontBaseSizes[UiConfiguration::BodyFont] = 12;
			fontBaseSizes[UiConfiguration::ButtonFont] = 12;
			fontBaseSizes[UiConfiguration::TitleFont] = 16;
			fontBaseSizes[UiConfiguration::HeadlineFont] = 20;
			break;
		}
		case 4: {
			paddingSize = 20.0f;
			marginSize = 20.0f;
			cornerRadius = 10;
			sliderTrackWidth = 300.0f;
			sliderTrackHeight = 6.0f;
			timelineMarkerWidth = 20.0f;
			fontBaseSizes[UiConfiguration::CaptionFont] = 10;
			fontBaseSizes[UiConfiguration::BodyFont] = 12;
			fontBaseSizes[UiConfiguration::ButtonFont] = 12;
			fontBaseSizes[UiConfiguration::TitleFont] = 16;
			fontBaseSizes[UiConfiguration::HeadlineFont] = 20;
			break;
		}
		default: {
			paddingSize = 16.0f;
			marginSize = 16.0f;
			cornerRadius = 8;
			sliderTrackWidth = 200.0f;
			sliderTrackHeight = 6.0f;
			timelineMarkerWidth = 16.0f;
			fontBaseSizes[UiConfiguration::CaptionFont] = 10;
			fontBaseSizes[UiConfiguration::BodyFont] = 12;
			fontBaseSizes[UiConfiguration::ButtonFont] = 12;
			fontBaseSizes[UiConfiguration::TitleFont] = 14;
			fontBaseSizes[UiConfiguration::HeadlineFont] = 16;
			break;
		}
	}
}
