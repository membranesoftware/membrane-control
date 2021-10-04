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
#include <math.h>
#include "App.h"
#include "ClassId.h"
#include "Log.h"
#include "StdString.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "SystemInterface.h"
#include "RecordStore.h"
#include "AgentControl.h"
#include "OsUtil.h"
#include "MediaUtil.h"
#include "Widget.h"
#include "Sprite.h"
#include "Color.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "TextFlow.h"
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
, streamSize (0)
, streamImage (NULL)
, nameLabel (NULL)
, detailText (NULL)
, mouseoverLabel (NULL)
, detailNameLabel (NULL)
, timestampLabel (NULL)
, viewButton (NULL)
, removeButton (NULL)
{
	classId = ClassId::StreamWindow;

	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	streamId = SystemInterface::instance->getCommandStringParam (streamItem, "id", "");
	agentId = SystemInterface::instance->getCommandAgentId (streamItem);
	streamName = SystemInterface::instance->getCommandStringParam (streamItem, "name", "");
	frameWidth = SystemInterface::instance->getCommandNumberParam (streamItem, "width", (int) 0);
	frameHeight = SystemInterface::instance->getCommandNumberParam (streamItem, "height", (int) 0);
	frameRate = SystemInterface::instance->getCommandNumberParam (streamItem, "frameRate", (float) 0.0f);
	bitrate = SystemInterface::instance->getCommandNumberParam (streamItem, "bitrate", (int64_t) 0);
	streamSize = SystemInterface::instance->getCommandNumberParam (streamItem, "size", (int64_t) 0);

	streamImage = (ImageWindow *) addWidget (new ImageWindow (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite))));
	streamImage->loadCallback = Widget::EventCallbackContext (StreamWindow::streamImageLoaded, this);
	streamImage->mouseClickCallback = Widget::EventCallbackContext (StreamWindow::streamImageClicked, this);
	streamImage->mouseLongPressCallback = Widget::EventCallbackContext (StreamWindow::streamImageLongPressed, this);
	streamImage->setLoadSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite));

	nameLabel = (Label *) addWidget (new Label (streamName, UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor));
	nameLabel->isInputSuspended = true;

	detailText = (TextFlow *) addWidget (new TextFlow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, UiConfiguration::CaptionFont));
	detailText->setTextColor (UiConfiguration::instance->inverseTextColor);
	detailText->zLevel = 1;
	detailText->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	detailText->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	detailText->isInputSuspended = true;
	detailText->isVisible = false;

	mouseoverLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->inverseTextColor)));
	mouseoverLabel->zLevel = 3;
	mouseoverLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	mouseoverLabel->isInputSuspended = true;
	mouseoverLabel->isVisible = false;

	detailNameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (streamName, UiConfiguration::HeadlineFont, UiConfiguration::instance->inverseTextColor)));
	detailNameLabel->zLevel = 1;
	detailNameLabel->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	detailNameLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	detailNameLabel->isInputSuspended = true;
	detailNameLabel->isVisible = false;

	timestampLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor)));
	timestampLabel->zLevel = 2;
	timestampLabel->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	timestampLabel->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize / 2.0f);
	timestampLabel->isInputSuspended = true;
	timestampLabel->isVisible = false;

	viewButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ImageButtonSprite)));
	viewButton->mouseClickCallback = Widget::EventCallbackContext (StreamWindow::viewButtonClicked, this);
	viewButton->zLevel = 4;
	viewButton->isTextureTargetDrawEnabled = false;
	viewButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	viewButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);
	viewButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ViewTimelineImagesTooltip));

	removeButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite)));
	removeButton->mouseClickCallback = Widget::EventCallbackContext (StreamWindow::removeButtonClicked, this);
	removeButton->zLevel = 4;
	removeButton->isTextureTargetDrawEnabled = false;
	removeButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	removeButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);
	removeButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::RemoveStream).capitalized ());
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
		streamImage->setImageUrl (AgentControl::instance->getAgentSecondaryUrl (agentId, App::instance->createCommand (SystemInterface::Command_GetThumbnailImage, params), thumbnailPath));
	}
}

