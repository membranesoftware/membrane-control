/*
* Copyright 2018 Membrane Software <author@membranesoftware.com>
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
#include "UiText.h"
#include "Util.h"
#include "Widget.h"
#include "Color.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "TextArea.h"
#include "Image.h"
#include "ImageWindow.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "StreamWindow.h"

StreamWindow::StreamWindow (Json *streamItem, Sprite *loadingThumbnailSprite, int cardLayout, float maxStreamImageWidth)
: Panel ()
, segmentCount (0)
, frameWidth (0)
, frameHeight (0)
, duration (0)
, frameRate (0.0f)
, bitrate (0)
, loadingThumbnailSprite (loadingThumbnailSprite)
, streamImage (NULL)
, nameLabel (NULL)
, detailText (NULL)
, mouseoverLabel (NULL)
, detailNameLabel (NULL)
{
	SystemInterface *interface;
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	interface = &(App::getInstance ()->systemInterface);
	streamId = interface->getCommandStringParam (streamItem, "id", "");
	streamName = interface->getCommandStringParam (streamItem, "name", "");
	frameWidth = interface->getCommandNumberParam (streamItem, "width", (int) 0);
	frameHeight = interface->getCommandNumberParam (streamItem, "height", (int) 0);
	frameRate = interface->getCommandNumberParam (streamItem, "frameRate", (float) 0.0f);
	bitrate = interface->getCommandNumberParam (streamItem, "bitrate", (int64_t) 0);

	streamImage = (ImageWindow *) addWidget (new ImageWindow (new Image (loadingThumbnailSprite)));
	nameLabel = (Label *) addWidget (new Label (streamName, UiConfiguration::CAPTION, uiconfig->primaryTextColor));

	detailText = (TextArea *) addWidget (new TextArea (UiConfiguration::CAPTION, uiconfig->inverseTextColor));
	detailText->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	detailText->setFillBg (true, 0.0f, 0.0f, 0.0f);
	detailText->setAlphaBlend (true, uiconfig->imageTextScrimAlpha);
	detailText->isVisible = false;

	mouseoverLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CAPTION, uiconfig->inverseTextColor)));
	mouseoverLabel->zLevel = 1;
	mouseoverLabel->setFillBg (true, 0.0f, 0.0f, 0.0f);
	mouseoverLabel->setAlphaBlend (true, uiconfig->imageTextScrimAlpha);
	mouseoverLabel->isVisible = false;

	detailNameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (streamName, UiConfiguration::HEADLINE, uiconfig->inverseTextColor)));
	detailNameLabel->zLevel = 1;
	detailNameLabel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	detailNameLabel->setFillBg (true, 0.0f, 0.0f, 0.0f);
	detailNameLabel->setAlphaBlend (true, uiconfig->imageTextScrimAlpha);
	detailNameLabel->isVisible = false;

	setLayout (cardLayout, maxStreamImageWidth);
}

StreamWindow::~StreamWindow () {

}

StdString StreamWindow::toStringDetail () {
	return (StdString::createSprintf (" StreamWindow streamName=\"%s\"", streamName.c_str ()));
}

bool StreamWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" StreamWindow streamName="));
}

StreamWindow *StreamWindow::castWidget (Widget *widget) {
	return (StreamWindow::isWidgetType (widget) ? (StreamWindow *) widget : NULL);
}

void StreamWindow::setLayout (int cardLayout, float maxImageWidth) {
	float w, h;

	if (cardLayout == layout) {
		return;
	}
	layout = cardLayout;
	w = maxImageWidth;
	h = frameHeight;
	h *= maxImageWidth;
	h /= frameWidth;
	w = floorf (w);
	h = floorf (h);
	streamImage->setWindowSize (w, h);
	streamImage->reload ();

	if (layout == StreamWindow::LOW_DETAIL) {
		mouseoverLabel->setText (streamName);
	}

	resetLayout ();
}

void StreamWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel::doProcessMouseState (mouseState);
	if (layout == StreamWindow::LOW_DETAIL) {
		if (mouseState.isEntered) {
			mouseoverLabel->isVisible = true;
		}
		else {
			mouseoverLabel->isVisible = false;
		}
	}
}

void StreamWindow::syncRecordStore (RecordStore *store) {
	App *app;
	SystemInterface *interface;
	Json *streamitem, *agentstatus, serverstatus, *params;
	StdString agentid, cmdjson;

	app = App::getInstance ();
	interface = &(app->systemInterface);
	streamitem = store->findRecord (streamId, SystemInterface::Command_StreamItem);
	if (! streamitem) {
		return;
	}

	agentid = interface->getCommandAgentId (streamitem);
	agentstatus = store->findRecord (RecordStore::matchAgentStatusSource, &agentid);
	if (! agentstatus) {
		return;
	}
	if (! interface->getCommandObjectParam (agentstatus, "streamServerStatus", &serverstatus)) {
		return;
	}

	thumbnailUrl = serverstatus.getString ("thumbnailUrl", "");
	hlsStreamUrl = serverstatus.getString ("hlsStreamUrl", "");
	segmentCount = interface->getCommandNumberParam (streamitem, "segmentCount", (int) 0);
	duration = interface->getCommandNumberParam (streamitem, "duration", (int64_t) 0);

	if (streamImage->isLoadUrlEmpty () && (segmentCount > 0) && (! thumbnailUrl.empty ())) {
		params = new Json ();
		params->set ("id", streamId);
		params->set ("thumbnailIndex", (segmentCount / 2));
		cmdjson = app->createCommandJson ("GetThumbnailImage", SystemInterface::Constant_Stream, params);
		if (! cmdjson.empty ()) {
			streamImage->setLoadUrl (interface->getInvokeUrl (thumbnailUrl, cmdjson), loadingThumbnailSprite);
		}
	}

	resetLayout ();
}

void StreamWindow::resetLayout () {
	UiConfiguration *uiconfig;
	StdString text;
	float x, y;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = 0.0f;
	y = 0.0f;
	streamImage->position.assign (x, y);

	switch (layout) {
		case StreamWindow::LOW_DETAIL: {
			nameLabel->isVisible = false;
			detailNameLabel->isVisible = false;
			detailText->isVisible = false;

			mouseoverLabel->position.assign (0.0f, streamImage->height - mouseoverLabel->height);
			setFixedSize (true, streamImage->width, streamImage->height);
			break;
		}
		case StreamWindow::MEDIUM_DETAIL: {
			detailNameLabel->isVisible = false;
			detailText->isVisible = false;

			x += uiconfig->paddingSize;
			y += streamImage->height + uiconfig->marginSize;
			nameLabel->position.assign (x, y);
			nameLabel->isVisible = true;
			resetSize ();
			setFixedSize (true, streamImage->width, maxWidgetY + uiconfig->paddingSize);
			break;
		}
		case StreamWindow::HIGH_DETAIL: {
			nameLabel->isVisible = false;

			detailNameLabel->position.assign (x, y);
			detailNameLabel->isVisible = true;

			text.sprintf ("%ix%i  %s  %s", frameWidth, frameHeight, Util::getBitrateDisplayString (bitrate).c_str (), Util::getDurationDisplayString (duration).c_str ());

			detailText->setText (text);
			detailText->position.assign (streamImage->position.x, streamImage->position.y + streamImage->height - detailText->height);
			detailText->isVisible = true;

			x += uiconfig->marginSize;
			y += streamImage->height + uiconfig->marginSize;

			resetSize ();
			setFixedSize (true, streamImage->width, maxWidgetY);
			break;
		}
	}
	resetSize ();
}
