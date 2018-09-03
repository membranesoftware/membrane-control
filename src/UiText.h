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
// Class that holds strings for use in UI text

#ifndef UI_TEXT_H
#define UI_TEXT_H

#include "StdString.h"
#include "Json.h"

#if PLATFORM_WINDOWS
#ifdef small
#undef small
#endif
#endif

class UiText {
public:
	UiText ();
	~UiText ();

	// Assign values to string members of this object
	void populate ();

	// Constants to use for getCountText types
	enum {
		SERVERS_CONNECTED = 0,
		MEDIA_SERVERS_CONNECTED = 1,
		VIDEO_FILES_AVAILABLE = 2,
		DISPLAYS_CONNECTED = 3,
		VIDEO_STREAMS_AVAILABLE = 4,
		SERVERS_RUNNING = 5,
		MASTERS_CONNECTED = 6,
		JOBS_RUNNING = 7,
		BYTES = 8,
		WEB_KIOSKS_AVAILABLE = 9,
		WEB_DISPLAY_INTENT_ITEMS = 10
	};
	// Return a string containing text suitable for displaying the specified count
	StdString getCountText (int type, int64_t amount);
	StdString getCountText (int type, int amount);

	// Return a string containing status text reflecting fields in the provided MonitorServerStatus object
	StdString getMonitorStatusText (Json *monitorStatus);

