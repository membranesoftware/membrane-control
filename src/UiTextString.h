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
	static const int removedMediaRecord = 88;
	static const int create = 89;
	static const int open = 90;
	static const int openHelp = 91;
	static const int loading = 92;
	static const int adminConsole = 93;
	static const int mainMenu = 94;
	static const int bitrate = 95;
	static const int frameSize = 96;
	static const int fileSize = 97;
	static const int duration = 98;
	static const int frameRate = 99;
	static const int playStreams = 100;
	static const int playInBrowser = 101;
	static const int manageStreams = 102;
	static const int managePlaylists = 103;
	static const int configureStream = 104;
	static const int streamServer = 105;
	static const int configure = 106;
	static const int configuration = 107;
	static const int save = 108;
	static const int apply = 109;
	static const int websiteList = 110;
	static const int addWebsite = 111;
	static const int showWebsite = 112;
	static const int browseWebsite = 113;
	static const int viewWebsites = 114;
	static const int managePrograms = 115;
	static const int serverUiDescription = 116;
	static const int mediaUiDescription = 117;
	static const int webKioskUiDescription = 118;
	static const int selectPlayPosition = 119;
	static const int enterAddressPrompt = 120;
	static const int enterUrlPrompt = 121;
	static const int enterWebPlaylistNamePrompt = 122;
	static const int enterPlaylistNamePrompt = 123;
	static const int enterSearchKeyPrompt = 124;
	static const int emptyWebPlaylistAddressList = 125;
	static const int selectToggleTooltip = 126;
	static const int expandToggleTooltip = 127;
	static const int mainMenuTooltip = 128;
	static const int moreActionsTooltip = 129;
	static const int textFieldEnterTooltip = 130;
	static const int textFieldPasteTooltip = 131;
	static const int textFieldClearTooltip = 132;
	static const int textFieldRandomizeTooltip = 133;
	static const int textFieldVisibilityToggleTooltip = 134;
	static const int serverConnected = 135;
	static const int serversConnected = 136;
	static const int mediaServerConnected = 137;
	static const int mediaServersConnected = 138;
	static const int videoFileInCatalog = 139;
	static const int videoFilesInCatalog = 140;
	static const int monitorConnected = 141;
	static const int monitorsConnected = 142;
	static const int videoStreamPlayable = 143;
	static const int videoStreamsPlayable = 144;
	static const int webPlaylistItem = 145;
	static const int webPlaylistItems = 146;
	static const int streamPlaylistItem = 147;
	static const int streamPlaylistItems = 148;
	static const int uiLaunchOpenButtonTooltip = 149;
	static const int serverUiHelpTitle = 150;
	static const int serverUiHelpText = 151;
	static const int serverUiHelpAction1Text = 152;
	static const int serverUiHelpAction2Text = 153;
	static const int serverUiNetworkAgentsTitle = 154;
	static const int serverUiEmptyAgentStatusTitle = 155;
	static const int serverUiEmptyAgentStatusText = 156;
	static const int serverUiAddressTooltip = 157;
	static const int serverUiBroadcastTooltip = 158;
	static const int serverUiContactingAgentMessage = 159;
	static const int serverUiContactingAgentDescription = 160;
	static const int serverUiUnauthorizedErrorTitle = 161;
	static const int serverUiUnauthorizedErrorText = 162;
	static const int serverUiFailedContactErrorTitle = 163;
	static const int serverUiFailedContactErrorText = 164;
	static const int sourceMediaPath = 165;
	static const int sourceMediaPathDescription = 166;
	static const int sourceMediaPathPrompt = 167;
	static const int sourceMediaPathPromptWindows = 168;
	static const int mediaDataPath = 169;
	static const int mediaDataPathDescription = 170;
	static const int mediaDataPathPrompt = 171;
	static const int mediaDataPathPromptWindows = 172;
	static const int mediaScanPeriod = 173;
	static const int mediaScanPeriodDescription = 174;
	static const int mediaScanPeriodNeverDescription = 175;
	static const int scanForMedia = 176;
	static const int scanForMediaStartedMessage = 177;
	static const int streamDataPath = 178;
	static const int streamDataPathDescription = 179;
	static const int streamDataPathPrompt = 180;
	static const int streamDataPathPromptWindows = 181;
	static const int agentEnabledDescription = 182;
	static const int agentDisplayNameDescription = 183;
	static const int agentDisplayNamePrompt = 184;
	static const int mediaUiShowMediaWithoutStreamsAction = 185;
	static const int mediaUiSearchTooltip = 186;
	static const int mediaUiSortTooltip = 187;
	static const int mediaUiEmptyAgentStatusTitle = 188;
	static const int mediaUiEmptyAgentStatusText = 189;
	static const int mediaUiEmptyMediaStatusTitle = 190;
	static const int mediaUiEmptyMediaStatusText = 191;
	static const int mediaUiEmptyMediaStreamStatusTitle = 192;
	static const int mediaUiEmptyMediaStreamStatusText = 193;
	static const int mediaUiHelpTitle = 194;
	static const int mediaUiHelpText = 195;
	static const int mediaUiHelpAction1Text = 196;
	static const int mediaUiHelpAction2Text = 197;
	static const int mediaUiHelpAction3Text = 198;
	static const int mediaUiHelpAction4Text = 199;
	static const int mediaUiNoMediaSelectedPrompt = 200;
	static const int mediaUiNoStreamableMediaSelectedPrompt = 201;
	static const int mediaUiNoMonitorSelectedPrompt = 202;
	static const int mediaUiNoPlaylistSelectedPrompt = 203;
	static const int mediaUiAddPlaylistItemTooltip = 204;
	static const int mediaUiCreatePlaylistTooltip = 205;
	static const int mediaUiWritePlaylistTooltip = 206;
	static const int mediaUiPlayTooltip = 207;
	static const int mediaUiStopTooltip = 208;
	static const int mediaUiBrowserPlayTooltip = 209;
	static const int mediaUiConfigureStreamTooltip = 210;
	static const int mediaUiAddCacheStreamTooltip = 211;
	static const int mediaUiDeleteStreamTooltip = 212;
	static const int mediaUiDeletePlaylistTooltip = 213;
	static const int mediaItemUiHelpTitle = 214;
	static const int mediaItemUiHelpText = 215;
	static const int mediaItemUiHelpAction1Text = 216;
	static const int mediaItemUiConfigureStreamTooltip = 217;
	static const int thumbnailImageSizeTooltip = 218;
	static const int uiBackTooltip = 219;
	static const int exitTooltip = 220;
	static const int moreHelpTopics = 221;
	static const int reloadTooltip = 222;
	static const int mainUiHelpTitle = 223;
	static const int mainUiHelpText = 224;
	static const int mainUiServersHelpActionText = 225;
	static const int mainUiShowAllTooltip = 226;
	static const int mainUiNextBannerTooltip = 227;
	static const int openHelpUrlError = 228;
	static const int openAboutUrlError = 229;
	static const int openFeedbackUrlError = 230;
	static const int openUpdateUrlError = 231;
	static const int launchedWebBrowser = 232;
	static const int launchWebBrowserError = 233;
	static const int sentBroadcast = 234;
	static const int invokeGetStatusAddressEmptyError = 235;
	static const int invokeGetStatusAddressParseError = 236;
	static const int internalError = 237;
	static const int serverContactError = 238;
	static const int invokeClearDisplayMessage = 239;
	static const int invokeCreateWebPlaylistMessage = 240;
	static const int invokeShowWebUrlMessage = 241;
	static const int webKioskUiHelpTitle = 242;
	static const int webKioskUiHelpText = 243;
	static const int webKioskUiHelpAction1Text = 244;
	static const int webKioskUiHelpAction2Text = 245;
	static const int webKioskUiAddUrlTooltip = 246;
	static const int webKioskUiBrowseUrlTooltip = 247;
	static const int webKioskUiShowUrlTooltip = 248;
	static const int webKioskUiAddPlaylistTooltip = 249;
	static const int webKioskUiClearDisplayTooltip = 250;
	static const int webKioskUiWritePlaylistTooltip = 251;
	static const int webKioskUiDeletePlaylistTooltip = 252;
	static const int webKioskNoAgentsSelectedPrompt = 253;
	static const int webKioskNoPlaylistsSelectedPrompt = 254;
	static const int webKioskNoAddressEnteredPrompt = 255;
	static const int websiteAddedMessage = 256;
	static const int webPlaylistCreatedMessage = 257;
	static const int shuffleTooltip = 258;
	static const int clickRenameTooltip = 259;
	static const int hyperlinkTooltip = 260;
	static const int serverAdminUiEmptyTaskListTitle = 261;
	static const int serverAdminUiEmptyTaskListText = 262;
	static const int serverAdminUiHelpTitle = 263;
	static const int serverAdminUiHelpText = 264;
	static const int serverAdminUiHelpAction1Text = 265;
	static const int serverAdminUiLockTooltip = 266;
	static const int serverAdminUiEmptyAdminPasswordText = 267;
	static const int serverAdminUiAdminPasswordDescription = 268;
	static const int serverAdminUiSetPasswordCompleteMessage = 269;
	static const int emptyStreamPlaylistText = 270;
	static const int invokeCreateCacheStreamMessage = 271;
	static const int invokeConfigureMediaStreamMessage = 272;
	static const int streamPlaylistCreatedMessage = 273;
	static const int streamItemAddedMessage = 274;
	static const int invokePlayMediaMessage = 275;
	static const int invokeCreateStreamPlaylistMessage = 276;
	static const int invokeRemoveStreamMessage = 277;
	static const int streamItemUiHelpTitle = 278;
	static const int streamItemUiHelpText = 279;
	static const int streamItemUiHelpAction1Text = 280;
	static const int getAgentConfigurationServerContactError = 281;
	static const int updateAgentConfigurationMessage = 282;
	static const int monitorCacheUiHelpTitle = 283;
	static const int monitorCacheUiHelpText = 284;
	static const int monitorCacheUiHelpAction1Text = 285;
	static const int monitorCacheUiHelpAction2Text = 286;
	static const int monitorCacheUiEmptyStreamStatusTitle = 287;
	static const int monitorCacheUiEmptyStreamStatusText = 288;
	static const int monitorCacheUiNoStreamSelectedPrompt = 289;
	static const int monitorCacheUiPlayTooltip = 290;
	static const int monitorCacheUiWritePlaylistTooltip = 291;
	static const int monitorCacheUiStopTooltip = 292;
	static const int monitorCacheUiDeleteTooltip = 293;
	static const int monitorCacheUiWritePlaylistMessage = 294;
	static const int viewTimelineImagesTooltip = 295;
	static const int unselectButtonTooltip = 296;
	static const int monitorActivityIconTooltip = 297;
	static const int storageTooltip = 298;
	static const int cachedStream = 299;
	static const int cachedStreams = 300;
	static const int taskInProgress = 301;
	static const int tasksInProgress = 302;
	static const int removeMedia = 303;
	static const int removeStream = 304;
	static const int deleteStream = 305;
	static const int cacheStream = 306;
	static const int mediaStreaming = 307;
	static const int serversHelpTitle = 308;
	static const int membraneSoftwareOverviewHelpTitle = 309;
	static const int serverPasswords = 310;
	static const int addAdminPasswordTooltip = 311;
	static const int createAdminPassword = 312;
	static const int setAdminPassword = 313;
	static const int adminPasswordNamePrompt = 314;
	static const int adminPasswordPrompt = 315;
	static const int emptyAdminSecretListText = 316;
	static const int normalVideoQualityDescription = 317;
	static const int highVideoQualityDescription = 318;
	static const int lowVideoQualityDescription = 319;
	static const int lowestVideoQualityDescription = 320;
	static const int videoQualityDescription = 321;
	static const int firstLaunchBannerMessage = 322;
	static const int helpKeyBannerMessage = 323;
	static const int freeApplicationBannerMessage = 324;
	static const int membraneSoftwareBannerMessage = 325;
	static const int donateBannerMessage = 326;
	static const int mediaAttributesTooltip = 327;
	static const int toolbarModeButtonTooltip = 328;
	static const int streamIconTooltip = 329;
	static const int createStreamUnavailableTooltip = 330;
	static const int removeMediaActionText = 331;
	static const int viewMonitorCacheTooltip = 332;
	static const int cameras = 333;
	static const int cameraUiDescription = 334;
	static const int cameraUiNoCameraSelectedPrompt = 335;
	static const int cameraUiConfigureTimelapseTooltip = 336;
	static const int cameraUiAutoReloadTooltip = 337;
	static const int cameraUiClearTimelapseTooltip = 338;
	static const int cameraUiHelpTitle = 339;
	static const int cameraUiHelpText = 340;
	static const int cameraUiHelpAction1Text = 341;
	static const int cameraUiEmptyAgentStatusTitle = 342;
	static const int cameraUiEmptyAgentStatusText = 343;
	static const int setTimelapse = 344;
	static const int clearTimelapse = 345;
	static const int timelapseImages = 346;
	static const int cameraConnected = 347;
	static const int camerasConnected = 348;
	static const int enableCapture = 349;
	static const int enableCaptureDescription = 350;
	static const int imageQuality = 351;
	static const int imageQualityDescription = 352;
	static const int normalImageQualityDescription = 353;
	static const int highImageQualityDescription = 354;
	static const int lowImageQualityDescription = 355;
	static const int lowestImageQualityDescription = 356;
	static const int capturePeriod = 357;
	static const int capturePeriodDescription = 358;
	static const int disableCameraTimelapseMessage = 359;
	static const int configureCameraTimelapseMessage = 360;
	static const int clearCameraTimelapseMessage = 361;
	static const int cameraActivityIconTooltip = 362;
	static const int images = 363;
	static const int before = 364;
	static const int after = 365;
	static const int captureTimespanTooltip = 366;
	static const int selectedCaptureTimespanTooltip = 367;
	static const int sortCaptureImagesTooltip = 368;
	static const int sortByName = 369;
	static const int sortByNewest = 370;
	static const int continuous = 371;
	static const int cameraTimelineUiHelpTitle = 372;
	static const int cameraTimelineUiHelpText = 373;
	static const int playCaptureTooltip = 374;
};

#endif
