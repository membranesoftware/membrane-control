/*
* Copyright 2018-2019 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include "OsUtil.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "Widget.h"
#include "Panel.h"
#include "Label.h"
#include "Image.h"
#include "Button.h"
#include "TextArea.h"
#include "Json.h"
#include "ImageWindow.h"
#include "CardView.h"
#include "CameraCaptureWindow.h"

CameraCaptureWindow::CameraCaptureWindow (Json *agentStatus)
: Panel ()
, captureWidth (0)
, captureHeight (0)
, captureTime (0)
, thumbnailImage (NULL)
, nameLabel (NULL)
, detailText (NULL)
, mouseoverLabel (NULL)
, detailNameLabel (NULL)
, viewButton (NULL)
, viewButtonClickCallback (NULL)
, viewButtonClickCallbackData (NULL)
{
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	UiText *uitext;
	Json serverstatus;

	classId = ClassId::CameraCaptureWindow;
	interface = &(App::instance->systemInterface);
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	setFillBg (true, uiconfig->mediumBackgroundColor);
	agentId = interface->getCommandAgentId (agentStatus);
	agentName = interface->getCommandAgentName (agentStatus);
	if (interface->getCommandObjectParam (agentStatus, "cameraServerStatus", &serverstatus)) {
		captureImagePath = serverstatus.getString ("captureImagePath", "");
		captureWidth = serverstatus.getNumber ("lastCaptureWidth", (int) 0);
		captureHeight = serverstatus.getNumber ("lastCaptureHeight", (int) 0);
		captureTime = serverstatus.getNumber ("lastCaptureTime", (int64_t) 0);
	}

	thumbnailImage = (ImageWindow *) addWidget (new ImageWindow (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite))));
	thumbnailImage->setLoadSprite (uiconfig->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite));
	thumbnailImage->setMouseLongPressCallback (CameraCaptureWindow::thumbnailImageLongPressed, this);

	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::BodyFont, uiconfig->primaryTextColor));
	nameLabel->isInputSuspended = true;
	if (captureTime <= 0) {
		nameLabel->setText (agentName);
	}
	else {
		nameLabel->setText (StdString::createSprintf ("%s, %s", agentName.c_str (), OsUtil::getTimestampDisplayString (captureTime).c_str ()));
	}

	detailText = (TextArea *) addWidget (new TextArea (UiConfiguration::CaptionFont, uiconfig->inverseTextColor));
	detailText->setPadding (uiconfig->paddingSize, uiconfig->paddingSize / 2.0f);
	detailText->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	if (captureTime <= 0) {
		detailText->setText (StdString (""));
	}
	else {
		detailText->setText (OsUtil::getTimestampDisplayString (captureTime));
	}
	detailText->zLevel = 2;
	detailText->isInputSuspended = true;
	detailText->isVisible = false;

	mouseoverLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (agentName, UiConfiguration::CaptionFont, uiconfig->inverseTextColor)));
	mouseoverLabel->zLevel = 1;
	mouseoverLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	mouseoverLabel->isTextureTargetDrawEnabled = false;
	mouseoverLabel->isInputSuspended = true;
	mouseoverLabel->isVisible = false;

	detailNameLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (agentName, UiConfiguration::HeadlineFont, uiconfig->inverseTextColor)));
	detailNameLabel->zLevel = 1;
	detailNameLabel->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	detailNameLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, uiconfig->scrimBackgroundAlpha));
	detailNameLabel->isInputSuspended = true;
	detailNameLabel->isVisible = false;

	viewButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::ImageButtonSprite)));
	viewButton->zLevel = 3;
	viewButton->isTextureTargetDrawEnabled = false;
	viewButton->setImageColor (uiconfig->flatButtonTextColor);
	viewButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	viewButton->setMouseHoverTooltip (uitext->getText (UiTextString::viewTimelineImagesTooltip));
	viewButton->setMouseClickCallback (CameraCaptureWindow::viewButtonClicked, this);
}

CameraCaptureWindow::~CameraCaptureWindow () {

}

StdString CameraCaptureWindow::toStringDetail () {
	return (StdString::createSprintf (" CameraCaptureWindow id=\"%s\"", agentId.c_str ()));
}

bool CameraCaptureWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::CameraCaptureWindow));
}

CameraCaptureWindow *CameraCaptureWindow::castWidget (Widget *widget) {
	return (CameraCaptureWindow::isWidgetType (widget) ? (CameraCaptureWindow *) widget : NULL);
}

void CameraCaptureWindow::setViewButtonClickCallback (Widget::EventCallback callback, void *callbackData) {
	viewButtonClickCallback = callback;
	viewButtonClickCallbackData = callbackData;
}

void CameraCaptureWindow::setLayout (int layoutType, float maxPanelWidth) {
	float w, h;

	if ((layoutType == layout) || (captureWidth <= 0) || (maxPanelWidth < 1.0f)) {
		return;
	}

	layout = layoutType;
	w = maxPanelWidth;
	h = captureHeight;
	h *= maxPanelWidth;
	h /= captureWidth;
	w = floorf (w);
	h = floorf (h);
	thumbnailImage->setWindowSize (w, h);
	if (hasCaptureImage ()) {
		thumbnailImage->setImageUrl (App::instance->agentControl.getAgentSecondaryUrl (agentId, App::instance->createCommand (SystemInterface::Command_GetCaptureImage, SystemInterface::Constant_Camera), captureImagePath));
	}
	thumbnailImage->reload ();

	if (layout == CardView::HighDetail) {
		nameLabel->setFont (UiConfiguration::BodyFont);
	}
	else {
		nameLabel->setFont (UiConfiguration::CaptionFont);
	}

	refreshLayout ();
}

void CameraCaptureWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y;

	uiconfig = &(App::instance->uiConfig);
	x = 0.0f;
	y = 0.0f;
	thumbnailImage->position.assign (x, y);
	mouseoverLabel->isVisible = false;

	switch (layout) {
		case CardView::LowDetail: {
			nameLabel->isVisible = false;
			detailNameLabel->isVisible = false;
			detailText->isVisible = false;
			mouseoverLabel->position.assign (0.0f, 0.0f);

			setFixedSize (true, thumbnailImage->width, thumbnailImage->height);
			viewButton->position.assign (thumbnailImage->position.x + thumbnailImage->width - viewButton->width - uiconfig->dropShadowWidth, thumbnailImage->position.y, thumbnailImage->height - viewButton->height);
			break;
		}
		case CardView::MediumDetail: {
			detailNameLabel->isVisible = false;
			detailText->isVisible = false;
			x += uiconfig->paddingSize;
			y += thumbnailImage->height + uiconfig->marginSize;
			nameLabel->position.assign (x, y);
			nameLabel->isVisible = true;
			viewButton->position.assign (thumbnailImage->position.x + thumbnailImage->width - viewButton->width - uiconfig->dropShadowWidth, thumbnailImage->position.y, thumbnailImage->height - viewButton->height);

			resetSize ();
			setFixedSize (true, thumbnailImage->width, maxWidgetY + uiconfig->paddingSize);
			break;
		}
		case CardView::HighDetail: {
			nameLabel->isVisible = false;

			detailNameLabel->position.assign (thumbnailImage->position.x, thumbnailImage->position.y);
			detailNameLabel->isVisible = true;

			detailText->position.assign (x, thumbnailImage->position.y + thumbnailImage->height - detailText->height);
			detailText->isVisible = true;
			setFixedSize (true, thumbnailImage->width, thumbnailImage->height);
			viewButton->position.assign (thumbnailImage->position.x + thumbnailImage->width - viewButton->width - uiconfig->dropShadowWidth, thumbnailImage->position.y, thumbnailImage->height - viewButton->height);
			break;
		}
	}
}

void CameraCaptureWindow::syncRecordStore () {
	RecordStore *store;
	SystemInterface *interface;
	Json *agentstatus, serverstatus;
	int64_t t;

	store = &(App::instance->agentControl.recordStore);
	interface = &(App::instance->systemInterface);
	agentstatus = store->findRecord (RecordStore::matchAgentStatusSource, &agentId);
	if (! agentstatus) {
		return;
	}
	if (interface->getCommandObjectParam (agentstatus, "cameraServerStatus", &serverstatus)) {
		captureImagePath = serverstatus.getString ("captureImagePath", "");
		captureWidth = serverstatus.getNumber ("lastCaptureWidth", (int) 0);
		captureHeight = serverstatus.getNumber ("lastCaptureHeight", (int) 0);

		t = serverstatus.getNumber ("lastCaptureTime", (int64_t) 0);
		if (t <= 0) {
			nameLabel->setText (agentName);
			detailText->setText (StdString (""));
		}
		else {
			nameLabel->setText (StdString::createSprintf ("%s, %s", agentName.c_str (), OsUtil::getTimestampDisplayString (t).c_str ()));
			detailText->setText (OsUtil::getTimestampDisplayString (t));
			if (t != captureTime) {
				thumbnailImage->setLoadSprite (NULL);
				thumbnailImage->reload ();
			}
		}
		captureTime = t;
	}
}

bool CameraCaptureWindow::hasCaptureImage () {
	return ((! captureImagePath.empty ()) && (captureWidth > 0) && (captureHeight > 0) && (captureTime > 0));
}

void CameraCaptureWindow::reloadCaptureImage () {
	if (hasCaptureImage ()) {
		thumbnailImage->reload ();
	}
}

void CameraCaptureWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
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

void CameraCaptureWindow::viewButtonClicked (void *windowPtr, Widget *widgetPtr) {
	CameraCaptureWindow *window;

	window = (CameraCaptureWindow *) windowPtr;
	if (window->viewButtonClickCallback) {
		window->viewButtonClickCallback (window->viewButtonClickCallbackData, window);
	}
}

void CameraCaptureWindow::thumbnailImageLongPressed (void *windowPtr, Widget *widgetPtr) {
	ImageWindow *image;

	image = (ImageWindow *) widgetPtr;
	App::instance->uiStack.showImageDialog (image->imageUrl);
}
