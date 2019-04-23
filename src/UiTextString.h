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
	static const int showInterfaceAnimations = 33;
	static const int media = 34;
	static const int mediaFile = 35;
	static const int mediaFiles = 36;
	static const int mediaItems = 37;
	static const int videoStream = 38;
	static const int videoStreams = 39;
	static const int videoQuality = 40;
	static const int storage = 41;
	static const int webKiosk = 42;
	static const int webKiosks = 43;
	static const int runProgram = 44;
	static const int createPlaylist = 45;
	static const int createProgram = 46;
	static const int runPlaylist = 47;
	static const int addPlaylistItem = 48;
	static const int deletePlaylist = 49;
	static const int deleteProgram = 50;
	static const int playAll = 51;
	static const int cache = 52;
	static const int shuffle = 53;
	static const int startPosition = 54;
	static const int playDuration = 55;
	static const int display = 56;
	static const int displayName = 57;
	static const int displayStatus = 58;
	static const int monitors = 59;
	static const int server = 60;
	static const int servers = 61;
	static const int tasks = 62;
	static const int program = 63;
	static const int programs = 64;
	static const int playlist = 65;
	static const int playlists = 66;
	static const int speed = 67;
	static const int smallest = 68;
	static const int small = 69;
	static const int medium = 70;
	static const int large = 71;
	static const int largest = 72;
	static const int slowest = 73;
	static const int slow = 74;
	static const int fast = 75;
	static const int fastest = 76;
	static const int never = 77;
	static const int play = 78;
	static const int stop = 79;
	static const int unknown = 80;
	static const int enabled = 81;
	static const int disabled = 82;
	static const int ready = 83;
	static const int learnMore = 84;
	static const int searchForHelp = 85;
	static const int startedCreatingStream = 86;
	static const int removedStream = 87;
	static const int create = 88;
	static const int open = 89;
	static const int openHelp = 90;
	static const int adminConsole = 91;
	static const int mainMenu = 92;
	static const int bitrate = 93;
	static const int frameSize = 94;
	static const int fileSize = 95;
	static const int duration = 96;
	static const int frameRate = 97;
	static const int playStreams = 98;
	static const int playInBrowser = 99;
	static const int manageStreams = 100;
	static const int managePlaylists = 101;
	static const int configureStream = 102;
	static const int streamServer = 103;
	static const int configure = 104;
	static const int configuration = 105;
	static const int save = 106;
	static const int apply = 107;
	static const int websiteList = 108;
	static const int addWebsite = 109;
	static const int showWebsite = 110;
	static const int browseWebsite = 111;
	static const int viewWebsites = 112;
	static const int managePrograms = 113;
	static const int serverUiDescription = 114;
	static const int mediaUiDescription = 115;
	static const int webKioskUiDescription = 116;
	static const int selectPlayPosition = 117;
	static const int enterAddressPrompt = 118;
	static const int enterUrlPrompt = 119;
	static const int enterWebPlaylistNamePrompt = 120;
	static const int enterPlaylistNamePrompt = 121;
	static const int enterSearchKeyPrompt = 122;
	static const int emptyWebPlaylistAddressList = 123;
	static const int selectToggleTooltip = 124;
	static const int expandToggleTooltip = 125;
	static const int mainMenuTooltip = 126;
	static const int moreActionsTooltip = 127;
	static const int textFieldEnterTooltip = 128;
	static const int textFieldPasteTooltip = 129;
	static const int textFieldClearTooltip = 130;
	static const int textFieldRandomizeTooltip = 131;
	static const int textFieldVisibilityToggleTooltip = 132;
	static const int serverConnected = 133;
	static const int serversConnected = 134;
	static const int mediaServerConnected = 135;
	static const int mediaServersConnected = 136;
	static const int videoFileInCatalog = 137;
	static const int videoFilesInCatalog = 138;
	static const int monitorConnected = 139;
	static const int monitorsConnected = 140;
	static const int videoStreamPlayable = 141;
	static const int videoStreamsPlayable = 142;
	static const int webPlaylistItem = 143;
	static const int webPlaylistItems = 144;
	static const int streamPlaylistItem = 145;
	static const int streamPlaylistItems = 146;
	static const int uiLaunchOpenButtonTooltip = 147;
	static const int serverUiHelpTitle = 148;
	static const int serverUiHelpText = 149;
	static const int serverUiHelpAction1Text = 150;
	static const int serverUiHelpAction2Text = 151;
	static const int serverUiNetworkAgentsTitle = 152;
	static const int serverUiEmptyAgentStatusTitle = 153;
	static const int serverUiEmptyAgentStatusText = 154;
	static const int serverUiAddressTooltip = 155;
	static const int serverUiBroadcastTooltip = 156;
	static const int serverUiContactingAgentMessage = 157;
	static const int serverUiContactingAgentDescription = 158;
	static const int serverUiUnauthorizedErrorTitle = 159;
	static const int serverUiUnauthorizedErrorText = 160;
	static const int serverUiFailedContactErrorTitle = 161;
	static const int serverUiFailedContactErrorText = 162;
	static const int sourceMediaPath = 163;
	static const int sourceMediaPathDescription = 164;
	static const int sourceMediaPathPrompt = 165;
	static const int sourceMediaPathPromptWindows = 166;
	static const int mediaDataPath = 167;
	static const int mediaDataPathDescription = 168;
	static const int mediaDataPathPrompt = 169;
	static const int mediaDataPathPromptWindows = 170;
	static const int mediaScanPeriod = 171;
	static const int mediaScanPeriodDescription = 172;
	static const int mediaScanPeriodNeverDescription = 173;
	static const int scanForMedia = 174;
	static const int scanForMediaStartedMessage = 175;
	static const int streamDataPath = 176;
	static const int streamDataPathDescription = 177;
	static const int streamDataPathPrompt = 178;
	static const int streamDataPathPromptWindows = 179;
	static const int agentEnabledDescription = 180;
	static const int agentDisplayNameDescription = 181;
	static const int agentDisplayNamePrompt = 182;
	static const int mediaUiSearchTooltip = 183;
	static const int mediaUiVisibilityTooltip = 184;
	static const int mediaUiEmptyAgentStatusTitle = 185;
	static const int mediaUiEmptyAgentStatusText = 186;
	static const int mediaUiEmptyMediaStatusTitle = 187;
	static const int mediaUiEmptyMediaStatusText = 188;
	static const int mediaUiHelpTitle = 189;
	static const int mediaUiHelpText = 190;
	static const int mediaUiHelpAction1Text = 191;
	static const int mediaUiHelpAction2Text = 192;
	static const int mediaUiHelpAction3Text = 193;
	static const int mediaUiHelpAction4Text = 194;
	static const int mediaUiNoMediaSelectedPrompt = 195;
	static const int mediaUiNoStreamableMediaSelectedPrompt = 196;
	static const int mediaUiNoMonitorSelectedPrompt = 197;
	static const int mediaUiNoPlaylistSelectedPrompt = 198;
	static const int mediaUiAddPlaylistItemTooltip = 199;
	static const int mediaUiCreatePlaylistTooltip = 200;
	static const int mediaUiWritePlaylistTooltip = 201;
	static const int mediaUiPlayTooltip = 202;
	static const int mediaUiStopTooltip = 203;
	static const int mediaUiBrowserPlayTooltip = 204;
	static const int mediaUiConfigureStreamTooltip = 205;
	static const int mediaUiAddCacheStreamTooltip = 206;
	static const int mediaUiDeleteStreamTooltip = 207;
	static const int mediaUiDeletePlaylistTooltip = 208;
	static const int mediaItemUiHelpTitle = 209;
	static const int mediaItemUiHelpText = 210;
	static const int mediaItemUiHelpAction1Text = 211;
	static const int mediaItemUiConfigureStreamTooltip = 212;
	static const int thumbnailImageSizeTooltip = 213;
	static const int uiBackTooltip = 214;
	static const int exitTooltip = 215;
	static const int moreHelpTopics = 216;
	static const int reloadTooltip = 217;
	static const int mainUiHelpTitle = 218;
	static const int mainUiHelpText = 219;
	static const int mainUiServersHelpActionText = 220;
	static const int mainUiShowAllTooltip = 221;
	static const int mainUiNextBannerTooltip = 222;
	static const int openHelpUrlError = 223;
	static const int openAboutUrlError = 224;
	static const int openFeedbackUrlError = 225;
	static const int openUpdateUrlError = 226;
	static const int launchedWebBrowser = 227;
	static const int launchWebBrowserError = 228;
	static const int sentBroadcast = 229;
	static const int invokeGetStatusAddressEmptyError = 230;
	static const int invokeGetStatusAddressParseError = 231;
	static const int internalError = 232;
	static const int serverContactError = 233;
	static const int invokeClearDisplayMessage = 234;
	static const int invokeCreateWebPlaylistMessage = 235;
	static const int invokeShowWebUrlMessage = 236;
	static const int webKioskUiHelpTitle = 237;
	static const int webKioskUiHelpText = 238;
	static const int webKioskUiHelpAction1Text = 239;
	static const int webKioskUiHelpAction2Text = 240;
	static const int webKioskUiAddUrlTooltip = 241;
	static const int webKioskUiBrowseUrlTooltip = 242;
	static const int webKioskUiShowUrlTooltip = 243;
	static const int webKioskUiAddPlaylistTooltip = 244;
	static const int webKioskUiClearDisplayTooltip = 245;
	static const int webKioskUiWritePlaylistTooltip = 246;
	static const int webKioskUiDeletePlaylistTooltip = 247;
	static const int webKioskNoAgentsSelectedPrompt = 248;
	static const int webKioskNoPlaylistsSelectedPrompt = 249;
	static const int webKioskNoAddressEnteredPrompt = 250;
	static const int websiteAddedMessage = 251;
	static const int webPlaylistCreatedMessage = 252;
	static const int shuffleTooltip = 253;
	static const int clickRenameTooltip = 254;
	static const int hyperlinkTooltip = 255;
	static const int serverAdminUiEmptyTaskListTitle = 256;
	static const int serverAdminUiEmptyTaskListText = 257;
	static const int serverAdminUiHelpTitle = 258;
	static const int serverAdminUiHelpText = 259;
	static const int serverAdminUiHelpAction1Text = 260;
	static const int serverAdminUiLockTooltip = 261;
	static const int serverAdminUiEmptyAdminPasswordText = 262;
	static const int serverAdminUiAdminPasswordDescription = 263;
	static const int serverAdminUiSetPasswordCompleteMessage = 264;
	static const int emptyStreamPlaylistText = 265;
	static const int invokeCreateCacheStreamMessage = 266;
	static const int invokeConfigureMediaStreamMessage = 267;
	static const int streamPlaylistCreatedMessage = 268;
	static const int streamItemAddedMessage = 269;
	static const int invokePlayMediaMessage = 270;
	static const int invokeCreateStreamPlaylistMessage = 271;
	static const int invokeRemoveStreamMessage = 272;
	static const int streamItemUiHelpTitle = 273;
	static const int streamItemUiHelpText = 274;
	static const int streamItemUiHelpAction1Text = 275;
	static const int getAgentConfigurationServerContactError = 276;
	static const int updateAgentConfigurationMessage = 277;
	static const int monitorCacheUiHelpTitle = 278;
	static const int monitorCacheUiHelpText = 279;
	static const int monitorCacheUiHelpAction1Text = 280;
	static const int monitorCacheUiHelpAction2Text = 281;
	static const int monitorCacheUiEmptyStreamStatusTitle = 282;
	static const int monitorCacheUiEmptyStreamStatusText = 283;
	static const int monitorCacheUiNoStreamSelectedPrompt = 284;
	static const int monitorCacheUiPlayTooltip = 285;
	static const int monitorCacheUiWritePlaylistTooltip = 286;
	static const int monitorCacheUiStopTooltip = 287;
	static const int monitorCacheUiDeleteTooltip = 288;
	static const int monitorCacheUiWritePlaylistMessage = 289;
	static const int viewTimelineImagesTooltip = 290;
	static const int unselectButtonTooltip = 291;
	static const int monitorActivityIconTooltip = 292;
	static const int storageTooltip = 293;
	static const int cachedStream = 294;
	static const int cachedStreams = 295;
	static const int taskInProgress = 296;
	static const int tasksInProgress = 297;
	static const int removeStream = 298;
	static const int deleteStream = 299;
	static const int cacheStream = 300;
	static const int mediaStreaming = 301;
	static const int serversHelpTitle = 302;
	static const int membraneSoftwareOverviewHelpTitle = 303;
	static const int serverPasswords = 304;
	static const int addAdminPasswordTooltip = 305;
	static const int createAdminPassword = 306;
	static const int setAdminPassword = 307;
	static const int adminPasswordNamePrompt = 308;
	static const int adminPasswordPrompt = 309;
	static const int emptyAdminSecretListText = 310;
	static const int normalVideoQualityDescription = 311;
	static const int highVideoQualityDescription = 312;
	static const int lowVideoQualityDescription = 313;
	static const int lowestVideoQualityDescription = 314;
	static const int videoQualityDescription = 315;
	static const int firstLaunchBannerMessage = 316;
	static const int helpKeyBannerMessage = 317;
	static const int freeApplicationBannerMessage = 318;
	static const int membraneSoftwareBannerMessage = 319;
	static const int donateBannerMessage = 320;
	static const int mediaAttributesTooltip = 321;
	static const int toolbarModeButtonTooltip = 322;
	static const int streamIconTooltip = 323;
	static const int viewMonitorCacheTooltip = 324;
};

#endif
