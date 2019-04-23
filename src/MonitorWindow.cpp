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
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "OsUtil.h"
#include "Widget.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "Color.h"
#include "Image.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "TextArea.h"
#include "Button.h"
#include "Toggle.h"
#include "StatsWindow.h"
#include "IconLabelWindow.h"
#include "MonitorWindow.h"

MonitorWindow::MonitorWindow (const StdString &agentId)
: Panel ()
, isSelected (false)
, isExpanded (false)
, isStorageDisplayEnabled (false)
, agentId (agentId)
, agentTaskCount (0)
, menuPositionX (0.0f)
, menuPositionY (0.0f)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionLabel (NULL)
, statusIcon (NULL)
, taskCountIcon (NULL)
, storageIcon (NULL)
, streamCountIcon (NULL)
, statsWindow (NULL)
, menuButton (NULL)
, selectToggle (NULL)
, expandToggle (NULL)
, cacheButton (NULL)
, menuClickCallback (NULL)
, menuClickCallbackData (NULL)
, selectStateChangeCallback (NULL)
, selectStateChangeCallbackData (NULL)
, expandStateChangeCallback (NULL)
, expandStateChangeCallbackData (NULL)
, cacheButtonClickCallback (NULL)
, cacheButtonMouseEnterCallback (NULL)
, cacheButtonMouseExitCallback (NULL)
, cacheButtonCallbackData (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	setPadding (uiconfig->paddingSize / 2.0f, uiconfig->paddingSize / 2.0f);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeDisplayIconSprite)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::BodyFont, uiconfig->primaryTextColor));

	descriptionLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	descriptionLabel->isVisible = false;

	statusIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::ActivityStateIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	statusIcon->setPadding (0.0f, 0.0f);
	statusIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	statusIcon->setMouseHoverTooltip (uitext->getText (UiTextString::monitorActivityIconTooltip));
	statusIcon->isVisible = false;

	taskCountIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::TaskCountIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	taskCountIcon->setPadding (0.0f, 0.0f);
	taskCountIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	taskCountIcon->isVisible = false;

	storageIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::StorageIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	storageIcon->setPadding (0.0f, 0.0f);
	storageIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	storageIcon->setMouseHoverTooltip (uitext->getText (UiTextString::storageTooltip));
	storageIcon->isVisible = false;

	streamCountIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	streamCountIcon->setPadding (0.0f, 0.0f);
	streamCountIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	streamCountIcon->isVisible = false;

	statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
	statsWindow->setPadding (uiconfig->paddingSize, 0.0f);
	statsWindow->isVisible = false;

	menuButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::MainMenuButtonSprite)));
	menuButton->setMouseClickCallback (MonitorWindow::menuButtonClicked, this);
	menuButton->setImageColor (uiconfig->flatButtonTextColor);
	menuButton->setMouseHoverTooltip (uitext->getText (UiTextString::moreActionsTooltip));
	menuButton->isVisible = false;

	selectToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::StarOutlineButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::StarButtonSprite)));
	selectToggle->setImageColor (uiconfig->flatButtonTextColor);
	selectToggle->setStateChangeCallback (MonitorWindow::selectToggleStateChanged, this);
	selectToggle->setMouseHoverTooltip (uitext->getText (UiTextString::selectToggleTooltip));
	selectToggle->isVisible = false;

	expandToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::ExpandMoreButtonSprite), uiconfig->coreSprites.getSprite (UiConfiguration::ExpandLessButtonSprite)));
	expandToggle->setImageColor (uiconfig->flatButtonTextColor);
	expandToggle->setStateChangeCallback (MonitorWindow::expandToggleStateChanged, this);
	expandToggle->setMouseHoverTooltip (uitext->getText (UiTextString::expandToggleTooltip));

	refreshLayout ();
}

MonitorWindow::~MonitorWindow () {

}

StdString MonitorWindow::toStringDetail () {
	return (StdString::createSprintf (" MonitorWindow agentId=\"%s\"", agentId.c_str ()));
}

bool MonitorWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" MonitorWindow agentId="));
}

