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
#include "ClassId.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "SystemInterface.h"
#include "OsUtil.h"
#include "MediaUtil.h"
#include "Widget.h"
#include "Sprite.h"
#include "Color.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "TextArea.h"
#include "Image.h"
#include "ImageWindow.h"
#include "CardView.h"
#include "StreamWindow.h"

StreamWindow::StreamWindow (Json *streamItem)
: Panel ()
, isSelected (false)
, displayTimestamp (-1.0f)
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
, timestampLabel (NULL)
, viewButton (NULL)
, streamImageClickCallback (NULL)
, streamImageClickCallbackData (NULL)
, viewButtonClickCallback (NULL)
, viewButtonClickCallbackData (NULL)
{
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	UiText *uitext;

	classId = ClassId::StreamWindow;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	interface = &(App::instance->systemInterface);
	streamId = interface->getCommandStringParam (streamItem, "id", "");
	agentId = interface->getCommandAgentId (streamItem);
	streamName = interface->getCommandStringParam (streamItem, "name", "");
	frameWidth = interface->getCommandNumberParam (streamItem, "width", (int) 0);
	frameHeight = interface->getCommandNumberParam (streamItem, "height", (int) 0);
	frameRate = interface->getCommandNumberParam (streamItem, "frameRate", (float) 0.0f);
	bitrate = interface->getCommandNumberParam (streamItem, "bitrate", (int64_t) 0);

	streamImage = (ImageWindow *) addWidget (new ImageWindow (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite))));
	streamImage->setLoadSprite (uiconfig->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite));
	streamImage->setMouseClickCallback (StreamWindow::streamImageClicked, this);
	streamImage->setLoadCallback (StreamWindow::streamImageLoaded, this);

	nameLabel = (Label *) addWidget (new Label (streamName, UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
	nameLabel->isInputSuspended = true;

	detailText = (TextArea *) addWidget (new TextArea (UiConfiguration::CaptionFont, uiconfig->inverseTextColor));
	detailText->zLevel = 1;
	detailText->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	detailText->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	detailText->isInputSuspended = true;
	detailText->isVisible = false;

	mouseoverLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->inverseTextColor)));
	mouseoverLabel->zLevel = 3;
	mouseoverLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	mouseoverLabel->isInputSuspended = true;
	mouseoverLabel->isVisible = false;

	detailNameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (streamName, UiConfiguration::HeadlineFont, uiconfig->inverseTextColor)));
	detailNameLabel->zLevel = 1;
	detailNameLabel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	detailNameLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	detailNameLabel->isInputSuspended = true;
	detailNameLabel->isVisible = false;

	timestampLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->primaryTextColor)));
	timestampLabel->zLevel = 2;
	timestampLabel->setFillBg (true, uiconfig->darkBackgroundColor);
	timestampLabel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize / 2.0f);
	timestampLabel->isInputSuspended = true;
	timestampLabel->isVisible = false;

	viewButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::ImageButtonSprite)));
	viewButton->zLevel = 3;
	viewButton->isTextureTargetDrawEnabled = false;
	viewButton->setImageColor (uiconfig->flatButtonTextColor);
	viewButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	viewButton->setMouseHoverTooltip (uitext->getText (UiTextString::viewTimelineImagesTooltip));
	viewButton->setMouseClickCallback (StreamWindow::viewButtonClicked, this);
}

StreamWindow::~StreamWindow () {

}

StdString StreamWindow::toStringDetail () {
	return (StdString::createSprintf (" StreamWindow streamName=\"%s\"", streamName.c_str ()));
}

bool StreamWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::StreamWindow));
}

StreamWindow *StreamWindow::castWidget (Widget *widget) {
	return (StreamWindow::isWidgetType (widget) ? (StreamWindow *) widget : NULL);
}

void StreamWindow::setLayout (int layoutType, float maxPanelWidth) {
	float w, h;

	if (layoutType == layout) {
		return;
	}
	layout = layoutType;
	w = maxPanelWidth;
	h = frameHeight;
	h *= maxPanelWidth;
	h /= frameWidth;
	w = floorf (w);
	h = floorf (h);
	streamImage->setWindowSize (w, h);
	streamImage->reload ();

	switch (layout) {
		case CardView::LowDetail: {
			mouseoverLabel->setText (streamName);
			nameLabel->isVisible = false;
			detailNameLabel->isVisible = false;
			detailText->isVisible = false;
			timestampLabel->isVisible = false;
			break;
		}
		case CardView::MediumDetail: {
			detailNameLabel->isVisible = false;
			detailText->isVisible = false;
			nameLabel->isVisible = true;
			timestampLabel->isVisible = (displayTimestamp > 0.0f);
			break;
		}
		case CardView::HighDetail: {
			nameLabel->isVisible = false;
			detailNameLabel->isVisible = true;
			detailText->isVisible = true;
			timestampLabel->isVisible = (displayTimestamp > 0.0f);
			break;
		}
	}

	refreshLayout ();
}

void StreamWindow::setStreamImageClickCallback (Widget::EventCallback callback, void *callbackData) {
	streamImageClickCallback = callback;
	streamImageClickCallbackData = callbackData;
}

void StreamWindow::setViewButtonClickCallback (Widget::EventCallback callback, void *callbackData) {
	viewButtonClickCallback = callback;
	viewButtonClickCallbackData = callbackData;
}

