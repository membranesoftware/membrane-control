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
// Panel that executes agent configuration operations

#ifndef AGENT_CONFIGURATION_WINDOW_H
#define AGENT_CONFIGURATION_WINDOW_H

#include <vector>
#include "StdString.h"
#include "SpriteGroup.h"
#include "Image.h"
#include "Json.h"
#include "HashMap.h"
#include "Label.h"
#include "Button.h"
#include "TextArea.h"
#include "IconLabelWindow.h"
#include "ToggleWindow.h"
#include "StatsWindow.h"
#include "ActionWindow.h"
#include "HyperlinkWindow.h"
#include "Panel.h"

class AgentConfigurationWindow : public Panel {
public:
	AgentConfigurationWindow (const StdString &serverHostname, SpriteGroup *linkUiSpriteGroup);
	virtual ~AgentConfigurationWindow ();

	// Read-only data members
	StdString agentId;
	StdString serverHostname;
	int serverTcpPort;

	// Set a callback function that should be invoked when the window's configure button is pressed
	void setConfigureCallback (Widget::EventCallback callback, void *callbackData);

	// Set a callback function that should be invoked when the window completes a successful configure invocation
	void setConfigureSuccessCallback (Widget::EventCallback callback, void *callbackData);

	// Set the window to reflect an uninstalled server state
	void setUninstalled ();

	// Set the window's title text
	void setTitle (const StdString &titleText);

	// Set the window to reflect a stopped server state
	void setStopped ();

	// Update the window's state by reading fields from a configuration status map, as obtained by the AgentControl::readSystemAgentConfiguration method
	void setSystemAgentConfiguration (HashMap *agentConfig, const StdString &agentId);

	// Update the window's state by reading fields from an AgentStatus command object
	void setAgentStatus (Json *agentStatusCommand);

	// Update the window's state by reading fields from an AgentConfiguration command object
	void setAgentConfiguration (Json *agentConfigurationCommand);

	// Invoke a GetAgentConfiguration command targeting the window's server hostname and update the window's content if successful
	void invokeGetAgentConfiguration ();

	// Return a newly created ActionWindow object with configure action content appropriate for the window's server type, or NULL if no such window could be created
	ActionWindow *createConfigureActionWindow ();

	// Invoke an UpdateAgentConfiguration command targeting the window's server hostname, using values from the provided ActionWindow object as provided by createConfigureActionWindow
	void invokeUpdateAgentConfiguration (ActionWindow *action);

	// Invoke a GetStatus command targeting the window's server hostname and update the window's content if successful
	void invokeGetStatus ();

	// Return a boolean value indicating if the provided Widget is a member of this class
	static bool isWidgetType (Widget *widget);

	// Return a typecasted pointer to the provided widget, or NULL if the widget does not appear to be of the correct type
	static AgentConfigurationWindow *castWidget (Widget *widget);

	// Callback functions
	static void configureButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void invokeGetAgentConfigurationComplete (void *windowPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand);
	static void invokeUpdateAgentConfigurationComplete (void *windowPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand);
	static void invokeGetStatusComplete (void *windowPtr, int64_t jobId, int jobResult, const StdString &agentId, Json *command, Json *responseCommand);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void resetLayout ();

private:
	// Populate widgets as appropriate for the window's initial state
	void populate ();

	SpriteGroup *spriteGroup;
	Json agentConfiguration;
	Image *iconImage;
	Label *nameLabel;
	TextArea *descriptionText;
	std::vector<IconLabelWindow *> noteIcons;
	HyperlinkWindow *helpLinkWindow;
	HyperlinkWindow *updateLinkWindow;
	StatsWindow *statsWindow;
	Button *configureButton;
	Widget::EventCallback configureCallback;
	void *configureCallbackData;
	Widget::EventCallback configureSuccessCallback;
	void *configureSuccessCallbackData;
};

#endif