MonitorWindow *MonitorWindow::castWidget (Widget *widget) {
	return (MonitorWindow::isWidgetType (widget) ? (MonitorWindow *) widget : NULL);
}

void MonitorWindow::syncRecordStore () {
	RecordStore *store;
	SystemInterface *interface;
	UiText *uitext;
	Json *record, serverstatus;
	StdString displayname, intentname, text;
	int intentlen, displaylen, count;

	store = &(App::instance->agentControl.recordStore);
	interface = &(App::instance->systemInterface);
	record = store->findRecord (agentId, SystemInterface::CommandId_AgentStatus);
	if (! record) {
		return;
	}
	if (! interface->getCommandObjectParam (record, "monitorServerStatus", &serverstatus)) {
		return;
	}

	uitext = &(App::instance->uiText);
	agentName.assign (interface->getCommandAgentName (record));
	nameLabel->setText (agentName);
	agentTaskCount = interface->getCommandNumberParam (record, "taskCount", (int) 0);

	descriptionLabel->setText (interface->getCommandStringParam (record, "applicationName", ""));

	intentlen = 24;
	displaylen = 32;
	intentname = serverstatus.getString ("intentName", "");
	if (serverstatus.getBoolean ("isPlaying", false)) {
		displayname = serverstatus.getString ("mediaName", "");
		if (! displayname.empty ()) {
			if (intentname.empty ()) {
				text.assign (uitext->getText (UiTextString::playing).capitalized ());
				displaylen += 8;
			}
			else {
				text.sprintf ("[%s]", intentname.truncated (intentlen).c_str ());
			}
			text.appendSprintf (" %s", displayname.truncated (displaylen).c_str ());
		}
	}
	else if (serverstatus.getBoolean ("isShowingUrl", false)) {
		displayname = serverstatus.getString ("showUrl", "");
		if (! displayname.empty ()) {
			if (intentname.empty ()) {
				text.assign (uitext->getText (UiTextString::playing).capitalized ());
				displaylen += 8;
			}
			else {
				text.sprintf ("[%s]", intentname.truncated (intentlen).c_str ());
			}
			text.appendSprintf (" %s", displayname.truncated (displaylen).c_str ());
		}
	}
	if (text.empty ()) {
		if (! intentname.empty ()) {
			text.sprintf ("[%s]", intentname.truncated (intentlen).c_str ());
		}
	}
	if (text.empty ()) {
		statusIcon->setIconImageFrame (UiConfiguration::InactiveStateIconFrame);
		statusIcon->setText (uitext->getText (UiTextString::inactive).capitalized ());
	}
	else {
		statusIcon->setIconImageFrame (UiConfiguration::ActiveStateIconFrame);
		statusIcon->setText (text);
	}

	taskCountIcon->setText (StdString::createSprintf ("%i", agentTaskCount));
	taskCountIcon->setMouseHoverTooltip (uitext->getCountText (agentTaskCount, UiTextString::taskInProgress, UiTextString::tasksInProgress));
	if ((agentTaskCount > 0) && isExpanded) {
		taskCountIcon->isVisible = true;
	}
	else {
		taskCountIcon->isVisible = false;
	}

	storageIcon->setText (OsUtil::getStorageAmountDisplayString (serverstatus.getNumber ("freeStorage", (int64_t) 0), serverstatus.getNumber ("totalStorage", (int64_t) 0)));

	count = serverstatus.getNumber ("streamCount", (int) 0);
	streamCountIcon->setText (StdString::createSprintf ("%i", count));
	streamCountIcon->setMouseHoverTooltip (uitext->getCountText (count, UiTextString::cachedStream, UiTextString::cachedStreams));

	statsWindow->setItem (uitext->getText (UiTextString::uptime).capitalized (), interface->getCommandStringParam (record, "uptime", ""));
	statsWindow->setItem (uitext->getText (UiTextString::address).capitalized (), OsUtil::getAddressDisplayString (interface->getCommandAgentAddress (record), SystemInterface::Constant_DefaultTcpPort1));
	statsWindow->setItem (uitext->getText (UiTextString::version).capitalized (), interface->getCommandStringParam (record, "version", ""));
	if (menuClickCallback) {
		menuButton->isVisible = true;
	}

	refreshLayout ();
	Panel::syncRecordStore ();
}

