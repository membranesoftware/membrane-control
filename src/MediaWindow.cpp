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
#include <math.h>
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "OsUtil.h"
#include "MediaUtil.h"
#include "Widget.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "Button.h"
#include "TextArea.h"
#include "Json.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "MediaWindow.h"

MediaWindow::MediaWindow (Json *mediaItem, int layoutType, float maxMediaImageWidth)
: Panel ()
, thumbnailCount (0)
, mediaWidth (0)
, mediaHeight (0)
, mediaDuration (0.0f)
, mediaFrameRate (0.0f)
, mediaSize (0)
, mediaBitrate (0)
, mediaImage (NULL)
, nameLabel (NULL)
, detailText (NULL)
, mouseoverLabel (NULL)
, detailNameLabel (NULL)
{
	SystemInterface *interface;
	UiConfiguration *uiconfig;

	interface = &(App::instance->systemInterface);
	uiconfig = &(App::instance->uiConfig);

	setFillBg (true, uiconfig->mediumBackgroundColor);
	mediaId = interface->getCommandStringParam (mediaItem, "id", "");
	mediaName = interface->getCommandStringParam (mediaItem, "name", "");
	agentId = interface->getCommandAgentId (mediaItem);
	mediaWidth = interface->getCommandNumberParam (mediaItem, "width", (int) 0);
	mediaHeight = interface->getCommandNumberParam (mediaItem, "height", (int) 0);

	mediaImage = (ImageWindow *) addWidget (new ImageWindow (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LOADING_IMAGE_ICON))));
	nameLabel = (Label *) addWidget (new Label (mediaName, UiConfiguration::BODY, uiconfig->primaryTextColor));

	detailText = (TextArea *) addWidget (new TextArea (UiConfiguration::CAPTION, uiconfig->inverseTextColor));
	detailText->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	detailText->setFillBg (true, 0.0f, 0.0f, 0.0f);
	detailText->setAlphaBlend (true, uiconfig->scrimBackgroundAlpha);
	detailText->isVisible = false;

	mouseoverLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CAPTION, uiconfig->inverseTextColor)));
	mouseoverLabel->zLevel = 1;
	mouseoverLabel->setFillBg (true, 0.0f, 0.0f, 0.0f);
	mouseoverLabel->setAlphaBlend (true, uiconfig->scrimBackgroundAlpha);
	mouseoverLabel->isVisible = false;

	detailNameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (mediaName, UiConfiguration::HEADLINE, uiconfig->inverseTextColor)));
	detailNameLabel->zLevel = 1;
	detailNameLabel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	detailNameLabel->setFillBg (true, 0.0f, 0.0f, 0.0f);
	detailNameLabel->setAlphaBlend (true, uiconfig->scrimBackgroundAlpha);
	detailNameLabel->isVisible = false;

	setLayout (layoutType, maxMediaImageWidth);
}

MediaWindow::~MediaWindow () {

}

StdString MediaWindow::toStringDetail () {
	return (StdString::createSprintf (" MediaWindow mediaName=\"%s\" size=%.2fx%.2f", mediaName.c_str (), width, height));
}

bool MediaWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" MediaWindow mediaName="));
}

MediaWindow *MediaWindow::castWidget (Widget *widget) {
	return (MediaWindow::isWidgetType (widget) ? (MediaWindow *) widget : NULL);
}

bool MediaWindow::hasThumbnails () {
	return ((! agentId.empty ()) && (! thumbnailPath.empty ()) && (thumbnailCount > 0) && (mediaWidth > 0) && (mediaHeight > 0));
}

void MediaWindow::syncRecordStore () {
	RecordStore *store;
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	Json *mediaitem, *agentstatus, serverstatus, *streamitem, *params;
	StdString agentid, recordid, agentname;

	store = &(App::instance->agentControl.recordStore);
	interface = &(App::instance->systemInterface);
	uiconfig = &(App::instance->uiConfig);
	mediaitem = store->findRecord (mediaId, SystemInterface::CommandId_MediaItem);
	if (! mediaitem) {
		return;
	}

	agentid = interface->getCommandAgentId (mediaitem);
	agentstatus = store->findRecord (RecordStore::matchAgentStatusSource, &agentid);
	if (! agentstatus) {
		return;
	}
	if (! interface->getCommandObjectParam (agentstatus, "mediaServerStatus", &serverstatus)) {
		return;
	}

	mediaPath = serverstatus.getString ("mediaPath", "");
	thumbnailPath = serverstatus.getString ("thumbnailPath", "");
	thumbnailCount = serverstatus.getNumber ("thumbnailCount", (int) 0);
	if (mediaPath.empty ()) {
		return;
	}

	mediaDuration = interface->getCommandNumberParam (mediaitem, "duration", (float) 0.0f);
	mediaFrameRate = interface->getCommandNumberParam (mediaitem, "frameRate", (float) 0.0f);
	mediaSize = interface->getCommandNumberParam (mediaitem, "size", (int64_t) 0);
	mediaBitrate = interface->getCommandNumberParam (mediaitem, "bitrate", (int64_t) 0);

	streamitem = store->findRecord (MediaWindow::matchStreamSourceId, &mediaId);

	// TODO: Check for a relevant TaskItem record and show a progress widget if one is found

	if (! streamitem) {
		streamAgentId.assign ("");
		streamId.assign ("");
		streamAgentName.assign ("");
	}
	else {
		recordid = interface->getCommandRecordId (streamitem);
		if (! recordid.empty ()) {
			agentid = interface->getCommandAgentId (streamitem);
		}
		if (! agentid.empty ()) {
			agentstatus = store->findRecord (RecordStore::matchAgentStatusSource, &agentid);
			if (agentstatus) {
				agentname = interface->getCommandAgentName (agentstatus);
			}
		}

		if ((! recordid.empty ()) && (! agentid.empty ()) && (! agentname.empty ())) {
			streamId.assign (recordid);
			streamAgentId.assign (agentid);
			streamAgentName.assign (agentname);
		}
	}

	if (hasThumbnails () && mediaImage->isLoadUrlEmpty ()) {
		params = new Json ();
		params->set ("id", mediaId);
		params->set ("thumbnailIndex", (thumbnailCount / 4));
		mediaImage->setLoadUrl (App::instance->agentControl.getAgentSecondaryUrl (agentId, App::instance->createCommand (SystemInterface::Command_GetThumbnailImage, SystemInterface::Constant_Media, params), thumbnailPath), uiconfig->coreSprites.getSprite (UiConfiguration::LOADING_IMAGE_ICON));
	}

	refreshLayout ();
}

