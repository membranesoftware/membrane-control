/*
* Copyright 2018-2022 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "StdString.h"
#include "UiConfiguration.h"
#include "SystemInterface.h"
#include "AgentControl.h"
#include "RecordStore.h"
#include "Agent.h"
#include "OsUtil.h"
#include "MediaUtil.h"
#include "Json.h"
#include "Widget.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "Button.h"
#include "TextFlow.h"
#include "CardView.h"
#include "MediaUi.h"
#include "MediaWindow.h"

MediaWindow::MediaWindow (Json *mediaItem, SpriteGroup *mediaUiSpriteGroup)
: Panel ()
, thumbnailCount (0)
, mediaWidth (0)
, mediaHeight (0)
, mediaDuration (0.0f)
, mediaFrameRate (0.0f)
, mediaSize (0)
, mediaBitrate (0)
, isCreateStreamAvailable (true)
, streamSize (0)
, playThumbnailIndex (0)
, displayTimestamp (-1.0f)
, isSelected (false)
, sprites (mediaUiSpriteGroup)
, mediaImage (NULL)
, nameLabel (NULL)
, detailText (NULL)
, mouseoverLabel (NULL)
, detailNameLabel (NULL)
, viewButton (NULL)
, browserPlayButton (NULL)
, timestampLabel (NULL)
, streamIconImage (NULL)
, createStreamUnavailableIconImage (NULL)
{
	classId = ClassId::MediaWindow;

	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);
	mediaId = SystemInterface::instance->getCommandStringParam (mediaItem, "id", "");
	mediaName = SystemInterface::instance->getCommandStringParam (mediaItem, "name", "");
	mediaSortKey = SystemInterface::instance->getCommandStringParam (mediaItem, "sortKey", "");
	agentId = SystemInterface::instance->getCommandAgentId (mediaItem);
	mediaWidth = SystemInterface::instance->getCommandNumberParam (mediaItem, "width", (int) 0);
	mediaHeight = SystemInterface::instance->getCommandNumberParam (mediaItem, "height", (int) 0);

	mediaImage = (ImageWindow *) addWidget (new ImageWindow (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite))));
	mediaImage->loadCallback = Widget::EventCallbackContext (MediaWindow::mediaImageLoaded, this);
	mediaImage->mouseClickCallback = Widget::EventCallbackContext (MediaWindow::mediaImageClicked, this);
	mediaImage->mouseLongPressCallback = Widget::EventCallbackContext (MediaWindow::mediaImageLongPressed, this);
	mediaImage->setLoadingSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite));

	nameLabel = (Label *) addWidget (new Label (mediaName, UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor));
	nameLabel->isInputSuspended = true;

	detailText = (TextFlow *) addWidget (new TextFlow (UiConfiguration::instance->textFieldMediumLineLength * UiConfiguration::instance->fonts[UiConfiguration::CaptionFont]->maxGlyphWidth, UiConfiguration::CaptionFont));
	detailText->setTextColor (UiConfiguration::instance->inverseTextColor);
	detailText->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize / 2.0f);
	detailText->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	detailText->zLevel = 2;
	detailText->isInputSuspended = true;
	detailText->isVisible = false;

	mouseoverLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->inverseTextColor)));
	mouseoverLabel->zLevel = 1;
	mouseoverLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	mouseoverLabel->isTextureTargetDrawEnabled = false;
	mouseoverLabel->isInputSuspended = true;
	mouseoverLabel->isVisible = false;

	detailNameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (mediaName, UiConfiguration::HeadlineFont, UiConfiguration::instance->inverseTextColor)));
	detailNameLabel->zLevel = 1;
	detailNameLabel->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	detailNameLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	detailNameLabel->isInputSuspended = true;
	detailNameLabel->isVisible = false;

	viewButton = (Button *) addWidget (new Button (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ImageButtonSprite)));
	viewButton->mouseClickCallback = Widget::EventCallbackContext (MediaWindow::viewButtonClicked, this);
	viewButton->zLevel = 4;
	viewButton->isTextureTargetDrawEnabled = false;
	viewButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	viewButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);
	viewButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::ViewTimelineImagesTooltip));

	browserPlayButton = (Button *) addWidget (new Button (sprites->getSprite (MediaUi::BrowserPlayButtonSprite)));
	browserPlayButton->mouseClickCallback = Widget::EventCallbackContext (MediaWindow::browserPlayButtonClicked, this);
	browserPlayButton->zLevel = 4;
	browserPlayButton->isTextureTargetDrawEnabled = false;
	browserPlayButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	browserPlayButton->setRaised (true, UiConfiguration::instance->raisedButtonBackgroundColor);
	browserPlayButton->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MediaUiBrowserPlayTooltip));
	browserPlayButton->isVisible = false;

	timestampLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->primaryTextColor)));
	timestampLabel->zLevel = 2;
	timestampLabel->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	timestampLabel->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize / 2.0f);
	timestampLabel->isInputSuspended = true;
	timestampLabel->isVisible = false;

	streamIconImage = (ImageWindow *) addWidget (new ImageWindow (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite))));
	streamIconImage->zLevel = 3;
	streamIconImage->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	streamIconImage->setPadding (UiConfiguration::instance->paddingSize / 2.0f, 0.0f);
	streamIconImage->setMouseHoverTooltip (UiText::instance->getText (UiTextString::StreamIconTooltip));
	streamIconImage->isVisible = false;

	createStreamUnavailableIconImage = (ImageWindow *) addWidget (new ImageWindow (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallErrorIconSprite))));
	createStreamUnavailableIconImage->zLevel = 3;
	createStreamUnavailableIconImage->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	createStreamUnavailableIconImage->setPadding (UiConfiguration::instance->paddingSize / 2.0f, 0.0f);
	createStreamUnavailableIconImage->setMouseHoverTooltip (UiText::instance->getText (UiTextString::CreateStreamUnavailableTooltip));
	createStreamUnavailableIconImage->isVisible = false;
}

MediaWindow::~MediaWindow () {

}

StdString MediaWindow::toStringDetail () {
	return (StdString::createSprintf (" MediaWindow mediaName=\"%s\" size=%.2fx%.2f", mediaName.c_str (), width, height));
}

bool MediaWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::MediaWindow));
}

MediaWindow *MediaWindow::castWidget (Widget *widget) {
	return (MediaWindow::isWidgetType (widget) ? (MediaWindow *) widget : NULL);
}

void MediaWindow::setDisplayTimestamp (float timestamp) {
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

void MediaWindow::setSelected (bool selected, bool shouldSkipStateChangeCallback) {
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
	if (! shouldSkipStateChangeCallback) {
		eventCallback (selectStateChangeCallback);
	}
	shouldRefreshTexture = true;
}

bool MediaWindow::hasThumbnails () {
	return ((! agentId.empty ()) && (! thumbnailPath.empty ()) && (thumbnailCount > 0) && (mediaWidth > 0) && (mediaHeight > 0));
}

void MediaWindow::setThumbnail (const StdString &imageUrl, int thumbnailIndex) {
	if (! imageUrl.empty ()) {
		playThumbnailUrl.assign (imageUrl);
		mediaImage->setImageUrl (playThumbnailUrl);
	}
	if (thumbnailIndex >= 0) {
		playThumbnailIndex = thumbnailIndex;
	}
}

void MediaWindow::syncRecordStore () {
	Json *mediaitem, *agentstatus, serverstatus, *streamitem, *params;
	StdString agentid, recordid, agentname, hlspath, htmlpath;

	mediaitem = RecordStore::instance->findRecord (mediaId, SystemInterface::CommandId_MediaItem);
	if (! mediaitem) {
		return;
	}

	agentid = SystemInterface::instance->getCommandAgentId (mediaitem);
	agentstatus = RecordStore::instance->findRecord (RecordStore::matchAgentStatusSource, &agentid);
	if (! agentstatus) {
		return;
	}
	if (! SystemInterface::instance->getCommandObjectParam (agentstatus, "mediaServerStatus", &serverstatus)) {
		return;
	}

	mediaPath = serverstatus.getString ("mediaPath", "");
	thumbnailPath = serverstatus.getString ("thumbnailPath", "");
	thumbnailCount = serverstatus.getNumber ("thumbnailCount", (int) 0);
	if (mediaPath.empty ()) {
		return;
	}

	mediaDuration = SystemInterface::instance->getCommandNumberParam (mediaitem, "duration", (float) 0.0f);
	mediaFrameRate = SystemInterface::instance->getCommandNumberParam (mediaitem, "frameRate", (float) 0.0f);
	mediaSize = SystemInterface::instance->getCommandNumberParam (mediaitem, "size", (int64_t) 0);
	mediaBitrate = SystemInterface::instance->getCommandNumberParam (mediaitem, "bitrate", (int64_t) 0);

	streamitem = RecordStore::instance->findRecord (MediaWindow::matchStreamSourceId, &mediaId);
	if (! streamitem) {
		streamAgentId.assign ("");
		streamId.assign ("");
		streamSize = 0;
		streamAgentName.assign ("");
		streamThumbnailPath.assign ("");
		hlsStreamPath.assign ("");
		htmlPlayerPath.assign ("");
		streamIconImage->isVisible = false;
		browserPlayButton->isVisible = false;
	}
	else {
		recordid = SystemInterface::instance->getCommandRecordId (streamitem);
		if (! recordid.empty ()) {
			agentid = SystemInterface::instance->getCommandAgentId (streamitem);
		}
		if (! agentid.empty ()) {
			agentstatus = RecordStore::instance->findRecord (RecordStore::matchAgentStatusSource, &agentid);
			if (agentstatus) {
				agentname = Agent::getCommandAgentName (agentstatus);
				if (SystemInterface::instance->getCommandObjectParam (agentstatus, "streamServerStatus", &serverstatus)) {
					hlspath = serverstatus.getString ("hlsStreamPath", "");
					htmlpath = serverstatus.getString ("htmlPlayerPath", "");
					streamThumbnailPath = serverstatus.getString ("thumbnailPath", "");
				}
			}
		}

		if (recordid.empty () || agentid.empty () || agentname.empty () || hlspath.empty () || htmlpath.empty ()) {
			streamIconImage->isVisible = false;
			browserPlayButton->isVisible = false;
		}
		else {
			streamId.assign (recordid);
			streamAgentId.assign (agentid);
			streamAgentName.assign (agentname);
			hlsStreamPath.assign (hlspath);
			htmlPlayerPath.assign (htmlpath);
			streamSize = SystemInterface::instance->getCommandNumberParam (streamitem, "size", (int64_t) 0);
			streamIconImage->isVisible = true;
			browserPlayButton->isVisible = true;
		}
	}

	isCreateStreamAvailable = SystemInterface::instance->getCommandBooleanParam (mediaitem, "isCreateStreamAvailable", true);
	if (isCreateStreamAvailable) {
		createStreamUnavailableIconImage->isVisible = false;
	}
	else {
		createStreamUnavailableIconImage->isVisible = true;
	}

	if (hasThumbnails () && mediaImage->isImageUrlEmpty ()) {
		if (streamitem) {
			playThumbnailIndex = SystemInterface::instance->getCommandNumberParam (streamitem, "segmentCount", (int) 0) / 4;
		}
		params = new Json ();
		params->set ("id", mediaId);
		params->set ("thumbnailIndex", (thumbnailCount / 4));
		playThumbnailUrl = AgentControl::instance->getAgentSecondaryUrl (agentId, thumbnailPath, App::instance->createCommand (SystemInterface::Command_GetThumbnailImage, params));
		mediaImage->setImageUrl (playThumbnailUrl);
	}

	shouldRefreshTexture = true;
	refreshLayout ();
}

bool MediaWindow::matchStreamSourceId (void *idStringPtr, Json *record) {
	StdString *id;

	if (SystemInterface::instance->getCommandId (record) != SystemInterface::CommandId_StreamItem) {
		return (false);
	}
	id = (StdString *) idStringPtr;
	return (id->equals (SystemInterface::instance->getCommandStringParam (record, "sourceId", "")));
}

void MediaWindow::refreshLayout () {
	float x, y;

	mouseoverLabel->isVisible = false;
	x = 0.0f;
	y = 0.0f;
	mediaImage->position.assign (x, y);

	switch (layout) {
		case CardView::LowDetail: {
			nameLabel->isVisible = false;
			detailNameLabel->isVisible = false;
			detailText->isVisible = false;

			if (createStreamUnavailableIconImage->isVisible) {
				createStreamUnavailableIconImage->position.assign (x, mediaImage->position.y + mediaImage->height - createStreamUnavailableIconImage->height);
				x += createStreamUnavailableIconImage->width;
			}
			if (streamIconImage->isVisible) {
				streamIconImage->position.assign (x, mediaImage->position.y + mediaImage->height - streamIconImage->height);
				x += streamIconImage->width;
			}
			mouseoverLabel->position.assign (0.0f, 0.0f);

			setFixedSize (true, mediaImage->width, mediaImage->height);
			viewButton->position.assign (mediaImage->position.x + mediaImage->width - viewButton->width - UiConfiguration::instance->dropShadowWidth, mediaImage->position.y + mediaImage->height - viewButton->height);
			browserPlayButton->position.assign (mediaImage->position.x, mediaImage->position.y + mediaImage->height - browserPlayButton->height);
			break;
		}
		case CardView::MediumDetail: {
			detailNameLabel->isVisible = false;
			detailText->isVisible = false;

			x += UiConfiguration::instance->paddingSize;
			y += mediaImage->height + UiConfiguration::instance->marginSize;
			nameLabel->position.assign (x, y);
			nameLabel->isVisible = true;

			x = 0.0f;
			if (createStreamUnavailableIconImage->isVisible) {
				createStreamUnavailableIconImage->position.assign (x, mediaImage->position.y + mediaImage->height - createStreamUnavailableIconImage->height);
				x += createStreamUnavailableIconImage->width;
			}
			if (streamIconImage->isVisible) {
				streamIconImage->position.assign (x, mediaImage->position.y + mediaImage->height - streamIconImage->height);
				x += streamIconImage->width;
			}

			viewButton->position.assign (mediaImage->position.x + mediaImage->width - viewButton->width - UiConfiguration::instance->dropShadowWidth, mediaImage->position.y + mediaImage->height - viewButton->height);
			browserPlayButton->position.assign (mediaImage->position.x, mediaImage->position.y + mediaImage->height - browserPlayButton->height);

			resetSize ();
			setFixedSize (true, mediaImage->width, maxWidgetY + UiConfiguration::instance->paddingSize);
			break;
		}
		case CardView::HighDetail: {
			nameLabel->isVisible = false;

			detailNameLabel->position.assign (mediaImage->position.x, mediaImage->position.y);
			detailNameLabel->isVisible = true;

			if (createStreamUnavailableIconImage->isVisible) {
				createStreamUnavailableIconImage->position.assign (x, mediaImage->position.y + mediaImage->height - createStreamUnavailableIconImage->height);
				x += createStreamUnavailableIconImage->width;
			}
			if (streamIconImage->isVisible) {
				streamIconImage->position.assign (x, mediaImage->position.y + mediaImage->height - streamIconImage->height);
				x += streamIconImage->width;
			}

			detailText->setText (StdString::createSprintf ("%ix%i  %s  %s  %s", mediaWidth, mediaHeight, MediaUtil::getBitrateDisplayString (mediaBitrate).c_str (), OsUtil::getByteCountDisplayString (mediaSize).c_str (), OsUtil::getDurationDisplayString (mediaDuration).c_str ()));
			detailText->position.assign (x, mediaImage->position.y + mediaImage->height - detailText->height);
			detailText->isVisible = true;
			setFixedSize (true, mediaImage->width, mediaImage->height);
			viewButton->position.assign (mediaImage->position.x + mediaImage->width - viewButton->width - UiConfiguration::instance->dropShadowWidth, mediaImage->position.y + mediaImage->height - viewButton->height);
			browserPlayButton->position.assign (mediaImage->position.x, mediaImage->position.y + mediaImage->height - browserPlayButton->height);
			break;
		}
	}

	if (timestampLabel->isVisible) {
		timestampLabel->position.assign (mediaImage->position.x + mediaImage->width - timestampLabel->width, mediaImage->position.y);
	}
}

bool MediaWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
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

void MediaWindow::setLayout (int layoutType, float maxPanelWidth) {
	float w, h;

	if ((layoutType == layout) || (mediaWidth <= 0) || (maxPanelWidth < 1.0f)) {
		return;
	}
	layout = layoutType;
	w = maxPanelWidth;
	h = mediaHeight;
	h *= maxPanelWidth;
	h /= mediaWidth;
	w = floorf (w);
	h = floorf (h);
	mediaImage->setLoadingSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite), w, h);
	mediaImage->setWindowSize (true, w, h);
	mediaImage->onLoadScale (w, h);
	mediaImage->reload ();

	if (layout == CardView::HighDetail) {
		nameLabel->setFont (UiConfiguration::BodyFont);
	}
	else {
		nameLabel->setFont (UiConfiguration::CaptionFont);
	}

	if (layout == CardView::LowDetail) {
		mouseoverLabel->setText (mediaName);
	}
	refreshLayout ();
}

void MediaWindow::mediaImageClicked (void *windowPtr, Widget *widgetPtr) {
	((MediaWindow *) windowPtr)->eventCallback (((MediaWindow *) windowPtr)->mediaImageClickCallback);
}

void MediaWindow::mediaImageLongPressed (void *windowPtr, Widget *widgetPtr) {
	ImageWindow *image;

	image = (ImageWindow *) widgetPtr;
	App::instance->uiStack.showImageDialog (image->imageUrl);
}

void MediaWindow::mediaImageLoaded (void *windowPtr, Widget *widgetPtr) {
	MediaWindow *window;
	ImageWindow *image;

	window = (MediaWindow *) windowPtr;
	image = (ImageWindow *) widgetPtr;
	if (! image->isLoaded ()) {
		return;
	}
	window->shouldRefreshTexture = true;
}

void MediaWindow::viewButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((MediaWindow *) windowPtr)->eventCallback (((MediaWindow *) windowPtr)->viewButtonClickCallback);
}

void MediaWindow::browserPlayButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((MediaWindow *) windowPtr)->eventCallback (((MediaWindow *) windowPtr)->browserPlayButtonClickCallback);
}