void MonitorWindow::setMenuClickCallback (Widget::EventCallback callback, void *callbackData) {
	menuClickCallback = callback;
	menuClickCallbackData = callbackData;
}

void MonitorWindow::setSelectStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	selectStateChangeCallback = callback;
	selectStateChangeCallbackData = callbackData;
	selectToggle->isVisible = selectStateChangeCallback ? true : false;
}

void MonitorWindow::setExpandStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	expandStateChangeCallback = callback;
	expandStateChangeCallbackData = callbackData;
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
	refreshLayout ();
}

void MonitorWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	UiConfiguration *uiconfig;

	if (expanded == isExpanded) {
		return;
	}

	uiconfig = &(App::instance->uiConfig);
	isExpanded = expanded;
	if (isExpanded) {
		setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
		expandToggle->setChecked (true, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::HeadlineFont);
		descriptionLabel->isVisible = true;
		statusIcon->isVisible = true;
		statsWindow->isVisible = true;

		if (agentTaskCount > 0) {
			taskCountIcon->isVisible = true;
		}
		else {
			taskCountIcon->isVisible = false;
		}

		if (isStorageDisplayEnabled) {
			storageIcon->isVisible = true;
			streamCountIcon->isVisible = true;
		}
		else {
			storageIcon->isVisible = false;
			streamCountIcon->isVisible = false;
		}

		if (cacheButton) {
			cacheButton->isVisible = true;
		}
	}
	else {
		setPadding (uiconfig->paddingSize / 2.0f, uiconfig->paddingSize / 2.0f);
		expandToggle->setChecked (false, shouldSkipStateChangeCallback);
		nameLabel->setFont (UiConfiguration::BodyFont);
		descriptionLabel->isVisible = false;
		statusIcon->isVisible = false;
		statsWindow->isVisible = false;
		taskCountIcon->isVisible = false;
		storageIcon->isVisible = false;
		streamCountIcon->isVisible = false;
		if (cacheButton) {
			cacheButton->isVisible = false;
		}
	}

	refreshLayout ();
}

void MonitorWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, x0, y0, x2, y2;

	uiconfig = &(App::instance->uiConfig);
	x = widthPadding;
	y = heightPadding;
	x0 = x;
	y0 = y;
	x2 = 0.0f;
	y2 = 0.0f;

	iconImage->flowRight (&x, y, &x2, &y2);
	nameLabel->flowDown (x, &y, &x2, &y2);
	descriptionLabel->flowRight (&x, y, &x2, &y2);

	x = x2 + uiconfig->marginSize;
	y = y0;
	expandToggle->flowRight (&x, y, &x2, &y2);
	if (menuButton->isVisible) {
		menuButton->flowRight (&x, y, &x2, &y2);
	}
	if (selectToggle->isVisible) {
		selectToggle->flowRight (&x, y, &x2, &y2);
	}
	if (statusIcon->isVisible) {
		x = x0;
		y = y2 + uiconfig->marginSize;
		x2 = 0.0f;
		statusIcon->flowDown (x, &y, &x2, &y2);
	}

	x = x0;
	y = y2 + uiconfig->marginSize;
	x2 = 0.0f;
	if (storageIcon->isVisible && streamCountIcon->isVisible) {
		storageIcon->flowRight (&x, y, &x2, &y2);
		streamCountIcon->flowDown (x, &y, &x2, &y2);
	}

	x = x0;
	y = y2 + uiconfig->marginSize;
	x2 = 0.0f;
	if (statsWindow->isVisible) {
		statsWindow->flowRight (&x, y, &x2, &y2);
	}
	if (taskCountIcon->isVisible) {
		taskCountIcon->flowRight (&x, y, &x2, &y2);
	}

	x = x0;
	y = y2 + uiconfig->marginSize;
	x2 = 0.0f;
	if (cacheButton && cacheButton->isVisible) {
		cacheButton->flowRight (&x, y, &x2, &y2);
	}

	resetSize ();

	x = width - widthPadding;
	if (cacheButton && cacheButton->isVisible) {
		cacheButton->flowLeft (&x);
	}

	x = width - widthPadding;
	if (selectToggle->isVisible) {
		selectToggle->flowLeft (&x);
	}
	if (menuButton->isVisible) {
		menuButton->flowLeft (&x);
		menuPositionX = menuButton->position.x;
		menuPositionY = menuButton->position.y + menuButton->height;
	}
	expandToggle->flowLeft (&x);
}

