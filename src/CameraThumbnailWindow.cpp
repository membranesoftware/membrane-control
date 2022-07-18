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
#include "AgentControl.h"
#include "OsUtil.h"
#include "Json.h"
#include "Widget.h"
#include "Sprite.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "CardView.h"
#include "CameraThumbnailWindow.h"

CameraThumbnailWindow::CameraThumbnailWindow (const StdString &agentId, int sensor, const StdString &captureImagePath, int64_t thumbnailTimestamp)
: Panel ()
, agentId (agentId)
, sensor (sensor)
, captureImagePath (captureImagePath)
, thumbnailTimestamp (thumbnailTimestamp)
, isHighlighted (false)
, windowWidth (0.0f)
, thumbnailImage (NULL)
, timestampLabel (NULL)
{
	classId = ClassId::CameraThumbnailWindow;

	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	thumbnailImage = (ImageWindow *) addWidget (new ImageWindow (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite))));
	thumbnailImage->loadCallback = Widget::EventCallbackContext (CameraThumbnailWindow::thumbnailImageLoaded, this);
	thumbnailImage->mouseLongPressCallback = Widget::EventCallbackContext (CameraThumbnailWindow::thumbnailImageLongPressed, this);
	thumbnailImage->setLoadingSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite));

	timestampLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->inverseTextColor)));
	timestampLabel->zLevel = 1;
	timestampLabel->setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
	timestampLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	timestampLabel->isVisible = false;
	if (thumbnailTimestamp > 0) {
		timestampLabel->setText (OsUtil::getTimestampDisplayString (thumbnailTimestamp));
	}
}

CameraThumbnailWindow::~CameraThumbnailWindow () {

}

StdString CameraThumbnailWindow::toStringDetail () {
	return (StdString::createSprintf (" CameraThumbnailWindow agentId=\"%s\" timestamp=%lli", agentId.c_str (), (long long int) thumbnailTimestamp));
}

bool CameraThumbnailWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::CameraThumbnailWindow));
}

CameraThumbnailWindow *CameraThumbnailWindow::castWidget (Widget *widget) {
	return (CameraThumbnailWindow::isWidgetType (widget) ? (CameraThumbnailWindow *) widget : NULL);
}

void CameraThumbnailWindow::setLayout (int layoutType, float maxPanelWidth) {
	if (layoutType == layout) {
		return;
	}
	layout = layoutType;
	windowWidth = maxPanelWidth;
	thumbnailImage->setLoadingSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite), maxPanelWidth, maxPanelWidth * 9.0f / 16.0f);
	thumbnailImage->onLoadScale (maxPanelWidth);
	thumbnailImage->setImageUrl (AgentControl::instance->getAgentSecondaryUrl (agentId, captureImagePath, App::instance->createCommand (SystemInterface::Command_GetCaptureImage, (new Json ())->set ("sensor", sensor)->set ("imageTime", thumbnailTimestamp))));
	thumbnailImage->reload ();

	refreshLayout ();
}

void CameraThumbnailWindow::setThumbnailTimestamp (int64_t timestamp) {
	if ((timestamp <= 0) || (timestamp == thumbnailTimestamp)) {
		return;
	}
	thumbnailTimestamp = timestamp;
	timestampLabel->setText (OsUtil::getTimestampDisplayString (thumbnailTimestamp));

	thumbnailImage->setLoadingSprite (NULL);
	thumbnailImage->setImageUrl (AgentControl::instance->getAgentSecondaryUrl (agentId, captureImagePath, App::instance->createCommand (SystemInterface::Command_GetCaptureImage, (new Json ())->set ("sensor", sensor)->set ("imageTime", thumbnailTimestamp))));
}

void CameraThumbnailWindow::refreshLayout () {
	float w, h;

	if ((! thumbnailImage->isLoaded ()) && (windowWidth >= 1.0f)) {
		w = windowWidth;
		h = (w * 9.0f) / 16.0f;
		thumbnailImage->position.assign ((w / 2.0f) - (thumbnailImage->width / 2.0f), (h / 2.0f) - (thumbnailImage->height / 2.0f));
	}
	else {
		w = thumbnailImage->width;
		h = thumbnailImage->height;
		thumbnailImage->position.assign (0.0f, 0.0f);
	}
	setFixedSize (true, w, h);

	timestampLabel->position.assign (0.0f, h - timestampLabel->height);
	switch (layout) {
		case CardView::LowDetail: {
			timestampLabel->isVisible = false;
			break;
		}
		case CardView::MediumDetail: {
			timestampLabel->isVisible = true;
			break;
		}
		case CardView::HighDetail: {
			timestampLabel->isVisible = true;
			break;
		}
	}
}

void CameraThumbnailWindow::setHighlighted (bool highlighted) {
	if (isHighlighted == highlighted) {
		return;
	}
	isHighlighted = highlighted;
	if (isHighlighted) {
		timestampLabel->translateTextColor (UiConfiguration::instance->mediumSecondaryColor, UiConfiguration::instance->shortColorTranslateDuration);
	}
	else {
		timestampLabel->translateTextColor (UiConfiguration::instance->inverseTextColor, UiConfiguration::instance->shortColorTranslateDuration);
	}
	refreshLayout ();
}

bool CameraThumbnailWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel::doProcessMouseState (mouseState);
	if (layout == CardView::LowDetail) {
		if (mouseState.isEntered) {
			timestampLabel->isVisible = true;
		}
		else {
			timestampLabel->isVisible = false;
		}
	}
	return (false);
}

void CameraThumbnailWindow::thumbnailImageLoaded (void *windowPtr, Widget *widgetPtr) {
	CameraThumbnailWindow *window;
	ImageWindow *image;

	window = (CameraThumbnailWindow *) windowPtr;
	image = (ImageWindow *) widgetPtr;
	if (! image->isLoaded ()) {
		return;
	}
	window->refreshLayout ();
	window->shouldRefreshTexture = true;
	window->eventCallback (window->loadCallback);
}

void CameraThumbnailWindow::thumbnailImageLongPressed (void *windowPtr, Widget *widgetPtr) {
	App::instance->uiStack.showImageDialog (((ImageWindow *) widgetPtr)->imageUrl);
}
