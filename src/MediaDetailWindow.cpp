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
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "OsUtil.h"
#include "MediaUtil.h"
#include "Widget.h"
#include "Color.h"
#include "Image.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "Button.h"
#include "Toggle.h"
#include "StatsWindow.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "MediaDetailWindow.h"

MediaDetailWindow::MediaDetailWindow (const StdString &recordId)
: Panel ()
, recordId (recordId)
, mediaWidth (0)
, mediaHeight (0)
, mediaDuration (0.0f)
, mediaFrameRate (0.0f)
, mediaSize (0)
, mediaBitrate (0)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionLabel (NULL)
, statsWindow (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	populate ();
	refreshLayout ();
}

MediaDetailWindow::~MediaDetailWindow () {

}

StdString MediaDetailWindow::toStringDetail () {
	return (StdString::createSprintf (" MediaDetailWindow recordId=\"%s\"", recordId.c_str ()));
}

bool MediaDetailWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" MediaDetailWindow recordId="));
}

MediaDetailWindow *MediaDetailWindow::castWidget (Widget *widget) {
	return (MediaDetailWindow::isWidgetType (widget) ? (MediaDetailWindow *) widget : NULL);
}

void MediaDetailWindow::populate () {
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LARGE_MEDIA_ICON)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::TITLE, uiconfig->primaryTextColor));
	descriptionLabel = (Label *) addWidget (new Label (uitext->getText (UiTextString::mediaFile).capitalized (), UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor));

	statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
	statsWindow->setItem (uitext->getText (UiTextString::server).capitalized (), StdString (""));
	statsWindow->setItem (uitext->getText (UiTextString::frameSize).capitalized (), StdString (""));
	statsWindow->setItem (uitext->getText (UiTextString::frameRate).capitalized (), StdString (""));
	statsWindow->setItem (uitext->getText (UiTextString::bitrate).capitalized (), StdString (""));
	statsWindow->setItem (uitext->getText (UiTextString::duration).capitalized (), StdString (""));
	statsWindow->setItem (uitext->getText (UiTextString::fileSize).capitalized (), StdString (""));
	statsWindow->setPadding (uiconfig->paddingSize, 0.0f);
}

void MediaDetailWindow::syncRecordStore () {
	RecordStore *store;
	SystemInterface *interface;
	UiText *uitext;
	Json *record;
	StdString text, rationame, framesizename;

	store = &(App::instance->agentControl.recordStore);
	interface = &(App::instance->systemInterface);
	uitext = &(App::instance->uiText);
	record = store->findRecord (recordId, SystemInterface::CommandId_MediaItem);
	if (! record) {
		return;
	}

	agentId = interface->getCommandAgentId (record);
	mediaName = interface->getCommandStringParam (record, "name", "");
	mediaWidth = interface->getCommandNumberParam (record, "width", (int) 0);
	mediaHeight = interface->getCommandNumberParam (record, "height", (int) 0);
	mediaSize = interface->getCommandNumberParam (record, "size", (int64_t) 0);
	mediaDuration = interface->getCommandNumberParam (record, "duration", (float) 0.0f);
	mediaFrameRate = interface->getCommandNumberParam (record, "frameRate", (float) 0.0f);
	mediaBitrate = interface->getCommandNumberParam (record, "bitrate", (int64_t) 0);

	mediaName.assign (interface->getCommandStringParam (record, "name", ""));
	nameLabel->setText (mediaName);

	statsWindow->setItem (uitext->getText (UiTextString::server).capitalized (), App::instance->agentControl.getAgentDisplayName (agentId));

	if ((mediaWidth > 0) && (mediaHeight > 0)) {
		text.sprintf ("%ix%i", mediaWidth, mediaHeight);
		rationame = MediaUtil::getAspectRatioDisplayString (mediaWidth, mediaHeight);
		framesizename = MediaUtil::getFrameSizeName (mediaWidth, mediaHeight);
		if ((! rationame.empty ()) || (! framesizename.empty ())) {
			text.append (" (");
			if (! rationame.empty ()) {
				text.append (rationame);
			}
			if (! framesizename.empty ()) {
				if (! rationame.empty ()) {
					text.append (", ");
				}
				text.append (framesizename);
			}
			text.append (")");
		}

		statsWindow->setItem (uitext->getText (UiTextString::frameSize).capitalized (), text);
	}

	if (mediaFrameRate > 0.0f) {
		statsWindow->setItem (uitext->getText (UiTextString::frameRate).capitalized (), StdString::createSprintf ("%.2ffps", mediaFrameRate));
	}

	if (mediaBitrate > 0.0f) {
		statsWindow->setItem (uitext->getText (UiTextString::bitrate).capitalized (), MediaUtil::getBitrateDisplayString (mediaBitrate));
	}

	if (mediaSize > 0) {
		statsWindow->setItem (uitext->getText (UiTextString::fileSize).capitalized (), OsUtil::getByteCountDisplayString (mediaSize));
	}

	if (mediaDuration > 0.0f) {
		statsWindow->setItem (uitext->getText (UiTextString::duration).capitalized (), OsUtil::getDurationDisplayString (mediaDuration));
	}

	refreshLayout ();
	Panel::syncRecordStore ();
}

void MediaDetailWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, x0, x2, y2;

	uiconfig = &(App::instance->uiConfig);
	x = widthPadding;
	y = heightPadding;
	x0 = x;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, NULL, &y2);
	x2 = x;
	nameLabel->flowRight (&x, y, NULL, &y2);
	descriptionLabel->position.assign (x2, descriptionLabel->getLinePosition (y + uiconfig->marginSize + nameLabel->maxLineHeight));

	y = y2 + uiconfig->marginSize;
	x = x0;
	statsWindow->flowDown (x, &y);

	resetSize ();
}
