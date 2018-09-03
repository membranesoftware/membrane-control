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
#include "UiConfiguration.h"
#include "Panel.h"
#include "Label.h"
#include "HashMap.h"
#include "Button.h"
#include "TextArea.h"
#include "Toggle.h"
#include "ToggleWindow.h"
#include "Image.h"
#include "Json.h"
#include "SystemInterface.h"
#include "IconLabelWindow.h"
#include "ActionWindow.h"
#include "HyperlinkWindow.h"
#include "LinkUi.h"
#include "AgentConfigurationWindow.h"

AgentConfigurationWindow::AgentConfigurationWindow (const StdString &serverHostname, SpriteGroup *linkUiSpriteGroup)
: Panel ()
, serverHostname (serverHostname)
, serverTcpPort (SystemInterface::Constant_DefaultTcpPort)
, spriteGroup (linkUiSpriteGroup)
, iconImage (NULL)
, nameLabel (NULL)
, descriptionText (NULL)
, helpLinkWindow (NULL)
, updateLinkWindow (NULL)
, statsWindow (NULL)
, configureButton (NULL)
, configureCallback (NULL)
, configureCallbackData (NULL)
, configureSuccessCallback (NULL)
, configureSuccessCallbackData (NULL)
{
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	setFillBg (true, uiconfig->mediumBackgroundColor);

	populate ();
	resetLayout ();
}

AgentConfigurationWindow::~AgentConfigurationWindow () {

}

StdString AgentConfigurationWindow::toStringDetail () {
  return (StdString (" AgentConfigurationWindow"));
}

bool AgentConfigurationWindow::isWidgetType (Widget *widget) {
	if (! widget) {
		return (false);
	}

	// This operation references output from the toStringDetail method, above
	return (widget->toString ().contains (" AgentConfigurationWindow"));
}

AgentConfigurationWindow *AgentConfigurationWindow::castWidget (Widget *widget) {
	return (AgentConfigurationWindow::isWidgetType (widget) ? (AgentConfigurationWindow *) widget : NULL);
}

void AgentConfigurationWindow::populate () {
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);

	iconImage = (Image *) addWidget (new Image (spriteGroup->getSprite (LinkUi::SERVER_CONFIGURE_ICON)));
	nameLabel = (Label *) addWidget (new Label (StdString (""), UiConfiguration::HEADLINE, uiconfig->primaryTextColor));

	descriptionText = (TextArea *) addWidget (new TextArea (UiConfiguration::CAPTION, uiconfig->lightPrimaryTextColor));
	descriptionText->setPadding (uiconfig->paddingSize, 0.0f);
	descriptionText->isVisible = false;

	helpLinkWindow = (HyperlinkWindow *) addWidget (new HyperlinkWindow ());
	helpLinkWindow->isVisible = false;

	updateLinkWindow = (HyperlinkWindow *) addWidget (new HyperlinkWindow ());
	updateLinkWindow->isVisible = false;

	statsWindow = (StatsWindow *) addWidget (new StatsWindow ());
	statsWindow->setPadding (uiconfig->paddingSize, 0.0f);
	statsWindow->setItem (uitext->version.capitalized (), "");
	statsWindow->isVisible = false;

	configureButton = (Button *) addWidget (new Button (uitext->configure.uppercased ()));
	configureButton->setMouseClickCallback (AgentConfigurationWindow::configureButtonClicked, this);
	configureButton->setRaised (true, uiconfig->raisedButtonBackgroundColor);
	configureButton->setMouseHoverTooltip (uitext->configureTooltip);
	configureButton->isVisible = false;
}

void AgentConfigurationWindow::setConfigureCallback (Widget::EventCallback callback, void *callbackData) {
	configureCallback = callback;
	configureCallbackData = callbackData;
}

void AgentConfigurationWindow::setConfigureSuccessCallback (Widget::EventCallback callback, void *callbackData) {
	configureSuccessCallback = callback;
	configureSuccessCallbackData = callbackData;
}

