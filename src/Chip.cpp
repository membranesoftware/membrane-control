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
#include "Config.h"
#include <stdlib.h>
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Widget.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "Chip.h"

Chip::Chip (const StdString &chipText, Sprite *iconSprite, bool shouldDestroySprite)
: Panel ()
, textLabel (NULL)
, iconImage (NULL)
{
	UiConfiguration *uiconfig;

	widgetType.assign ("Chip");

	uiconfig = &(App::instance->uiConfig);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize / 2.0f);
	setFillBg (true, uiconfig->mediumPrimaryColor);

	if (! chipText.empty ()) {
		textLabel = (Label *) addWidget (new Label (chipText, UiConfiguration::CaptionFont, uiconfig->inverseTextColor));
	}
	if (iconSprite) {
		iconImage = (Image *) addWidget (new Image (iconSprite, UiConfiguration::ChipIconFrame, shouldDestroySprite));
	}

	refreshLayout ();
}

Chip::~Chip () {

}

StdString Chip::toStringDetail () {
  return (StdString::createSprintf (" text=\"%s\"", textLabel ? textLabel->text.c_str () : ""));
}

void Chip::setText (const StdString &text) {
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);
	if (text.empty ()) {
		if (textLabel) {
			textLabel->isDestroyed = true;
			textLabel = NULL;
		}
	}
	else {
		if (! textLabel) {
			textLabel = (Label *) addWidget (new Label (text, UiConfiguration::CaptionFont, uiconfig->inverseTextColor));
		}
		else {
			textLabel->setText (text);
		}
	}
	refreshLayout ();
}

void Chip::setIconSprite (Sprite *iconSprite, bool shouldDestroySprite) {
	if (iconImage) {
		iconImage->isDestroyed = true;
		iconImage = NULL;
	}
	if (iconSprite) {
		iconImage = (Image *) addWidget (new Image (iconSprite, UiConfiguration::ChipIconFrame, shouldDestroySprite));
	}
	refreshLayout ();
}

void Chip::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y;

	uiconfig = &(App::instance->uiConfig);
	x = uiconfig->paddingSize;
	y = uiconfig->paddingSize;

	if (iconImage) {
		iconImage->position.assign (x, y);
		x += iconImage->width + uiconfig->marginSize;
	}
	if (textLabel) {
		textLabel->position.assign (x, y);
	}

	resetSize ();
	if (iconImage) {
		iconImage->position.assign (iconImage->position.x, (height / 2.0f) - (iconImage->height / 2.0f));
	}
	if (textLabel) {
		textLabel->position.assign (textLabel->position.x, (height / 2.0f) - (textLabel->height / 2.0f));
	}
}
