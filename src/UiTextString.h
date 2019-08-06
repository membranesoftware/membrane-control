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
	static const int mediaItems = 38;
	static const int videoStream = 39;
	static const int videoStreams = 40;
	static const int videoQuality = 41;
	static const int storage = 42;
	static const int webKiosk = 43;
	static const int webKiosks = 44;
	static const int runProgram = 45;
	static const int createPlaylist = 46;
	static const int createProgram = 47;
	static const int runPlaylist = 48;
	static const int addPlaylistItem = 49;
	static const int deletePlaylist = 50;
	static const int deleteProgram = 51;
	static const int playAll = 52;
	static const int cache = 53;
	static const int shuffle = 54;
	static const int startPosition = 55;
	static const int playDuration = 56;
	static const int display = 57;
	static const int displayName = 58;
	static const int online = 59;
	static const int offline = 60;
	static const int contacting = 61;
	static const int monitors = 62;
	static const int server = 63;
	static const int servers = 64;
	static const int removeServer = 65;
	static const int removeAttachedServerDescription = 66;
	static const int removeUnattachedServerDescription = 67;
	static const int attachServerTooltip = 68;
	static const int detachServerTooltip = 69;
	static const int tasks = 70;
	static const int program = 71;
	static const int programs = 72;
	static const int playlist = 73;
	static const int playlists = 74;
	static const int speed = 75;
	static const int smallest = 76;
	static const int small = 77;
	static const int medium = 78;
	static const int large = 79;
	static const int largest = 80;
	static const int slowest = 81;
	static const int slow = 82;
	static const int fast = 83;
	static const int fastest = 84;
	static const int never = 85;
	static const int play = 86;
	static const int stop = 87;
	static const int enabled = 88;
	static const int disabled = 89;
	static const int learnMore = 90;
	static const int searchForHelp = 91;
	static const int startedCreatingStream = 92;
	static const int removedStream = 93;
	static const int create = 94;
	static const int open = 95;
	static const int openHelp = 96;
	static const int loading = 97;
	static const int openAdminConsole = 98;
	static const int mainMenu = 99;
	static const int bitrate = 100;
	static const int frameSize = 101;
	static const int fileSize = 102;
	static const int duration = 103;
	static const int frameRate = 104;
	static const int playStreams = 105;
	static const int playInBrowser = 106;
	static const int manageStreams = 107;
	static const int managePlaylists = 108;
	static const int configureStream = 109;
	static const int streamServer = 110;
	static const int configure = 111;
	static const int configuration = 112;
	static const int save = 113;
	static const int apply = 114;
	static const int websiteList = 115;
	static const int addWebsite = 116;
	static const int showWebsite = 117;
	static const int browseWebsite = 118;
	static const int viewWebsites = 119;
	static const int managePrograms = 120;
	static const int serverUiDescription = 121;
	static const int mediaUiDescription = 122;
	static const int webKioskUiDescription = 123;
	static const int selectPlayPosition = 124;
	static const int enterAddressPrompt = 125;
	static const int enterUrlPrompt = 126;
	static const int enterWebPlaylistNamePrompt = 127;
	static const int enterPlaylistNamePrompt = 128;
	static const int enterSearchKeyPrompt = 129;
	static const int emptyWebPlaylistAddressList = 130;
	static const int selectToggleTooltip = 131;
	static const int expandToggleTooltip = 132;
	static const int mainMenuTooltip = 133;
	static const int moreActionsTooltip = 134;
	static const int textFieldEnterTooltip = 135;
	static const int textFieldPasteTooltip = 136;
	static const int textFieldClearTooltip = 137;
	static const int textFieldRandomizeTooltip = 138;
	static const int textFieldVisibilityToggleTooltip = 139;
	static const int serverConnected = 140;
	static const int serversConnected = 141;
	static const int mediaServerConnected = 142;
	static const int mediaServersConnected = 143;
	static const int videoFileInCatalog = 144;
	static const int videoFilesInCatalog = 145;
	static const int monitorConnected = 146;
	static const int monitorsConnected = 147;
	static const int videoStreamPlayable = 148;
	static const int videoStreamsPlayable = 149;
	static const int webPlaylistItem = 150;
	static const int webPlaylistItems = 151;
	static const int streamPlaylistItem = 152;
	static const int streamPlaylistItems = 153;
	static const int uiLaunchOpenButtonTooltip = 154;
	static const int serverUiHelpTitle = 155;
	static const int serverUiHelpText = 156;
	static const int serverUiHelpAction1Text = 157;
	static const int serverUiHelpAction2Text = 158;
	static const int serverUiAttachedAgentsTitle = 159;
	static const int serverUiUnattachedAgentsTitle = 160;
	static const int serverUiEmptyAgentStatusTitle = 161;
	static const int serverUiEmptyAgentStatusText1 = 162;
	static const int serverUiEmptyAgentStatusText2 = 163;
	static const int serverUiAddressTooltip = 164;
	static const int serverUiBroadcastTooltip = 165;
	static const int serverUiContactingAgentMessage = 166;
	static const int serverUiContactingAgentDescription = 167;
	static const int serverUiUnauthorizedErrorTitle = 168;
	static const int serverUiUnauthorizedErrorText = 169;
	static const int serverUiFailedContactErrorTitle = 170;
	static const int serverUiFailedContactErrorText = 171;
	static const int sourceMediaPath = 172;
	static const int sourceMediaPathDescription = 173;
	static const int sourceMediaPathPrompt = 174;
	static const int sourceMediaPathPromptWindows = 175;
	static const int mediaDataPath = 176;
	static const int mediaDataPathDescription = 177;
	static const int mediaDataPathPrompt = 178;
	static const int mediaDataPathPromptWindows = 179;
	static const int mediaScanPeriod = 180;
	static const int mediaScanPeriodDescription = 181;
	static const int mediaScanPeriodNeverDescription = 182;
	static const int scanForMedia = 183;
	static const int scanForMediaStartedMessage = 184;
	static const int streamDataPath = 185;
	static const int streamDataPathDescription = 186;
	static const int streamDataPathPrompt = 187;
	static const int streamDataPathPromptWindows = 188;
	static const int agentEnabledDescription = 189;
	static const int agentDisplayNameDescription = 190;
	static const int agentDisplayNamePrompt = 191;
	static const int mediaUiShowMediaWithoutStreamsAction = 192;
	static const int mediaUiSearchTooltip = 193;
	static const int mediaUiSortTooltip = 194;
	static const int mediaUiEmptyAgentStatusTitle = 195;
	static const int mediaUiEmptyAgentStatusText = 196;
	static const int mediaUiEmptyMediaStatusTitle = 197;
	static const int mediaUiEmptyMediaStatusText = 198;
	static const int mediaUiEmptyMediaStreamStatusTitle = 199;
	static const int mediaUiEmptyMediaStreamStatusText = 200;
	static const int mediaUiHelpTitle = 201;
	static const int mediaUiHelpText = 202;
	static const int mediaUiHelpAction1Text = 203;
	static const int mediaUiHelpAction2Text = 204;
	static const int mediaUiHelpAction3Text = 205;
	static const int mediaUiHelpAction4Text = 206;
	static const int mediaUiNoMediaSelectedPrompt = 207;
	static const int mediaUiNoStreamableMediaSelectedPrompt = 208;
	static const int mediaUiNoMonitorSelectedPrompt = 209;
	static const int mediaUiNoPlaylistSelectedPrompt = 210;
	static const int mediaUiAddPlaylistItemTooltip = 211;
	static const int mediaUiCreatePlaylistTooltip = 212;
	static const int mediaUiWritePlaylistTooltip = 213;
	static const int mediaUiPlayTooltip = 214;
	static const int mediaUiStopTooltip = 215;
	static const int mediaUiBrowserPlayTooltip = 216;
	static const int mediaUiConfigureStreamTooltip = 217;
	static const int mediaUiAddCacheStreamTooltip = 218;
	static const int mediaUiDeleteStreamTooltip = 219;
	static const int mediaUiDeletePlaylistTooltip = 220;
	static const int mediaItemUiHelpTitle = 221;
	static const int mediaItemUiHelpText = 222;
	static const int mediaItemUiHelpAction1Text = 223;
	static const int mediaItemUiConfigureStreamTooltip = 224;
	static const int thumbnailImageSizeTooltip = 225;
	static const int uiBackTooltip = 226;
	static const int exitTooltip = 227;
	static const int moreHelpTopics = 228;
	static const int reloadTooltip = 229;
	static const int mainUiHelpTitle = 230;
	static const int mainUiHelpText = 231;
	static const int mainUiServersHelpActionText = 232;
	static const int mainUiShowAllTooltip = 233;
	static const int mainUiNextBannerTooltip = 234;
	static const int openHelpUrlError = 235;
	static const int openAboutUrlError = 236;
	static const int openFeedbackUrlError = 237;
	static const int openUpdateUrlError = 238;
	static const int launchedWebBrowser = 239;
	static const int launchWebBrowserError = 240;
	static const int sentBroadcast = 241;
	static const int invokeGetStatusAddressEmptyError = 242;
	static const int invokeGetStatusAddressParseError = 243;
	static const int internalError = 244;
	static const int serverContactError = 245;
	static const int invokeClearDisplayMessage = 246;
	static const int invokeCreateWebPlaylistMessage = 247;
	static const int invokeShowWebUrlMessage = 248;
	static const int webKioskUiHelpTitle = 249;
	static const int webKioskUiHelpText = 250;
	static const int webKioskUiHelpAction1Text = 251;
	static const int webKioskUiHelpAction2Text = 252;
	static const int webKioskUiAddUrlTooltip = 253;
	static const int webKioskUiBrowseUrlTooltip = 254;
	static const int webKioskUiShowUrlTooltip = 255;
	static const int webKioskUiAddPlaylistTooltip = 256;
	static const int webKioskUiClearDisplayTooltip = 257;
	static const int webKioskUiWritePlaylistTooltip = 258;
	static const int webKioskUiDeletePlaylistTooltip = 259;
	static const int webKioskNoAgentsSelectedPrompt = 260;
	static const int webKioskNoPlaylistsSelectedPrompt = 261;
	static const int webKioskNoAddressEnteredPrompt = 262;
	static const int websiteAddedMessage = 263;
	static const int webPlaylistCreatedMessage = 264;
	static const int shuffleTooltip = 265;
	static const int clickRenameTooltip = 266;
	static const int hyperlinkTooltip = 267;
	static const int serverAdminUiEmptyTaskListTitle = 268;
	static const int serverAdminUiEmptyTaskListText = 269;
	static const int serverAdminUiHelpTitle = 270;
	static const int serverAdminUiHelpText = 271;
	static const int serverAdminUiHelpAction1Text = 272;
	static const int serverAdminUiLockTooltip = 273;
	static const int serverAdminUiEmptyAdminPasswordText = 274;
	static const int serverAdminUiAdminPasswordDescription = 275;
	static const int serverAdminUiSetPasswordCompleteMessage = 276;
	static const int emptyStreamPlaylistText = 277;
	static const int invokeCreateCacheStreamMessage = 278;
	static const int invokeConfigureMediaStreamMessage = 279;
	static const int streamPlaylistCreatedMessage = 280;
	static const int streamItemAddedMessage = 281;
	static const int invokePlayMediaMessage = 282;
	static const int invokeCreateStreamPlaylistMessage = 283;
	static const int invokeRemoveStreamMessage = 284;
	static const int streamItemUiHelpTitle = 285;
	static const int streamItemUiHelpText = 286;
	static const int streamItemUiHelpAction1Text = 287;
	static const int getAgentConfigurationServerContactError = 288;
	static const int updateAgentConfigurationMessage = 289;
	static const int monitorCacheUiHelpTitle = 290;
	static const int monitorCacheUiHelpText = 291;
	static const int monitorCacheUiHelpAction1Text = 292;
	static const int monitorCacheUiHelpAction2Text = 293;
	static const int monitorCacheUiEmptyStreamStatusTitle = 294;
	static const int monitorCacheUiEmptyStreamStatusText = 295;
	static const int monitorCacheUiNoStreamSelectedPrompt = 296;
	static const int monitorCacheUiPlayTooltip = 297;
	static const int monitorCacheUiWritePlaylistTooltip = 298;
	static const int monitorCacheUiStopTooltip = 299;
	static const int monitorCacheUiDeleteTooltip = 300;
	static const int monitorCacheUiWritePlaylistMessage = 301;
	static const int viewTimelineImagesTooltip = 302;
	static const int monitorActivityIconTooltip = 303;
	static const int storageTooltip = 304;
	static const int cachedStream = 305;
	static const int cachedStreams = 306;
	static const int taskInProgress = 307;
	static const int tasksInProgress = 308;
	static const int removeMedia = 309;
	static const int removeStream = 310;
	static const int deleteStream = 311;
	static const int cacheStream = 312;
	static const int mediaStreaming = 313;
	static const int serversHelpTitle = 314;
	static const int membraneSoftwareOverviewHelpTitle = 315;
	static const int serverPasswords = 316;
	static const int addAdminPasswordTooltip = 317;
	static const int createAdminPassword = 318;
	static const int setAdminPassword = 319;
	static const int adminPasswordNamePrompt = 320;
	static const int adminPasswordPrompt = 321;
	static const int emptyAdminSecretListText = 322;
	static const int normalVideoQualityDescription = 323;
	static const int highVideoQualityDescription = 324;
	static const int lowVideoQualityDescription = 325;
	static const int lowestVideoQualityDescription = 326;
	static const int videoQualityDescription = 327;
	static const int firstLaunchBannerMessage = 328;
	static const int helpKeyBannerMessage = 329;
	static const int freeApplicationBannerMessage = 330;
	static const int membraneSoftwareBannerMessage = 331;
	static const int donateBannerMessage = 332;
	static const int mediaAttributesTooltip = 333;
	static const int toolbarModeButtonTooltip = 334;
	static const int streamIconTooltip = 335;
	static const int createStreamUnavailableTooltip = 336;
	static const int removeMediaActionText = 337;
	static const int viewMonitorCacheTooltip = 338;
	static const int cameras = 339;
	static const int cameraUiDescription = 340;
	static const int cameraUiNoCameraSelectedPrompt = 341;
	static const int cameraUiConfigureTimelapseTooltip = 342;
	static const int cameraUiAutoReloadTooltip = 343;
	static const int cameraUiClearTimelapseTooltip = 344;
	static const int cameraUiHelpTitle = 345;
	static const int cameraUiHelpText = 346;
	static const int cameraUiHelpAction1Text = 347;
	static const int cameraUiEmptyAgentStatusTitle = 348;
	static const int cameraUiEmptyAgentStatusText = 349;
	static const int setTimelapse = 350;
	static const int clearTimelapse = 351;
	static const int timelapseImages = 352;
	static const int cameraConnected = 353;
	static const int camerasConnected = 354;
	static const int enableCapture = 355;
	static const int enableCaptureDescription = 356;
	static const int imageQuality = 357;
	static const int imageQualityDescription = 358;
	static const int normalImageQualityDescription = 359;
	static const int highImageQualityDescription = 360;
	static const int lowImageQualityDescription = 361;
	static const int lowestImageQualityDescription = 362;
	static const int capturePeriod = 363;
	static const int capturePeriodDescription = 364;
	static const int disableCameraTimelapseMessage = 365;
	static const int configureCameraTimelapseMessage = 366;
	static const int clearCameraTimelapseMessage = 367;
	static const int cameraActivityIconTooltip = 368;
	static const int before = 369;
	static const int after = 370;
	static const int captureTimespanTooltip = 371;
	static const int selectedCaptureTimespanTooltip = 372;
	static const int sortCaptureImagesTooltip = 373;
	static const int sortByName = 374;
	static const int sortByNewest = 375;
	static const int continuous = 376;
	static const int cameraTimelineUiHelpTitle = 377;
	static const int cameraTimelineUiHelpText = 378;
	static const int playCaptureTooltip = 379;
};

#endif
