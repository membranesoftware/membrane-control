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
#include "SDL2/SDL.h"
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Util.h"
#include "Widget.h"
#include "Sprite.h"
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

MediaWindow::MediaWindow (Json *mediaItem, Sprite *loadingThumbnailSprite, int cardLayout, float maxMediaImageWidth)
: Panel ()
, thumbnailCount (0)
, mediaWidth (0)
, mediaHeight (0)
, mediaDuration (0.0f)
, mediaFrameRate (0.0f)
, mediaSize (0)
, mediaBitrate (0)
, loadingThumbnailSprite (loadingThumbnailSprite)
, mediaImage (NULL)
, nameLabel (NULL)
, detailText (NULL)
, mouseoverLabel (NULL)
, detailNameLabel (NULL)
, viewThumbnailsButton (NULL)
, createStreamButton (NULL)
, removeStreamButton (NULL)
, actionCallbackData (NULL)
, viewThumbnailsCallback (NULL)
, createStreamCallback (NULL)
, removeStreamCallback (NULL)
{
	SystemInterface *interface;
	UiText *uitext;
	UiConfiguration *uiconfig;

	interface = &(App::getInstance ()->systemInterface);
	uitext = &(App::getInstance ()->uiText);
	uiconfig = &(App::getInstance ()->uiConfig);

	setFillBg (true, uiconfig->mediumBackgroundColor);
	mediaId = interface->getCommandStringParam (mediaItem, "id", "");
	mediaName = interface->getCommandStringParam (mediaItem, "name", "");
	mediaWidth = interface->getCommandNumberParam (mediaItem, "width", (int) 0);
	mediaHeight = interface->getCommandNumberParam (mediaItem, "height", (int) 0);

	mediaImage = (ImageWindow *) addWidget (new ImageWindow (new Image (loadingThumbnailSprite)));
	nameLabel = (Label *) addWidget (new Label (mediaName, UiConfiguration::BODY, uiconfig->primaryTextColor));

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

	detailNameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (mediaName, UiConfiguration::HEADLINE, uiconfig->inverseTextColor)));
	detailNameLabel->zLevel = 1;
	detailNameLabel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	detailNameLabel->setFillBg (true, 0.0f, 0.0f, 0.0f);
	detailNameLabel->setAlphaBlend (true, uiconfig->imageTextScrimAlpha);
	detailNameLabel->isVisible = false;

	viewThumbnailsButton = (Button *) addWidget (new Button (uitext->viewThumbnails.uppercased ()));
	viewThumbnailsButton->setMouseClickCallback (MediaWindow::viewThumbnailsButtonClicked, this);
	viewThumbnailsButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	viewThumbnailsButton->setTextColor (uiconfig->raisedButtonTextColor);
	viewThumbnailsButton->isVisible = false;

	createStreamButton = (Button *) addWidget (new Button (uitext->createStream.uppercased ()));
	createStreamButton->setMouseClickCallback (MediaWindow::createStreamButtonClicked, this);
	createStreamButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	createStreamButton->setTextColor (uiconfig->raisedButtonTextColor);
	createStreamButton->isVisible = false;

	removeStreamButton = (Button *) addWidget (new Button (uitext->removeStream.uppercased ()));
	removeStreamButton->setMouseClickCallback (MediaWindow::removeStreamButtonClicked, this);
	removeStreamButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	removeStreamButton->setTextColor (uiconfig->raisedButtonTextColor);
	removeStreamButton->isVisible = false;

	setLayout (cardLayout, maxMediaImageWidth);
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

void MediaWindow::setActionCallbacks (void *callbackData, Widget::EventCallback viewThumbnails, Widget::EventCallback createStream, Widget::EventCallback removeStream) {
	actionCallbackData = callbackData;
	viewThumbnailsCallback = viewThumbnails;
	createStreamCallback = createStream;
	removeStreamCallback = removeStream;
}

