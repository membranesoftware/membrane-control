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
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Util.h"
#include "Json.h"
#include "UiText.h"

UiText::UiText () {
	populate ();
}

UiText::~UiText () {

}

void UiText::populate () {
	// TODO: Possibly assign values from another language if appropriate

	windowTitle.assign ("Membrane Control");
	shuttingDownApp.sprintf ("Shutting down %s", windowTitle.c_str ());
	ok.assign ("ok");
	yes.assign ("yes");
	no.assign ("no");
	cancel.assign ("cancel");
	confirm.assign ("confirm");
	edit.assign ("edit");
	help.assign ("help");
	remove.assign ("remove");
	rename.assign ("rename");
	networkUplink.assign ("network uplink");
	uplinkStatus.assign ("uplink status");
	address.assign ("address");
	addresses.assign ("addresses");
	version.assign ("version");
	uptime.assign ("uptime");
	status.assign ("status");
	active.assign ("active");
	inactive.assign ("inactive");
	playing.assign ("playing");
	showing.assign ("showing");
	connect.assign ("connect");
	disconnect.assign ("disconnect");
	connected.assign ("connected");
	notConnected.assign ("not connected");
	connecting.assign ("connecting");
	exit.assign ("exit");
	select.assign ("select");
	clear.assign ("clear");
	settings.assign ("settings");
	about.assign ("about");
	checkForUpdates.assign ("check for updates");
	sendFeedback.assign ("send feedback");
	windowSize.assign ("window size");
	textSize.assign ("text size");
	showClock.assign ("show clock");
	media.assign ("media");
	mediaItems.assign ("media items");
	videoStreams.assign ("video streams");
	totalStorage.assign ("total storage");
	freeStorage.assign ("free storage");
	webKiosk.assign ("web kiosk");
	webKiosks.assign ("web kiosks");
	writeProgram.assign ("write program");
	shuffle.assign ("shuffle");
	display.assign ("display");
	displayName.assign ("display name");
	displays.assign ("displays");
	displayStatus.assign ("display status");
	servers.assign ("servers");
	program.assign ("program");
	programs.assign ("programs");
	speed.assign ("speed");
	serverTypes.assign ("server types");
	mediaServers.assign ("media servers");
	smallest.assign ("smallest");
	small.assign ("small");
	medium.assign ("medium");
	large.assign ("large");
	largest.assign ("largest");
	slowest.assign ("slowest");
	slow.assign ("slow");
	fast.assign ("fast");
	fastest.assign ("fastest");
	play.assign ("play");
	stop.assign ("stop");
	assign.assign ("assign");
	unassign.assign ("unassign");
	assignedTo.assign ("assigned to");
	unassigned.assign ("unassigned");
	unknown.assign ("unknown");
	enabled.assign ("enabled");
	disabled.assign ("disabled");
	learnMore.assign ("learn more");
	searchForHelp.assign ("Search for help");
	membraneSoftwareOverview.assign ("Membrane Software overview");
	startedCreatingStream.assign ("started creating stream");
	removedStream.assign ("removed stream");
	create.assign ("create");
	open.assign ("open");
	openHelp.assign ("open help");
	mainMenu.assign ("main menu");
	viewThumbnails.assign ("view thumbnails");
	bitrate.assign ("bitrate");
	frameSize.assign ("frame size");
	fileSize.assign ("file size");
	duration.assign ("duration");
	frameRate.assign ("frame rate");
	createStream.assign ("create stream");
	removeStream.assign ("remove stream");
	mediaServer.assign ("media server");
	streamServer.assign ("stream server");
	linkServer.assign ("link server");
	master.assign ("master");
	masters.assign ("masters");
	job.assign ("job");
	jobs.assign ("jobs");
	jobTypes.assign ("job types");
	createJob.assign ("create job");
	removeJob.assign ("remove job");
	download.assign ("download");
	viewDetails.assign ("view details");
	configure.assign ("configure");
	save.assign ("save");
	lastContact.assign ("last contact");
	websiteList.assign ("website list");
	addWebsite.assign ("add website");
	showWebsite.assign ("show website");
	showWebsites.assign ("show websites");
	browseWebsite.assign ("browse website");
	linkServerDescription.assign ("Provide a central hub of communication for servers on the network.");
	mediaServerDescription.assign ("Read media files from this computer and make them available for browsing.");
	streamServerDescription.assign ("Acquire video items from one or more media servers and make them available for streaming playback.");
	masterServerDescription.assign ("Run automated jobs that make use of servers available on the network.");
	displayServerDescription.assign ("Show websites and video streams on a display and accept control commands from this application.");
	linkUiDescription.assign ("Connect to Membrane Server applications running on this computer or on the network.");
	mediaUiDescription.assign ("Browse video items from a media server.");
	displayUiDescription.assign ("Command displays to show content from video streams or web pages.");
	masterUiDescription.assign ("Create automated processes to manage operations with networked servers.");
	webKioskUiDescription.assign ("Track your favorite websites by launching browser applications on other computers.");
	createServer.assign ("create server");
	selectPlayPosition.assign ("select play position");
	enterAddressPrompt.assign ("Enter server address...");
	enterUrlPrompt.assign ("Enter website address...");
	enterIntentNamePrompt.assign ("Enter program name...");
	autoConnectLinkServer.assign ("Auto-connect");
	emptyNotificationList.assign ("(Notification list is empty)");
	emptyWebDisplayIntentAddressList.assign ("(Addresses in this list are shown on the kiosk display)");
	selectToggleTooltip.assign ("Select or unselect this item");
	mainMenuTooltip.assign ("View the main menu");
	moreActionsTooltip.assign ("View more actions");
	textFieldPasteTooltip.assign ("Paste text from the clipboard into this field");
	textFieldClearTooltip.assign ("Remove all text from this field");
	newsButtonTooltip.assign ("View the notification list");
	uiLaunchOpenButtonTooltip.assign ("Open this interface");
	linkUiHelpTitle.assign ("Servers");
	linkUiHelpText.assign ("This interface finds and displays Membrane Server applications running on this computer or other computers. Connecting to a server here makes it available for use in other interfaces, as launched from the main menu.");
	linkUiHelpAction1Text.assign ("No Membrane Servers are in contact.\n\n- Check if Membrane Server is installed and running, and that its computer is connected to the network.\n\n- Click the broadcast button in the bottom right corner to discover any servers available on the local network.\n\n- Click the address button to directly contact a server by name.");
	linkUiHelpAction2Text.assign ("- Click the broadcast button in the bottom right corner to discover any servers available on the local network.\n\n- Click the address button to directly contact a server by name.");
	linkUiAgentConfigurationTitle.assign ("Server administration");
	linkUiNetworkAgentsTitle.assign ("Server status");
	linkUiLocalAgentTitle.assign ("On this computer");
	linkUiEmptyAgentStatusTitle.assign ("No servers contacted");
	linkUiEmptyAgentStatusText.assign ("To find Membrane Servers on the network:\n\n- Click the broadcast button in the bottom right corner to discover any servers available on the local network.\n- Click the address button to directly contact a server by name.");
	linkUiAddressTooltip.assign ("<F1> Enter an address to contact a Membrane Server on the network");
	linkUiBroadcastTooltip.assign ("<F2> Scan for Membrane Servers on the local network");
	agentConfigurationUninstalledTitle.assign ("On this computer");
	agentConfigurationUninstalledDescription.assign ("Install Membrane Server to enable this computer for access by other Membrane systems.");
	agentConfigurationStoppedTitle.assign ("On this computer");
	agentConfigurationStoppedDescription.assign ("Membrane Server is installed on this computer but is not running.");
	agentConfigurationStoppedLinkText.assign ("How to start Membrane Server");
	agentConfigurationActiveDescription.assign ("Running on this computer");
	sourceMediaPath.assign ("source media path");
	sourceMediaPathDescription.assign ("The directory path in which media files can be found");
	mediaDataPath.assign ("media data path");
	mediaDataPathDescription.assign ("The directory path in which the media server should write data files");
	mediaScanPeriod.assign ("media scan period");
	mediaScanPeriodDescription.assign ("The interval to use for periodic scans of the source media path, in seconds. A zero value disables periodic scans.");
	streamDataPath.assign ("stream data path");
	streamDataPathDescription.assign ("The directory path in which the stream server should write data files");
	mediaUiViewTooltip.assign ("Set the thumbnail size to show for media items");
	uiBackTooltip.assign ("Return to the previous screen");
	exitTooltip.assign ("Close this application");
	moreHelpTopics.assign ("more help topics");
	connectTooltip.assign ("Contact this network uplink, gaining access to its connected servers");
	disconnectTooltip.assign ("Break contact with this network uplink and its connected servers");
	configureTooltip.assign ("Set configuration parameters for this server");
	reloadTooltip.assign ("Refresh server state");
	linkServerConnectedNewsTitle.assign ("Network connected");
	linkServerConnectedNewsText.assign ("Resources on this network are available for use.");
	mainUiWelcomeSnackbarText.assign ("Welcome to Membrane Control. Need information to get started?");
	mainUiHelpTitle.assign ("Membrane Control");
	mainUiHelpText.assign ("This application connects to other computers running Membrane Server, allowing you to access and control any resources they offer. The main menu shows Membrane Control's primary functions, each of which can be launched using its \"OPEN\" button.");
	mainUiServersHelpActionText.assign ("No Membrane Servers have been contacted. Open the \"Servers\" interface to locate other computers offering this service on the network.");
	mainUiShowAllTooltip.assign ("If enabled: Show all functions, even those that haven't yet established required server connections.");
	openHelpUrlError.sprintf ("An error occurred in launching the web browser. For help, see: %s", Util::serverUrl.c_str ());
	downloadUrlError.sprintf ("An error occurred in launching the web browser. To download software, see: %s", Util::serverUrl.c_str ());
	openAboutUrlError.sprintf ("An error occurred in launching the web browser. For information, see: %s", Util::getHelpUrl ("about-membrane-control").c_str ());
	openFeedbackUrlError.sprintf ("An error occurred in launching the web browser. To send feedback, see: %s", Util::getFeedbackUrl ().c_str ());
	openUpdateUrlError.sprintf ("An error occurred in launching the web browser. For software updates, see: %s", Util::serverUrl.c_str ());
	launchedWebBrowser.assign ("launched web browser");
	launchWebBrowserError.assign ("An error occurred in launching the web browser.");
	sentBroadcast.assign ("Network broadcast sent. Any server that responds will be shown in the top row.");
	invokeGetStatusAddressEmptyError.assign ("Enter a server address in the bottom toolbar to attempt contact.");
	invokeGetStatusAddressParseError.assign ("The provided server address doesn't use the correct format.");
	invokeGetStatusInternalError.assign ("An internal error prevented this operation from succeeding. Our apologies.");
	invokeGetStatusConfirmation.assign ("Attempting contact. If the server responds, it will be shown in the top row.");
	webKioskUiHelpTitle.assign ("Web kiosk");
	webKioskUiHelpText.assign ("This interface commands Membrane Monitor servers to behave as web kiosks. Select servers or programs using the star icons in their upper right corners, then use the bottom toolbar to execute commands.");
	webKioskUiHelpAction1Text.assign ("No Membrane Monitor servers are connected. To find one, return to the main menu and open the \"Servers\" interface.");
	webKioskUiHelpAction2Text.assign ("- Create and edit programs capable of running on web kiosks to show a set of sites.\n\n- Enter a website address and add it to a program, launch a web browser to view it, or command a web kiosk to display it.\n\n- Write a program to one or more web kiosks, causing them to cycle through the listed sites with an optional shuffle.");
	webKioskUiAddUrlTooltip.assign ("<F1> Add this website to the selected program");
	webKioskUiBrowseUrlTooltip.assign ("<F2> Launch a browser viewing this website");
	webKioskUiShowUrlTooltip.assign ("<F3> Show this website on all selected web kiosks");
	webKioskUiAddIntentTooltip.assign ("<F6> Create a new website list that can run on web kiosks as a program");
	webKioskUiClearDisplayTooltip.assign ("<F4> Clear all selected web kiosks");
	webKioskUiWriteIntentTooltip.assign ("<F5> Write the selected program to all selected web kiosks");
	webKioskUiShuffleTooltip.assign ("If enabled: Show items from the website list in randomized order");
	webKioskNoAgentsSelectedPrompt.assign ("<Select a web kiosk>");
	webKioskNoIntentsSelectedPrompt.assign ("<Select a program>");
	webKioskNoAddressEnteredPrompt.assign ("<Enter a website address>");
	clickRenameTooltip.assign ("(Click to rename)");
	hyperlinkTooltip.assign ("Open this site using the system\'s web browser");
	emptyInterfaceTitle.assign ("Interface in development");
	mediaUiEmptyInterfaceText.assign ("This functionality is planned for a future update. When completed, Membrane Control will be able to browse video files available from a Membrane Media Library computer.");
	displayUiEmptyInterfaceText.assign ("This functionality is planned for a future update. When completed, Membrane Control will be able to command screens to play specific video clips, or to continuously loop through a clip playlist.");
}

