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
#include "App.h"
#include "ClassId.h"
#include "Log.h"
#include "StdString.h"
#include "UiText.h"
#include "OsUtil.h"
#include "Widget.h"
#include "SystemInterface.h"
#include "RecordStore.h"
#include "Agent.h"
#include "AgentControl.h"
#include "UiConfiguration.h"
#include "Color.h"
#include "Image.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "Button.h"
#include "Toggle.h"
#include "IconLabelWindow.h"
#include "MonitorWindow.h"

const float MonitorWindow::ScreenshotImageScale = 0.27f;

MonitorWindow::MonitorWindow (const StdString &agentId)
: Panel ()
, isSelected (false)
, isExpanded (false)
, isSelectEnabled (false)
, isScreenshotDisplayEnabled (false)
, isStorageDisplayEnabled (false)
, agentId (agentId)
, agentTaskCount (0)
, screenshotTime (0)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionLabel (NULL)
, dividerPanel (NULL)
, screenshotImage (NULL)
, statusIcon (NULL)
, taskCountIcon (NULL)
, storageIcon (NULL)
, streamCountIcon (NULL)
, selectToggle (NULL)
, expandToggle (NULL)
, actionButton (NULL)
, agentTaskWindow (NULL)
{
	classId = ClassId::MonitorWindow;

	setPadding (UiConfiguration::instance->paddingSize / 2.0f, UiConfiguration::instance->paddingSize / 2.0f);
	setCornerRadius (UiConfiguration::instance->cornerRadius);
	setFillBg (true, UiConfiguration::instance->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeDisplayIconSprite)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::BodyFont, UiConfiguration::instance->primaryTextColor));

	descriptionLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	descriptionLabel->isVisible = false;

	dividerPanel = (Panel *) addWidget (new Panel ());
	dividerPanel->setFillBg (true, UiConfiguration::instance->dividerColor);
	dividerPanel->setFixedSize (true, 1.0f, UiConfiguration::instance->headlineDividerLineWidth);
	dividerPanel->isPanelSizeClipEnabled = true;
	dividerPanel->isVisible = false;

	screenshotImage = (ImageWindow *) addWidget (new ImageWindow (new Image (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::LargeLoadingIconSprite))));
	screenshotImage->loadCallback = Widget::EventCallbackContext (MonitorWindow::screenshotImageLoaded, this);
	screenshotImage->mouseLongPressCallback = Widget::EventCallbackContext (MonitorWindow::screenshotImageLongPressed, this);
	screenshotImage->setLoadResize (true, ((float) App::instance->windowWidth) * MonitorWindow::ScreenshotImageScale);
	screenshotImage->setFillBg (true, UiConfiguration::instance->darkBackgroundColor);
	screenshotImage->isVisible = false;

	statusIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::InactiveStateIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	statusIcon->setPadding (0.0f, 0.0f);
	statusIcon->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);
	statusIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::MonitorActivityIconTooltip));
	statusIcon->isVisible = false;

	taskCountIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::TaskCountIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	taskCountIcon->setPadding (0.0f, 0.0f);
	taskCountIcon->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);
	taskCountIcon->isVisible = false;

	storageIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StorageIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	storageIcon->setPadding (0.0f, 0.0f);
	storageIcon->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);
	storageIcon->setMouseHoverTooltip (UiText::instance->getText (UiTextString::StorageTooltip));
	storageIcon->isVisible = false;

	streamCountIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->lightPrimaryTextColor));
	streamCountIcon->setPadding (0.0f, 0.0f);
	streamCountIcon->setTextChangeHighlight (true, UiConfiguration::instance->primaryTextColor);
	streamCountIcon->isVisible = false;

	selectToggle = (Toggle *) addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StarOutlineButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::StarButtonSprite)));
	selectToggle->stateChangeCallback = Widget::EventCallbackContext (MonitorWindow::selectToggleStateChanged, this);
	selectToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	selectToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::UnselectedToggleTooltip), UiText::instance->getText (UiTextString::SelectedToggleTooltip));
	selectToggle->isVisible = false;

	expandToggle = (Toggle *) addWidget (new Toggle (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->stateChangeCallback = Widget::EventCallbackContext (MonitorWindow::expandToggleStateChanged, this);
	expandToggle->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	expandToggle->setStateMouseHoverTooltips (UiText::instance->getText (UiTextString::Expand).capitalized (), UiText::instance->getText (UiTextString::Minimize).capitalized ());

	agentTaskWindow = (AgentTaskWindow *) addWidget (new AgentTaskWindow (agentId));

	refreshLayout ();
}

MonitorWindow::~MonitorWindow () {

}

StdString MonitorWindow::toStringDetail () {
	return (StdString::createSprintf (" MonitorWindow agentId=\"%s\"", agentId.c_str ()));
}

bool MonitorWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::MonitorWindow));
}