void AgentConfigurationWindow::setUninstalled () {
	UiConfiguration *uiconfig;
	UiText *uitext;

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);
	configureButton->isVisible = false;
	statsWindow->isVisible = false;
	updateLinkWindow->isVisible = false;

	nameLabel->setText (uitext->agentConfigurationUninstalledTitle.capitalized ());

	helpLinkWindow->setLink (uitext->learnMore.capitalized (), Util::getApplicationUrl ("membrane-server"));
	helpLinkWindow->isVisible = true;

	descriptionText->setPadding (uiconfig->paddingSize, uiconfig->paddingSize);
	descriptionText->setText (uitext->agentConfigurationUninstalledDescription);
	descriptionText->isVisible = true;

	resetLayout ();
}

void AgentConfigurationWindow::setTitle (const StdString &titleText) {
	nameLabel->setText (titleText);
	resetLayout ();
}

void AgentConfigurationWindow::setStopped () {
	UiConfiguration *uiconfig;
	UiText *uitext;
	StdString url;

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);
	configureButton->isVisible = false;
	statsWindow->isVisible = false;
	updateLinkWindow->isVisible = false;

	nameLabel->setText (uitext->agentConfigurationStoppedTitle.capitalized ());

	url.assign (Util::getHelpUrl ("start-membrane-server-raspberry-pi"));
#if PLATFORM_WINDOWS
	url.assign (Util::getHelpUrl ("start-membrane-server-windows"));
#endif
#if PLATFORM_MACOS
	url.assign (Util::getHelpUrl ("start-membrane-server-macos"));
#endif
	helpLinkWindow->setLink (uitext->agentConfigurationStoppedLinkText, url);
	helpLinkWindow->isVisible = true;

	descriptionText->setPadding (uiconfig->paddingSize, 0.0f);
	descriptionText->setText (uitext->agentConfigurationStoppedDescription);
	descriptionText->isVisible = true;

	resetLayout ();
}

void AgentConfigurationWindow::setSystemAgentConfiguration (HashMap *agentConfig, const StdString &agentId) {
	UiText *uitext;
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);

	serverTcpPort = agentConfig->find (App::prefsTcpPort, SystemInterface::Constant_DefaultTcpPort);
	nameLabel->setText (agentConfig->find (App::prefsApplicationName, "Membrane Server"));
	descriptionText->setPadding (uiconfig->paddingSize, 0.0f);
	descriptionText->setText (uitext->agentConfigurationActiveDescription);
	descriptionText->isVisible = true;

	resetLayout ();
}

void AgentConfigurationWindow::setAgentStatus (Json *agentStatusCommand) {
	SystemInterface *interface;
	UiConfiguration *uiconfig;
	UiText *uitext;
	StdString version, platform, updateurl;

	interface = &(App::getInstance ()->systemInterface);
	uiconfig = &(App::getInstance ()->uiConfig);
	uitext = &(App::getInstance ()->uiText);
	agentId.assign (interface->getCommandAgentId (agentStatusCommand));

	helpLinkWindow->isVisible = false;

	nameLabel->setText (interface->getCommandStringParam (agentStatusCommand, "displayName", ""));

	descriptionText->setPadding (uiconfig->paddingSize, 0.0f);
	descriptionText->setText (interface->getCommandStringParam (agentStatusCommand, "applicationName", ""));
	descriptionText->isVisible = true;

	version = interface->getCommandStringParam (agentStatusCommand, "version", "");
	if (! version.empty ()) {
		platform = interface->getCommandStringParam (agentStatusCommand, "platform", "");
		if (! platform.empty ()) {
			version.appendSprintf ("_%s", platform.c_str ());
			updateurl = Util::getUpdateUrl (version);
		}
		statsWindow->setItem (uitext->version.capitalized (), version);
	}
	statsWindow->isVisible = true;

	if (updateurl.empty ()) {
		updateLinkWindow->isVisible = false;
	}
	else {
		updateLinkWindow->setLink (uitext->checkForUpdates.capitalized (), updateurl);
		updateLinkWindow->isVisible = true;
	}

	resetLayout ();
}

