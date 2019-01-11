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
#include "Util.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "Widget.h"
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
#include "MediaUi.h"
#include "MediaLibraryWindow.h"

MediaLibraryWindow::MediaLibraryWindow (const StdString &agentId)
: Panel ()
, isSelected (false)
, isExpanded (false)
, agentId (agentId)
, agentTaskCount (0)
, menuPositionX (0.0f)
, menuPositionY (0.0f)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionLabel (NULL)
, taskCountIcon (NULL)
, storageIcon (NULL)
, mediaCountIcon (NULL)
, streamCountIcon (NULL)
, statsWindow (NULL)
, menuButton (NULL)
, selectToggle (NULL)
, expandToggle (NULL)
, menuClickCallback (NULL)
, menuClickCallbackData (NULL)
, selectStateChangeCallback (NULL)
, selectStateChangeCallbackData (NULL)
, expandStateChangeCallback (NULL)
, expandStateChangeCallbackData (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	populate ();
	refreshLayout ();
}

MediaLibraryWindow::~MediaLibraryWindow () {

}

StdString MediaLibraryWindow::toStringDetail () {
	return (StdString::createSprintf (" MediaLibraryWindow agentId=\"%s\"", agentId.c_str ()));
}

bool MediaLibraryWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" MediaLibraryWindow agentId="));
}

MediaLibraryWindow *MediaLibraryWindow::castWidget (Widget *widget) {
	return (MediaLibraryWindow::isWidgetType (widget) ? (MediaLibraryWindow *) widget : NULL);
}

void MediaLibraryWindow::populate () {
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LARGE_MEDIA_ICON)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::HEADLINE, uiconfig->primaryTextColor));

	descriptionLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor));
	descriptionLabel->isVisible = false;

	taskCountIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::TASK_COUNT_ICON), StdString (""), UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor));
	taskCountIcon->setPadding (0.0f, 0.0f);
	taskCountIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	taskCountIcon->isVisible = false;

	storageIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::STORAGE_ICON), StdString (""), UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor));
	storageIcon->setPadding (0.0f, 0.0f);
	storageIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	storageIcon->setMouseHoverTooltip (uitext->getText (UiTextString::storageTooltip));
	storageIcon->isVisible = false;

	mediaCountIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SMALL_MEDIA_ICON), StdString (""), UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor));
	mediaCountIcon->setPadding (0.0f, 0.0f);
	mediaCountIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	mediaCountIcon->isVisible = false;

	streamCountIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SMALL_STREAM_ICON), StdString (""), UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor));
	streamCountIcon->setPadding (0.0f, 0.0f);
	streamCountIcon->setTextChangeHighlight (true, uiconfig->primaryTextColor);
	streamCountIcon->isVisible = false;

	statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
	statsWindow->setPadding (uiconfig->paddingSize, 0.0f);
	statsWindow->isVisible = false;

	menuButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::MAIN_MENU_BUTTON)));
	menuButton->setMouseClickCallback (MediaLibraryWindow::menuButtonClicked, this);
	menuButton->setImageColor (uiconfig->flatButtonTextColor);
	menuButton->setMouseHoverTooltip (uitext->getText (UiTextString::moreActionsTooltip));
	menuButton->isVisible = false;

	selectToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::STAR_OUTLINE_BUTTON), uiconfig->coreSprites.getSprite (UiConfiguration::STAR_BUTTON)));
	selectToggle->setImageColor (uiconfig->flatButtonTextColor);
	selectToggle->setStateChangeCallback (MediaLibraryWindow::selectToggleStateChanged, this);
	selectToggle->setMouseHoverTooltip (uitext->getText (UiTextString::selectToggleTooltip));
	selectToggle->isVisible = false;

	expandToggle = (Toggle *) addWidget (new Toggle (uiconfig->coreSprites.getSprite (UiConfiguration::EXPAND_MORE_BUTTON), uiconfig->coreSprites.getSprite (UiConfiguration::EXPAND_LESS_BUTTON)));
	expandToggle->setImageColor (uiconfig->flatButtonTextColor);
	expandToggle->setStateChangeCallback (MediaLibraryWindow::expandToggleStateChanged, this);
	expandToggle->setMouseHoverTooltip (uitext->getText (UiTextString::expandToggleTooltip));
}

