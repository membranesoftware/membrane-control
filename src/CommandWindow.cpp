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
#include <vector>
#include "Result.h"
#include "ClassId.h"
#include "Log.h"
#include "StdString.h"
#include "StringList.h"
#include "App.h"
#include "UiConfiguration.h"
#include "UiText.h"
#include "AgentControl.h"
#include "Json.h"
#include "SystemInterface.h"
#include "Sprite.h"
#include "SpriteGroup.h"
#include "Widget.h"
#include "Color.h"
#include "Image.h"
#include "CommandUi.h"
#include "CommandWindow.h"

CommandWindow::CommandWindow (int64_t storeId, SpriteGroup *commandUiSpriteGroup)
: Panel ()
, storeId (storeId)
, commandId (-1)
, sprites (commandUiSpriteGroup)
, iconImage (NULL)
, nameLabel (NULL)
, targetAgentsIcon (NULL)
, parameterIcon (NULL)
, saveButton (NULL)
, executeButton (NULL)
, deleteButton (NULL)
, nameClickCallback (NULL)
, nameClickCallbackData (NULL)
, saveClickCallback (NULL)
, saveClickCallbackData (NULL)
, executeClickCallback (NULL)
, executeClickCallbackData (NULL)
, deleteClickCallback (NULL)
, deleteClickCallbackData (NULL)
{
	UiConfiguration *uiconfig;
	UiText *uitext;

	classId = ClassId::CommandWindow;
	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);

	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setCornerRadius (uiconfig->cornerRadius);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	iconImage = (Image *) addWidget (new Image (uiconfig->coreSprites.getSprite (UiConfiguration::LargeCommandIconSprite)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::HeadlineFont, uiconfig->primaryTextColor));
	nameLabel->setMouseClickCallback (CommandWindow::nameLabelClicked, this);

	targetAgentsIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallCommandIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	targetAgentsIcon->setPadding (0.0f, 0.0f);
	targetAgentsIcon->setMouseHoverTooltip (uitext->getText (UiTextString::servers).capitalized ());
	targetAgentsIcon->isVisible = false;

	parameterIcon = (IconLabelWindow *) addWidget (new IconLabelWindow (uiconfig->coreSprites.getSprite (UiConfiguration::SmallCommandIconSprite), StdString (""), UiConfiguration::CaptionFont, uiconfig->lightPrimaryTextColor));
	parameterIcon->setPadding (0.0f, 0.0f);
	parameterIcon->setMouseHoverTooltip (uitext->getText (UiTextString::commandDetails).capitalized ());
	parameterIcon->isVisible = false;

	saveButton = (Button *) addWidget (new Button (StdString (""), sprites->getSprite (CommandUi::SaveCommandButtonSprite)));
	saveButton->setMouseClickCallback (CommandWindow::saveButtonClicked, this);
	saveButton->setImageColor (uiconfig->flatButtonTextColor);
	saveButton->setMouseHoverTooltip (uitext->getText (UiTextString::saveCommandTooltip));
	saveButton->isVisible = false;

	executeButton = (Button *) addWidget (new Button (StdString (""), sprites->getSprite (CommandUi::ExecuteCommandButtonSprite)));
	executeButton->setMouseClickCallback (CommandWindow::executeButtonClicked, this);
	executeButton->setImageColor (uiconfig->flatButtonTextColor);
	executeButton->setMouseHoverTooltip (uitext->getText (UiTextString::executeCommand).capitalized ());
	executeButton->isVisible = false;

	deleteButton = (Button *) addWidget (new Button (StdString (""), uiconfig->coreSprites.getSprite (UiConfiguration::DeleteButtonSprite)));
	deleteButton->setMouseClickCallback (CommandWindow::deleteButtonClicked, this);
	deleteButton->setImageColor (uiconfig->flatButtonTextColor);
	deleteButton->setMouseHoverTooltip (uitext->getText (UiTextString::deleteCommand).capitalized ());
	deleteButton->isVisible = false;

	refreshLayout ();
}

CommandWindow::~CommandWindow () {

}

StdString CommandWindow::toStringDetail () {
	return (StdString (" CommandWindow"));
}

bool CommandWindow::isWidgetType (Widget *widget) {
	return (widget && (widget->classId == ClassId::CommandWindow));
}

CommandWindow *CommandWindow::castWidget (Widget *widget) {
	return (CommandWindow::isWidgetType (widget) ? (CommandWindow *) widget : NULL);
}