MonitorWindow *MonitorWindow::castWidget (Widget *widget) {
	return (MonitorWindow::isWidgetType (widget) ? (MonitorWindow *) widget : NULL);
}

void MonitorWindow::syncRecordStore () {
	Json *record, serverstatus;
	StdString path, displaytarget, intentname, text;
	int64_t t;
	int intentlen, displaystate, displaylen, count;

	record = RecordStore::instance->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
	if (! record) {
		return;
	}
	if (! SystemInterface::instance->getCommandObjectParam (record, "monitorServerStatus", &serverstatus)) {
		return;
	}

	agentName.assign (Agent::getCommandAgentName (record));
	nameLabel->setText (agentName);
	agentTaskCount = SystemInterface::instance->getCommandNumberParam (record, "taskCount", (int) 0);
	descriptionLabel->setText (SystemInterface::instance->getCommandStringParam (record, "applicationName", ""));

	path = serverstatus.getString ("screenshotPath", "");
	if ((! isScreenshotDisplayEnabled) || path.empty ()) {
		screenshotImage->isVisible = false;
	}
	else {
		screenshotImage->setImageUrl (AgentControl::instance->getAgentSecondaryUrl (agentId, NULL, path));
		screenshotImage->isVisible = isExpanded;
		t = serverstatus.getNumber ("screenshotTime", (int64_t) -1);
		if ((t > 0) && (t != screenshotTime)) {
			screenshotTime = t;
			if (screenshotImage->isVisible) {
				screenshotImage->reload ();
			}
		}
	}

	intentlen = 24;
	displaylen = 32;
	intentname = serverstatus.getString ("intentName", "");
	if (! intentname.empty ()) {
		text.appendSprintf ("[%s]", intentname.truncated (intentlen).c_str ());
		displaylen += 8;
	}

	displaytarget = serverstatus.getString ("displayTarget", "");
	if (! displaytarget.empty ()) {
		if (text.empty ()) {
			displaystate = serverstatus.getNumber ("displayState", SystemInterface::Constant_DefaultDisplayState);
			switch (displaystate) {
				case SystemInterface::Constant_ShowUrlDisplayState: {
					text.assign (UiText::instance->getText (UiTextString::Showing).capitalized ());
					break;
				}
				case SystemInterface::Constant_PlayMediaDisplayState: {
					text.assign (UiText::instance->getText (UiTextString::Playing).capitalized ());
					break;
				}
				case SystemInterface::Constant_ShowImageDisplayState: {
					text.assign (UiText::instance->getText (UiTextString::ShowingImage).capitalized ());
					text.append (":");
					break;
				}
				case SystemInterface::Constant_PlayCameraStreamDisplayState: {
					text.assign (UiText::instance->getText (UiTextString::PlayingLiveCamera).capitalized ());
					text.append (":");
					break;
				}
			}
		}
		text.append (" ");
		text.append (displaytarget.truncated (displaylen));
	}

	if (text.empty ()) {
		statusIcon->setIconSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::InactiveStateIconSprite));
		statusIcon->setText (UiText::instance->getText (UiTextString::Inactive).capitalized ());
	}
	else {
		if (serverstatus.getBoolean ("isPlayPaused", false)) {
			statusIcon->setIconSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::PauseIconSprite));
		}
		else {
			statusIcon->setIconSprite (UiConfiguration::instance->coreSprites.getSprite (UiConfiguration::ActiveStateIconSprite));
		}
		statusIcon->setText (text);
	}

	taskCountIcon->setText (StdString::createSprintf ("%i", agentTaskCount));
	taskCountIcon->setMouseHoverTooltip (UiText::instance->getCountText (agentTaskCount, UiTextString::TaskInProgress, UiTextString::TasksInProgress));

	agentTaskWindow->syncRecordStore ();
	if ((agentTaskCount > 0) && isExpanded) {
		taskCountIcon->isVisible = true;
		if (! agentTaskWindow->isTaskRunning) {
			agentTaskWindow->isVisible = false;
		}
		else {
			agentTaskWindow->isVisible = true;
		}
	}
	else {
		taskCountIcon->isVisible = false;
		agentTaskWindow->isVisible = false;
	}

	storageIcon->setText (OsUtil::getStorageAmountDisplayString (serverstatus.getNumber ("freeStorage", (int64_t) 0), serverstatus.getNumber ("totalStorage", (int64_t) 0)));

	count = serverstatus.getNumber ("streamCount", (int) 0);
	streamCountIcon->setText (StdString::createSprintf ("%i", count));
	streamCountIcon->setMouseHoverTooltip (UiText::instance->getCountText (count, UiTextString::CachedStream, UiTextString::CachedStreams));

	refreshLayout ();
	Panel::syncRecordStore ();
}

