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
	static const int name = 12;
	static const int password = 13;
	static const int address = 14;
	static const int version = 15;
	static const int platform = 16;
	static const int uptime = 17;
	static const int status = 18;
	static const int active = 19;
	static const int inactive = 20;
	static const int playing = 21;
	static const int showing = 22;
	static const int exit = 23;
	static const int select = 24;
	static const int clear = 25;
	static const int settings = 26;
	static const int about = 27;
	static const int checkForUpdates = 28;
	static const int sendFeedback = 29;
	static const int windowSize = 30;
	static const int textSize = 31;
	static const int showClock = 32;
	static const int media = 33;
	static const int mediaFile = 34;
	static const int mediaFiles = 35;
	static const int mediaItems = 36;
	static const int videoStream = 37;
	static const int videoStreams = 38;
	static const int videoQuality = 39;
	static const int storage = 40;
	static const int webKiosk = 41;
	static const int webKiosks = 42;
	static const int runProgram = 43;
	static const int createPlaylist = 44;
	static const int createProgram = 45;
	static const int runPlaylist = 46;
	static const int addPlaylistItem = 47;
	static const int playAll = 48;
	static const int cache = 49;
	static const int shuffle = 50;
	static const int startPosition = 51;
	static const int playDuration = 52;
	static const int display = 53;
	static const int displayName = 54;
	static const int displayStatus = 55;
	static const int monitors = 56;
	static const int server = 57;
	static const int servers = 58;
	static const int tasks = 59;
	static const int program = 60;
	static const int programs = 61;
	static const int playlist = 62;
	static const int playlists = 63;
	static const int speed = 64;
	static const int smallest = 65;
	static const int small = 66;
	static const int medium = 67;
	static const int large = 68;
	static const int largest = 69;
	static const int slowest = 70;
	static const int slow = 71;
	static const int fast = 72;
	static const int fastest = 73;
	static const int never = 74;
	static const int play = 75;
	static const int stop = 76;
	static const int unknown = 77;
	static const int enabled = 78;
	static const int disabled = 79;
	static const int ready = 80;
	static const int learnMore = 81;
	static const int searchForHelp = 82;
	static const int startedCreatingStream = 83;
	static const int removedStream = 84;
	static const int create = 85;
	static const int open = 86;
	static const int openHelp = 87;
	static const int adminConsole = 88;
	static const int mainMenu = 89;
	static const int bitrate = 90;
	static const int frameSize = 91;
	static const int fileSize = 92;
	static const int duration = 93;
	static const int frameRate = 94;
	static const int createStream = 95;
	static const int createStreamTooltip = 96;
	static const int streamServer = 97;
	static const int configure = 98;
	static const int configuration = 99;
	static const int save = 100;
	static const int apply = 101;
	static const int websiteList = 102;
	static const int addWebsite = 103;
	static const int showWebsite = 104;
	static const int browseWebsite = 105;
	static const int serverUiDescription = 106;
	static const int mediaUiDescription = 107;
	static const int monitorUiTitle = 108;
	static const int monitorUiDescription = 109;
	static const int webKioskUiDescription = 110;
	static const int selectPlayPosition = 111;
	static const int enterAddressPrompt = 112;
	static const int enterUrlPrompt = 113;
	static const int enterWebPlaylistNamePrompt = 114;
	static const int enterPlaylistNamePrompt = 115;
	static const int enterSearchKeyPrompt = 116;
	static const int emptyNotificationList = 117;
	static const int emptyWebPlaylistAddressList = 118;
	static const int selectToggleTooltip = 119;
	static const int expandToggleTooltip = 120;
	static const int mainMenuTooltip = 121;
	static const int moreActionsTooltip = 122;
	static const int textFieldEnterTooltip = 123;
	static const int textFieldPasteTooltip = 124;
	static const int textFieldClearTooltip = 125;
	static const int textFieldRandomizeTooltip = 126;
	static const int textFieldVisibilityToggleTooltip = 127;
	static const int newsButtonTooltip = 128;
	static const int serverConnected = 129;
	static const int serversConnected = 130;
	static const int mediaServerConnected = 131;
	static const int mediaServersConnected = 132;
	static const int videoFileAvailable = 133;
	static const int videoFilesAvailable = 134;
	static const int monitorConnected = 135;
	static const int monitorsConnected = 136;
	static const int videoStreamAvailable = 137;
	static const int videoStreamsAvailable = 138;
	static const int webKioskAvailable = 139;
	static const int webKiosksAvailable = 140;
	static const int webPlaylistItem = 141;
	static const int webPlaylistItems = 142;
	static const int streamPlaylistItem = 143;
	static const int streamPlaylistItems = 144;
	static const int uiLaunchOpenButtonTooltip = 145;
	static const int serverUiHelpTitle = 146;
	static const int serverUiHelpText = 147;
	static const int serverUiHelpAction1Text = 148;
	static const int serverUiHelpAction2Text = 149;
	static const int serverUiNetworkAgentsTitle = 150;
	static const int serverUiEmptyAgentStatusTitle = 151;
	static const int serverUiEmptyAgentStatusText = 152;
	static const int serverUiAddressTooltip = 153;
	static const int serverUiBroadcastTooltip = 154;
	static const int serverUiContactingAgentMessage = 155;
	static const int serverUiContactingAgentDescription = 156;
	static const int serverUiUnauthorizedErrorTitle = 157;
	static const int serverUiUnauthorizedErrorText = 158;
	static const int serverUiFailedContactErrorTitle = 159;
	static const int serverUiFailedContactErrorText = 160;
	static const int sourceMediaPath = 161;
	static const int sourceMediaPathDescription = 162;
	static const int sourceMediaPathPrompt = 163;
	static const int sourceMediaPathPromptWindows = 164;
	static const int mediaDataPath = 165;
	static const int mediaDataPathDescription = 166;
	static const int mediaDataPathPrompt = 167;
	static const int mediaDataPathPromptWindows = 168;
	static const int mediaScanPeriod = 169;
	static const int mediaScanPeriodDescription = 170;
	static const int mediaScanPeriodNeverDescription = 171;
	static const int scanForMedia = 172;
	static const int scanForMediaStartedMessage = 173;
	static const int streamDataPath = 174;
	static const int streamDataPathDescription = 175;
	static const int streamDataPathPrompt = 176;
	static const int streamDataPathPromptWindows = 177;
	static const int agentEnabledDescription = 178;
	static const int agentDisplayNameDescription = 179;
	static const int agentDisplayNamePrompt = 180;
	static const int mediaUiSearchTooltip = 181;
	static const int mediaUiEmptyAgentStatusTitle = 182;
	static const int mediaUiEmptyAgentStatusText = 183;
	static const int mediaUiEmptyMediaStatusTitle = 184;
	static const int mediaUiEmptyMediaStatusText = 185;
	static const int mediaUiHelpTitle = 186;
	static const int mediaUiHelpText = 187;
	static const int mediaUiHelpAction1Text = 188;
	static const int mediaUiHelpAction2Text = 189;
	static const int mediaUiHelpAction3Text = 190;
	static const int mediaItemUiHelpTitle = 191;
	static const int mediaItemUiHelpText = 192;
	static const int mediaItemUiHelpAction1Text = 193;
	static const int thumbnailImageSizeTooltip = 194;
	static const int uiBackTooltip = 195;
	static const int exitTooltip = 196;
	static const int moreHelpTopics = 197;
	static const int configureTooltip = 198;
	static const int reloadTooltip = 199;
	static const int mainUiHelpTitle = 200;
	static const int mainUiHelpText = 201;
	static const int mainUiServersHelpActionText = 202;
	static const int mainUiShowAllTooltip = 203;
	static const int mainUiNextBannerTooltip = 204;
	static const int openHelpUrlError = 205;
	static const int openAboutUrlError = 206;
	static const int openFeedbackUrlError = 207;
	static const int openUpdateUrlError = 208;
	static const int launchedWebBrowser = 209;
	static const int launchWebBrowserError = 210;
	static const int sentBroadcast = 211;
	static const int invokeGetStatusAddressEmptyError = 212;
	static const int invokeGetStatusAddressParseError = 213;
	static const int internalError = 214;
	static const int serverContactError = 215;
	static const int invokeClearDisplayMessage = 216;
	static const int invokeCreateWebPlaylistMessage = 217;
	static const int invokeShowWebUrlMessage = 218;
	static const int webKioskUiHelpTitle = 219;
	static const int webKioskUiHelpText = 220;
	static const int webKioskUiHelpAction1Text = 221;
	static const int webKioskUiHelpAction2Text = 222;
	static const int webKioskUiAddUrlTooltip = 223;
	static const int webKioskUiBrowseUrlTooltip = 224;
	static const int webKioskUiShowUrlTooltip = 225;
	static const int webKioskUiAddPlaylistTooltip = 226;
	static const int webKioskUiClearDisplayTooltip = 227;
	static const int webKioskUiWritePlaylistTooltip = 228;
	static const int webKioskNoAgentsSelectedPrompt = 229;
	static const int webKioskNoPlaylistsSelectedPrompt = 230;
	static const int webKioskNoAddressEnteredPrompt = 231;
	static const int websiteAddedMessage = 232;
	static const int webPlaylistCreatedMessage = 233;
	static const int shuffleTooltip = 234;
	static const int cacheTooltip = 235;
	static const int clickRenameTooltip = 236;
	static const int hyperlinkTooltip = 237;
	static const int serverAdminUiEmptyTaskListTitle = 238;
	static const int serverAdminUiEmptyTaskListText = 239;
	static const int serverAdminUiHelpTitle = 240;
	static const int serverAdminUiHelpText = 241;
	static const int serverAdminUiHelpAction1Text = 242;
	static const int serverAdminUiLockTooltip = 243;
	static const int serverAdminUiEmptyAdminPasswordText = 244;
	static const int serverAdminUiAdminPasswordDescription = 245;
	static const int serverAdminUiSetPasswordCompleteMessage = 246;
	static const int monitorUiHelpTitle = 247;
	static const int monitorUiHelpText = 248;
	static const int monitorUiHelpAction1Text = 249;
	static const int monitorUiHelpAction2Text = 250;
	static const int monitorUiHelpAction3Text = 251;
	static const int monitorUiHelpAction4Text = 252;
	static const int monitorUiHelpAction5Text = 253;
	static const int monitorUiEmptyAgentStatusTitle = 254;
	static const int monitorUiEmptyAgentStatusText = 255;
	static const int monitorUiEmptyStreamStatusTitle = 256;
	static const int monitorUiEmptyStreamStatusText = 257;
	static const int monitorUiNoAgentsSelectedPrompt = 258;
	static const int monitorUiNoStreamSelectedPrompt = 259;
	static const int monitorUiNoPlaylistSelectedPrompt = 260;
	static const int emptyStreamPlaylistText = 261;
	static const int monitorUiPlayTooltip = 262;
	static const int monitorUiWritePlaylistTooltip = 263;
	static const int monitorUiStopTooltip = 264;
	static const int monitorUiAddPlaylistItemTooltip = 265;
	static const int monitorUiCreatePlaylistTooltip = 266;
	static const int monitorUiViewCacheTooltip = 267;
	static const int monitorUiAddCacheStreamTooltip = 268;
	static const int monitorUiAddCachePlaylistTooltip = 269;
	static const int invokeCreateCacheStreamMessage = 270;
	static const int streamPlaylistCreatedMessage = 271;
	static const int streamItemAddedMessage = 272;
	static const int invokePlayMediaMessage = 273;
	static const int invokeCreateStreamPlaylistMessage = 274;
	static const int streamItemUiHelpTitle = 275;
	static const int streamItemUiHelpText = 276;
	static const int streamItemUiHelpAction1Text = 277;
	static const int getAgentConfigurationServerContactError = 278;
	static const int updateAgentConfigurationMessage = 279;
	static const int monitorCacheUiHelpTitle = 280;
	static const int monitorCacheUiHelpText = 281;
	static const int monitorCacheUiHelpAction1Text = 282;
	static const int monitorCacheUiHelpAction2Text = 283;
	static const int monitorCacheUiEmptyStreamStatusTitle = 284;
	static const int monitorCacheUiEmptyStreamStatusText = 285;
	static const int monitorCacheUiPlayTooltip = 286;
	static const int monitorCacheUiWritePlaylistTooltip = 287;
	static const int monitorCacheUiStopTooltip = 288;
	static const int monitorCacheUiDeleteTooltip = 289;
	static const int monitorCacheUiWritePlaylistMessage = 290;
	static const int viewStreamTooltip = 291;
	static const int unselectButtonTooltip = 292;
	static const int monitorActivityIconTooltip = 293;
	static const int storageTooltip = 294;
	static const int cachedStream = 295;
	static const int cachedStreams = 296;
	static const int taskInProgress = 297;
	static const int tasksInProgress = 298;
	static const int cacheVideoStream = 299;
	static const int cachePlaylistStreams = 300;
	static const int removeStream = 301;
	static const int mediaStreaming = 302;
	static const int serversHelpTitle = 303;
	static const int membraneSoftwareOverviewHelpTitle = 304;
	static const int serverPasswords = 305;
	static const int addAdminPasswordTooltip = 306;
	static const int createAdminPassword = 307;
	static const int setAdminPassword = 308;
	static const int adminPasswordNamePrompt = 309;
	static const int adminPasswordPrompt = 310;
	static const int emptyAdminSecretListText = 311;
	static const int normalVideoQualityDescription = 312;
	static const int highVideoQualityDescription = 313;
	static const int lowVideoQualityDescription = 314;
	static const int lowestVideoQualityDescription = 315;
	static const int videoQualityDescription = 316;
	static const int firstLaunchBannerMessage = 317;
	static const int helpKeyBannerMessage = 318;
	static const int freeApplicationBannerMessage = 319;
	static const int membraneSoftwareBannerMessage = 320;
	static const int donateBannerMessage = 321;
};

#endif