void StreamWindow::setSelected (bool selected) {
	if (selected == isSelected) {
		return;
	}
	isSelected = selected;
	if (isSelected) {
		setBorder (true, UiConfiguration::instance->mediumSecondaryColor.copy (UiConfiguration::instance->selectionBorderAlpha), UiConfiguration::instance->selectionBorderWidth);
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

bool StreamWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel::doProcessMouseState (mouseState);
	if (layout == CardView::LowDetail) {
		if (mouseState.isEntered) {
			mouseoverLabel->isVisible = true;
		}
		else {
			mouseoverLabel->isVisible = false;
		}
	}

	return (false);
}

void StreamWindow::syncRecordStore () {
	Json *record, *agentstatus, serverstatus;

	record = RecordStore::instance->findRecord (streamId, SystemInterface::CommandId_StreamItem);
	if (! record) {
		return;
	}

	agentstatus = RecordStore::instance->findRecord (RecordStore::matchAgentStatusSource, &agentId);
	if (! agentstatus) {
		return;
	}
	if (SystemInterface::instance->getCommandObjectParam (agentstatus, "streamServerStatus", &serverstatus)) {
		thumbnailPath = serverstatus.getString ("thumbnailPath", "");
		hlsStreamPath = serverstatus.getString ("hlsStreamPath", "");
	}
	else if (SystemInterface::instance->getCommandObjectParam (agentstatus, "monitorServerStatus", &serverstatus)) {
		thumbnailPath = serverstatus.getString ("thumbnailPath", "");
		hlsStreamPath.assign ("");
	}

	segmentCount = SystemInterface::instance->getCommandNumberParam (record, "segmentCount", (int) 0);
	SystemInterface::instance->getCommandNumberArrayParam (record, "segmentPositions", &segmentPositions, true);
	duration = SystemInterface::instance->getCommandNumberParam (record, "duration", (int64_t) 0);

	detailText->setText (StdString::createSprintf ("%ix%i  %s  %s", frameWidth, frameHeight, MediaUtil::getBitrateDisplayString (bitrate).c_str (), OsUtil::getDurationDisplayString (duration).c_str ()));

	if (streamImage->isImageUrlEmpty () && (segmentCount > 0) && (! thumbnailPath.empty ())) {
		setThumbnailIndex (segmentCount / 8);
	}

	refreshLayout ();
}

void StreamWindow::refreshLayout () {
	float x, y;

	x = 0.0f;
	y = 0.0f;
	streamImage->position.assign (x, y);

	switch (layout) {
		case CardView::LowDetail: {
			mouseoverLabel->position.assign (0.0f, streamImage->height - mouseoverLabel->height);
			setFixedSize (true, streamImage->width, streamImage->height);
			viewButton->position.assign (streamImage->position.x + streamImage->width - viewButton->width - UiConfiguration::instance->dropShadowWidth, streamImage->position.y + streamImage->height - viewButton->height);
			removeButton->position.assign (streamImage->position.x, streamImage->position.y + streamImage->height - removeButton->height);
			break;
		}
		case CardView::MediumDetail: {
			x += UiConfiguration::instance->paddingSize;
			y += streamImage->height + UiConfiguration::instance->marginSize;
			nameLabel->position.assign (x, y);

			viewButton->position.assign (streamImage->position.x + streamImage->width - viewButton->width - UiConfiguration::instance->dropShadowWidth, streamImage->position.y + streamImage->height - viewButton->height);
			removeButton->position.assign (streamImage->position.x, streamImage->position.y + streamImage->height - removeButton->height);

			resetSize ();
			setFixedSize (true, streamImage->width, maxWidgetY + UiConfiguration::instance->paddingSize);
			break;
		}
		case CardView::HighDetail: {
			detailNameLabel->position.assign (x, y);
			detailText->position.assign (streamImage->position.x, streamImage->position.y + streamImage->height - detailText->height);

			viewButton->position.assign (streamImage->position.x + streamImage->width - viewButton->width - UiConfiguration::instance->dropShadowWidth, streamImage->position.y + streamImage->height - viewButton->height);
			removeButton->position.assign (streamImage->position.x, streamImage->position.y + streamImage->height - removeButton->height);

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
	((StreamWindow *) windowPtr)->eventCallback (((StreamWindow *) windowPtr)->streamImageClickCallback);
}

void StreamWindow::streamImageLongPressed (void *windowPtr, Widget *widgetPtr) {
	ImageWindow *image;

	image = (ImageWindow *) widgetPtr;
	App::instance->uiStack.showImageDialog (image->imageUrl);
}

void StreamWindow::streamImageLoaded (void *windowPtr, Widget *widgetPtr) {
	StreamWindow *window;
	ImageWindow *image;

	window = (StreamWindow *) windowPtr;
	image = (ImageWindow *) widgetPtr;
	if (! image->isLoaded ()) {
		return;
	}
	window->shouldRefreshTexture = true;
}

void StreamWindow::viewButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((StreamWindow *) windowPtr)->eventCallback (((StreamWindow *) windowPtr)->viewButtonClickCallback);
}

void StreamWindow::removeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((StreamWindow *) windowPtr)->eventCallback (((StreamWindow *) windowPtr)->removeButtonClickCallback);
}
