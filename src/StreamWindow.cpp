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
#include "UiText.h"
#include "Util.h"
#include "Widget.h"
#include "Sprite.h"
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

StreamWindow::StreamWindow (Json *streamItem, int layoutType, float maxStreamImageWidth)
: Panel ()
, isSelected (false)
, selectedTimestamp (0.0f)
, thumbnailIndex (-1)
, segmentCount (0)
, frameWidth (0)
, frameHeight (0)
, duration (0)
, frameRate (0.0f)
, bitrate (0)
, streamImage (NULL)
, nameLabel (NULL)
, detailText (NULL)
, mouseoverLabel (NULL)
, detailNameLabel (NULL)
, selectPanel (NULL)
, spacerPanel (NULL)
, timestampLabel (NULL)
, selectButton (NULL)
, viewButton (NULL)
, streamImageClickCallback (NULL)
, streamImageClickCallbackData (NULL)
, viewButtonClickCallback (NULL)
, viewButtonClickCallbackData (NULL)
{
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	interface = &(App::getInstance ()->systemInterface);
	streamId = interface->getCommandStringParam (streamItem, "id", "");
	agentId = interface->getCommandAgentId (streamItem);
	streamName = interface->getCommandStringParam (streamItem, "name", "");
	frameWidth = interface->getCommandNumberParam (streamItem, "width", (int) 0);
	frameHeight = interface->getCommandNumberParam (streamItem, "height", (int) 0);
	frameRate = interface->getCommandNumberParam (streamItem, "frameRate", (float) 0.0f);
	bitrate = interface->getCommandNumberParam (streamItem, "bitrate", (int64_t) 0);

	streamImage = (ImageWindow *) addWidget (new ImageWindow (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LOADING_IMAGE_ICON))));
	streamImage->setMouseClickCallback (StreamWindow::streamImageClicked, this);

	nameLabel = (Label *) addWidget (new Label (streamName, UiConfiguration::CAPTION, uiconfig->primaryTextColor));
	nameLabel->isInputSuspended = true;

	detailText = (TextArea *) addWidget (new TextArea (UiConfiguration::CAPTION, uiconfig->inverseTextColor));
	detailText->zLevel = 1;
	detailText->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	detailText->setFillBg (true, 0.0f, 0.0f, 0.0f);
	detailText->setAlphaBlend (true, uiconfig->scrimBackgroundAlpha);
	detailText->isInputSuspended = true;
	detailText->isVisible = false;

	mouseoverLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CAPTION, uiconfig->inverseTextColor)));
	mouseoverLabel->zLevel = 3;
	mouseoverLabel->setFillBg (true, 0.0f, 0.0f, 0.0f);
	mouseoverLabel->setAlphaBlend (true, uiconfig->scrimBackgroundAlpha);
	mouseoverLabel->isInputSuspended = true;
	mouseoverLabel->isVisible = false;

	detailNameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (streamName, UiConfiguration::HEADLINE, uiconfig->inverseTextColor)));
	detailNameLabel->zLevel = 1;
	detailNameLabel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	detailNameLabel->setFillBg (true, 0.0f, 0.0f, 0.0f);
	detailNameLabel->setAlphaBlend (true, uiconfig->scrimBackgroundAlpha);
	detailNameLabel->isInputSuspended = true;
	detailNameLabel->isVisible = false;

	selectPanel = (Panel *) addWidget (new Panel ());
	selectPanel->zLevel = 2;
	selectPanel->setFillBg (true, uiconfig->darkBackgroundColor);
	selectPanel->setLayout (Panel::HORIZONTAL);
	selectPanel->isVisible = false;

	spacerPanel = (Panel *) selectPanel->addWidget (new Panel ());
	spacerPanel->setFixedSize (true, 1.0f, 1.0f);
	spacerPanel->isInputSuspended = true;

	timestampLabel = (Label *) selectPanel->addWidget (new Label (StdString (""), UiConfiguration::CAPTION, uiconfig->primaryTextColor));
	timestampLabel->isInputSuspended = true;

	viewButton = (Button *) selectPanel->addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::IMAGE_BUTTON)));
	viewButton->setImageColor (uiconfig->flatButtonTextColor);
	viewButton->setMouseHoverTooltip (uitext->getText (UiTextString::viewStreamTooltip));
	viewButton->setMouseClickCallback (StreamWindow::viewButtonClicked, this);
	viewButton->isVisible = false;

	selectButton = (Button *) selectPanel->addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::STAR_BUTTON)));
	selectButton->setImageColor (uiconfig->flatButtonTextColor);
	selectButton->setMouseHoverTooltip (uitext->getText (UiTextString::unselectButtonTooltip));
	selectButton->setMouseClickCallback (StreamWindow::selectButtonClicked, this);

	setLayout (layoutType, maxStreamImageWidth);
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