void StreamWindow::setThumbnailIndex (int index) {
	Json *params;

	if ((index == thumbnailIndex) || (segmentCount <= 0) || (index < 0) || (index >= segmentCount)) {
		return;
	}

	thumbnailIndex = index;
	if (! thumbnailPath.empty ()) {
		params = new Json ();
		params->set ("id", streamId);
		params->set ("thumbnailIndex", thumbnailIndex);
		streamImage->setLoadUrl (App::instance->agentControl.getAgentSecondaryUrl (agentId, App::instance->createCommand (SystemInterface::Command_GetThumbnailImage, SystemInterface::Constant_Stream, params), thumbnailPath));
	}
}

void StreamWindow::setSelected (bool selected) {
	UiConfiguration *uiconfig;

	if (selected == isSelected) {
		return;
	}
	uiconfig = &(App::instance->uiConfig);
	isSelected = selected;
	if (isSelected) {
		setBorder (true, uiconfig->mediumSecondaryColor.copy (uiconfig->selectionBorderAlpha), uiconfig->selectionBorderWidth);
	}
	else {
		setBorder (false);
	}
	refreshLayout ();
	shouldRefreshTexture = true;
}

void StreamWindow::setDisplayTimestamp (float timestamp) {
	if (FLOAT_EQUALS (timestamp, displayTimestamp)) {
		return;
	}
	displayTimestamp = timestamp;
	if (displayTimestamp < 0.0f) {
		timestampLabel->isVisible = false;
	}
	else {
		timestampLabel->setText (OsUtil::getDurationString ((int64_t) displayTimestamp, OsUtil::HoursUnit));
		timestampLabel->isVisible = true;
	}

	refreshLayout ();
}

void StreamWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel::doProcessMouseState (mouseState);
	if (layout == CardView::LowDetail) {
		if (mouseState.isEntered) {
			mouseoverLabel->isVisible = true;
		}
		else {
			mouseoverLabel->isVisible = false;
		}
	}
}

void StreamWindow::syncRecordStore () {
	RecordStore *store;
	SystemInterface *interface;
	Json *record, *agentstatus, serverstatus;

	store = &(App::instance->agentControl.recordStore);
	interface = &(App::instance->systemInterface);
	record = store->findRecord (streamId, SystemInterface::CommandId_StreamItem);
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

	detailText->setText (StdString::createSprintf ("%ix%i  %s  %s", frameWidth, frameHeight, MediaUtil::getBitrateDisplayString (bitrate).c_str (), OsUtil::getDurationDisplayString (duration).c_str ()));

	if (streamImage->isLoadUrlEmpty () && (segmentCount > 0) && (! thumbnailPath.empty ())) {
		setThumbnailIndex (segmentCount / 8);
	}

	refreshLayout ();
}

void StreamWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y;

	uiconfig = &(App::instance->uiConfig);
	x = 0.0f;
	y = 0.0f;
	streamImage->position.assign (x, y);

	switch (layout) {
		case CardView::LowDetail: {
			mouseoverLabel->position.assign (0.0f, streamImage->height - mouseoverLabel->height);
			setFixedSize (true, streamImage->width, streamImage->height);
			viewButton->position.assign (streamImage->position.x + streamImage->width - viewButton->width - uiconfig->dropShadowWidth, streamImage->position.y, streamImage->height - viewButton->height);
			break;
		}
		case CardView::MediumDetail: {
			x += uiconfig->paddingSize;
			y += streamImage->height + uiconfig->marginSize;
			nameLabel->position.assign (x, y);

			viewButton->position.assign (streamImage->position.x + streamImage->width - viewButton->width - uiconfig->dropShadowWidth, streamImage->position.y, streamImage->height - viewButton->height);

			resetSize ();
			setFixedSize (true, streamImage->width, maxWidgetY + uiconfig->paddingSize);
			break;
		}
		case CardView::HighDetail: {
			detailNameLabel->position.assign (x, y);
			detailText->position.assign (streamImage->position.x, streamImage->position.y + streamImage->height - detailText->height);

			viewButton->position.assign (streamImage->position.x + streamImage->width - viewButton->width - uiconfig->dropShadowWidth, streamImage->position.y, streamImage->height - viewButton->height);

			resetSize ();
			setFixedSize (true, streamImage->width, maxWidgetY);
			break;
		}
	}

	if (timestampLabel->isVisible) {
		timestampLabel->position.assign (streamImage->position.x + streamImage->width - timestampLabel->width, streamImage->position.y);
	}
}

void StreamWindow::streamImageClicked (void *windowPtr, Widget *widgetPtr) {
	StreamWindow *window;

	window = (StreamWindow *) windowPtr;
	if (window->streamImageClickCallback) {
		window->streamImageClickCallback (window->streamImageClickCallbackData, window);
	}
}

void StreamWindow::streamImageLoaded (void *windowPtr, Widget *widgetPtr) {
	StreamWindow *window;

	window = (StreamWindow *) windowPtr;
	window->shouldRefreshTexture = true;
}

void StreamWindow::viewButtonClicked (void *windowPtr, Widget *widgetPtr) {
	StreamWindow *window;

	window = (StreamWindow *) windowPtr;
	if (window->viewButtonClickCallback) {
		window->viewButtonClickCallback (window->viewButtonClickCallbackData, window);
	}
}
