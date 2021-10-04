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
#include "Config.h"
#include <stdlib.h>
#include "App.h"
#include "ClassId.h"
#include "StdString.h"
#include "StringList.h"
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
#include "SystemInterface.h"
#include "RecordStore.h"
#include "AgentControl.h"
#include "UiConfiguration.h"
#include "MediaItemUi.h"
#include "MediaDetailWindow.h"

MediaDetailWindow::MediaDetailWindow (const StdString &recordId, SpriteGroup *mediaItemUiSpriteGroup)
: Panel ()
, recordId (recordId)
, mediaWidth (0)
, mediaHeight (0)
, mediaDuration (0.0f)
, mediaFrameRate (0.0f)
, mediaSize (0)
, mediaBitrate (0)
, sprites (mediaItemUiSpriteGroup)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionLabel (NULL)
, attributesIcon (NULL)
, fileSizeIcon (NULL)
, durationIcon (NULL)
{
	classId = ClassId::MediaDetailWindow;

	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeMediaIconSprite)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor));
	descriptionLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));

	attributesIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (MediaItemUi::AttributesIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	attributesIcon->setPadding (0.0f, 0.0f);
	attributesIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaAttributesTooltip));

	fileSizeIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StorageIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	fileSizeIcon->setPadding (0.0f, 0.0f);
	fileSizeIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::FileSize).capitalized ());

	durationIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (MediaItemUi::DurationIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	durationIcon->setPadding (0.0f, 0.0f);
	durationIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::Duration).capitalized ());

	refreshLayout ();
}

MediaDetailWindow::~MediaDetailWindow () {

}

StdString MediaDetailWindow::toStringDetail () {
	return (StdString::createSprintf (" MediaDetailWindow recordId=\"%s\"", recordId.c_str ()));
}

bool MediaDetailWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::MediaDetailWindow));
}

MediaDetailWindow *MediaDetailWindow::castWidget (Widget *widget) {
	return (MediaDetailWindow::isWidgetType (widget) ? (MediaDetailWindow *) widget : NULL);
}

void MediaDetailWindow::syncRecordStore () {
	Json *record;
	StdString text, rationame, framesizename;
	StringList attributes;

	record = RecordStore::instance->findRecord (recordId, SystemInterface::CommandId_MediaItem);
	if (! record) {
		return;
	}

	agentId = SystemInterface::instance->getCommandAgentId (record);
	mediaName = SystemInterface::instance->getCommandStringParam (record, "name", "");
	mediaWidth = SystemInterface::instance->getCommandNumberParam (record, "width", (int) 0);
	mediaHeight = SystemInterface::instance->getCommandNumberParam (record, "height", (int) 0);
	mediaSize = SystemInterface::instance->getCommandNumberParam (record, "size", (int64_t) 0);
	mediaDuration = SystemInterface::instance->getCommandNumberParam (record, "duration", (float) 0.0f);
	mediaFrameRate = SystemInterface::instance->getCommandNumberParam (record, "frameRate", (float) 0.0f);
	mediaBitrate = SystemInterface::instance->getCommandNumberParam (record, "bitrate", (int64_t) 0);

	nameLabel->setText (mediaName);
	descriptionLabel->setText (AgentControl::instance->getAgentDisplayName (agentId));

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

		attributes.push_back (text);
	}

	if (mediaFrameRate > 0.0f) {
		attributes.push_back (StdString::createSprintf ("%.2ffps", mediaFrameRate));
	}

	if (mediaBitrate > 0.0f) {
		attributes.push_back (MediaUtil::getBitrateDisplayString (mediaBitrate));
	}

	attributesIcon->setText (attributes.join (", "));

	if (mediaSize > 0) {
		fileSizeIcon->setText (StdString::createSprintf ("%s (%lli)", OsUtil::getByteCountDisplayString (mediaSize).c_str (), (long long int) mediaSize));
	}

	if (mediaDuration > 0.0f) {
		durationIcon->setText (OsUtil::getDurationDisplayString (mediaDuration));
	}

	refreshLayout ();
	Panel::syncRecordStore ();
}

void MediaDetailWindow::refreshLayout () {
	float x, y, x0, x2, y2;

	x = widthPadding;
	y = heightPadding;
	x0 = x;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, NULL, &y2);
	x2 = x;
	nameLabel->flowRight (&x, y, NULL, &y2);
	descriptionLabel->position.assign (x2, descriptionLabel->getLinePosition (y + UiConfiguration::instance->marginSize + nameLabel->maxLineHeight));

	y = y2 + UiConfiguration::instance->marginSize;
	x = x0;
	attributesIcon->flowDown (x, &y, &x2, &y2);

	y = y2 + UiConfiguration::instance->marginSize;
	x = x0;
	fileSizeIcon->flowRight (&x, y, &x2, &y2);
	durationIcon->flowDown (x, &y, &x2, &y2);

	resetSize ();
}
