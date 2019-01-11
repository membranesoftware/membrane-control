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
	static const int mediaFiles = 32;
	static const int mediaItems = 33;
	static const int videoStream = 34;
	static const int videoStreams = 35;
	static const int storage = 36;
	static const int webKiosk = 37;
	static const int webKiosks = 38;
	static const int runProgram = 39;
	static const int createPlaylist = 40;
	static const int createProgram = 41;
	static const int runPlaylist = 42;
	static const int addPlaylistItem = 43;
	static const int playAll = 44;
	static const int cache = 45;
	static const int shuffle = 46;
	static const int startPosition = 47;
	static const int playDuration = 48;
	static const int display = 49;
	static const int displayName = 50;
	static const int displayStatus = 51;
	static const int monitors = 52;
	static const int server = 53;
	static const int servers = 54;
	static const int tasks = 55;
	static const int program = 56;
	static const int programs = 57;
	static const int playlist = 58;
	static const int playlists = 59;
	static const int speed = 60;
	static const int smallest = 61;
	static const int small = 62;
	static const int medium = 63;
	static const int large = 64;
	static const int largest = 65;
	static const int slowest = 66;
	static const int slow = 67;
	static const int fast = 68;
	static const int fastest = 69;
	static const int never = 70;
	static const int play = 71;
	static const int stop = 72;
	static const int unknown = 73;
	static const int enabled = 74;
	static const int disabled = 75;
	static const int ready = 76;
	static const int learnMore = 77;
	static const int searchForHelp = 78;
	static const int startedCreatingStream = 79;
	static const int removedStream = 80;
	static const int create = 81;
	static const int open = 82;
	static const int openHelp = 83;
	static const int adminConsole = 84;
	static const int mainMenu = 85;
	static const int bitrate = 86;
	static const int frameSize = 87;
	static const int fileSize = 88;
	static const int duration = 89;
	static const int frameRate = 90;
	static const int createStream = 91;
	static const int createStreamTooltip = 92;
	static const int streamServer = 93;
	static const int configure = 94;
	static const int configuration = 95;
	static const int save = 96;
	static const int apply = 97;
	static const int websiteList = 98;
	static const int addWebsite = 99;
	static const int showWebsite = 100;
	static const int browseWebsite = 101;
	static const int serverUiDescription = 102;
	static const int mediaUiDescription = 103;
	static const int monitorUiTitle = 104;
	static const int monitorUiDescription = 105;
	static const int webKioskUiDescription = 106;
	static const int selectPlayPosition = 107;
	static const int enterAddressPrompt = 108;
	static const int enterUrlPrompt = 109;
	static const int enterWebPlaylistNamePrompt = 110;
	static const int enterPlaylistNamePrompt = 111;
	static const int enterSearchKeyPrompt = 112;
	static const int emptyNotificationList = 113;
	static const int emptyWebPlaylistAddressList = 114;
	static const int selectToggleTooltip = 115;
	static const int expandToggleTooltip = 116;
	static const int mainMenuTooltip = 117;
	static const int moreActionsTooltip = 118;
	static const int textFieldEnterTooltip = 119;
	static const int textFieldPasteTooltip = 120;
	static const int textFieldClearTooltip = 121;
	static const int newsButtonTooltip = 122;
	static const int serverConnected = 123;
	static const int serversConnected = 124;
	static const int mediaServerConnected = 125;
	static const int mediaServersConnected = 126;
	static const int videoFileAvailable = 127;
	static const int videoFilesAvailable = 128;
	static const int monitorConnected = 129;
	static const int monitorsConnected = 130;
	static const int videoStreamAvailable = 131;
	static const int videoStreamsAvailable = 132;
	static const int webKioskAvailable = 133;
	static const int webKiosksAvailable = 134;
	static const int webPlaylistItem = 135;
	static const int webPlaylistItems = 136;
	static const int streamPlaylistItem = 137;
	static const int streamPlaylistItems = 138;
	static const int uiLaunchOpenButtonTooltip = 139;
	static const int serverUiHelpTitle = 140;
	static const int serverUiHelpText = 141;
	static const int serverUiHelpAction1Text = 142;
	static const int serverUiHelpAction2Text = 143;
	static const int serverUiNetworkAgentsTitle = 144;
	static const int serverUiEmptyAgentStatusTitle = 145;
	static const int serverUiEmptyAgentStatusText = 146;
	static const int serverUiAddressTooltip = 147;
	static const int serverUiBroadcastTooltip = 148;
	static const int serverUiUninstalledAgentTitle = 149;
	static const int serverUiUninstalledAgentDescription = 150;
	static const int serverUiUncontactedAgentTitle = 151;
	static const int serverUiUncontactedAgentDescription = 152;
	static const int serverUiContactedAgentTitle = 153;
	static const int serverUiContactedAgentDescription = 154;
	static const int serverUiStartLinkText = 155;
	static const int serverUiContactingAgentMessage = 156;
	static const int sourceMediaPath = 157;
	static const int sourceMediaPathDescription = 158;
	static const int sourceMediaPathPrompt = 159;
	static const int sourceMediaPathPromptWindows = 160;
	static const int mediaDataPath = 161;
	static const int mediaDataPathDescription = 162;
	static const int mediaDataPathPrompt = 163;
	static const int mediaDataPathPromptWindows = 164;
	static const int mediaScanPeriod = 165;
	static const int mediaScanPeriodDescription = 166;
	static const int mediaScanPeriodNeverDescription = 167;
	static const int scanForMedia = 168;
	static const int scanForMediaStartedMessage = 169;
	static const int streamDataPath = 170;
	static const int streamDataPathDescription = 171;
	static const int streamDataPathPrompt = 172;
	static const int streamDataPathPromptWindows = 173;
	static const int agentEnabledDescription = 174;
	static const int agentDisplayNameDescription = 175;
	static const int agentDisplayNamePrompt = 176;
	static const int mediaUiThumbnailSizeTooltip = 177;
	static const int mediaUiSearchTooltip = 178;
	static const int mediaUiEmptyAgentStatusTitle = 179;
	static const int mediaUiEmptyAgentStatusText = 180;
	static const int mediaUiEmptyMediaStatusTitle = 181;
	static const int mediaUiEmptyMediaStatusText = 182;
	static const int mediaUiHelpTitle = 183;
	static const int mediaUiHelpText = 184;
	static const int mediaUiHelpAction1Text = 185;
	static const int mediaUiHelpAction2Text = 186;
	static const int mediaUiHelpAction3Text = 187;
	static const int mediaItemUiHelpTitle = 188;
	static const int mediaItemUiHelpText = 189;
	static const int mediaItemUiHelpAction1Text = 190;
	static const int thumbnailImageSizeTooltip = 191;
	static const int uiBackTooltip = 192;
	static const int exitTooltip = 193;
	static const int moreHelpTopics = 194;
	static const int configureTooltip = 195;
	static const int reloadTooltip = 196;
	static const int mainUiWelcomeSnackbarText = 197;
	static const int mainUiHelpTitle = 198;
	static const int mainUiHelpText = 199;
	static const int mainUiServersHelpActionText = 200;
	static const int mainUiShowAllTooltip = 201;
	static const int openHelpUrlError = 202;
	static const int openAboutUrlError = 203;
	static const int openFeedbackUrlError = 204;
	static const int openUpdateUrlError = 205;
	static const int launchedWebBrowser = 206;
	static const int launchWebBrowserError = 207;
	static const int sentBroadcast = 208;
	static const int invokeGetStatusAddressEmptyError = 209;
	static const int invokeGetStatusAddressParseError = 210;
	static const int internalError = 211;
	static const int serverContactError = 212;
	static const int invokeClearDisplayMessage = 213;
	static const int invokeCreateWebPlaylistMessage = 214;
	static const int invokeShowWebUrlMessage = 215;
	static const int webKioskUiHelpTitle = 216;
	static const int webKioskUiHelpText = 217;
	static const int webKioskUiHelpAction1Text = 218;
	static const int webKioskUiHelpAction2Text = 219;
	static const int webKioskUiAddUrlTooltip = 220;
	static const int webKioskUiBrowseUrlTooltip = 221;
	static const int webKioskUiShowUrlTooltip = 222;
	static const int webKioskUiAddPlaylistTooltip = 223;
	static const int webKioskUiClearDisplayTooltip = 224;
	static const int webKioskUiWritePlaylistTooltip = 225;
	static const int webKioskNoAgentsSelectedPrompt = 226;
	static const int webKioskNoPlaylistsSelectedPrompt = 227;
	static const int webKioskNoAddressEnteredPrompt = 228;
	static const int websiteAddedMessage = 229;
	static const int webPlaylistCreatedMessage = 230;
	static const int shuffleTooltip = 231;
	static const int cacheTooltip = 232;
	static const int clickRenameTooltip = 233;
	static const int hyperlinkTooltip = 234;
	static const int serverAdminUiEmptyTaskListTitle = 235;
	static const int serverAdminUiEmptyTaskListText = 236;
	static const int serverAdminUiHelpTitle = 237;
	static const int serverAdminUiHelpText = 238;
	static const int monitorUiHelpTitle = 239;
	static const int monitorUiHelpText = 240;
	static const int monitorUiHelpAction1Text = 241;
	static const int monitorUiHelpAction2Text = 242;
	static const int monitorUiHelpAction3Text = 243;
	static const int monitorUiHelpAction4Text = 244;
	static const int monitorUiHelpAction5Text = 245;
	static const int monitorUiEmptyAgentStatusTitle = 246;
	static const int monitorUiEmptyAgentStatusText = 247;
	static const int monitorUiEmptyStreamStatusTitle = 248;
	static const int monitorUiEmptyStreamStatusText = 249;
	static const int monitorUiNoAgentsSelectedPrompt = 250;
	static const int monitorUiNoStreamSelectedPrompt = 251;
	static const int monitorUiNoPlaylistSelectedPrompt = 252;
	static const int emptyStreamPlaylistText = 253;
	static const int monitorUiPlayTooltip = 254;
	static const int monitorUiWritePlaylistTooltip = 255;
	static const int monitorUiStopTooltip = 256;
	static const int monitorUiAddPlaylistItemTooltip = 257;
	static const int monitorUiCreatePlaylistTooltip = 258;
	static const int monitorUiViewCacheTooltip = 259;
	static const int monitorUiAddCacheStreamTooltip = 260;
	static const int monitorUiAddCachePlaylistTooltip = 261;
	static const int invokeCreateCacheStreamMessage = 262;
	static const int streamPlaylistCreatedMessage = 263;
	static const int streamItemAddedMessage = 264;
	static const int invokePlayMediaMessage = 265;
	static const int invokeCreateStreamPlaylistMessage = 266;
	static const int streamItemUiHelpTitle = 267;
	static const int streamItemUiHelpText = 268;
	static const int streamItemUiHelpAction1Text = 269;
	static const int getAgentConfigurationServerContactError = 270;
	static const int updateAgentConfigurationMessage = 271;
	static const int monitorCacheUiHelpTitle = 272;
	static const int monitorCacheUiHelpText = 273;
	static const int monitorCacheUiHelpAction1Text = 274;
	static const int monitorCacheUiHelpAction2Text = 275;
	static const int monitorCacheUiEmptyStreamStatusTitle = 276;
	static const int monitorCacheUiEmptyStreamStatusText = 277;
	static const int monitorCacheUiPlayTooltip = 278;
	static const int monitorCacheUiWritePlaylistTooltip = 279;
	static const int monitorCacheUiStopTooltip = 280;
	static const int monitorCacheUiDeleteTooltip = 281;
	static const int monitorCacheUiWritePlaylistMessage = 282;
	static const int viewStreamTooltip = 283;
	static const int unselectButtonTooltip = 284;
	static const int monitorActivityIconTooltip = 285;
	static const int storageTooltip = 286;
	static const int cachedStream = 287;
	static const int cachedStreams = 288;
	static const int taskInProgress = 289;
	static const int tasksInProgress = 290;
	static const int cacheVideoStream = 291;
	static const int cachePlaylistStreams = 292;
	static const int removeStream = 293;
	static const int mediaStreaming = 294;
	static const int serversHelpTitle = 295;
	static const int membraneSoftwareOverviewHelpTitle = 296;
};

#endif
