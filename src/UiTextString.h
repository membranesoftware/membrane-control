/*
* Copyright 2018-2019 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
	static const int passwordLocked = 14;
	static const int address = 15;
	static const int version = 16;
	static const int platform = 17;
	static const int uptime = 18;
	static const int status = 19;
	static const int active = 20;
	static const int inactive = 21;
	static const int playing = 22;
	static const int showing = 23;
	static const int exit = 24;
	static const int select = 25;
	static const int clear = 26;
	static const int settings = 27;
	static const int about = 28;
	static const int checkForUpdates = 29;
	static const int sendFeedback = 30;
	static const int windowSize = 31;
	static const int textSize = 32;
	static const int showClock = 33;
	static const int showInterfaceAnimations = 34;
	static const int media = 35;
	static const int mediaFile = 36;
	static const int mediaFiles = 37;
	static const int videoStream = 38;
	static const int videoStreams = 39;
	static const int videoQuality = 40;
	static const int storage = 41;
	static const int webKiosk = 42;
	static const int webKiosks = 43;
	static const int createPlaylist = 44;
	static const int runPlaylist = 45;
	static const int addPlaylistItem = 46;
	static const int deletePlaylist = 47;
	static const int playAll = 48;
	static const int cache = 49;
	static const int shuffle = 50;
	static const int startPosition = 51;
	static const int playDuration = 52;
	static const int display = 53;
	static const int displayName = 54;
	static const int online = 55;
	static const int offline = 56;
	static const int contacting = 57;
	static const int monitors = 58;
	static const int server = 59;
	static const int servers = 60;
	static const int removeServer = 61;
	static const int removeAttachedServerDescription = 62;
	static const int removeUnattachedServerDescription = 63;
	static const int attachServerTooltip = 64;
	static const int detachServerTooltip = 65;
	static const int tasks = 66;
	static const int playlist = 67;
	static const int playlists = 68;
	static const int speed = 69;
	static const int smallest = 70;
	static const int small = 71;
	static const int medium = 72;
	static const int large = 73;
	static const int largest = 74;
	static const int slowest = 75;
	static const int slow = 76;
	static const int fast = 77;
	static const int fastest = 78;
	static const int never = 79;
	static const int play = 80;
	static const int stop = 81;
	static const int enabled = 82;
	static const int disabled = 83;
	static const int learnMore = 84;
	static const int searchForHelp = 85;
	static const int startedCreatingStream = 86;
	static const int removedStream = 87;
	static const int create = 88;
	static const int open = 89;
	static const int openHelp = 90;
	static const int loading = 91;
	static const int openAdminConsole = 92;
	static const int mainMenu = 93;
	static const int bitrate = 94;
	static const int frameSize = 95;
	static const int fileSize = 96;
	static const int duration = 97;
	static const int frameRate = 98;
	static const int playStreams = 99;
	static const int playInBrowser = 100;
	static const int manageStreams = 101;
	static const int managePlaylists = 102;
	static const int configureStream = 103;
	static const int streamServer = 104;
	static const int configure = 105;
	static const int configuration = 106;
	static const int save = 107;
	static const int apply = 108;
	static const int websiteList = 109;
	static const int addWebsite = 110;
	static const int showWebsite = 111;
	static const int browseWebsite = 112;
	static const int viewWebsites = 113;
	static const int serverUiDescription = 114;
	static const int mediaUiDescription = 115;
	static const int webKioskUiDescription = 116;
	static const int selectPlayPosition = 117;
	static const int enterAddressPrompt = 118;
	static const int enterUrlPrompt = 119;
	static const int enterWebPlaylistNamePrompt = 120;
	static const int enterPlaylistNamePrompt = 121;
	static const int enterSearchKeyPrompt = 122;
	static const int enterStoredCommandNamePrompt = 123;
	static const int emptyWebPlaylistAddressList = 124;
	static const int selectToggleTooltip = 125;
	static const int expandToggleTooltip = 126;
	static const int mainMenuTooltip = 127;
	static const int moreActionsTooltip = 128;
	static const int textFieldEnterTooltip = 129;
	static const int textFieldPasteTooltip = 130;
	static const int textFieldClearTooltip = 131;
	static const int textFieldRandomizeTooltip = 132;
	static const int textFieldVisibilityToggleTooltip = 133;
	static const int serverConnected = 134;
	static const int serversConnected = 135;
	static const int mediaServerConnected = 136;
	static const int mediaServersConnected = 137;
	static const int videoFileInCatalog = 138;
	static const int videoFilesInCatalog = 139;
	static const int monitorConnected = 140;
	static const int monitorsConnected = 141;
	static const int videoStreamPlayable = 142;
	static const int videoStreamsPlayable = 143;
	static const int webPlaylistItem = 144;
	static const int webPlaylistItems = 145;
	static const int streamPlaylistItem = 146;
	static const int streamPlaylistItems = 147;
	static const int uiLaunchOpenButtonTooltip = 148;
	static const int serverUiHelpTitle = 149;
	static const int serverUiHelpText = 150;
	static const int serverUiHelpAction1Text = 151;
	static const int serverUiHelpAction2Text = 152;
	static const int serverUiAttachedAgentsTitle = 153;
	static const int serverUiUnattachedAgentsTitle = 154;
	static const int serverUiEmptyAgentStatusTitle = 155;
	static const int serverUiEmptyAgentStatusText1 = 156;
	static const int serverUiEmptyAgentStatusText2 = 157;
	static const int serverUiAddressTooltip = 158;
	static const int serverUiBroadcastTooltip = 159;
	static const int serverUiContactingAgentMessage = 160;
	static const int serverUiContactingAgentDescription = 161;
	static const int serverUiUnauthorizedErrorTitle = 162;
	static const int serverUiUnauthorizedErrorText = 163;
	static const int serverUiFailedContactErrorTitle = 164;
	static const int serverUiFailedContactErrorText = 165;
	static const int sourceMediaPath = 166;
	static const int sourceMediaPathDescription = 167;
	static const int sourceMediaPathPrompt = 168;
	static const int sourceMediaPathPromptWindows = 169;
	static const int mediaDataPath = 170;
	static const int mediaDataPathDescription = 171;
	static const int mediaDataPathPrompt = 172;
	static const int mediaDataPathPromptWindows = 173;
	static const int mediaScanPeriod = 174;
	static const int mediaScanPeriodDescription = 175;
	static const int mediaScanPeriodNeverDescription = 176;
	static const int scanForMedia = 177;
	static const int scanForMediaStartedMessage = 178;
	static const int streamDataPath = 179;
	static const int streamDataPathDescription = 180;
	static const int streamDataPathPrompt = 181;
	static const int streamDataPathPromptWindows = 182;
	static const int agentEnabledDescription = 183;
	static const int agentDisplayNameDescription = 184;
	static const int agentDisplayNamePrompt = 185;
	static const int mediaUiShowMediaWithoutStreamsAction = 186;
	static const int mediaUiSearchTooltip = 187;
	static const int mediaUiSortTooltip = 188;
	static const int mediaUiEmptyAgentStatusTitle = 189;
	static const int mediaUiEmptyAgentStatusText = 190;
	static const int mediaUiEmptyMediaStatusTitle = 191;
	static const int mediaUiEmptyMediaStatusText = 192;
	static const int mediaUiEmptyMediaStreamStatusTitle = 193;
	static const int mediaUiEmptyMediaStreamStatusText = 194;
	static const int mediaUiHelpTitle = 195;
	static const int mediaUiHelpText = 196;
	static const int mediaUiHelpAction1Text = 197;
	static const int mediaUiHelpAction2Text = 198;
	static const int mediaUiHelpAction3Text = 199;
	static const int mediaUiHelpAction4Text = 200;
	static const int mediaUiNoMediaSelectedPrompt = 201;
	static const int mediaUiNoStreamableMediaSelectedPrompt = 202;
	static const int mediaUiNoMonitorSelectedPrompt = 203;
	static const int mediaUiNoPlaylistSelectedPrompt = 204;
	static const int mediaUiEmptyPlaylistSelectedPrompt = 205;
	static const int mediaUiAddPlaylistItemTooltip = 206;
	static const int mediaUiCreatePlaylistTooltip = 207;
	static const int mediaUiWritePlaylistTooltip = 208;
	static const int mediaUiPlayTooltip = 209;
	static const int mediaUiStopTooltip = 210;
	static const int mediaUiBrowserPlayTooltip = 211;
	static const int mediaUiConfigureStreamTooltip = 212;
	static const int mediaUiAddCacheStreamTooltip = 213;
	static const int mediaUiDeleteStreamTooltip = 214;
	static const int mediaUiDeletePlaylistTooltip = 215;
	static const int mediaItemUiHelpTitle = 216;
	static const int mediaItemUiHelpText = 217;
	static const int mediaItemUiHelpAction1Text = 218;
	static const int mediaItemUiConfigureStreamTooltip = 219;
	static const int thumbnailImageSizeTooltip = 220;
	static const int uiBackTooltip = 221;
	static const int exitTooltip = 222;
	static const int moreHelpTopics = 223;
	static const int reloadTooltip = 224;
	static const int mainUiHelpTitle = 225;
	static const int mainUiHelpText = 226;
	static const int mainUiServersHelpActionText = 227;
	static const int mainUiShowAllTooltip = 228;
	static const int mainUiNextBannerTooltip = 229;
	static const int openHelpUrlError = 230;
	static const int openAboutUrlError = 231;
	static const int openFeedbackUrlError = 232;
	static const int openUpdateUrlError = 233;
	static const int launchedWebBrowser = 234;
	static const int launchWebBrowserError = 235;
	static const int sentBroadcast = 236;
	static const int invokeGetStatusAddressEmptyError = 237;
	static const int invokeGetStatusAddressParseError = 238;
	static const int internalError = 239;
	static const int serverContactError = 240;
	static const int invokeClearDisplayMessage = 241;
	static const int invokeCreateWebPlaylistMessage = 242;
	static const int invokeShowWebUrlMessage = 243;
	static const int webKioskUiHelpTitle = 244;
	static const int webKioskUiHelpText = 245;
	static const int webKioskUiHelpAction1Text = 246;
	static const int webKioskUiHelpAction2Text = 247;
	static const int webKioskUiAddUrlTooltip = 248;
	static const int webKioskUiBrowseUrlTooltip = 249;
	static const int webKioskUiShowUrlTooltip = 250;
	static const int webKioskUiAddPlaylistTooltip = 251;
	static const int webKioskUiClearDisplayTooltip = 252;
	static const int webKioskUiWritePlaylistTooltip = 253;
	static const int webKioskUiDeletePlaylistTooltip = 254;
	static const int webKioskUiNoAgentSelectedPrompt = 255;
	static const int webKioskUiNoPlaylistSelectedPrompt = 256;
	static const int webKioskUiEmptyPlaylistSelectedPrompt = 257;
	static const int webKioskUiNoAddressEnteredPrompt = 258;
	static const int websiteAddedMessage = 259;
	static const int webPlaylistCreatedMessage = 260;
	static const int shuffleTooltip = 261;
	static const int clickRenameTooltip = 262;
	static const int hyperlinkTooltip = 263;
	static const int serverAdminUiEmptyTaskListTitle = 264;
	static const int serverAdminUiEmptyTaskListText = 265;
	static const int serverAdminUiHelpTitle = 266;
	static const int serverAdminUiHelpText = 267;
	static const int serverAdminUiHelpAction1Text = 268;
	static const int serverAdminUiLockTooltip = 269;
	static const int serverAdminUiEmptyAdminPasswordText = 270;
	static const int serverAdminUiAdminPasswordDescription = 271;
	static const int serverAdminUiSetPasswordCompleteMessage = 272;
	static const int emptyStreamPlaylistText = 273;
	static const int invokeCreateCacheStreamMessage = 274;
	static const int invokeConfigureMediaStreamMessage = 275;
	static const int streamPlaylistCreatedMessage = 276;
	static const int streamItemAddedMessage = 277;
	static const int invokePlayMediaMessage = 278;
	static const int invokeCreateStreamPlaylistMessage = 279;
	static const int invokeRemoveStreamMessage = 280;
	static const int streamItemUiHelpTitle = 281;
	static const int streamItemUiHelpText = 282;
	static const int streamItemUiHelpAction1Text = 283;
	static const int getAgentConfigurationServerContactError = 284;
	static const int updateAgentConfigurationMessage = 285;
	static const int monitorCacheUiHelpTitle = 286;
	static const int monitorCacheUiHelpText = 287;
	static const int monitorCacheUiHelpAction1Text = 288;
	static const int monitorCacheUiHelpAction2Text = 289;
	static const int monitorCacheUiEmptyStreamStatusTitle = 290;
	static const int monitorCacheUiEmptyStreamStatusText = 291;
	static const int monitorCacheUiNoStreamSelectedPrompt = 292;
	static const int monitorCacheUiPlayTooltip = 293;
	static const int monitorCacheUiWritePlaylistTooltip = 294;
	static const int monitorCacheUiStopTooltip = 295;
	static const int monitorCacheUiDeleteTooltip = 296;
	static const int monitorCacheUiWritePlaylistMessage = 297;
	static const int viewTimelineImagesTooltip = 298;
	static const int monitorActivityIconTooltip = 299;
	static const int storageTooltip = 300;
	static const int cachedStream = 301;
	static const int cachedStreams = 302;
	static const int taskInProgress = 303;
	static const int tasksInProgress = 304;
	static const int removeMedia = 305;
	static const int removeStream = 306;
	static const int deleteStream = 307;
	static const int cacheStream = 308;
	static const int mediaStreaming = 309;
	static const int serversHelpTitle = 310;
	static const int membraneSoftwareOverviewHelpTitle = 311;
	static const int serverPasswords = 312;
	static const int addAdminPasswordTooltip = 313;
	static const int createAdminPassword = 314;
	static const int setAdminPassword = 315;
	static const int adminPasswordNamePrompt = 316;
	static const int adminPasswordPrompt = 317;
	static const int emptyAdminSecretListText = 318;
	static const int normalVideoQualityDescription = 319;
	static const int highVideoQualityDescription = 320;
	static const int lowVideoQualityDescription = 321;
	static const int lowestVideoQualityDescription = 322;
	static const int videoQualityDescription = 323;
	static const int firstLaunchBannerMessage = 324;
	static const int helpKeyBannerMessage = 325;
	static const int freeApplicationBannerMessage = 326;
	static const int membraneSoftwareBannerMessage = 327;
	static const int donateBannerMessage = 328;
	static const int mediaAttributesTooltip = 329;
	static const int toolbarModeButtonTooltip = 330;
	static const int streamIconTooltip = 331;
	static const int createStreamUnavailableTooltip = 332;
	static const int removeMediaActionText = 333;
	static const int viewMonitorCacheTooltip = 334;
	static const int cameras = 335;
	static const int cameraUiDescription = 336;
	static const int cameraUiNoCameraSelectedPrompt = 337;
	static const int cameraUiConfigureTimelapseTooltip = 338;
	static const int cameraUiAutoReloadTooltip = 339;
	static const int cameraUiClearTimelapseTooltip = 340;
	static const int cameraUiHelpTitle = 341;
	static const int cameraUiHelpText = 342;
	static const int cameraUiHelpAction1Text = 343;
	static const int cameraUiEmptyAgentStatusTitle = 344;
	static const int cameraUiEmptyAgentStatusText = 345;
	static const int setTimelapse = 346;
	static const int clearTimelapse = 347;
	static const int timelapseImages = 348;
	static const int cameraConnected = 349;
	static const int camerasConnected = 350;
	static const int enableCapture = 351;
	static const int enableCaptureDescription = 352;
	static const int imageQuality = 353;
	static const int imageQualityDescription = 354;
	static const int normalImageQualityDescription = 355;
	static const int highImageQualityDescription = 356;
	static const int lowImageQualityDescription = 357;
	static const int lowestImageQualityDescription = 358;
	static const int capturePeriod = 359;
	static const int capturePeriodDescription = 360;
	static const int disableCameraTimelapseMessage = 361;
	static const int configureCameraTimelapseMessage = 362;
	static const int clearCameraTimelapseMessage = 363;
	static const int cameraActivityIconTooltip = 364;
	static const int before = 365;
	static const int after = 366;
	static const int captureTimespanTooltip = 367;
	static const int selectedCaptureTimespanTooltip = 368;
	static const int sortCaptureImagesTooltip = 369;
	static const int sortByName = 370;
	static const int sortByNewest = 371;
	static const int continuous = 372;
	static const int cameraTimelineUiHelpTitle = 373;
	static const int cameraTimelineUiHelpText = 374;
	static const int playCaptureTooltip = 375;
	static const int commands = 376;
	static const int commandUiDescription = 377;
	static const int clearMonitors = 378;
	static const int playMedia = 379;
	static const int runMediaPlaylist = 380;
	static const int runWebsitePlaylist = 381;
	static const int storedCommand = 382;
	static const int storedCommands = 383;
	static const int recentCommand = 384;
	static const int recentCommands = 385;
	static const int unknownServer = 386;
	static const int unknownServers = 387;
	static const int commandUiEmptyStatusTitle = 388;
	static const int commandUiEmptyStatusText = 389;
	static const int storedCommandFailedExecutionError = 390;
	static const int storedCommandExecutedMessage = 391;
	static const int commandDetails = 392;
	static const int executeCommand = 393;
	static const int deleteCommand = 394;
	static const int saveCommandTooltip = 395;
	static const int commandUiHelpTitle = 396;
	static const int commandUiHelpText = 397;
	static const int commandUiHelpActionText = 398;
};

#endif
