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
#include <vector>
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "Util.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "Button.h"
#include "TextArea.h"
#include "Toggle.h"
#include "ToggleWindow.h"
#include "Image.h"
#include "Json.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "MainUi.h"
#include "WebKioskUi.h"
#include "IconLabelWindow.h"
#include "UiLaunchWindow.h"

UiLaunchWindow::UiLaunchWindow (int uiType, SpriteGroup *mainUiSpriteGroup)
: Panel ()
, uiType (uiType)
, spriteGroup (mainUiSpriteGroup)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionText (NULL)
, openButton (NULL)
, openCallback (NULL)
, openCallbackData (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	populate ();
	refreshLayout ();
}

UiLaunchWindow::~UiLaunchWindow () {

}

StdString UiLaunchWindow::toStringDetail () {
  return (StdString (" UiLaunchWindow"));
}

bool UiLaunchWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" UiLaunchWindow"));
}

UiLaunchWindow *UiLaunchWindow::castWidget (Widget *widget) {
	return (UiLaunchWindow::isWidgetType (widget) ? (UiLaunchWindow *) widget : NULL);
}

void UiLaunchWindow::setOpenCallback (Widget::EventCallback callback, void *callbackData) {
	openCallback = callback;
	openCallbackData = callbackData;
}

void UiLaunchWindow::populate () {
	UiConfiguration *uiconfig;
	UiText *uitext;
	Sprite *sprite;
	IconLabelWindow *icon;
	StdString name, text;

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);

	switch (uiType) {
		case UiLaunchWindow::SERVER_UI: {
			name.assign (uitext->getText (UiTextString::servers).capitalized ());
			text.assign (uitext->getText (UiTextString::serverUiDescription));

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (spriteGroup->getSprite (MainUi::SERVER_ICON), uitext->getCountText (0, UiTextString::serverConnected, UiTextString::serversConnected), UiConfiguration::CAPTION, uiconfig->primaryTextColor));
			noteIcons.push_back (icon);
			break;
		}
		case UiLaunchWindow::MEDIA_UI: {
			name.assign (uitext->getText (UiTextString::media).capitalized ());
			text.assign (uitext->getText (UiTextString::mediaUiDescription));

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (spriteGroup->getSprite (MainUi::MEDIA_SERVER_ICON), uitext->getCountText (0, UiTextString::mediaServerConnected, UiTextString::mediaServersConnected), UiConfiguration::CAPTION, uiconfig->primaryTextColor));
			noteIcons.push_back (icon);
			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (spriteGroup->getSprite (MainUi::STREAM_ICON), uitext->getCountText (0, UiTextString::videoFileAvailable, UiTextString::videoFilesAvailable), UiConfiguration::CAPTION, uiconfig->primaryTextColor));
			noteIcons.push_back (icon);
			break;
		}
		case UiLaunchWindow::MONITOR_UI: {
			name.assign (uitext->getText (UiTextString::monitorUiTitle));
			text.assign (uitext->getText (UiTextString::monitorUiDescription));

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (spriteGroup->getSprite (MainUi::DISPLAY_ICON), uitext->getCountText (0, UiTextString::monitorConnected, UiTextString::monitorsConnected), UiConfiguration::CAPTION, uiconfig->primaryTextColor));
			noteIcons.push_back (icon);
			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (spriteGroup->getSprite (MainUi::STREAM_ICON), uitext->getCountText (0, UiTextString::videoStreamAvailable, UiTextString::videoStreamsAvailable), UiConfiguration::CAPTION, uiconfig->primaryTextColor));
			noteIcons.push_back (icon);
			break;
		}
		case UiLaunchWindow::WEB_KIOSK_UI: {
			name.assign (uitext->getText (UiTextString::webKiosk).capitalized ());
			text.assign (uitext->getText (UiTextString::webKioskUiDescription));

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (spriteGroup->getSprite (MainUi::DISPLAY_ICON), uitext->getCountText (0, UiTextString::webKioskAvailable, UiTextString::webKiosksAvailable), UiConfiguration::CAPTION, uiconfig->primaryTextColor));
			noteIcons.push_back (icon);
			break;
		}
	}

	sprite = spriteGroup->getSprite (MainUi::UI_ICON);
	if (! sprite) {
		sprite = uiconfig->coreSprites.getSprite (UiConfiguration::SETTINGS_BUTTON);
	}
	iconImage = (Image *) addWidget (new Image (sprite));
	nameLabel = (Label *) addWidget (new Label (name, UiConfiguration::HEADLINE, uiconfig->primaryTextColor));

	descriptionText = (TextArea *) addWidget (new TextArea (UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor));
	descriptionText->setText (text);

	openButton = (Button *) addWidget (new Button (uitext->getText (UiTextString::open).uppercased ()));
	openButton->zLevel = 1;
	openButton->setMouseClickCallback (UiLaunchWindow::openButtonClicked, this);
	openButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	openButton->setMouseHoverTooltip (uitext->getText (UiTextString::uiLaunchOpenButtonTooltip));
}