void StreamWindow::setLayout (int layoutType, float maxImageWidth) {
	float w, h;

	if (layoutType == layout) {
		return;
	}
	layout = layoutType;
	w = maxImageWidth;
	h = frameHeight;
	h *= maxImageWidth;
	h /= frameWidth;
	w = floorf (w);
	h = floorf (h);
	streamImage->setWindowSize (w, h);
	streamImage->reload ();

	switch (layout) {
		case StreamWindow::LOW_DETAIL: {
			mouseoverLabel->setText (streamName);
			nameLabel->isVisible = false;
			detailNameLabel->isVisible = false;
			detailText->isVisible = false;
			spacerPanel->isVisible = false;
			timestampLabel->isVisible = false;
			break;
		}
		case StreamWindow::MEDIUM_DETAIL: {
			detailNameLabel->isVisible = false;
			detailText->isVisible = false;
			nameLabel->isVisible = true;
			spacerPanel->isVisible = true;
			timestampLabel->isVisible = true;
			break;
		}
		case StreamWindow::HIGH_DETAIL: {
			nameLabel->isVisible = false;
			detailNameLabel->isVisible = true;
			detailText->isVisible = true;
			spacerPanel->isVisible = true;
			timestampLabel->isVisible = true;
			break;
		}
	}

	selectPanel->refresh ();
	refreshLayout ();
}

void StreamWindow::setStreamImageClickCallback (Widget::EventCallback callback, void *callbackData) {
	streamImageClickCallback = callback;
	streamImageClickCallbackData = callbackData;
}

void StreamWindow::setViewButtonClickCallback (Widget::EventCallback callback, void *callbackData) {
	viewButtonClickCallback = callback;
	viewButtonClickCallbackData = callbackData;
	if (viewButtonClickCallback) {
		viewButton->isVisible = true;
	}
	else {
		viewButton->isVisible = false;
	}
}

void StreamWindow::setThumbnailIndex (int index) {
	App *app;
	UiConfiguration *uiconfig;
	Json *params;

	if ((index == thumbnailIndex) || (segmentCount <= 0) || (index < 0) || (index >= segmentCount)) {
		return;
	}

	app = App::getInstance ();
	uiconfig = &(app->uiConfig);
	thumbnailIndex = index;
	if (! thumbnailPath.empty ()) {
		params = new Json ();
		params->set ("id", streamId);
		params->set ("thumbnailIndex", thumbnailIndex);
		streamImage->setLoadUrl (app->agentControl.getAgentSecondaryUrl (agentId, app->createCommand ("GetThumbnailImage", SystemInterface::Constant_Stream, params), thumbnailPath), uiconfig->coreSprites.getSprite (UiConfiguration::LOADING_IMAGE_ICON));
	}
}

