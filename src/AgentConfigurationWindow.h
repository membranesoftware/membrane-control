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
// Panel that shows an agent's configuration and controls for changing configuration fields

#ifndef AGENT_CONFIGURATION_WINDOW_H
#define AGENT_CONFIGURATION_WINDOW_H

#include "StdString.h"
#include "Json.h"
#include "Label.h"
#include "Image.h"
#include "Button.h"
#include "Toggle.h"
#include "RecordStore.h"
#include "ActionWindow.h"
#include "Panel.h"

class AgentConfigurationWindow : public Panel {
public:
	AgentConfigurationWindow (const StdString &agentId);
	virtual ~AgentConfigurationWindow ();

	static const int MediaScanPeriods[]; // seconds

	// Read-write data members
	Widget::EventCallbackContext expandStateChangeCallback;

	// Read-only data members
	StdString agentId;
	Json agentConfiguration;
	bool isExpanded;

	// Set the window's expand state, then execute any expand state change callback that might be configured unless shouldSkipStateChangeCallback is true
	void setExpanded (bool expanded, bool shouldSkipStateChangeCallback = false);

	// Invoke a command on the target agent to load its configuration fields
	void loadConfiguration ();

	// Update widget state as appropriate for records present in the application's RecordStore object, which has been locked prior to invocation
	void syncRecordStore ();

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Callback functions
	static void applyButtonClicked (void *windowPtr, Widget *widgetPtr);
	static void actionOptionChanged (void *windowPtr, Widget *widgetPtr);
	static void expandToggleStateChanged (void *windowPtr, Widget *widgetPtr);
	static void invokeGetAgentConfigurationComplete (void *windowPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand);
	static void invokeUpdateAgentConfigurationComplete (void *windowPtr, int invokeResult, const StdString &invokeHostname, int invokeTcpPort, const StdString &agentId, Json *invokeCommand, Json *responseCommand);
	static StdString mediaScanPeriodSliderValueName (float sliderValue);

	// Populate widgets as appropriate for fields in an AgentConfiguration command and return a Result value
	int populateConfiguration (Json *command);

	StdString agentPlatform;
	Image *iconImage;
	Label *titleLabel;
	Toggle *expandToggle;
	Panel *dividerPanel;
	ActionWindow *actionWindow;
	Button *applyButton;
};

#endif