void AgentConfigurationWindow::setAgentConfiguration (Json *agentConfigurationCommand) {
	SystemInterface *interface;
	UiText *uitext;
	Json cfg;

	interface = &(App::getInstance ()->systemInterface);
	uitext = &(App::getInstance ()->uiText);

	helpLinkWindow->isVisible = false;

	if (agentConfiguration.copy (agentConfigurationCommand) != Result::SUCCESS) {
		statsWindow->isVisible = false;
		descriptionText->isVisible = false;
	}
	else {
		nameLabel->setText (interface->getCommandStringParam (&agentConfiguration, "displayName", ""));

		statsWindow->setItem (uitext->enabled.capitalized (), interface->getCommandBooleanParam (&agentConfiguration, "isEnabled", true) ? uitext->yes.capitalized () : uitext->no.capitalized ());
		// TODO: Show description text for each stats item
		if (interface->getCommandObjectParam (&agentConfiguration, "mediaServerConfiguration", &cfg)) {
			statsWindow->setItem (uitext->sourceMediaPath.capitalized (), cfg.getString ("mediaPath", ""));
			statsWindow->setItem (uitext->mediaDataPath.capitalized (), cfg.getString ("dataPath", ""));
			statsWindow->setItem (uitext->mediaScanPeriod.capitalized (), StdString::createSprintf ("%i", cfg.getNumber ("scanPeriod", (int) 0)));
		}
		if (interface->getCommandObjectParam (&agentConfiguration, "streamServerConfiguration", &cfg)) {
			statsWindow->setItem (uitext->streamDataPath.capitalized (), cfg.getString ("dataPath", ""));
		}

		statsWindow->isVisible = true;
	}

	configureButton->isVisible = true;
	resetLayout ();
}

void AgentConfigurationWindow::resetLayout () {
	UiConfiguration *uiconfig;
	float x, y;

	uiconfig = &(App::getInstance ()->uiConfig);
	x = widthPadding;
	y = heightPadding;

	iconImage->position.assign (x, y);
	x += iconImage->width + uiconfig->marginSize;
	nameLabel->position.assign (x, y + (iconImage->height / 2.0f) - (nameLabel->height / 2.0f));

	x = widthPadding;
	y += iconImage->height;

	if (descriptionText->isVisible) {
		descriptionText->position.assign (x, y);
		y += descriptionText->height + uiconfig->marginSize;
	}
	if (statsWindow->isVisible) {
		statsWindow->position.assign (x, y);
		y += statsWindow->height + uiconfig->marginSize;
	}
	if (helpLinkWindow->isVisible) {
		helpLinkWindow->position.assign (x, helpLinkWindow->getLinePosition (y));
		y += helpLinkWindow->height + uiconfig->marginSize;
	}
	if (updateLinkWindow->isVisible) {
		updateLinkWindow->position.assign (x, updateLinkWindow->getLinePosition (y));
		y += updateLinkWindow->height + uiconfig->marginSize;
	}
	if (configureButton->isVisible) {
		configureButton->position.assign (x, y);
		y += configureButton->height + uiconfig->marginSize;
	}

	resetSize ();

	x = width - widthPadding;
	if (helpLinkWindow->isVisible) {
		x -= helpLinkWindow->width;
		helpLinkWindow->position.assignX (x);
		x -= uiconfig->marginSize;
	}

	x = width - widthPadding;
	if (updateLinkWindow->isVisible) {
		x -= updateLinkWindow->width;
		updateLinkWindow->position.assignX (x);
		x -= uiconfig->marginSize;
	}

	x = width - widthPadding;
	if (configureButton->isVisible) {
		x -= configureButton->width;
		configureButton->position.assignX (x);
		x -= uiconfig->marginSize;
	}
}