void CommandWindow::setNameClickCallback (Widget::EventCallback callback, void *callbackData) {
	UiText *uitext;

	uitext = &(App::instance->uiText);
	nameClickCallback = callback;
	nameClickCallbackData = callbackData;
	if (nameClickCallback) {
		nameLabel->setMouseHoverTooltip (uitext->getText (UiTextString::clickRenameTooltip));
	}
}

void CommandWindow::setSaveClickCallback (Widget::EventCallback callback, void *callbackData) {
	saveClickCallback = callback;
	saveClickCallbackData = callbackData;
	saveButton->isVisible = saveClickCallback ? true : false;
	refreshLayout ();
}

void CommandWindow::setExecuteClickCallback (Widget::EventCallback callback, void *callbackData) {
	executeClickCallback = callback;
	executeClickCallbackData = callbackData;
	executeButton->isVisible = executeClickCallback ? true : false;
	refreshLayout ();
}

void CommandWindow::setDeleteClickCallback (Widget::EventCallback callback, void *callbackData) {
	deleteClickCallback = callback;
	deleteClickCallbackData = callbackData;
	deleteButton->isVisible = deleteClickCallback ? true : false;
	refreshLayout ();
}

void CommandWindow::refreshCommandData () {
	UiConfiguration *uiconfig;
	UiText *uitext;
	AgentControl *agentcontrol;
	CommandStore *store;
	SystemInterface *interface;
	StringList names;
	StringList::iterator i, end;
	StdString name, text;
	int targeticon, paramicon, emptyagentcount;

	uiconfig = &(App::instance->uiConfig);
	uitext = &(App::instance->uiText);
	agentcontrol = &(App::instance->agentControl);
	store = &(App::instance->agentControl.commandStore);
	interface = &(App::instance->systemInterface);

	if (! store->getCommand (storeId, &commandRecord)) {
		return;
	}

	commandId = interface->getCommandId (commandRecord.command);
	if (commandRecord.targetAgentIds.empty ()) {
		targetAgentsIcon->isVisible = false;
	}
	else {
		emptyagentcount = 0;
		i = commandRecord.targetAgentIds.begin ();
		end = commandRecord.targetAgentIds.end ();
		while (i != end) {
			name = agentcontrol->getAgentDisplayName (*i);
			if (name.empty ()) {
				++emptyagentcount;
			}
			else {
				names.push_back (name);
			}
			++i;
		}
		if (names.empty ()) {
			text.sprintf ("<%s>", uitext->getCountText (emptyagentcount, UiTextString::unknownServer, UiTextString::unknownServers).c_str ());
		}
		else {
			text.assign ("");
			if (emptyagentcount > 0) {
				text.sprintf ("<%s>, ", uitext->getCountText (emptyagentcount, UiTextString::unknownServer, UiTextString::unknownServers).c_str ());
			}
			names.sort (StringList::compareCaseInsensitiveAscending);
			text.append (names.join (", "));
			Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * 0.33f, StdString::createSprintf ("... (%i)", (int) names.size ()));
		}
		targetAgentsIcon->setText (text);
		targetAgentsIcon->isVisible = true;
	}

	targeticon = UiConfiguration::SmallCommandIconSprite;
	paramicon = UiConfiguration::SmallCommandIconSprite;
	name.assign (commandRecord.name);
	text.assign ("");
	switch (commandId) {
		case SystemInterface::CommandId_ClearDisplay: {
			targeticon = UiConfiguration::SmallDisplayIconSprite;
			if (name.empty ()) {
				name = uitext->getText (UiTextString::clearMonitors).capitalized ();
			}
			break;
		}
		case SystemInterface::CommandId_PlayMedia: {
			targeticon = UiConfiguration::SmallDisplayIconSprite;
			paramicon = UiConfiguration::SmallStreamIconSprite;
			if (name.empty ()) {
				name = uitext->getText (UiTextString::playMedia).capitalized ();
			}
			text = interface->getCommandStringParam (commandRecord.command, "mediaName", "");
			break;
		}
		case SystemInterface::CommandId_ShowWebUrl: {
			targeticon = UiConfiguration::SmallDisplayIconSprite;
			paramicon = UiConfiguration::SmallPlaylistIconSprite;
			if (name.empty ()) {
				name = uitext->getText (UiTextString::showWebsite).capitalized ();
			}
			text = interface->getCommandStringParam (commandRecord.command, "url", "");
			break;
		}
		case SystemInterface::CommandId_CreateMediaDisplayIntent: {
			targeticon = UiConfiguration::SmallDisplayIconSprite;
			paramicon = UiConfiguration::SmallStreamIconSprite;
			if (name.empty ()) {
				name = uitext->getText (UiTextString::runMediaPlaylist).capitalized ();
			}
			text.sprintf ("[%s] ", uitext->getText (UiTextString::playlist).capitalized ().c_str ());
			text.append (interface->getCommandStringParam (commandRecord.command, "displayName", ""));
			break;
		}
		case SystemInterface::CommandId_CreateWebDisplayIntent: {
			targeticon = UiConfiguration::SmallDisplayIconSprite;
			paramicon = UiConfiguration::SmallPlaylistIconSprite;
			if (name.empty ()) {
				name = uitext->getText (UiTextString::runWebsitePlaylist).capitalized ();
			}
			text.sprintf ("[%s] ", uitext->getText (UiTextString::playlist).capitalized ().c_str ());
			text.append (interface->getCommandStringParam (commandRecord.command, "displayName", ""));
			break;
		}
		default: {
			if (name.empty ()) {
				name = interface->getCommandName (commandRecord.command);
			}
			break;
		}
	}

	targetAgentsIcon->setIconSprite (uiconfig->coreSprites.getSprite (targeticon));
	parameterIcon->setIconSprite (uiconfig->coreSprites.getSprite (paramicon));
	if (name.empty ()) {
		name.sprintf ("Command %i", commandId);
	}
	nameLabel->setText (name);
	commandName.assign (name);

	if (text.empty ()) {
		parameterIcon->isVisible = false;
	}
	else {
		Label::truncateText (&text, UiConfiguration::CaptionFont, App::instance->rootPanel->width * 0.33f, StdString::createSprintf ("..."));
		parameterIcon->setText (text);
		parameterIcon->isVisible = true;
	}
	refreshLayout ();
}

