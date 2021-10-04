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
#include "SystemInterface.h"
#include "AgentControl.h"
#include "RecordStore.h"
#include "UiConfiguration.h"
#include "StreamItemUi.h"
#include "StreamDetailWindow.h"

StreamDetailWindow::StreamDetailWindow (const StdString &recordId, SpriteGroup *streamItemUiSpriteGroup)
: Panel ()
, recordId (recordId)
, streamWidth (0)
, streamHeight (0)
, streamDuration (0.0f)
, streamFrameRate (0.0f)
, streamSize (0)
, streamBitrate (0)
, streamProfile (-1)
, sprites (streamItemUiSpriteGroup)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionLabel (NULL)
, attributesIcon (NULL)
, fileSizeIcon (NULL)
, durationIcon (NULL)
{
	classId = ClassId::StreamDetailWindow;

	setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeStreamIconSprite)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::TitleFont, UiConfiguration::instance->primaryTextColor));
	descriptionLabel = (Label *) addWidget (new Label (UiText::instance->getText (UiTextString::VideoStream).capitalized (), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));

	attributesIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (StreamItemUi::AttributesIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	attributesIcon->setPadding (0.0f, 0.0f);
	attributesIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaAttributesTooltip));

	fileSizeIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StorageIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	fileSizeIcon->setPadding (0.0f, 0.0f);
	fileSizeIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::FileSize).capitalized ());

	durationIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (StreamItemUi::DurationIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	durationIcon->setPadding (0.0f, 0.0f);
	durationIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::Duration).capitalized ());

	refreshLayout ();
}

StreamDetailWindow::~StreamDetailWindow () {

}

StdString StreamDetailWindow::toStringDetail () {
	return (StdString::createSprintf (" StreamDetailWindow recordId=\"%s\"", recordId.c_str ()));
}

bool StreamDetailWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::StreamDetailWindow));
}

StreamDetailWindow *StreamDetailWindow::castWidget (Widget *widget) {
	return (StreamDetailWindow::isWidgetType (widget) ? (StreamDetailWindow *) widget : NULL);
}

void StreamDetailWindow::syncRecordStore () {
	Json *record;
	StdString text, profilename, rationame, framesizename;
	StringList attributes;

	record = RecordStore::instance->findRecord (recordId, SystemInterface::CommandId_StreamItem);
	if (! record) {
		return;
	}

	agentId = SystemInterface::instance->getCommandAgentId (record);
	streamName = SystemInterface::instance->getCommandStringParam (record, "name", "");
	streamWidth = SystemInterface::instance->getCommandNumberParam (record, "width", (int) 0);
	streamHeight = SystemInterface::instance->getCommandNumberParam (record, "height", (int) 0);
	streamSize = SystemInterface::instance->getCommandNumberParam (record, "size", (int64_t) 0);
	streamDuration = SystemInterface::instance->getCommandNumberParam (record, "duration", (float) 0.0f);
	streamFrameRate = SystemInterface::instance->getCommandNumberParam (record, "frameRate", (float) 0.0f);
	streamBitrate = SystemInterface::instance->getCommandNumberParam (record, "bitrate", (int64_t) 0);
	streamProfile = SystemInterface::instance->getCommandNumberParam (record, "profile", (int) -1);

	streamName.assign (SystemInterface::instance->getCommandStringParam (record, "name", ""));
	nameLabel->setText (streamName);
	descriptionLabel->setText (AgentControl::instance->getAgentDisplayName (agentId));

	if ((streamWidth > 0) && (streamHeight > 0)) {
		text.sprintf ("%ix%i", streamWidth, streamHeight);
		rationame = MediaUtil::getAspectRatioDisplayString (streamWidth, streamHeight);
		framesizename = MediaUtil::getFrameSizeName (streamWidth, streamHeight);
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

	if (streamFrameRate > 0.0f) {
		attributes.push_back (StdString::createSprintf ("%.2ffps", streamFrameRate));
	}

	if (streamBitrate > 0.0f) {
		attributes.push_back (MediaUtil::getBitrateDisplayString (streamBitrate));
	}

	text.assign ("");
	profilename = MediaUtil::getStreamProfileName (streamProfile);
	if (! profilename.empty ()) {
		text.appendSprintf ("[%s] ", profilename.c_str ());
	}
	text.append (attributes.join (", "));
	attributesIcon->setText (text);

	if (streamSize > 0) {
		fileSizeIcon->setText (StdString::createSprintf ("%s (%lli)", OsUtil::getByteCountDisplayString (streamSize).c_str (), (long long int) streamSize));
	}

	if (streamDuration > 0.0f) {
		durationIcon->setText (OsUtil::getDurationDisplayString (streamDuration));
	}

	refreshLayout ();
	Panel::syncRecordStore ();
}

void StreamDetailWindow::refreshLayout () {
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