void MediaLibraryWindow::syncRecordStore (RecordStore *store) {
	SystemInterface *interface;
	UiText *uitext;
	Json *record, mediaserverstatus, streamserverstatus;
	int count;

	interface = &(App::getInstance ()->systemInterface);
	record = store->findRecord (agentId, SystemInterface::Command_AgentStatus);
	if (! record) {
		return;
	}
	if (! interface->getCommandObjectParam (record, "mediaServerStatus", &mediaserverstatus)) {
		return;
	}
	if (! interface->getCommandObjectParam (record, "streamServerStatus", &streamserverstatus)) {
		return;
	}

	uitext = &(App::getInstance ()->uiText);
	agentName.assign (interface->getCommandAgentName (record));
	nameLabel->setText (agentName);
	agentTaskCount = interface->getCommandNumberParam (record, "taskCount", (int) 0);

	descriptionLabel->setText (interface->getCommandStringParam (record, "applicationName", ""));

	taskCountIcon->setText (StdString::createSprintf ("%i", agentTaskCount));
	taskCountIcon->setMouseHoverTooltip (uitext->getCountText (agentTaskCount, UiTextString::taskInProgress, UiTextString::tasksInProgress));
	if ((agentTaskCount > 0) && isExpanded) {
		taskCountIcon->isVisible = true;
	}
	else {
		taskCountIcon->isVisible = false;
	}

	storageIcon->setText (Util::getStorageAmountDisplayString (streamserverstatus.getNumber ("freeStorage", (int64_t) 0), streamserverstatus.getNumber ("totalStorage", (int64_t) 0)));

	count = mediaserverstatus.getNumber ("mediaCount", (int) 0);
	mediaCountIcon->setText (StdString::createSprintf ("%i", count));
	mediaCountIcon->setMouseHoverTooltip (uitext->getCountText (count, UiTextString::mediaFile, UiTextString::mediaFiles));

	count = streamserverstatus.getNumber ("streamCount", (int) 0);
	streamCountIcon->setText (StdString::createSprintf ("%i", count));
	streamCountIcon->setMouseHoverTooltip (uitext->getCountText (count, UiTextString::videoStream, UiTextString::videoStreams));

	statsWindow->setItem (uitext->getText (UiTextString::address).capitalized (), Util::getAddressDisplayString (interface->getCommandAgentAddress (record), SystemInterface::Constant_DefaultTcpPort1));
	statsWindow->setItem (uitext->getText (UiTextString::version).capitalized (), interface->getCommandStringParam (record, "version", ""));
	statsWindow->setItem (uitext->getText (UiTextString::uptime).capitalized (), interface->getCommandStringParam (record, "uptime", ""));
	if (menuClickCallback) {
		menuButton->isVisible = true;
	}

	refreshLayout ();
	Panel::syncRecordStore (store);
}

void MediaLibraryWindow::setMenuClickCallback (Widget::EventCallback callback, void *callbackData) {
	menuClickCallback = callback;
	menuClickCallbackData = callbackData;
}

void MediaLibraryWindow::setSelectStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	selectStateChangeCallback = callback;
	selectStateChangeCallbackData = callbackData;
	selectToggle->isVisible = selectStateChangeCallback ? true : false;
}

void MediaLibraryWindow::setExpandStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	expandStateChangeCallback = callback;
	expandStateChangeCallbackData = callbackData;
}

void MediaLibraryWindow::setExpanded (bool expanded, bool shouldSkipStateChangeCallback) {
	Widget::EventCallback callback;
	void *callbackdata;

	if (expanded == isExpanded) {
		return;
	}
	callback = NULL;
	callbackdata = NULL;
	if (shouldSkipStateChangeCallback) {
		callback = expandStateChangeCallback;
		callbackdata = expandStateChangeCallbackData;
		expandStateChangeCallback = NULL;
		expandStateChangeCallbackData = NULL;
	}
	isExpanded = expanded;
	if (isExpanded) {
		expandToggle->setChecked (true);
		descriptionLabel->isVisible = true;
		statsWindow->isVisible = true;
		if (agentTaskCount > 0) {
			taskCountIcon->isVisible = true;
		}
		else {
			taskCountIcon->isVisible = false;
		}
		storageIcon->isVisible = true;
		mediaCountIcon->isVisible = true;
		streamCountIcon->isVisible = true;
	}
	else {
		expandToggle->setChecked (false);
		descriptionLabel->isVisible = false;
		statsWindow->isVisible = false;
		taskCountIcon->isVisible = false;
		storageIcon->isVisible = false;
		mediaCountIcon->isVisible = false;
		streamCountIcon->isVisible = false;
	}

	refreshLayout ();
	if (shouldSkipStateChangeCallback) {
		expandStateChangeCallback = callback;
		expandStateChangeCallbackData = callbackdata;
	}
}

void MediaLibraryWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, x0, y0, x2, y2;

	uiconfig = &(App::getInstance ()->uiConfig);
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
		selectToggle->flowDown (x, &y, &x2, &y2);
	}

	x = x0;
	y = y2 + uiconfig->marginSize;
	x2 = 0.0f;
	if (storageIcon->isVisible) {
		storageIcon->flowRight (&x, y, &x2, &y2);
	}
	if (mediaCountIcon->isVisible) {
		mediaCountIcon->flowRight (&x, y, &x2, &y2);
	}
	if (streamCountIcon->isVisible) {
		streamCountIcon->flowRight (&x, y, &x2, &y2);
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

	resetSize ();

	x = width - uiconfig->paddingSize;
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

void MediaLibraryWindow::menuButtonClicked (void *windowPtr, Widget *widgetPtr) {
	MediaLibraryWindow *window;

	window = (MediaLibraryWindow *) windowPtr;
	if (window->menuClickCallback) {
		window->menuClickCallback (window->menuClickCallbackData, window);
	}
}

void MediaLibraryWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	MediaLibraryWindow *window;
	Toggle *toggle;

	window = (MediaLibraryWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->isSelected = toggle->isChecked;
	if (window->selectStateChangeCallback) {
		window->selectStateChangeCallback (window->selectStateChangeCallbackData, window);
	}
}

void MediaLibraryWindow::expandToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	MediaLibraryWindow *window;
	Toggle *toggle;

	window = (MediaLibraryWindow *) windowPtr;
	toggle = (Toggle *) widgetPtr;
	window->setExpanded (toggle->isChecked, true);
	if (window->expandStateChangeCallback) {
		window->expandStateChangeCallback (window->expandStateChangeCallbackData, window);
	}
}