void CommandWindow::refreshLayout () {
	UiConfiguration *uiconfig;
	float x, y, x0, x2, y2;

	uiconfig = &(App::instance->uiConfig);
	x = widthPadding;
	y = heightPadding;
	x0 = x;
	x2 = 0.0f;
	y2 = 0.0f;
	iconImage->flowRight (&x, y, &x2, &y2);
	nameLabel->flowDown (x, &y, &x2, &y2);

	x0 += uiconfig->marginSize;
	x = x0;
	y = y2 + uiconfig->marginSize;
	if (targetAgentsIcon->isVisible) {
		x2 = 0.0f;
		y2 = 0.0f;
		targetAgentsIcon->flowDown (x, &y, &x2, &y2);
	}
	y = y2;
	if (parameterIcon->isVisible) {
		parameterIcon->flowDown (x, &y, &x2, &y2);
	}

	x = x0;
	y = y2 + uiconfig->marginSize;
	x2 = 0.0f;
	y2 = 0.0f;
	if (deleteButton->isVisible) {
		deleteButton->flowRight (&x, y, &x2, &y2);
	}
	if (saveButton->isVisible) {
		saveButton->flowRight (&x, y, &x2, &y2);
	}
	if (executeButton->isVisible) {
		executeButton->flowRight (&x, y, &x2, &y2);
	}

	resetSize ();

	x = width - widthPadding;
	if (executeButton->isVisible) {
		executeButton->flowLeft (&x);
	}
	if (saveButton->isVisible) {
		saveButton->flowLeft (&x);
	}
	if (deleteButton->isVisible) {
		deleteButton->flowLeft (&x);
	}
}

void CommandWindow::nameLabelClicked (void *windowPtr, Widget *widgetPtr) {
	CommandWindow *window;

	window = (CommandWindow *) windowPtr;
	if (window->nameClickCallback) {
		window->nameClickCallback (window->nameClickCallbackData, window);
	}
}

void CommandWindow::saveButtonClicked (void *windowPtr, Widget *widgetPtr) {
	CommandWindow *window;

	window = (CommandWindow *) windowPtr;
	if (window->saveClickCallback) {
		window->saveClickCallback (window->saveClickCallbackData, window);
	}
}

void CommandWindow::executeButtonClicked (void *windowPtr, Widget *widgetPtr) {
	CommandWindow *window;

	window = (CommandWindow *) windowPtr;
	if (window->executeClickCallback) {
		window->executeClickCallback (window->executeClickCallbackData, window);
	}
}

void CommandWindow::deleteButtonClicked (void *windowPtr, Widget *widgetPtr) {
	CommandWindow *window;

	window = (CommandWindow *) windowPtr;
	if (window->deleteClickCallback) {
		window->deleteClickCallback (window->deleteClickCallbackData, window);
	}
}