void MonitorWindow::menuButtonClicked (void *windowPtr, Widget *widgetPtr) {
	MonitorWindow *window;

	window = (MonitorWindow *) windowPtr;
	if (window->menuClickCallback) {
		window->menuClickCallback (window->menuClickCallbackData, window);
	}
}

void MonitorWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	MonitorWindow *window;
	Toggle *toggle;

	window = (MonitorWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->isSelected = toggle->isChecked;
	if (window->selectStateChangeCallback) {
		window->selectStateChangeCallback (window->selectStateChangeCallbackData, window);
	}
}

void MonitorWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	MonitorWindow *window;
	Toggle *toggle;

	window = (MonitorWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	if (window->expandStateChangeCallback) {
		window->expandStateChangeCallback (window->expandStateChangeCallbackData, window);
	}
}

void MonitorWindow::addCacheButton (Sprite *sprite, Widget::EventCallback clickCallback, Widget::EventCallback mouseEnterCallback, Widget::EventCallback mouseExitCallback, void *callbackData) {
	UiConfiguration *uiconfig;

	uiconfig = &(App::instance->uiConfig);

	if (cacheButton) {
		cacheButton->isDestroyed = true;
	}
	cacheButtonClickCallback = clickCallback;
	cacheButtonMouseEnterCallback = mouseEnterCallback;
	cacheButtonMouseExitCallback = mouseExitCallback;
	cacheButtonCallbackData = callbackData;
	cacheButton = (Button *) addWidget (new Button (StdString (""), sprite));
	cacheButton->setImageColor (uiconfig->flatButtonTextColor);
	cacheButton->setMouseClickCallback (MonitorWindow::cacheButtonClicked, this);
	cacheButton->setMouseEnterCallback (MonitorWindow::cacheButtonMouseEntered, this);
	cacheButton->setMouseExitCallback (MonitorWindow::cacheButtonMouseExited, this);
	cacheButton->setMouseHoverTooltip (App::instance->uiText.getText (UiTextString::viewMonitorCacheTooltip));

	if (isExpanded) {
		cacheButton->isVisible = true;
	}
	else {
		cacheButton->isVisible = false;
	}
	refreshLayout ();
}

void MonitorWindow::cacheButtonClicked (void *windowPtr, Widget *widgetPtr) {
	MonitorWindow *window;

	window = (MonitorWindow *) windowPtr;
	if (window->cacheButtonClickCallback) {
		window->cacheButtonClickCallback (window->cacheButtonCallbackData, window);
	}
}

void MonitorWindow::cacheButtonMouseEntered (void *windowPtr, Widget *widgetPtr) {
	MonitorWindow *window;
	Button *button;

	window = (MonitorWindow *) windowPtr;
	button = (Button *) widgetPtr;
	Button::mouseEntered (button, button);

	if (window->cacheButtonMouseEnterCallback) {
		window->cacheButtonMouseEnterCallback (window->cacheButtonCallbackData, window);
	}
}

void MonitorWindow::cacheButtonMouseExited (void *windowPtr, Widget *widgetPtr) {
	MonitorWindow *window;
	Button *button;

	window = (MonitorWindow *) windowPtr;
	button = (Button *) widgetPtr;
	Button::mouseExited (button, button);

	if (window->cacheButtonMouseExitCallback) {
		window->cacheButtonMouseExitCallback (window->cacheButtonCallbackData, window);
	}
}