void MediaWindow::syncRecordStore (RecordStore *store) {
	App *app;
	SystemInterface *interface;
	Json *mediaitem, *agentstatus, serverstatus, *streamitem, *params;
	StdString agentid, recordid, agentname, cmdjson;

	app = App::getInstance ();
	interface = &(app->systemInterface);
	mediaitem = store->findRecord (mediaId, SystemInterface::Command_MediaItem);
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

	mediaUrl = serverstatus.getString ("mediaUrl", "");
	thumbnailUrl = serverstatus.getString ("thumbnailUrl", "");
	thumbnailCount = serverstatus.getNumber ("thumbnailCount", (int) 0);
	if (mediaUrl.empty ()) {
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

	if (mediaImage->isLoadUrlEmpty () && (thumbnailCount > 0) && (! thumbnailUrl.empty ()) && (mediaWidth > 0) && (mediaHeight > 0)) {
		params = new Json ();
		params->set ("id", mediaId);
		params->set ("thumbnailIndex", (thumbnailCount / 4));
		cmdjson = app->createCommandJson ("GetThumbnailImage", SystemInterface::Constant_Media, params);
		if (! cmdjson.empty ()) {
			mediaImage->setLoadUrl (interface->getInvokeUrl (thumbnailUrl, cmdjson), loadingThumbnailSprite);
		}
	}

	resetLayout ();
}

bool MediaWindow::matchStreamSourceId (void *idStringPtr, Json *record) {
	StdString *id;
	SystemInterface *interface;

	interface = &(App::getInstance ()->systemInterface);
	if (interface->isRecordClosed (record)) {
		return (false);
	}
	if (interface->getCommandId (record) != SystemInterface::Command_StreamItem) {
		return (false);
	}

	id = (StdString *) idStringPtr;
	return (id->equals (App::getInstance ()->systemInterface.getCommandStringParam (record, "sourceId", "")));
}

void MediaWindow::resetLayout () {
	UiConfiguration *uiconfig;
	UiText *uitext;
	StdString text;
	float x, y;

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);
	mouseoverLabel->isVisible = false;
	x = 0.0f;
	y = 0.0f;
	mediaImage->position.assign (x, y);

	switch (layout) {
		case MediaWindow::LOW_DETAIL: {
			nameLabel->isVisible = false;
			detailNameLabel->isVisible = false;
			detailText->isVisible = false;
			viewThumbnailsButton->isVisible = false;
			createStreamButton->isVisible = false;
			removeStreamButton->isVisible = false;

			mouseoverLabel->position.assign (0.0f, mediaImage->height - mouseoverLabel->height);
			setFixedSize (true, mediaImage->width, mediaImage->height);
			break;
		}
		case MediaWindow::MEDIUM_DETAIL: {
			detailNameLabel->isVisible = false;
			detailText->isVisible = false;
			viewThumbnailsButton->isVisible = false;
			createStreamButton->isVisible = false;
			removeStreamButton->isVisible = false;

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

			text.sprintf ("%ix%i  %s  %s  %s", mediaWidth, mediaHeight, Util::getBitrateDisplayString (mediaBitrate).c_str (), Util::getFileSizeDisplayString (mediaSize).c_str (), Util::getDurationDisplayString (mediaDuration).c_str ());
			if (! streamAgentName.empty ()) {
				text.appendSprintf ("\n%s: %s", uitext->streamServer.capitalized ().c_str (), streamAgentName.c_str ());
			}

			detailText->setText (text);
			detailText->position.assign (mediaImage->position.x, mediaImage->position.y + mediaImage->height - detailText->height);
			detailText->isVisible = true;

			x += uiconfig->marginSize;
			y += mediaImage->height + uiconfig->marginSize;

			viewThumbnailsButton->position.assign (x, y);
			viewThumbnailsButton->isVisible = true;
			x += viewThumbnailsButton->width + uiconfig->marginSize;
			if (streamId.empty ()) {
				removeStreamButton->isVisible = false;
				createStreamButton->position.assign (x, y);
				createStreamButton->isVisible = true;
			}
			else {
				createStreamButton->isVisible = false;
				removeStreamButton->position.assign (x, y);
				removeStreamButton->isVisible = true;
			}
			resetSize ();
			setFixedSize (true, mediaImage->width, maxWidgetY + uiconfig->paddingSize);

			x = width - uiconfig->paddingSize;
			if (createStreamButton->isVisible) {
				x -= createStreamButton->width;
				createStreamButton->position.assignX (x);
				x -= uiconfig->marginSize;
			}
			if (removeStreamButton->isVisible) {
				x -= removeStreamButton->width;
				removeStreamButton->position.assignX (x);
				x -= uiconfig->marginSize;
			}
			x -= viewThumbnailsButton->width;
			viewThumbnailsButton->position.assignX (x);
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

void MediaWindow::setLayout (int cardLayout, float maxImageWidth) {
	float w, h;

	if (cardLayout == layout) {
		return;
	}
	layout = cardLayout;
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
	resetLayout ();
}

void MediaWindow::viewThumbnailsButtonClicked (void *windowPtr, Widget *widgetPtr) {
	MediaWindow *window;

	window = (MediaWindow *) windowPtr;
	if (window->viewThumbnailsCallback) {
		window->viewThumbnailsCallback (window->actionCallbackData, window);
	}
}

void MediaWindow::createStreamButtonClicked (void *windowPtr, Widget *widgetPtr) {
	MediaWindow *window;

	window = (MediaWindow *) windowPtr;
	if (window->createStreamCallback) {
		window->createStreamCallback (window->actionCallbackData, window);
	}
}

void MediaWindow::removeStreamButtonClicked (void *windowPtr, Widget *widgetPtr) {
	MediaWindow *window;

	window = (MediaWindow *) windowPtr;
	if (window->removeStreamCallback) {
		window->removeStreamCallback (window->actionCallbackData, window);
	}
}