void StreamWindow::setSelected (bool selected, float timestamp) {
	isSelected = selected;
	if (! isSelected) {
		selectPanel->isVisible = false;
	}
	else {
		selectedTimestamp = timestamp;
		selectPanel->isVisible = true;

		timestampLabel->setText (Util::getDurationString ((int64_t) selectedTimestamp, Util::HOURS));
		selectPanel->refresh ();
	}

	refreshLayout ();
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
	Json *record, *agentstatus, serverstatus;

	app = App::getInstance ();
	interface = &(app->systemInterface);
	record = store->findRecord (streamId, SystemInterface::Command_StreamItem);
	if (! record) {
		return;
	}

	agentstatus = store->findRecord (RecordStore::matchAgentStatusSource, &agentId);
	if (! agentstatus) {
		return;
	}
	if (interface->getCommandObjectParam (agentstatus, "streamServerStatus", &serverstatus)) {
		thumbnailPath = serverstatus.getString ("thumbnailPath", "");
		hlsStreamPath = serverstatus.getString ("hlsStreamPath", "");
	}
	else if (interface->getCommandObjectParam (agentstatus, "monitorServerStatus", &serverstatus)) {
		thumbnailPath = serverstatus.getString ("thumbnailPath", "");
		hlsStreamPath.assign ("");
	}

	segmentCount = interface->getCommandNumberParam (record, "segmentCount", (int) 0);
	interface->getCommandNumberArrayParam (record, "segmentPositions", &segmentPositions, true);
	duration = interface->getCommandNumberParam (record, "duration", (int64_t) 0);

	detailText->setText (StdString::createSprintf ("%ix%i  %s  %s", frameWidth, frameHeight, Util::getBitrateDisplayString (bitrate).c_str (), Util::getDurationDisplayString (duration).c_str ()));

	if (streamImage->isLoadUrlEmpty () && (segmentCount > 0) && (! thumbnailPath.empty ())) {
		setThumbnailIndex (segmentCount / 8);
	}

	refreshLayout ();
}

void StreamWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = 0.0f;
	y = 0.0f;
	streamImage->position.assign (x, y);

	switch (layout) {
		case StreamWindow::LOW_DETAIL: {
			mouseoverLabel->position.assign (0.0f, streamImage->height - mouseoverLabel->height);
			setFixedSize (true, streamImage->width, streamImage->height);
			break;
		}
		case StreamWindow::MEDIUM_DETAIL: {
			x += uiconfig->paddingSize;
			y += streamImage->height + uiconfig->marginSize;
			nameLabel->position.assign (x, y);

			resetSize ();
			setFixedSize (true, streamImage->width, maxWidgetY + uiconfig->paddingSize);
			break;
		}
		case StreamWindow::HIGH_DETAIL: {
			detailNameLabel->position.assign (x, y);
			detailText->position.assign (streamImage->position.x, streamImage->position.y + streamImage->height - detailText->height);

			resetSize ();
			setFixedSize (true, streamImage->width, maxWidgetY);
			break;
		}
	}

	x = width;
	if (selectPanel->isVisible) {
		timestampLabel->position.assignY (timestampLabel->getLinePosition ((selectPanel->height / 2.0f) - (timestampLabel->maxLineHeight / 2.0f)));
		selectPanel->position.assignY (0.0f);
		selectPanel->flowLeft (&x);
	}
	resetSize ();
}

void StreamWindow::streamImageClicked (void *windowPtr, Widget *widgetPtr) {
	StreamWindow *window;

	window = (StreamWindow *) windowPtr;
	if (window->streamImageClickCallback) {
		window->streamImageClickCallback (window->streamImageClickCallbackData, window);
	}
}

void StreamWindow::selectButtonClicked (void *windowPtr, Widget *widgetPtr) {
	StreamWindow *window;

	window = (StreamWindow *) windowPtr;
	window->setSelected (false);
}

void StreamWindow::viewButtonClicked (void *windowPtr, Widget *widgetPtr) {
	StreamWindow *window;

	window = (StreamWindow *) windowPtr;
	if (window->viewButtonClickCallback) {
		window->viewButtonClickCallback (window->viewButtonClickCallbackData, window);
	}
}
