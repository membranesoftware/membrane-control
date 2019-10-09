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
	static const int selectedToggleTooltip = 125;
	static const int unselectedToggleTooltip = 126;
	static const int expand = 127;
	static const int minimize = 128;
	static const int expandAll = 129;
	static const int minimizeAll = 130;
	static const int mainMenuTooltip = 131;
	static const int moreActionsTooltip = 132;
	static const int textFieldEnterTooltip = 133;
	static const int textFieldPasteTooltip = 134;
	static const int textFieldClearTooltip = 135;
	static const int textFieldRandomizeTooltip = 136;
	static const int textFieldVisibilityToggleTooltip = 137;
	static const int serverConnected = 138;
	static const int serversConnected = 139;
	static const int mediaServerConnected = 140;
	static const int mediaServersConnected = 141;
	static const int videoFileInCatalog = 142;
	static const int videoFilesInCatalog = 143;
	static const int monitorConnected = 144;
	static const int monitorsConnected = 145;
	static const int videoStreamPlayable = 146;
	static const int videoStreamsPlayable = 147;
	static const int webPlaylistItem = 148;
	static const int webPlaylistItems = 149;
	static const int streamPlaylistItem = 150;
	static const int streamPlaylistItems = 151;
	static const int uiLaunchOpenButtonTooltip = 152;
	static const int serverUiHelpTitle = 153;
	static const int serverUiHelpText = 154;
	static const int serverUiHelpAction1Text = 155;
	static const int serverUiHelpAction2Text = 156;
	static const int serverUiAttachedAgentsTitle = 157;
	static const int serverUiUnattachedAgentsTitle = 158;
	static const int serverUiEmptyAgentStatusTitle = 159;
	static const int serverUiEmptyAgentStatusText1 = 160;
	static const int serverUiEmptyAgentStatusText2 = 161;
	static const int serverUiAddressTooltip = 162;
	static const int serverUiBroadcastTooltip = 163;
	static const int serverUiContactingAgentMessage = 164;
	static const int serverUiContactingAgentDescription = 165;
	static const int serverUiUnauthorizedErrorTitle = 166;
	static const int serverUiUnauthorizedErrorText = 167;
	static const int serverUiFailedContactErrorTitle = 168;
	static const int serverUiFailedContactErrorText = 169;
	static const int sourceMediaPath = 170;
	static const int sourceMediaPathDescription = 171;
	static const int sourceMediaPathPrompt = 172;
	static const int sourceMediaPathPromptWindows = 173;
	static const int mediaDataPath = 174;
	static const int mediaDataPathDescription = 175;
	static const int mediaDataPathPrompt = 176;
	static const int mediaDataPathPromptWindows = 177;
	static const int mediaScanPeriod = 178;
	static const int mediaScanPeriodDescription = 179;
	static const int mediaScanPeriodNeverDescription = 180;
	static const int scanForMedia = 181;
	static const int scanForMediaStartedMessage = 182;
	static const int streamDataPath = 183;
	static const int streamDataPathDescription = 184;
	static const int streamDataPathPrompt = 185;
	static const int streamDataPathPromptWindows = 186;
	static const int agentEnabledDescription = 187;
	static const int agentDisplayNameDescription = 188;
	static const int agentDisplayNamePrompt = 189;
	static const int mediaUiShowMediaWithoutStreamsAction = 190;
	static const int mediaUiSearchTooltip = 191;
	static const int mediaUiSortTooltip = 192;
	static const int mediaUiEmptyAgentStatusTitle = 193;
	static const int mediaUiEmptyAgentStatusText = 194;
	static const int mediaUiEmptyMediaStatusTitle = 195;
	static const int mediaUiEmptyMediaStatusText = 196;
	static const int mediaUiEmptyMediaStreamStatusTitle = 197;
	static const int mediaUiEmptyMediaStreamStatusText = 198;
	static const int mediaUiHelpTitle = 199;
	static const int mediaUiHelpText = 200;
	static const int mediaUiHelpAction1Text = 201;
	static const int mediaUiHelpAction2Text = 202;
	static const int mediaUiHelpAction3Text = 203;
	static const int mediaUiHelpAction4Text = 204;
	static const int mediaUiNoMediaSelectedPrompt = 205;
	static const int mediaUiNoStreamableMediaSelectedPrompt = 206;
	static const int mediaUiNoMonitorSelectedPrompt = 207;
	static const int mediaUiNoPlaylistSelectedPrompt = 208;
	static const int mediaUiEmptyPlaylistSelectedPrompt = 209;
	static const int mediaUiAddPlaylistItemTooltip = 210;
	static const int mediaUiCreatePlaylistTooltip = 211;
	static const int mediaUiWritePlaylistTooltip = 212;
	static const int mediaUiPlayTooltip = 213;
	static const int mediaUiStopTooltip = 214;
	static const int mediaUiBrowserPlayTooltip = 215;
	static const int mediaUiConfigureStreamTooltip = 216;
	static const int mediaUiAddCacheStreamTooltip = 217;
	static const int mediaUiDeleteStreamTooltip = 218;
	static const int mediaUiDeletePlaylistTooltip = 219;
	static const int mediaItemUiHelpTitle = 220;
	static const int mediaItemUiHelpText = 221;
	static const int mediaItemUiConfigureStreamTooltip = 222;
	static const int thumbnailImageSizeTooltip = 223;
	static const int uiBackTooltip = 224;
	static const int exitTooltip = 225;
	static const int moreHelpTopics = 226;
	static const int reloadTooltip = 227;
	static const int mainUiHelpTitle = 228;
	static const int mainUiHelpText = 229;
	static const int mainUiServersHelpActionText = 230;
	static const int mainUiNextBannerTooltip = 231;
	static const int openHelpUrlError = 232;
	static const int openAboutUrlError = 233;
	static const int openFeedbackUrlError = 234;
	static const int openUpdateUrlError = 235;
	static const int launchedWebBrowser = 236;
	static const int launchWebBrowserError = 237;
	static const int sentBroadcast = 238;
	static const int invokeGetStatusAddressEmptyError = 239;
	static const int invokeGetStatusAddressParseError = 240;
	static const int internalError = 241;
	static const int serverContactError = 242;
	static const int invokeClearDisplayMessage = 243;
	static const int invokeCreateWebPlaylistMessage = 244;
	static const int invokeShowWebUrlMessage = 245;
	static const int webKioskUiHelpTitle = 246;
	static const int webKioskUiHelpText = 247;
	static const int webKioskUiHelpAction1Text = 248;
	static const int webKioskUiHelpAction2Text = 249;
	static const int webKioskUiAddUrlTooltip = 250;
	static const int webKioskUiBrowseUrlTooltip = 251;
	static const int webKioskUiShowUrlTooltip = 252;
	static const int webKioskUiAddPlaylistTooltip = 253;
	static const int webKioskUiClearDisplayTooltip = 254;
	static const int webKioskUiWritePlaylistTooltip = 255;
	static const int webKioskUiDeletePlaylistTooltip = 256;
	static const int webKioskUiNoAgentSelectedPrompt = 257;
	static const int webKioskUiNoPlaylistSelectedPrompt = 258;
	static const int webKioskUiEmptyPlaylistSelectedPrompt = 259;
	static const int webKioskUiNoAddressEnteredPrompt = 260;
	static const int websiteAddedMessage = 261;
	static const int webPlaylistCreatedMessage = 262;
	static const int shuffleTooltip = 263;
	static const int clickRenameTooltip = 264;
	static const int hyperlinkTooltip = 265;
	static const int serverAdminUiEmptyTaskListTitle = 266;
	static const int serverAdminUiEmptyTaskListText = 267;
	static const int serverAdminUiHelpTitle = 268;
	static const int serverAdminUiHelpText = 269;
	static const int serverAdminUiHelpAction1Text = 270;
	static const int serverAdminUiLockTooltip = 271;
	static const int serverAdminUiEmptyAdminPasswordText = 272;
	static const int serverAdminUiAdminPasswordDescription = 273;
	static const int serverAdminUiSetPasswordCompleteMessage = 274;
	static const int emptyStreamPlaylistText = 275;
	static const int invokeCreateCacheStreamMessage = 276;
	static const int invokeConfigureMediaStreamMessage = 277;
	static const int streamPlaylistCreatedMessage = 278;
	static const int streamItemAddedMessage = 279;
	static const int invokePlayMediaMessage = 280;
	static const int invokeCreateStreamPlaylistMessage = 281;
	static const int invokeRemoveStreamMessage = 282;
	static const int streamItemUiHelpTitle = 283;
	static const int streamItemUiHelpText = 284;
	static const int streamItemUiHelpAction1Text = 285;
	static const int getAgentConfigurationServerContactError = 286;
	static const int updateAgentConfigurationMessage = 287;
	static const int monitorCacheUiHelpTitle = 288;
	static const int monitorCacheUiHelpText = 289;
	static const int monitorCacheUiHelpAction1Text = 290;
	static const int monitorCacheUiHelpAction2Text = 291;
	static const int monitorCacheUiEmptyStreamStatusTitle = 292;
	static const int monitorCacheUiEmptyStreamStatusText = 293;
	static const int monitorCacheUiNoStreamSelectedPrompt = 294;
	static const int monitorCacheUiPlayTooltip = 295;
	static const int monitorCacheUiWritePlaylistTooltip = 296;
	static const int monitorCacheUiStopTooltip = 297;
	static const int monitorCacheUiDeleteTooltip = 298;
	static const int monitorCacheUiWritePlaylistMessage = 299;
	static const int viewTimelineImagesTooltip = 300;
	static const int monitorActivityIconTooltip = 301;
	static const int storageTooltip = 302;
	static const int cachedStream = 303;
	static const int cachedStreams = 304;
	static const int taskInProgress = 305;
	static const int tasksInProgress = 306;
	static const int removeMedia = 307;
	static const int removeStream = 308;
	static const int deleteStream = 309;
	static const int cacheStream = 310;
	static const int mediaStreaming = 311;
	static const int serversHelpTitle = 312;
	static const int membraneSoftwareOverviewHelpTitle = 313;
	static const int serverPasswords = 314;
	static const int addAdminPasswordTooltip = 315;
	static const int createAdminPassword = 316;
	static const int setAdminPassword = 317;
	static const int adminPasswordNamePrompt = 318;
	static const int adminPasswordPrompt = 319;
	static const int emptyAdminSecretListText = 320;
	static const int normalVideoQualityDescription = 321;
	static const int highVideoQualityDescription = 322;
	static const int lowVideoQualityDescription = 323;
	static const int lowestVideoQualityDescription = 324;
	static const int videoQualityDescription = 325;
	static const int firstLaunchBannerMessage = 326;
	static const int helpKeyBannerMessage = 327;
	static const int freeApplicationBannerMessage = 328;
	static const int membraneSoftwareBannerMessage = 329;
	static const int donateBannerMessage = 330;
	static const int mouseHoverBannerMessage = 331;
	static const int mediaAttributesTooltip = 332;
	static const int toolbarModeButtonTooltip = 333;
	static const int streamIconTooltip = 334;
	static const int createStreamUnavailableTooltip = 335;
	static const int removeMediaActionText = 336;
	static const int viewMonitorCacheTooltip = 337;
	static const int cameras = 338;
	static const int cameraUiDescription = 339;
	static const int cameraUiNoCameraSelectedPrompt = 340;
	static const int cameraUiConfigureTimelapseTooltip = 341;
	static const int cameraUiAutoReloadTooltip = 342;
	static const int cameraUiClearTimelapseTooltip = 343;
	static const int cameraUiHelpTitle = 344;
	static const int cameraUiHelpText = 345;
	static const int cameraUiHelpAction1Text = 346;
	static const int cameraUiEmptyAgentStatusTitle = 347;
	static const int cameraUiEmptyAgentStatusText = 348;
	static const int setTimelapse = 349;
	static const int clearTimelapse = 350;
	static const int timelapseImages = 351;
	static const int cameraConnected = 352;
	static const int camerasConnected = 353;
	static const int enableCapture = 354;
	static const int enableCaptureDescription = 355;
	static const int imageQuality = 356;
	static const int imageQualityDescription = 357;
	static const int normalImageQualityDescription = 358;
	static const int highImageQualityDescription = 359;
	static const int lowImageQualityDescription = 360;
	static const int lowestImageQualityDescription = 361;
	static const int capturePeriod = 362;
	static const int capturePeriodDescription = 363;
	static const int disableCameraTimelapseMessage = 364;
	static const int configureCameraTimelapseMessage = 365;
	static const int clearCameraTimelapseMessage = 366;
	static const int cameraActivityIconTooltip = 367;
	static const int before = 368;
	static const int after = 369;
	static const int captureTimespanTooltip = 370;
	static const int selectedCaptureTimespanTooltip = 371;
	static const int sortCaptureImagesTooltip = 372;
	static const int sortByName = 373;
	static const int sortByNewest = 374;
	static const int continuous = 375;
	static const int cameraTimelineUiHelpTitle = 376;
	static const int cameraTimelineUiHelpText = 377;
	static const int playCaptureTooltip = 378;
	static const int commands = 379;
	static const int commandUiDescription = 380;
	static const int clearMonitors = 381;
	static const int playMedia = 382;
	static const int runMediaPlaylist = 383;
	static const int runWebsitePlaylist = 384;
	static const int storedCommand = 385;
	static const int storedCommands = 386;
	static const int recentCommand = 387;
	static const int recentCommands = 388;
	static const int unknownServer = 389;
	static const int unknownServers = 390;
	static const int commandUiEmptyStatusTitle = 391;
	static const int commandUiEmptyStatusText = 392;
	static const int storedCommandFailedExecutionError = 393;
	static const int storedCommandExecutedMessage = 394;
	static const int commandDetails = 395;
	static const int executeCommand = 396;
	static const int deleteCommand = 397;
	static const int saveCommandTooltip = 398;
	static const int commandUiHelpTitle = 399;
	static const int commandUiHelpText = 400;
	static const int commandUiHelpActionText = 401;
};

#endif