void MonitorWindow::setSelectEnabled (bool enable) {
	if (isSelectEnabled == enable) {
		return;
	}
	isSelectEnabled = enable;
	if (isSelectEnabled) {
		selectToggle->isVisible = true;
	}
	else {
		selectToggle->isVisible = false;
	}
	refreshLayout ();
}

void MonitorWindow::setScreenshotDisplayEnabled (bool enable) {
	if (isScreenshotDisplayEnabled == enable) {
		return;
	}
	isScreenshotDisplayEnabled = enable;
	if (isScreenshotDisplayEnabled) {
		screenshotImage->isVisible = isExpanded;
	}
	else {
		screenshotImage->isVisible = false;
	}
	refreshLayout ();
}

void MonitorWindow::setStorageDisplayEnabled (bool enable) {
	if (isStorageDisplayEnabled == enable) {
		return;
	}
	isStorageDisplayEnabled = enable;
	if (isStorageDisplayEnabled) {
		if (isExpanded) {
			storageIcon->isVisible = true;
			streamCountIcon->isVisible = true;
		}
		else {
			storageIcon->isVisible = false;
			streamCountIcon->isVisible = false;
		}
	}
	else {
		storageIcon->isVisible = false;
		streamCountIcon->isVisible = false;
	}
	refreshLayout ();
}

void MonitorWindow::setSelected (bool selected, bool shouldSkipStateChangeCallback) {
	if (selected == isSelected) {
		return;
	}
	isSelected = selected;
	selectToggle->setChecked (isSelected, shouldSkipStateChangeCallback);
	if (isSelected) {
		setCornerRadius (0, UiConfiguration::instance->cornerRadius, 0, UiConfiguration::instance->cornerRadius);
	}
	else {
		setCornerRadius (UiConfiguration::instance->cornerRadius);
	}
	refreshLayout ();
}

void MonitorWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	if (expanded == isExpanded) {
		return;
	}
	isExpanded = expanded;
	if (isExpanded) {
		setPadding (UiConfiguration::instance->paddingSize, UiConfiguration::instance->paddingSize);
		expandToggle->setChecked (true, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::HeadlineFont);
		descriptionLabel->isVisible = true;
		dividerPanel->isVisible = true;

		if (screenshotImage->isImageUrlEmpty ()) {
			screenshotImage->isVisible = false;
		}
		else {
			screenshotImage->isVisible = isScreenshotDisplayEnabled;
		}

		statusIcon->isVisible = true;

		if (agentTaskCount > 0) {
			taskCountIcon->isVisible = true;
			if (! agentTaskWindow->isTaskRunning) {
				agentTaskWindow->isVisible = false;
			}
			else {
				agentTaskWindow->isVisible = true;
			}
		}
		else {
			taskCountIcon->isVisible = false;
			agentTaskWindow->isVisible = false;
		}

		if (isStorageDisplayEnabled) {
			storageIcon->isVisible = true;
			streamCountIcon->isVisible = true;
		}
		else {
			storageIcon->isVisible = false;
			streamCountIcon->isVisible = false;
		}

		if (actionButton) {
			actionButton->isVisible = true;
		}
	}
	else {
		setPadding (UiConfiguration::instance->paddingSize / 2.0f, UiConfiguration::instance->paddingSize / 2.0f);
		expandToggle->setChecked (false, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::BodyFont);
		descriptionLabel->isVisible = false;
		dividerPanel->isVisible = false;
		screenshotImage->isVisible = false;
		statusIcon->isVisible = false;
		taskCountIcon->isVisible = false;
		agentTaskWindow->isVisible = false;
		storageIcon->isVisible = false;
		streamCountIcon->isVisible = false;
		if (actionButton) {
			actionButton->isVisible = false;
		}
	}

	refreshLayout ();
}