bool MediaWindow::matchStreamSourceId (void *idStringPtr, Json *record) {
	StdString *id;
	SystemInterface *interface;

	interface = &(App::instance->systemInterface);
	if (interface->isRecordClosed (record)) {
		return (false);
	}
	if (interface->getCommandId (record) != SystemInterface::CommandId_StreamItem) {
		return (false);
	}

	id = (StdString *) idStringPtr;
	return (id->equals (App::instance->systemInterface.getCommandStringParam (record, "sourceId", "")));
}

void MediaWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	UiText *uitext;
	StdString text;
	float x, y;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	mouseoverLabel->isVisible = false;
	x = 0.0f;
	y = 0.0f;
	mediaImage->position.assign (x, y);

	switch (layout) {
		case MediaWindow::LOW_DETAIL: {
			nameLabel->isVisible = false;
			detailNameLabel->isVisible = false;
			detailText->isVisible = false;

			mouseoverLabel->position.assign (0.0f, mediaImage->height - mouseoverLabel->height);
			setFixedSize (true, mediaImage->width, mediaImage->height);
			break;
		}
		case MediaWindow::MEDIUM_DETAIL: {
			detailNameLabel->isVisible = false;
			detailText->isVisible = false;

			x += uiconfig->paddingSize;
			y += mediaImage->height + uiconfig->marginSize;
			nameLabel->position.assign (x, y);
			nameLabel->isVisible = true;
			resetSize ();
			setFixedSize (true, mediaImage->width, maxWidgetY + uiconfig->paddingSize);
			break;
		}
		case MediaWindow::HIGH_DETAIL: {
			nameLabel->isVisible = false;

			detailNameLabel->position.assign (x, y);
			detailNameLabel->isVisible = true;

			text.sprintf ("%ix%i  %s  %s  %s", mediaWidth, mediaHeight, MediaUtil::getBitrateDisplayString (mediaBitrate).c_str (), OsUtil::getByteCountDisplayString (mediaSize).c_str (), OsUtil::getDurationDisplayString (mediaDuration).c_str ());
			if (! streamAgentName.empty ()) {
				text.appendSprintf ("\n%s: %s", uitext->getText (UiTextString::streamServer).capitalized ().c_str (), streamAgentName.c_str ());
			}

			detailText->setText (text);
			detailText->position.assign (mediaImage->position.x, mediaImage->position.y + mediaImage->height - detailText->height);
			detailText->isVisible = true;
			setFixedSize (true, mediaImage->width, mediaImage->height);
			break;
		}
	}
}

void MediaWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel::doProcessMouseState (mouseState);
	if (layout == MediaWindow::LOW_DETAIL) {
		if (mouseState.isEntered) {
			mouseoverLabel->isVisible = true;
		}
		else {
			mouseoverLabel->isVisible = false;
		}
	}
}

void MediaWindow::setLayout (int layoutType, float maxImageWidth) {
	float w, h;

	if (layoutType == layout) {
		return;
	}
	layout = layoutType;
	w = maxImageWidth;
	h = mediaHeight;
	h *= maxImageWidth;
	h /= mediaWidth;
	w = floorf (w);
	h = floorf (h);
	mediaImage->setWindowSize (w, h);
	mediaImage->reload ();

	if (layout == MediaWindow::HIGH_DETAIL) {
		nameLabel->setFont (UiConfiguration::BODY);
	}
	else {
		nameLabel->setFont (UiConfiguration::CAPTION);
	}

	if (layout == MediaWindow::LOW_DETAIL) {
		mouseoverLabel->setText (mediaName);
	}
	refreshLayout ();
}