	// Read-only data members
	StdString windowTitle;
	StdString shuttingDownApp;
	StdString ok;
	StdString yes;
	StdString no;
	StdString cancel;
	StdString confirm;
	StdString edit;
	StdString help;
	StdString remove;
	StdString rename;
	StdString networkUplink;
	StdString uplinkStatus;
	StdString address;
	StdString addresses;
	StdString version;
	StdString uptime;
	StdString status;
	StdString active;
	StdString inactive;
	StdString playing;
	StdString showing;
	StdString connect;
	StdString disconnect;
	StdString connected;
	StdString notConnected;
	StdString connecting;
	StdString exit;
	StdString select;
	StdString clear;
	StdString settings;
	StdString about;
	StdString checkForUpdates;
	StdString sendFeedback;
	StdString windowSize;
	StdString textSize;
	StdString showClock;
	StdString media;
	StdString mediaItems;
	StdString videoStreams;
	StdString totalStorage;
	StdString freeStorage;
	StdString webKiosk;
	StdString webKiosks;
	StdString writeProgram;
	StdString shuffle;
	StdString display;
	StdString displayName;
	StdString displays;
	StdString displayStatus;
	StdString servers;
	StdString program;
	StdString programs;
	StdString speed;
	StdString serverTypes;
	StdString mediaServers;
	StdString smallest;
	StdString small;
	StdString medium;
	StdString large;
	StdString largest;
	StdString slowest;
	StdString slow;
	StdString fast;
	StdString fastest;
	StdString play;
	StdString stop;
	StdString assign;
	StdString unassign;
	StdString assignedTo;
	StdString unassigned;
	StdString unknown;
	StdString enabled;
	StdString disabled;
	StdString learnMore;
	StdString searchForHelp;
	StdString membraneSoftwareOverview;
	StdString startedCreatingStream;
	StdString removedStream;
	StdString create;
	StdString open;
	StdString openHelp;
	StdString mainMenu;
	StdString viewThumbnails;
	StdString bitrate;
	StdString frameSize;
	StdString fileSize;
	StdString duration;
	StdString frameRate;
	StdString createStream;
	StdString removeStream;
	StdString mediaServer;
	StdString streamServer;
	StdString linkServer;
	StdString master;
	StdString masters;
	StdString job;
	StdString jobs;
	StdString jobTypes;
	StdString createJob;
	StdString removeJob;
	StdString download;
	StdString viewDetails;
	StdString configure;
	StdString save;
	StdString lastContact;
	StdString websiteList;
	StdString addWebsite;
	StdString showWebsite;
	StdString showWebsites;
	StdString browseWebsite;
	StdString linkServerDescription;
	StdString mediaServerDescription;
	StdString streamServerDescription;
	StdString masterServerDescription;
	StdString displayServerDescription;
	StdString linkUiDescription;
	StdString mediaUiDescription;
	StdString displayUiDescription;
	StdString masterUiDescription;
	StdString webKioskUiDescription;
	StdString createServer;
	StdString selectPlayPosition;
	StdString enterAddressPrompt;
	StdString enterUrlPrompt;
	StdString enterIntentNamePrompt;
	StdString autoConnectLinkServer;
	StdString emptyNotificationList;
	StdString emptyWebDisplayIntentAddressList;
	StdString selectToggleTooltip;
	StdString mainMenuTooltip;
	StdString moreActionsTooltip;
	StdString textFieldPasteTooltip;
	StdString textFieldClearTooltip;
	StdString newsButtonTooltip;
	StdString uiLaunchOpenButtonTooltip;
	StdString linkUiHelpTitle;
	StdString linkUiHelpText;
	StdString linkUiHelpAction1Text;
	StdString linkUiHelpAction2Text;
	StdString linkUiAgentConfigurationTitle;
	StdString linkUiNetworkAgentsTitle;
	StdString linkUiLocalAgentTitle;
	StdString linkUiEmptyAgentStatusTitle;
	StdString linkUiEmptyAgentStatusText;
	StdString linkUiAddressTooltip;
	StdString linkUiBroadcastTooltip;
	StdString agentConfigurationUninstalledTitle;
	StdString agentConfigurationUninstalledDescription;
	StdString agentConfigurationStoppedTitle;
	StdString agentConfigurationStoppedDescription;
	StdString agentConfigurationStoppedLinkText;
	StdString agentConfigurationActiveDescription;
	StdString sourceMediaPath;
	StdString sourceMediaPathDescription;
	StdString mediaDataPath;
	StdString mediaDataPathDescription;
	StdString mediaScanPeriod;
	StdString mediaScanPeriodDescription;
	StdString streamDataPath;
	StdString streamDataPathDescription;
	StdString mediaUiViewTooltip;
	StdString uiBackTooltip;
	StdString exitTooltip;
	StdString connectTooltip;
	StdString disconnectTooltip;
	StdString configureTooltip;
	StdString reloadTooltip;
	StdString moreHelpTopics;
	StdString linkServerConnectedNewsTitle;
	StdString linkServerConnectedNewsText;
	StdString mainUiWelcomeSnackbarText;
	StdString mainUiHelpTitle;
	StdString mainUiHelpText;
	StdString mainUiServersHelpActionText;
	StdString mainUiShowAllTooltip;
	StdString openHelpUrlError;
	StdString downloadUrlError;
	StdString openAboutUrlError;
	StdString openFeedbackUrlError;
	StdString openUpdateUrlError;
	StdString launchedWebBrowser;
	StdString launchWebBrowserError;
	StdString sentBroadcast;
	StdString invokeGetStatusAddressEmptyError;
	StdString invokeGetStatusAddressParseError;
	StdString invokeGetStatusInternalError;
	StdString invokeGetStatusConfirmation;
	StdString webKioskUiHelpTitle;
	StdString webKioskUiHelpText;
	StdString webKioskUiHelpAction1Text;
	StdString webKioskUiHelpAction2Text;
	StdString webKioskUiAddUrlTooltip;
	StdString webKioskUiBrowseUrlTooltip;
	StdString webKioskUiShowUrlTooltip;
	StdString webKioskUiAddIntentTooltip;
	StdString webKioskUiWriteIntentTooltip;
	StdString webKioskUiClearDisplayTooltip;
	StdString webKioskUiShuffleTooltip;
	StdString webKioskNoAgentsSelectedPrompt;
	StdString webKioskNoIntentsSelectedPrompt;
	StdString webKioskNoAddressEnteredPrompt;
	StdString clickRenameTooltip;
	StdString hyperlinkTooltip;
	StdString emptyInterfaceTitle;
	StdString mediaUiEmptyInterfaceText;
	StdString displayUiEmptyInterfaceText;
};

#endif