void UiLaunchWindow::syncRecordStore (RecordStore *store) {
	App *app;
	UiText *uitext;
	IconLabelWindow *icon;
	int count;
	int64_t maxage;

	app = App::getInstance ();
	uitext = &(App::getInstance ()->uiText);

	maxage = app->prefsMap.find (App::prefsServerTimeout, App::defaultServerTimeout) * 1000;
	switch (uiType) {
		case UiLaunchWindow::SERVER_UI: {
			icon = noteIcons.at (0);
			count = store->countCommandRecords (SystemInterface::Command_AgentStatus, maxage);
			icon->setText (uitext->getCountText (count, UiTextString::serverConnected, UiTextString::serversConnected));
			break;
		}
		case UiLaunchWindow::MEDIA_UI: {
			icon = noteIcons.at (0);
			count = store->countAgentRecords ("mediaServerStatus", maxage);
			icon->setText (uitext->getCountText (count, UiTextString::mediaServerConnected, UiTextString::mediaServersConnected));

			icon = noteIcons.at (1);
			count = countMediaItems (store);
			icon->setText (uitext->getCountText (count, UiTextString::videoFileAvailable, UiTextString::videoFilesAvailable));
			break;
		}
		case UiLaunchWindow::MONITOR_UI: {
			icon = noteIcons.at (0);
			count = store->countAgentRecords ("monitorServerStatus", maxage);
			icon->setText (uitext->getCountText (count, UiTextString::monitorConnected, UiTextString::monitorsConnected));

			icon = noteIcons.at (1);
			count = countStreamItems (store);
			icon->setText (uitext->getCountText (count, UiTextString::videoStreamAvailable, UiTextString::videoStreamsAvailable));
			break;
		}
		case UiLaunchWindow::WEB_KIOSK_UI: {
			icon = noteIcons.at (0);
			count = store->countRecords (WebKioskUi::matchWebKioskAgentStatus, NULL, maxage);
			icon->setText (uitext->getCountText (count, UiTextString::webKioskAvailable, UiTextString::webKiosksAvailable));
			break;
		}
	}

	Panel::syncRecordStore (store);
}

int UiLaunchWindow::countMediaItems (RecordStore *store) {
	int sum;

	sum = 0;
	store->processAgentRecords ("mediaServerStatus", UiLaunchWindow::addMediaCount, &sum);
	return (sum);
}

void UiLaunchWindow::addMediaCount (void *sumPtr, Json *record, const StdString &recordId) {
	SystemInterface *interface;
	Json serverstatus;
	int *sum;

	sum = (int *) sumPtr;
	interface = &(App::getInstance ()->systemInterface);
	if (interface->getCommandObjectParam (record, "mediaServerStatus", &serverstatus)) {
		*sum += serverstatus.getNumber ("mediaCount", 0);
	}
}

int UiLaunchWindow::countStreamItems (RecordStore *store) {
	int sum;

	sum = 0;
	store->processAgentRecords ("streamServerStatus", UiLaunchWindow::addStreamCount, &sum);
	return (sum);
}

void UiLaunchWindow::addStreamCount (void *sumPtr, Json *record, const StdString &recordId) {
	SystemInterface *interface;
	Json serverstatus;
	int *sum;

	sum = (int *) sumPtr;
	interface = &(App::getInstance ()->systemInterface);
	if (interface->getCommandObjectParam (record, "streamServerStatus", &serverstatus)) {
		*sum += serverstatus.getNumber ("streamCount", 0);
	}
}

bool UiLaunchWindow::isReadyState (int uiType, RecordStore *store) {
	App *app;
	int64_t maxage;

	app = App::getInstance ();
	maxage = app->prefsMap.find (App::prefsServerTimeout, App::defaultServerTimeout) * 1000;
	switch (uiType) {
		case UiLaunchWindow::MEDIA_UI: {
			if (store->countAgentRecords ("mediaServerStatus", maxage) > 0) {
				return (true);
			}
			break;
		}
		case UiLaunchWindow::MONITOR_UI: {
			if (store->countAgentRecords ("monitorServerStatus", maxage) > 0) {
				return (true);
			}
			break;
		}
		case UiLaunchWindow::WEB_KIOSK_UI: {
			if (store->countRecords (WebKioskUi::matchWebKioskAgentStatus, NULL, maxage) > 0) {
				return (true);
			}
			break;
		}
	}

	return (false);
}

void UiLaunchWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	IconLabelWindow *icon;
	std::vector<IconLabelWindow *>::iterator i, end;
	float x, y;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = uiconfig->paddingSize;
	y = uiconfig->paddingSize;
	iconImage->position.assign (x, y);

	x += iconImage->width + uiconfig->marginSize;
	nameLabel->position.assign (x, y + (iconImage->height / 2.0f) - (nameLabel->height / 2.0f));

	x = uiconfig->paddingSize;
	y += iconImage->height + uiconfig->marginSize;

	descriptionText->position.assign (x, y);
	y += descriptionText->height + uiconfig->marginSize;

	i = noteIcons.begin ();
	end = noteIcons.end ();
	while (i != end) {
		icon = *i;
		icon->position.assign (x, y);
		y += icon->height + uiconfig->marginSize;
		++i;
	}

	openButton->position.assign (x, y);

	resetSize ();

	x = width - uiconfig->paddingSize;
	x -= openButton->width;
	openButton->position.assign (x, y);
}

void UiLaunchWindow::openButtonClicked (void *windowPtr, Widget *widgetPtr) {
	UiLaunchWindow *window;

	window = (UiLaunchWindow *) windowPtr;
	if (window->openCallback) {
		window->openCallback (window->openCallbackData, window);
	}
}