StdString UiText::getCountText (int type, int64_t amount) {
	// TODO: Possibly assign values from another language if appropriate
	switch (type) {
		case UiText::SERVERS_CONNECTED: {
			return (StdString::createSprintf ("%lli server%s connected", (long long int) amount, (amount != 1) ? "s" : ""));
		}
		case UiText::MEDIA_SERVERS_CONNECTED: {
			return (StdString::createSprintf ("%lli media server%s connected", (long long int) amount, (amount != 1) ? "s" : ""));
		}
		case UiText::VIDEO_FILES_AVAILABLE: {
			return (StdString::createSprintf ("%lli video file%s available", (long long int) amount, (amount != 1) ? "s" : ""));
		}
		case UiText::DISPLAYS_CONNECTED: {
			return (StdString::createSprintf ("%lli display%s connected", (long long int) amount, (amount != 1) ? "s" : ""));
		}
		case UiText::VIDEO_STREAMS_AVAILABLE: {
			return (StdString::createSprintf ("%lli video stream%s available", (long long int) amount, (amount != 1) ? "s" : ""));
		}
		case UiText::SERVERS_RUNNING: {
			return (StdString::createSprintf ("%lli server%s running", (long long int) amount, (amount != 1) ? "s" : ""));
		}
		case UiText::MASTERS_CONNECTED: {
			return (StdString::createSprintf ("%lli master%s connected", (long long int) amount, (amount != 1) ? "s" : ""));
		}
		case UiText::JOBS_RUNNING: {
			return (StdString::createSprintf ("%lli job%s running", (long long int) amount, (amount != 1) ? "s" : ""));
		}
		case UiText::BYTES: {
			return (StdString::createSprintf ("%lli byte%s", (long long int) amount, (amount != 1) ? "s" : ""));
		}
		case UiText::WEB_KIOSKS_AVAILABLE: {
			return (StdString::createSprintf ("%lli web kiosk%s available", (long long int) amount, (amount != 1) ? "s" : ""));
		}
		case UiText::WEB_DISPLAY_INTENT_ITEMS: {
			return (StdString::createSprintf ("%lli site%s in this program", (long long int) amount, (amount != 1) ? "s" : ""));
		}
	}

	return (StdString::createSprintf ("%lli", (long long int) amount));
}

StdString UiText::getCountText (int type, int amount) {
	return (getCountText (type, (int64_t) amount));
}

StdString UiText::getMonitorStatusText (Json *monitorStatus) {
	StdString name, text;
	int truncateLength = 24;

	if (monitorStatus->getBoolean ("isPlaying", false)) {
		name = monitorStatus->getString ("mediaName", "");
		if (! name.empty ()) {
			text.sprintf ("%s %s", playing.capitalized ().c_str (), name.truncated (truncateLength).c_str ());
		}
	}
	else if (monitorStatus->getBoolean ("isShowingUrl", false)) {
		name = monitorStatus->getString ("showUrl", "");
		if (! name.empty ()) {
			text.sprintf ("%s %s", showing.capitalized ().c_str (), name.truncated (truncateLength).c_str ());
		}
	}

	if (text.empty ()) {
		text.assign (inactive.capitalized ());
	}
	return (text);
}
