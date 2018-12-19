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
// Index values that reference UiText strings

#ifndef UI_TEXT_STRING_H
#define UI_TEXT_STRING_H

#if PLATFORM_WINDOWS
#ifdef small
#undef small
#endif
#endif

class UiTextString {
public:
	static const int windowTitle = 1;
	static const int shuttingDownApp = 2;
	static const int ok = 3;
	static const int yes = 4;
	static const int no = 5;
	static const int cancel = 6;
	static const int confirm = 7;
	static const int edit = 8;
	static const int help = 9;
	static const int remove = 10;
	static const int rename = 11;
	static const int address = 12;
	static const int version = 13;
	static const int uptime = 14;
	static const int status = 15;
	static const int active = 16;
	static const int inactive = 17;
	static const int playing = 18;
	static const int showing = 19;
	static const int exit = 20;
	static const int select = 21;
	static const int clear = 22;
	static const int settings = 23;
	static const int about = 24;
	static const int checkForUpdates = 25;
	static const int sendFeedback = 26;
	static const int windowSize = 27;
	static const int textSize = 28;
	static const int showClock = 29;
	static const int media = 30;
	static const int mediaFile = 31;
	static const int mediaItems = 32;
	static const int videoStream = 33;
	static const int videoStreams = 34;
	static const int totalStorage = 35;
	static const int freeStorage = 36;
	static const int webKiosk = 37;
	static const int webKiosks = 38;
	static const int runProgram = 39;
	static const int createPlaylist = 40;
	static const int createProgram = 41;
	static const int runPlaylist = 42;
	static const int addPlaylistItem = 43;
	static const int shuffle = 44;
	static const int startPosition = 45;
	static const int playDuration = 46;
	static const int display = 47;
	static const int displayName = 48;
	static const int displayStatus = 49;
	static const int monitors = 50;
	static const int server = 51;
	static const int servers = 52;
	static const int tasks = 53;
	static const int program = 54;
	static const int programs = 55;
	static const int playlist = 56;
	static const int playlists = 57;
	static const int speed = 58;
	static const int smallest = 59;
	static const int small = 60;
	static const int medium = 61;
	static const int large = 62;
	static const int largest = 63;
	static const int slowest = 64;
	static const int slow = 65;
	static const int fast = 66;
	static const int fastest = 67;
	static const int never = 68;
	static const int play = 69;
	static const int stop = 70;
	static const int unknown = 71;
	static const int enabled = 72;
	static const int disabled = 73;
	static const int ready = 74;
	static const int learnMore = 75;
	static const int searchForHelp = 76;
	static const int membraneSoftwareOverview = 77;
	static const int startedCreatingStream = 78;
	static const int removedStream = 79;
	static const int create = 80;
	static const int open = 81;
	static const int openHelp = 82;
	static const int adminConsole = 83;
	static const int mainMenu = 84;
	static const int bitrate = 85;
	static const int frameSize = 86;
	static const int fileSize = 87;
	static const int duration = 88;
	static const int frameRate = 89;
	static const int createStream = 90;
	static const int createStreamTooltip = 91;
	static const int streamServer = 92;
	static const int configure = 93;
	static const int configuration = 94;
	static const int save = 95;
	static const int apply = 96;
	static const int websiteList = 97;
	static const int addWebsite = 98;
	static const int showWebsite = 99;
	static const int browseWebsite = 100;
	static const int serverUiDescription = 101;
	static const int mediaUiDescription = 102;
	static const int monitorUiTitle = 103;
	static const int monitorUiDescription = 104;
	static const int webKioskUiDescription = 105;
	static const int selectPlayPosition = 106;
	static const int enterAddressPrompt = 107;
	static const int enterUrlPrompt = 108;
	static const int enterWebPlaylistNamePrompt = 109;
	static const int enterPlaylistNamePrompt = 110;
	static const int enterSearchKeyPrompt = 111;
	static const int emptyNotificationList = 112;
	static const int emptyWebPlaylistAddressList = 113;
	static const int selectToggleTooltip = 114;
	static const int expandToggleTooltip = 115;
	static const int mainMenuTooltip = 116;
	static const int moreActionsTooltip = 117;
	static const int textFieldEnterTooltip = 118;
	static const int textFieldPasteTooltip = 119;
	static const int textFieldClearTooltip = 120;
	static const int newsButtonTooltip = 121;
	static const int serverConnected = 122;
	static const int serversConnected = 123;
	static const int mediaServerConnected = 124;
	static const int mediaServersConnected = 125;
	static const int videoFileAvailable = 126;
	static const int videoFilesAvailable = 127;
	static const int monitorConnected = 128;
	static const int monitorsConnected = 129;
	static const int videoStreamAvailable = 130;
	static const int videoStreamsAvailable = 131;
	static const int webKioskAvailable = 132;
	static const int webKiosksAvailable = 133;
	static const int webPlaylistItem = 134;
	static const int webPlaylistItems = 135;
	static const int streamPlaylistItem = 136;
	static const int streamPlaylistItems = 137;
	static const int uiLaunchOpenButtonTooltip = 138;
	static const int serverUiHelpTitle = 139;
	static const int serverUiHelpText = 140;
	static const int serverUiHelpAction1Text = 141;
	static const int serverUiHelpAction2Text = 142;
	static const int serverUiNetworkAgentsTitle = 143;
	static const int serverUiEmptyAgentStatusTitle = 144;
	static const int serverUiEmptyAgentStatusText = 145;
	static const int serverUiAddressTooltip = 146;
	static const int serverUiBroadcastTooltip = 147;
	static const int serverUiUninstalledAgentTitle = 148;
	static const int serverUiUninstalledAgentDescription = 149;
	static const int serverUiUncontactedAgentTitle = 150;
	static const int serverUiUncontactedAgentDescription = 151;
	static const int serverUiContactedAgentTitle = 152;
	static const int serverUiContactedAgentDescription = 153;
	static const int serverUiStartLinkText = 154;
	static const int serverUiContactingAgentMessage = 155;
	static const int sourceMediaPath = 156;
	static const int sourceMediaPathDescription = 157;
	static const int sourceMediaPathPrompt = 158;
	static const int sourceMediaPathPromptWindows = 159;
	static const int mediaDataPath = 160;
	static const int mediaDataPathDescription = 161;
	static const int mediaDataPathPrompt = 162;
	static const int mediaDataPathPromptWindows = 163;
	static const int mediaScanPeriod = 164;
	static const int mediaScanPeriodDescription = 165;
	static const int streamDataPath = 166;
	static const int streamDataPathDescription = 167;
	static const int streamDataPathPrompt = 168;
	static const int streamDataPathPromptWindows = 169;
	static const int agentEnabledDescription = 170;
	static const int agentDisplayNameDescription = 171;
	static const int agentDisplayNamePrompt = 172;
	static const int mediaUiThumbnailSizeTooltip = 173;
	static const int mediaUiSearchTooltip = 174;
	static const int mediaUiEmptyAgentStatusTitle = 175;
	static const int mediaUiEmptyAgentStatusText = 176;
	static const int mediaUiEmptyMediaStatusTitle = 177;
	static const int mediaUiEmptyMediaStatusText = 178;
	static const int mediaUiHelpTitle = 179;
	static const int mediaUiHelpText = 180;
	static const int mediaUiHelpAction1Text = 181;
	static const int mediaUiHelpAction2Text = 182;
	static const int mediaUiHelpAction3Text = 183;
	static const int mediaItemUiHelpTitle = 184;
	static const int mediaItemUiHelpText = 185;
	static const int mediaItemUiHelpAction1Text = 186;
	static const int thumbnailImageSizeTooltip = 187;
	static const int uiBackTooltip = 188;
	static const int exitTooltip = 189;
	static const int moreHelpTopics = 190;
	static const int configureTooltip = 191;
	static const int reloadTooltip = 192;
	static const int mainUiWelcomeSnackbarText = 193;
	static const int mainUiHelpTitle = 194;
	static const int mainUiHelpText = 195;
	static const int mainUiServersHelpActionText = 196;
	static const int mainUiShowAllTooltip = 197;
	static const int openHelpUrlError = 198;
	static const int openAboutUrlError = 199;
	static const int openFeedbackUrlError = 200;
	static const int openUpdateUrlError = 201;
	static const int launchedWebBrowser = 202;
	static const int launchWebBrowserError = 203;
	static const int sentBroadcast = 204;
	static const int invokeGetStatusAddressEmptyError = 205;
	static const int invokeGetStatusAddressParseError = 206;
	static const int internalError = 207;
	static const int serverContactError = 208;
	static const int invokeClearDisplayMessage = 209;
	static const int invokeCreateWebPlaylistMessage = 210;
	static const int invokeShowWebUrlMessage = 211;
	static const int webKioskUiHelpTitle = 212;
	static const int webKioskUiHelpText = 213;
	static const int webKioskUiHelpAction1Text = 214;
	static const int webKioskUiHelpAction2Text = 215;
	static const int webKioskUiAddUrlTooltip = 216;
	static const int webKioskUiBrowseUrlTooltip = 217;
	static const int webKioskUiShowUrlTooltip = 218;
	static const int webKioskUiAddPlaylistTooltip = 219;
	static const int webKioskUiClearDisplayTooltip = 220;
	static const int webKioskUiWritePlaylistTooltip = 221;
	static const int webKioskNoAgentsSelectedPrompt = 222;
	static const int webKioskNoPlaylistsSelectedPrompt = 223;
	static const int webKioskNoAddressEnteredPrompt = 224;
	static const int websiteAddedMessage = 225;
	static const int webPlaylistCreatedMessage = 226;
	static const int shuffleTooltip = 227;
	static const int clickRenameTooltip = 228;
	static const int hyperlinkTooltip = 229;
	static const int serverAdminUiEmptyTaskListTitle = 230;
	static const int serverAdminUiEmptyTaskListText = 231;
	static const int serverAdminUiHelpTitle = 232;
	static const int serverAdminUiHelpText = 233;
	static const int monitorUiHelpTitle = 234;
	static const int monitorUiHelpText = 235;
	static const int monitorUiHelpAction1Text = 236;
	static const int monitorUiHelpAction2Text = 237;
	static const int monitorUiHelpAction3Text = 238;
	static const int monitorUiHelpAction4Text = 239;
	static const int monitorUiHelpAction5Text = 240;
	static const int monitorUiEmptyAgentStatusTitle = 241;
	static const int monitorUiEmptyAgentStatusText = 242;
	static const int monitorUiEmptyStreamStatusTitle = 243;
	static const int monitorUiEmptyStreamStatusText = 244;
	static const int monitorUiNoAgentsSelectedPrompt = 245;
	static const int monitorUiNoStreamSelectedPrompt = 246;
	static const int monitorUiNoPlaylistSelectedPrompt = 247;
	static const int emptyStreamPlaylistText = 248;
	static const int monitorUiPlayTooltip = 249;
	static const int monitorUiWritePlaylistTooltip = 250;
	static const int monitorUiStopTooltip = 251;
	static const int monitorUiAddPlaylistItemTooltip = 252;
	static const int monitorUiCreatePlaylistTooltip = 253;
	static const int streamPlaylistCreatedMessage = 254;
	static const int streamItemAddedMessage = 255;
	static const int invokePlayMediaMessage = 256;
	static const int invokeCreateStreamPlaylistMessage = 257;
	static const int streamItemUiHelpTitle = 258;
	static const int streamItemUiHelpText = 259;
	static const int streamItemUiHelpAction1Text = 260;
	static const int getAgentConfigurationServerContactError = 261;
	static const int updateAgentConfigurationMessage = 262;
};

#endif
