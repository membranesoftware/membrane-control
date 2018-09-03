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
#include <map>
#include <list>
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "UiText.h"
#include "Util.h"
#include "Widget.h"
#include "Color.h"
#include "Image.h"
#include "Json.h"
#include "Panel.h"
#include "Label.h"
#include "TextArea.h"
#include "Toggle.h"
#include "ToggleWindow.h"
#include "StatsWindow.h"
#include "SystemInterface.h"
#include "UiConfiguration.h"
#include "DisplayServerWindow.h"

DisplayServerWindow::DisplayServerWindow (Json *agentStatus, Sprite *iconSprite, int cardLayout, const StdString &referenceIntentId)
: Panel ()
, isSelected (false)
, referenceIntentId (referenceIntentId)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionText (NULL)
, statsWindow (NULL)
, selectToggle (NULL)
, selectStateChangeCallback (NULL)
, selectStateChangeCallbackData (NULL)
, assignButton (NULL)
, unassignButton (NULL)
{
	SystemInterface *interface;
	UiText *uitext;
	UiConfiguration *uiconfig;
	Json serverstatus;

	uiconfig = &(App::getInstance ()->uiConfig);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	interface = &(App::getInstance ()->systemInterface);
	uitext = &(App::getInstance ()->uiText);
	agentId = interface->getCommandStringParam (agentStatus, "id", "");
	agentName = interface->getCommandAgentName (agentStatus);
	agentAddress = interface->getCommandAgentAddress (agentStatus);
	agentVersion = interface->getCommandStringParam (agentStatus, "version", "");
	agentUptime = interface->getCommandStringParam (agentStatus, "uptime", "");
	if (interface->getCommandObjectParam (agentStatus, "displayServerStatus", &serverstatus)) {
		controllerId = serverstatus.getString ("controllerId", "");
	}

	iconImage = (Image *) addWidget (new Image (iconSprite));
	nameLabel = (Label *) addWidget (new Label (agentName, UiConfiguration::HEADLINE, uiconfig->primaryTextColor));

	descriptionText = (TextArea *) addWidget (new TextArea (UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor));
	descriptionText->setText (uitext->display.capitalized ());

	statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
	statsWindow->setPadding (uiconfig->paddingSize, 0.0f);

	selectToggle = (ToggleWindow *) addWidget (new ToggleWindow (new Toggle (), uitext->select.capitalized ()));
	selectToggle->setStateChangeCallback (DisplayServerWindow::selectToggleStateChanged, this);
	selectToggle->isVisible = false;

	assignButton = (Button *) addWidget (new Button (uitext->assign.uppercased ()));
	assignButton->setMouseClickCallback (DisplayServerWindow::assignButtonClicked, this);
	assignButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	assignButton->isVisible = false;

	unassignButton = (Button *) addWidget (new Button (uitext->unassign.uppercased ()));
	unassignButton->setMouseClickCallback (DisplayServerWindow::unassignButtonClicked, this);
	unassignButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	unassignButton->isVisible = false;

	setLayout (cardLayout);
}

DisplayServerWindow::~DisplayServerWindow () {

}

StdString DisplayServerWindow::toStringDetail () {
	return (StdString::createSprintf (" DisplayServerWindow agentId=\"%s\"", agentId.c_str ()));
}

bool DisplayServerWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" DisplayServerWindow agentId="));
}

DisplayServerWindow *DisplayServerWindow::castWidget (Widget *widget) {
	return (DisplayServerWindow::isWidgetType (widget) ? (DisplayServerWindow *) widget : NULL);
}

void DisplayServerWindow::setLayout (int cardLayout) {
	if (cardLayout == layout) {
		return;
	}

	layout = cardLayout;
	resetLayout ();
}

void DisplayServerWindow::syncRecordStore (RecordStore *store) {
	SystemInterface *interface;
	Json *record, serverstatus;

	interface = &(App::getInstance ()->systemInterface);
	record = store->findRecord (agentId, SystemInterface::Command_AgentStatus);
	if (! record) {
		return;
	}
	if (! interface->getCommandObjectParam (record, "displayServerStatus", &serverstatus)) {
		return;
	}

	controllerId = serverstatus.getString ("controllerId", "");
	if (! controllerId.empty ()) {
		record = store->findRecord (controllerId, SystemInterface::Command_IntentState);
		if (! record) {
			controllerName.assign ("");
		}
		else {
			controllerName = interface->getCommandStringParam (record, "displayName", "");
		}
	}

	resetLayout ();
	Panel::syncRecordStore (store);
}

void DisplayServerWindow::setSelectStateChangeCallback (Widget::EventCallback callback, void *callbackData) {
	selectStateChangeCallback = callback;
	selectStateChangeCallbackData = callbackData;
}

