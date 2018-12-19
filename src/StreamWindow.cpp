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
#include "MonitorUi.h"
#include "StreamWindow.h"

StreamWindow::StreamWindow (Json *streamItem, SpriteGroup *monitorUiSpriteGroup, int layoutType, float maxStreamImageWidth)
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
, spriteGroup (monitorUiSpriteGroup)
, streamImage (NULL)
, nameLabel (NULL)
, detailText (NULL)
, mouseoverLabel (NULL)
, detailNameLabel (NULL)
, selectPanel (NULL)
, streamImageClickCallback (NULL)
, streamImageClickCallbackData (NULL)
{
	SystemInterface *interface;
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	interface = &(App::getInstance ()->systemInterface);
	streamId = interface->getCommandStringParam (streamItem, "id", "");
	agentId = interface->getCommandAgentId (streamItem);
	streamName = interface->getCommandStringParam (streamItem, "name", "");
	frameWidth = interface->getCommandNumberParam (streamItem, "width", (int) 0);
	frameHeight = interface->getCommandNumberParam (streamItem, "height", (int) 0);
	frameRate = interface->getCommandNumberParam (streamItem, "frameRate", (float) 0.0f);
	bitrate = interface->getCommandNumberParam (streamItem, "bitrate", (int64_t) 0);

	streamImage = (ImageWindow *) addWidget (new ImageWindow (new Image (spriteGroup->getSprite (MonitorUi::LOADING_IMAGE_ICON))));
	streamImage->setMouseClickCallback (StreamWindow::streamImageClicked, this);

	nameLabel = (Label *) addWidget (new Label (streamName, UiConfiguration::CAPTION, uiconfig->primaryTextColor));

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

	detailNameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (streamName, UiConfiguration::HEADLINE, uiconfig->inverseTextColor)));
	detailNameLabel->zLevel = 1;
	detailNameLabel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	detailNameLabel->setFillBg (true, 0.0f, 0.0f, 0.0f);
	detailNameLabel->setAlphaBlend (true, uiconfig->scrimBackgroundAlpha);
	detailNameLabel->isVisible = false;

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
			break;
		}
		case StreamWindow::MEDIUM_DETAIL: {
			detailNameLabel->isVisible = false;
			detailText->isVisible = false;
			nameLabel->isVisible = true;
			break;
		}
		case StreamWindow::HIGH_DETAIL: {
			nameLabel->isVisible = false;
			detailNameLabel->isVisible = true;
			detailText->isVisible = true;
			break;
		}
	}

	refreshLayout ();
}

void StreamWindow::setStreamImageClickCallback (Widget::EventCallback callback, void *callbackData) {
	streamImageClickCallback = callback;
	streamImageClickCallbackData = callbackData;
}

void StreamWindow::setThumbnailIndex (int index) {
	App *app;
	Json *params;

	if ((index == thumbnailIndex) || (segmentCount <= 0) || (index < 0) || (index >= segmentCount)) {
		return;
	}

	app = App::getInstance ();
	thumbnailIndex = index;
	if (! thumbnailPath.empty ()) {
		params = new Json ();
		params->set ("id", streamId);
		params->set ("thumbnailIndex", thumbnailIndex);
		streamImage->setLoadUrl (app->agentControl.getAgentSecondaryUrl (agentId, app->createCommand ("GetThumbnailImage", SystemInterface::Constant_Stream, params), thumbnailPath), spriteGroup->getSprite (MonitorUi::LOADING_IMAGE_ICON));
	}
}

void StreamWindow::setSelected (bool selected, float timestamp) {
	UiConfiguration *uiconfig;
	Label *label;
	Button *button;
	float x, y;

	uiconfig = &(App::getInstance ()->uiConfig);
	isSelected = selected;
	if (selectPanel) {
		selectPanel->isDestroyed = true;
		selectPanel = NULL;
	}
	if (! isSelected) {
		selectedTimestamp = 0.0f;
	}
	else {
		selectedTimestamp = timestamp;
		selectPanel = (Panel *) addWidget (new Panel ());
		selectPanel->zLevel = 2;
		selectPanel->setFillBg (true, uiconfig->darkBackgroundColor);

		label = (Label *) selectPanel->addWidget (new Label (Util::getDurationString ((int64_t) selectedTimestamp, Util::HOURS), UiConfiguration::CAPTION, uiconfig->primaryTextColor));
		button = (Button *) selectPanel->addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::STAR_BUTTON)));
		button->setImageColor (uiconfig->flatButtonTextColor);
		button->setMouseClickCallback (StreamWindow::selectButtonClicked, this);

		x = uiconfig->paddingSize;
		y = 0.0f;
		label->flowRight (&x, y);
		button->flowRight (&x, y);

		selectPanel->refresh ();
		label->position.assignY (label->getLinePosition ((selectPanel->height / 2.0f) - (label->maxLineHeight / 2.0f)));
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
	if (! interface->getCommandObjectParam (agentstatus, "streamServerStatus", &serverstatus)) {
		return;
	}

	thumbnailPath = serverstatus.getString ("thumbnailPath", "");
	hlsStreamPath = serverstatus.getString ("hlsStreamPath", "");
	segmentCount = interface->getCommandNumberParam (record, "segmentCount", (int) 0);
	interface->getCommandNumberArrayParam (record, "segmentPositions", &segmentPositions, true);
	duration = interface->getCommandNumberParam (record, "duration", (int64_t) 0);

	detailText->setText (StdString::createSprintf ("%ix%i  %s  %s", frameWidth, frameHeight, Util::getBitrateDisplayString (bitrate).c_str (), Util::getDurationDisplayString (duration).c_str ()));

	if (streamImage->isLoadUrlEmpty () && (segmentCount > 0) && (! thumbnailPath.empty ())) {
		setThumbnailIndex (segmentCount / 2);
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
	if (selectPanel) {
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