void AgentConfigurationWindow::configureButtonClicked (void *windowPtr, Widget *widgetPtr) {
	AgentConfigurationWindow *window;

	window = (AgentConfigurationWindow *) windowPtr;
	if (window->configureCallback) {
		window->configureCallback (window->configureCallbackData, window);
	}
}

ActionWindow *AgentConfigurationWindow::createConfigureActionWindow () {
	UiText *uitext;
	SystemInterface *interface;
	ActionWindow *action;
	Json cfg;

	uitext = &(App::getInstance ()->uiText);
	interface = &(App::getInstance ()->systemInterface);
	action = new ActionWindow (StdString::createSprintf ("%s: %s", uitext->configure.capitalized ().c_str (), nameLabel->text.c_str ()), uitext->save.uppercased ());
	action->sourceWidget.assign (this);

	action->addToggleOption (uitext->enabled.capitalized (), interface->getCommandBooleanParam (&agentConfiguration, "isEnabled", true));
	action->addTextFieldOption (uitext->displayName.capitalized (), StdString (""), interface->getCommandStringParam (&agentConfiguration, "displayName", ""));

	// TODO: Show description text for each option
	if (interface->getCommandObjectParam (&agentConfiguration, "mediaServerConfiguration", &cfg)) {
		action->addTextFieldOption (uitext->sourceMediaPath.capitalized (), StdString (""), cfg.getString ("mediaPath", ""));
		action->addTextFieldOption (uitext->mediaDataPath.capitalized (), StdString (""), cfg.getString ("dataPath", ""));
		action->addTextFieldOption (uitext->mediaScanPeriod.capitalized (), StdString (""), StdString::createSprintf ("%i", cfg.getNumber ("scanPeriod", (int) 0)));
	}
	if (interface->getCommandObjectParam (&agentConfiguration, "streamServerConfiguration", &cfg)) {
		action->addTextFieldOption (uitext->streamDataPath.capitalized (), StdString (""), cfg.getString ("dataPath", ""));
	}

	return (action);
}

void AgentConfigurationWindow::invokeUpdateAgentConfiguration (ActionWindow *action) {
	App *app;
	UiText *uitext;
	SystemInterface *interface;
	Json *params, *agentconfig, *cfg, *cmd;
	int result;

	app = App::getInstance ();
	uitext = &(app->uiText);
	interface = &(app->systemInterface);
	params = new Json ();

	agentconfig = new Json ();
	agentconfig->set ("isEnabled", action->getOptionValue (uitext->enabled, true));
	agentconfig->set ("displayName", action->getOptionValue (uitext->displayName, ""));
	if (interface->getCommandObjectParam (&agentConfiguration, "mediaServerConfiguration", NULL)) {
		cfg = new Json ();
		cfg->set ("mediaPath", action->getOptionValue (uitext->sourceMediaPath, ""));
		cfg->set ("dataPath", action->getOptionValue (uitext->mediaDataPath, ""));
		agentconfig->set ("mediaServerConfiguration", cfg);
	}
	if (interface->getCommandObjectParam (&agentConfiguration, "streamServerConfiguration", NULL)) {
		cfg = new Json ();
		cfg->set ("dataPath", action->getOptionValue (uitext->streamDataPath, ""));
		agentconfig->set ("streamServerConfiguration", cfg);
	}
	params->set ("agentConfiguration", agentconfig);

	cmd = app->createCommand ("UpdateAgentConfiguration", SystemInterface::Constant_DefaultCommandType, params);
	if (! cmd) {
		return;
	}

	retain ();
	if (agentId.empty ()) {
		result = app->agentControl.invokeCommand (serverHostname, serverTcpPort, cmd, AgentConfigurationWindow::invokeUpdateAgentConfigurationComplete, this);
	}
	else {
		result = app->agentControl.invokeCommand (agentId, cmd, AgentConfigurationWindow::invokeUpdateAgentConfigurationComplete, this);
	}
	if (result != Result::SUCCESS) {
		// TODO: Show an error message here
		Log::write (Log::ERR, "Failed to execute UpdateAgentConfiguration command; err=%i", result);
		release ();
		return;
	}

	setWaiting (true);
}