void DisplayServerWindow::resetLayout () {
	UiText *uitext;
	UiConfiguration *uiconfig;
	float x, y, x0, y0;

	uitext = &(App::getInstance ()->uiText);
	uiconfig = &(App::getInstance ()->uiConfig);

	switch (layout) {
		case DisplayServerWindow::LINK_UI_CARD: {
			statsWindow->setItem (uitext->address.capitalized (), agentAddress);
			break;
		}
	}

	if (controllerId.empty ()) {
		// TODO: Possibly add an "unassigned" item here
//		lines.push_back (uitext->unassigned.capitalized ());
	}
	else {
		statsWindow->setItem (uitext->assignedTo.capitalized (), controllerName.empty () ? uitext->unknown.capitalized ().c_str () : controllerName.c_str ());
	}

	x = widthPadding;
	y = heightPadding;
	x0 = x;
	y0 = y;

	iconImage->position.assign (x, y);
	x += iconImage->width + uiconfig->marginSize;

	nameLabel->position.assign (x, nameLabel->getLinePosition (y));
	y += nameLabel->maxLineHeight + uiconfig->textLineHeightMargin;

	if (y < (y0 + iconImage->height)) {
		y = y0 + iconImage->height;
	}

	x = x0;
	switch (layout) {
		case DisplayServerWindow::LINK_UI_CARD:
		case DisplayServerWindow::INTENT_UI_CARD: {
			descriptionText->position.assign (x + uiconfig->paddingSize, y);
			descriptionText->isVisible = true;
			y += descriptionText->height + uiconfig->marginSize;
			break;
		}
		case DisplayServerWindow::DISPLAY_UI_CARD: {
			descriptionText->isVisible = false;
			break;
		}
	}

	x = x0;
	statsWindow->position.assign (x, y);
	y += statsWindow->height + uiconfig->marginSize;

	switch (layout) {
		case DisplayServerWindow::LINK_UI_CARD:
		case DisplayServerWindow::INTENT_UI_CARD: {
			selectToggle->isVisible = false;
			break;
		}
		case DisplayServerWindow::DISPLAY_UI_CARD: {
			selectToggle->position.assign (x, y);
			selectToggle->isVisible = true;
			y += selectToggle->height + uiconfig->marginSize;
			break;
		}
	}

	x = x0;
	switch (layout) {
		case DisplayServerWindow::LINK_UI_CARD:
		case DisplayServerWindow::DISPLAY_UI_CARD: {
			assignButton->isVisible = false;
			unassignButton->isVisible = false;
			break;
		}
		case DisplayServerWindow::INTENT_UI_CARD: {
			if (referenceIntentId.empty () && controllerId.empty ()) {
				assignButton->isVisible = false;
				unassignButton->isVisible = false;
			}
			else if (referenceIntentId.equals (controllerId)) {
				assignButton->isVisible = false;

				unassignButton->position.assign (x, y);
				unassignButton->isVisible = true;
				x += unassignButton->width + uiconfig->marginSize;
			}
			else {
				unassignButton->isVisible = false;

				assignButton->position.assign (x, y);
				assignButton->isVisible = true;
				x += assignButton->width + uiconfig->marginSize;
			}

			break;
		}
	}

	resetSize ();

	x = width - uiconfig->marginSize;
	if (assignButton->isVisible) {
		x -= assignButton->width;
		assignButton->position.assignX (x);
	}
	if (unassignButton->isVisible) {
		x -= unassignButton->width;
		unassignButton->position.assignX (x);
	}
}

void DisplayServerWindow::selectToggleStateChanged (void *windowPtr, Widget *widgetPtr) {
	DisplayServerWindow *window;
	ToggleWindow *toggle;

	window = (DisplayServerWindow *) windowPtr;
	toggle = (ToggleWindow *) widgetPtr;
	window->isSelected = toggle->isChecked;
	if (window->selectStateChangeCallback) {
		window->selectStateChangeCallback (window->selectStateChangeCallbackData, window);
	}
}

void DisplayServerWindow::assignButtonClicked (void *windowPtr, Widget *widgetPtr) {
	DisplayServerWindow *window;
	App *app;
	Json *cmd, *params;
	int result;

	window = (DisplayServerWindow *) windowPtr;
	app = App::getInstance ();

	params = new Json ();
	params->set ("id", window->referenceIntentId);

	// TODO: Use the intent's priority value here
	params->set ("priority", 10);

	cmd = app->createCommand ("SetController", SystemInterface::Constant_Display, params);
	if (! cmd) {
		return;
	}

	window->retain ();
	result = app->agentControl.invokeCommand (window->agentId, cmd, DisplayServerWindow::setControllerComplete, window, NULL, window->referenceIntentId);
	if (result != Result::SUCCESS) {
		window->release ();
		return;
	}

	window->assignButton->isInputSuspended = true;
// TODO: Possibly implement a window wait state here
//	window->assignButton->isWaiting = true;
}

void DisplayServerWindow::setControllerComplete (void *windowPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	DisplayServerWindow *window;
	App *app;
	SystemInterface *interface;
	StdString recordid;
	Json item;

	window = (DisplayServerWindow *) windowPtr;
	if (window->isDestroyed) {
		window->release ();
		return;
	}

	if (responseCommand) {
		app = App::getInstance ();
		interface = &(app->systemInterface);
		if ((interface->getCommandId (responseCommand) == SystemInterface::Command_CommandResult) && interface->getCommandBooleanParam (responseCommand, "success", false) && interface->getCommandObjectParam (responseCommand, "item", &item)) {
			recordid = interface->getCommandStringParam (&item, "id", "");
			if (! recordid.empty ()) {
				app->agentControl.recordStore.addRecord (recordid, &item);
				app->shouldSyncRecordStore = true;
			}
		}
	}

	window->assignButton->isInputSuspended = false;
	window->unassignButton->isInputSuspended = false;
	window->release ();
}

void DisplayServerWindow::unassignButtonClicked (void *windowPtr, Widget *widgetPtr) {
	DisplayServerWindow *window;
	App *app;
	Json *cmd, *params;
	int result;

	window = (DisplayServerWindow *) windowPtr;
	app = App::getInstance ();

	params = new Json ();
	params->set ("id", "");
	params->set ("priority", 0);
	cmd = app->createCommand ("SetController", SystemInterface::Constant_Display, params);
	if (! cmd) {
		return;
	}

	window->retain ();
	result = app->agentControl.invokeCommand (window->agentId, cmd, DisplayServerWindow::setControllerComplete, window, NULL, window->referenceIntentId);
	if (result != Result::SUCCESS) {
		window->release ();
		return;
	}

	window->unassignButton->isInputSuspended = true;
// TODO: Possibly implement a window wait state here
//	window->unassignButton->isWaiting = true;
}