void MonitorWindow::refreshLayout () {
	float x, y, x0, y0, x2, y2;

	x = widthPadding;
	y = heightPadding;
	x0 = x;
	y0 = y;
	x2 = 0.0f;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, &x2, &y2);
	nameLabel->flowDown (x, &y, &x2, &y2);
	if (descriptionLabel->isVisible) {
		descriptionLabel->flowRight (&x, y, &x2, &y2);
	}

	x = x2 + UiConfiguration::instance->marginSize;
	y = y0;
	expandToggle->flowRight (&x, y, &x2, &y2);
	if (selectToggle->isVisible) {
		selectToggle->flowRight (&x, y, &x2, &y2);
	}

	x = x0;
	y = y2 + UiConfiguration::instance->marginSize;
	x2 = 0.0f;
	if (dividerPanel->isVisible) {
		dividerPanel->flowDown (0.0f, &y, &x2, &y2);
	}
	if (statusIcon->isVisible) {
		statusIcon->flowDown (x, &y, &x2, &y2);
	}

	if (screenshotImage->isVisible) {
		x = x0;
		y = y2 + UiConfiguration::instance->marginSize;
		screenshotImage->flowDown (x, &y, &x2, &y2);
	}

	x = x0;
	y = y2 + UiConfiguration::instance->marginSize;
	x2 = 0.0f;
	if (storageIcon->isVisible && streamCountIcon->isVisible) {
		storageIcon->flowRight (&x, y, &x2, &y2);
		streamCountIcon->flowRight (&x, y, &x2, &y2);
	}
	if (taskCountIcon->isVisible) {
		taskCountIcon->flowRight (&x, y, &x2, &y2);
	}
	if (agentTaskWindow->isVisible) {
		x = x0;
		y = y2 + UiConfiguration::instance->marginSize;
		x2 = 0.0f;
		agentTaskWindow->flowDown (x, &y, &x2, &y2);
	}
	if (actionButton && actionButton->isVisible) {
		actionButton->flowRight (&x, y, &x2, &y2);
	}

	resetSize ();

	if (dividerPanel->isVisible) {
		dividerPanel->setFixedSize (true, width, UiConfiguration::instance->headlineDividerLineWidth);
	}

	x = width - widthPadding;
	if (actionButton && actionButton->isVisible) {
		actionButton->flowLeft (&x);
	}

	x = width - widthPadding;
	if (selectToggle->isVisible) {
		selectToggle->flowLeft (&x);
	}
	expandToggle->flowLeft (&x);
}

void MonitorWindow::screenshotImageLoaded (void *windowPtr, Widget *widgetPtr) {
	MonitorWindow *window;
	ImageWindow *image;

	window = (MonitorWindow *) windowPtr;
	image = (ImageWindow *) widgetPtr;
	if (! image->isLoaded ()) {
		return;
	}
	window->eventCallback (window->screenshotLoadCallback);
}

void MonitorWindow::screenshotImageLongPressed (void *windowPtr, Widget *widgetPtr) {
	ImageWindow *image;

	image = (ImageWindow *) widgetPtr;
	App::instance->uiStack.showImageDialog (image->imageUrl);
}

void MonitorWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	MonitorWindow *window;
	Toggle *toggle;

	window = (MonitorWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;

	window->isSelected = toggle->isChecked;
	if (window->isSelected) {
		window->setCornerRadius (0, UiConfiguration::instance->cornerRadius, 0, UiConfiguration::instance->cornerRadius);
	}
	else {
		window->setCornerRadius (UiConfiguration::instance->cornerRadius);
	}
	window->eventCallback (window->selectStateChangeCallback);
}

void MonitorWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	MonitorWindow *window;
	Toggle *toggle;

	window = (MonitorWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	window->eventCallback (window->expandStateChangeCallback);
}

void MonitorWindow::addActionButton (Sprite *sprite, const StdString &tooltipText, Widget::EventCallbackContext clickCallback) {
	if (actionButton) {
		actionButton->isDestroyed = true;
	}
	actionButton = (Button *) addWidget (new Button (sprite));
	actionButton->mouseClickCallback = Widget::EventCallbackContext (MonitorWindow::actionButtonClicked, this);
	actionButton->setImageColor (UiConfiguration::instance->flatButtonTextColor);
	if (! tooltipText.empty ()) {
		actionButton->setMouseHoverTooltip (tooltipText);
	}
	actionClickCallback = clickCallback;

	if (isExpanded) {
		actionButton->isVisible = true;
	}
	else {
		actionButton->isVisible = false;
	}
	refreshLayout ();
}

void MonitorWindow::actionButtonClicked (void *windowPtr, Widget *widgetPtr) {
	((MonitorWindow *) windowPtr)->eventCallback (((MonitorWindow *) windowPtr)->actionClickCallback);
}
