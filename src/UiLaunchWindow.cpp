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
#include <vector>
#include "Result.h"
#include "ClassId.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "UiText.h"
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
, sprites (mainUiSpriteGroup)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionText (NULL)
, openButton (NULL)
, openCallback (NULL)
, openCallbackData (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;
	Sprite *sprite;
	IconLabelWindow *icon;
	StdString name, text;

	classId = ClassId::UiLaunchWindow;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	switch (uiType) {
		case UiLaunchWindow::ServerUi: {
			name.assign (uitext->getText (UiTextString::servers).capitalized ());
			text.assign (uitext->getText (UiTextString::serverUiDescription));

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (MainUi::ServerIconSprite), uitext->getCountText (0, UiTextString::serverConnected, UiTextString::serversConnected), UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
			noteIcons.push_back (icon);
			break;
		}
		case UiLaunchWindow::MediaUi: {
			name.assign (uitext->getText (UiTextString::media).capitalized ());
			text.assign (uitext->getText (UiTextString::mediaUiDescription));

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (sprites->getSprite (MainUi::ServerIconSprite), uitext->getCountText (0, UiTextString::mediaServerConnected, UiTextString::mediaServersConnected), UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
			noteIcons.push_back (icon);
			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallMediaIconSprite), uitext->getCountText (0, UiTextString::videoFileInCatalog, UiTextString::videoFilesInCatalog), UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
			noteIcons.push_back (icon);
			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallStreamIconSprite), uitext->getCountText (0, UiTextString::videoStreamPlayable, UiTextString::videoStreamsPlayable), UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
			noteIcons.push_back (icon);
			break;
		}
		case UiLaunchWindow::WebKioskUi: {
			name.assign (uitext->getText (UiTextString::webKiosk).capitalized ());
			text.assign (uitext->getText (UiTextString::webKioskUiDescription));

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallDisplayIconSprite), uitext->getCountText (0, UiTextString::monitorConnected, UiTextString::monitorsConnected), UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
			noteIcons.push_back (icon);
			break;
		}
		case UiLaunchWindow::CameraUi: {
			name.assign (uitext->getText (UiTextString::cameras).capitalized ());
			text.assign (uitext->getText (UiTextString::cameraUiDescription));

			icon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallCameraIconSprite), uitext->getCountText (0, UiTextString::cameraConnected, UiTextString::camerasConnected), UiConfiguration::CaptionFont, uiconfig->primaryTextColor));
			noteIcons.push_back (icon);
			break;
		}
	}

	sprite = sprites->getSprite (MainUi::UiIconSprite);
	if (! sprite) {
		sprite = uiconfig->coreSprites.getSprite (UiConfiguration::SettingsButtonSprite);
	}
	iconImage = (Image *) addWidget (new Image (sprite));
	nameLabel = (Label *) addWidget (new Label (name, UiConfiguration::HeadlineFont, uiconfig->primaryTextColor));

	descriptionText = (TextArea *) addWidget (new TextArea (UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	descriptionText->setText (text);

	openButton = (Button *) addWidget (new Button (uitext->getText (UiTextString::open).uppercased ()));
	openButton->zLevel = 1;
	openButton->setMouseClickCallback (UiLaunchWindow::openButtonClicked, this);
	openButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	openButton->setMouseHoverTooltip (uitext->getText (UiTextString::uiLaunchOpenButtonTooltip));

	refreshLayout ();
}

UiLaunchWindow::~UiLaunchWindow () {

}

StdString UiLaunchWindow::toStringDetail () {
  return (StdString (" UiLaunchWindow"));
}

bool UiLaunchWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::UiLaunchWindow));
}

UiLaunchWindow *UiLaunchWindow::castWidget (Widget *widget) {
	return (UiLaunchWindow::isWidgetType (widget) ? (UiLaunchWindow *) widget : NULL);
}

void UiLaunchWindow::setOpenCallback (Widget::EventCallback callback, void *callbackData) {
	openCallback = callback;
	openCallbackData = callbackData;
}

void UiLaunchWindow::syncRecordStore () {
	UiText *uitext;
	RecordStore *store;
	IconLabelWindow *icon;
	int count;
	int64_t maxage;

	uitext = &(App::instance->uiText);
	store = &(App::instance->agentControl.recordStore);

	maxage = App::instance->prefsMap.find (App::ServerTimeoutKey, App::defaultServerTimeout) * 1000;
	switch (uiType) {
		case UiLaunchWindow::ServerUi: {
			icon = noteIcons.at (0);
			count = store->countCommandRecords (SystemInterface::CommandId_AgentStatus, maxage);
			icon->setText (uitext->getCountText (count, UiTextString::serverConnected, UiTextString::serversConnected));
			break;
		}
		case UiLaunchWindow::MediaUi: {
			icon = noteIcons.at (0);
			count = store->countAgentRecords ("mediaServerStatus", maxage);
			icon->setText (uitext->getCountText (count, UiTextString::mediaServerConnected, UiTextString::mediaServersConnected));

			icon = noteIcons.at (1);
			count = countMediaItems (store);
			icon->setText (uitext->getCountText (count, UiTextString::videoFileInCatalog, UiTextString::videoFilesInCatalog));

			icon = noteIcons.at (2);
			count = countStreamItems (store);
			icon->setText (uitext->getCountText (count, UiTextString::videoStreamPlayable, UiTextString::videoStreamsPlayable));
			break;
		}
		case UiLaunchWindow::WebKioskUi: {
			icon = noteIcons.at (0);
			count = store->countRecords (WebKioskUi::matchWebKioskAgentStatus, NULL, maxage);
			icon->setText (uitext->getCountText (count, UiTextString::monitorConnected, UiTextString::monitorsConnected));
			break;
		}
		case UiLaunchWindow::CameraUi: {
			icon = noteIcons.at (0);
			count = store->countAgentRecords ("cameraServerStatus", maxage);
			icon->setText (uitext->getCountText (count, UiTextString::cameraConnected, UiTextString::camerasConnected));
			break;
		}
	}

	Panel::syncRecordStore ();
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
	interface = &(App::instance->systemInterface);
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
	interface = &(App::instance->systemInterface);
	if (interface->getCommandObjectParam (record, "streamServerStatus", &serverstatus)) {
		*sum += serverstatus.getNumber ("streamCount", 0);
	}
}

bool UiLaunchWindow::isReadyState (int uiType, RecordStore *store) {
	int64_t maxage;

	maxage = App::instance->prefsMap.find (App::ServerTimeoutKey, App::defaultServerTimeout) * 1000;
	switch (uiType) {
		case UiLaunchWindow::MediaUi: {
			if (store->countAgentRecords ("mediaServerStatus", maxage) > 0) {
				return (true);
			}
			break;
		}
		case UiLaunchWindow::WebKioskUi: {
			if (store->countRecords (WebKioskUi::matchWebKioskAgentStatus, NULL, maxage) > 0) {
				return (true);
			}
			break;
		}
		case UiLaunchWindow::CameraUi: {
			if (store->countAgentRecords ("cameraServerStatus", maxage) > 0) {
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

	uiconfig = &(App::instance->uiConfig);
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