void AgentConfigurationWindow::invokeUpdateAgentConfigurationComplete (void *windowPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	AgentConfigurationWindow *window;
	SystemInterface *interface;

	window = (AgentConfigurationWindow *) windowPtr;
	interface = &(App::getInstance ()->systemInterface);
	if (responseCommand && (interface->getCommandId (responseCommand) == SystemInterface::Command_AgentConfiguration)) {
		window->setAgentConfiguration (responseCommand);
		if (window->configureSuccessCallback) {
			window->configureSuccessCallback (window->configureSuccessCallbackData, window);
		}
	}
	window->setWaiting (false);
	window->release ();
}

void AgentConfigurationWindow::invokeGetAgentConfiguration () {
	App *app;
	Json *cmd;
	int result;

	app = App::getInstance ();
	cmd = app->createCommand ("GetAgentConfiguration", SystemInterface::Constant_DefaultCommandType, NULL);
	if (cmd) {
		retain ();
		if (agentId.empty ()) {
			result = app->agentControl.invokeCommand (serverHostname, serverTcpPort, cmd, AgentConfigurationWindow::invokeGetAgentConfigurationComplete, this);
		}
		else {
			result = app->agentControl.invokeCommand (agentId, cmd, AgentConfigurationWindow::invokeGetAgentConfigurationComplete, this);
		}
		if (result != Result::SUCCESS) {
			// TODO: Show an error message here
			Log::write (Log::ERR, "Failed to execute GetAgentConfiguration command; err=%i", result);
			release ();
		}
	}
}

void AgentConfigurationWindow::invokeGetAgentConfigurationComplete (void *windowPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	AgentConfigurationWindow *window;
	App *app;
	SystemInterface *interface;

	window = (AgentConfigurationWindow *) windowPtr;
	app = App::getInstance ();
	interface = &(App::getInstance ()->systemInterface);
	if (responseCommand && (interface->getCommandId (responseCommand) == SystemInterface::Command_AgentConfiguration)) {
		window->setAgentConfiguration (responseCommand);
		app->shouldRefreshUi = true;
	}
	window->release ();
}

void AgentConfigurationWindow::invokeGetStatus () {
	App *app;
	Json *cmd;
	int result;

	app = App::getInstance ();
	cmd = app->createCommand ("GetStatus", SystemInterface::Constant_DefaultCommandType, NULL);
	if (cmd) {
		retain ();
		if (agentId.empty ()) {
			result = app->agentControl.invokeCommand (serverHostname, serverTcpPort, cmd, AgentConfigurationWindow::invokeGetStatusComplete, this);
		}
		else {
			result = app->agentControl.invokeCommand (agentId, cmd, AgentConfigurationWindow::invokeGetStatusComplete, this);
		}
		if (result != Result::SUCCESS) {
			// TODO: Show an error message here
			Log::write (Log::ERR, "Failed to execute GetStatus command; err=%i", result);
			release ();
		}
	}
}

void AgentConfigurationWindow::invokeGetStatusComplete (void *windowPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand) {
	AgentConfigurationWindow *window;
	App *app;
	SystemInterface *interface;

	window = (AgentConfigurationWindow *) windowPtr;
	app = App::getInstance ();
	interface = &(App::getInstance ()->systemInterface);
	if (responseCommand && (interface->getCommandId (responseCommand) == SystemInterface::Command_AgentStatus)) {
		window->setAgentStatus (responseCommand);
		app->shouldRefreshUi = true;
	}
	else {
		if (window->serverHostname.equals (AgentControl::localHostname)) {
			window->setStopped ();
		}
	}
	window->release ();
}
