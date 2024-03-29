/*
* Copyright 2018-2022 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#include <stdlib.h>
#include "Config.h"
#include "SystemInterface.h"

const char *SystemInterface::version = "25-stable-71094b9f";
const char *SystemInterface::Command_AddMediaTag = "AddMediaTag";
const char *SystemInterface::Command_AgentConfiguration = "AgentConfiguration";
const char *SystemInterface::Command_AgentContact = "AgentContact";
const char *SystemInterface::Command_AgentStatus = "AgentStatus";
const char *SystemInterface::Command_ApplicationNews = "ApplicationNews";
const char *SystemInterface::Command_AuthorizationRequired = "AuthorizationRequired";
const char *SystemInterface::Command_Authorize = "Authorize";
const char *SystemInterface::Command_AuthorizeResult = "AuthorizeResult";
const char *SystemInterface::Command_CameraImageDisplayIntentState = "CameraImageDisplayIntentState";
const char *SystemInterface::Command_CameraServerStatus = "CameraServerStatus";
const char *SystemInterface::Command_CameraStreamDisplayIntentState = "CameraStreamDisplayIntentState";
const char *SystemInterface::Command_CancelTask = "CancelTask";
const char *SystemInterface::Command_ClearCache = "ClearCache";
const char *SystemInterface::Command_ClearDisplay = "ClearDisplay";
const char *SystemInterface::Command_ClearTimelapse = "ClearTimelapse";
const char *SystemInterface::Command_CommandResult = "CommandResult";
const char *SystemInterface::Command_ConfigureCamera = "ConfigureCamera";
const char *SystemInterface::Command_ConfigureMediaStream = "ConfigureMediaStream";
const char *SystemInterface::Command_CreateCacheStream = "CreateCacheStream";
const char *SystemInterface::Command_CreateCameraImageDisplayIntent = "CreateCameraImageDisplayIntent";
const char *SystemInterface::Command_CreateCameraStreamDisplayIntent = "CreateCameraStreamDisplayIntent";
const char *SystemInterface::Command_CreateMediaDisplayIntent = "CreateMediaDisplayIntent";
const char *SystemInterface::Command_CreateMediaStream = "CreateMediaStream";
const char *SystemInterface::Command_CreateMonitorProgram = "CreateMonitorProgram";
const char *SystemInterface::Command_CreateStreamCacheDisplayIntent = "CreateStreamCacheDisplayIntent";
const char *SystemInterface::Command_CreateTimelapseCaptureIntent = "CreateTimelapseCaptureIntent";
const char *SystemInterface::Command_CreateWebDisplayIntent = "CreateWebDisplayIntent";
const char *SystemInterface::Command_EndSet = "EndSet";
const char *SystemInterface::Command_FindCaptureImages = "FindCaptureImages";
const char *SystemInterface::Command_FindCaptureImagesResult = "FindCaptureImagesResult";
const char *SystemInterface::Command_FindMediaItems = "FindMediaItems";
const char *SystemInterface::Command_FindMediaItemsResult = "FindMediaItemsResult";
const char *SystemInterface::Command_FindMediaStreams = "FindMediaStreams";
const char *SystemInterface::Command_FindMediaStreamsResult = "FindMediaStreamsResult";
const char *SystemInterface::Command_FindStreamItems = "FindStreamItems";
const char *SystemInterface::Command_FindStreamItemsResult = "FindStreamItemsResult";
const char *SystemInterface::Command_GetAgentConfiguration = "GetAgentConfiguration";
const char *SystemInterface::Command_GetCameraStream = "GetCameraStream";
const char *SystemInterface::Command_GetCameraStreamResult = "GetCameraStreamResult";
const char *SystemInterface::Command_GetCaptureImage = "GetCaptureImage";
const char *SystemInterface::Command_GetCaptureVideo = "GetCaptureVideo";
const char *SystemInterface::Command_GetDashMpd = "GetDashMpd";
const char *SystemInterface::Command_GetDashSegment = "GetDashSegment";
const char *SystemInterface::Command_GetHlsManifest = "GetHlsManifest";
const char *SystemInterface::Command_GetHlsSegment = "GetHlsSegment";
const char *SystemInterface::Command_GetMedia = "GetMedia";
const char *SystemInterface::Command_GetStatus = "GetStatus";
const char *SystemInterface::Command_GetStreamItem = "GetStreamItem";
const char *SystemInterface::Command_GetThumbnailImage = "GetThumbnailImage";
const char *SystemInterface::Command_IntentState = "IntentState";
const char *SystemInterface::Command_LinkSuccess = "LinkSuccess";
const char *SystemInterface::Command_MediaDisplayIntentState = "MediaDisplayIntentState";
const char *SystemInterface::Command_MediaItem = "MediaItem";
const char *SystemInterface::Command_MediaServerStatus = "MediaServerStatus";
const char *SystemInterface::Command_MonitorServerStatus = "MonitorServerStatus";
const char *SystemInterface::Command_PauseMedia = "PauseMedia";
const char *SystemInterface::Command_PlayCacheStream = "PlayCacheStream";
const char *SystemInterface::Command_PlayCameraStream = "PlayCameraStream";
const char *SystemInterface::Command_PlayMedia = "PlayMedia";
const char *SystemInterface::Command_ReadTasks = "ReadTasks";
const char *SystemInterface::Command_RemoveIntent = "RemoveIntent";
const char *SystemInterface::Command_RemoveMedia = "RemoveMedia";
const char *SystemInterface::Command_RemoveMediaTag = "RemoveMediaTag";
const char *SystemInterface::Command_RemoveStream = "RemoveStream";
const char *SystemInterface::Command_ReportContact = "ReportContact";
const char *SystemInterface::Command_ReportStatus = "ReportStatus";
const char *SystemInterface::Command_ScanMediaItems = "ScanMediaItems";
const char *SystemInterface::Command_ServerError = "ServerError";
const char *SystemInterface::Command_SetAdminSecret = "SetAdminSecret";
const char *SystemInterface::Command_SetIntentActive = "SetIntentActive";
const char *SystemInterface::Command_ShowAgentStatus = "ShowAgentStatus";
const char *SystemInterface::Command_ShowCameraImage = "ShowCameraImage";
const char *SystemInterface::Command_ShowDesktopCountdown = "ShowDesktopCountdown";
const char *SystemInterface::Command_ShowWebUrl = "ShowWebUrl";
const char *SystemInterface::Command_ShutdownAgent = "ShutdownAgent";
const char *SystemInterface::Command_StartServers = "StartServers";
const char *SystemInterface::Command_StopCapture = "StopCapture";
const char *SystemInterface::Command_StopServers = "StopServers";
const char *SystemInterface::Command_StreamCacheDisplayIntentState = "StreamCacheDisplayIntentState";
const char *SystemInterface::Command_StreamItem = "StreamItem";
const char *SystemInterface::Command_StreamServerStatus = "StreamServerStatus";
const char *SystemInterface::Command_TaskItem = "TaskItem";
const char *SystemInterface::Command_TimelapseCaptureIntentState = "TimelapseCaptureIntentState";
const char *SystemInterface::Command_UpdateAgentConfiguration = "UpdateAgentConfiguration";
const char *SystemInterface::Command_UpdateIntentState = "UpdateIntentState";
const char *SystemInterface::Command_WatchStatus = "WatchStatus";
const char *SystemInterface::Command_WatchTasks = "WatchTasks";
const char *SystemInterface::Command_WebDisplayIntentState = "WebDisplayIntentState";
const char *SystemInterface::Constant_AgentIdPrefixField = "b";
const char *SystemInterface::Constant_AuthorizationHashAlgorithm = "sha256";
const char *SystemInterface::Constant_AuthorizationHashPrefixField = "g";
const char *SystemInterface::Constant_AuthorizationTokenPrefixField = "h";
const char *SystemInterface::Constant_CreateTimePrefixField = "a";
const char *SystemInterface::Constant_DefaultAuthorizePath = "C18HZb3wsXQoMQN6Laz8S5Lq";
const char *SystemInterface::Constant_DefaultInvokePath = "/";
const char *SystemInterface::Constant_DefaultLinkPath = "mNODP0RPYCLhTiPGiCifPJA9";
const char *SystemInterface::Constant_DisplayCondition = "a";
const char *SystemInterface::Constant_DurationPrefixField = "f";
const char *SystemInterface::Constant_PriorityPrefixField = "d";
const char *SystemInterface::Constant_StartTimePrefixField = "e";
const char *SystemInterface::Constant_UrlQueryParameter = "c";
const char *SystemInterface::Constant_UserIdPrefixField = "c";
const char *SystemInterface::Constant_WebSocketEvent = "SystemInterface";
void SystemInterface::populate () {
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("AddMediaTag"), SystemInterface::Command (233, StdString ("AddMediaTag"), StdString ("AddMediaTag"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("AgentConfiguration"), SystemInterface::Command (45, StdString ("AgentConfiguration"), StdString ("AgentConfiguration"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("AgentContact"), SystemInterface::Command (33, StdString ("AgentContact"), StdString ("AgentContact"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("AgentStatus"), SystemInterface::Command (1, StdString ("AgentStatus"), StdString ("AgentStatus"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ApplicationNews"), SystemInterface::Command (64, StdString ("ApplicationNews"), StdString ("ApplicationNews"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("AuthorizationRequired"), SystemInterface::Command (62, StdString ("AuthorizationRequired"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("Authorize"), SystemInterface::Command (19, StdString ("Authorize"), StdString ("Authorize"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("AuthorizeResult"), SystemInterface::Command (13, StdString ("AuthorizeResult"), StdString ("AuthorizeResult"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CameraImageDisplayIntentState"), SystemInterface::Command (108, StdString ("CameraImageDisplayIntentState"), StdString ("CameraImageDisplayIntentState"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CameraServerStatus"), SystemInterface::Command (69, StdString ("CameraServerStatus"), StdString ("CameraServerStatus"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CameraStreamDisplayIntentState"), SystemInterface::Command (239, StdString ("CameraStreamDisplayIntentState"), StdString ("CameraStreamDisplayIntentState"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CancelTask"), SystemInterface::Command (28, StdString ("CancelTask"), StdString ("CancelTask"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ClearCache"), SystemInterface::Command (59, StdString ("ClearCache"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ClearDisplay"), SystemInterface::Command (31, StdString ("ClearDisplay"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ClearTimelapse"), SystemInterface::Command (76, StdString ("ClearTimelapse"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CommandResult"), SystemInterface::Command (0, StdString ("CommandResult"), StdString ("CommandResult"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ConfigureCamera"), SystemInterface::Command (109, StdString ("ConfigureCamera"), StdString ("ConfigureCamera"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ConfigureMediaStream"), SystemInterface::Command (65, StdString ("ConfigureMediaStream"), StdString ("ConfigureMediaStream"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CreateCacheStream"), SystemInterface::Command (60, StdString ("CreateCacheStream"), StdString ("CreateCacheStream"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CreateCameraImageDisplayIntent"), SystemInterface::Command (107, StdString ("CreateCameraImageDisplayIntent"), StdString ("CreateCameraImageDisplayIntent"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CreateCameraStreamDisplayIntent"), SystemInterface::Command (238, StdString ("CreateCameraStreamDisplayIntent"), StdString ("CreateCameraStreamDisplayIntent"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CreateMediaDisplayIntent"), SystemInterface::Command (50, StdString ("CreateMediaDisplayIntent"), StdString ("CreateMediaDisplayIntent"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CreateMediaStream"), SystemInterface::Command (14, StdString ("CreateMediaStream"), StdString ("CreateMediaStream"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CreateMonitorProgram"), SystemInterface::Command (102, StdString ("CreateMonitorProgram"), StdString ("CreateMonitorProgram"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CreateStreamCacheDisplayIntent"), SystemInterface::Command (103, StdString ("CreateStreamCacheDisplayIntent"), StdString ("CreateStreamCacheDisplayIntent"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CreateTimelapseCaptureIntent"), SystemInterface::Command (70, StdString ("CreateTimelapseCaptureIntent"), StdString ("CreateTimelapseCaptureIntent"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CreateWebDisplayIntent"), SystemInterface::Command (35, StdString ("CreateWebDisplayIntent"), StdString ("CreateWebDisplayIntent"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("EndSet"), SystemInterface::Command (21, StdString ("EndSet"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("FindCaptureImages"), SystemInterface::Command (74, StdString ("FindCaptureImages"), StdString ("FindCaptureImages"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("FindCaptureImagesResult"), SystemInterface::Command (75, StdString ("FindCaptureImagesResult"), StdString ("FindCaptureImagesResult"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("FindMediaItems"), SystemInterface::Command (3, StdString ("FindMediaItems"), StdString ("FindMediaItems"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("FindMediaItemsResult"), SystemInterface::Command (48, StdString ("FindMediaItemsResult"), StdString ("FindMediaItemsResult"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("FindMediaStreams"), SystemInterface::Command (78, StdString ("FindMediaStreams"), StdString ("FindMediaStreams"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("FindMediaStreamsResult"), SystemInterface::Command (79, StdString ("FindMediaStreamsResult"), StdString ("FindMediaStreamsResult"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("FindStreamItems"), SystemInterface::Command (211, StdString ("FindStreamItems"), StdString ("FindStreamItems"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("FindStreamItemsResult"), SystemInterface::Command (4, StdString ("FindStreamItemsResult"), StdString ("FindStreamItemsResult"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetAgentConfiguration"), SystemInterface::Command (44, StdString ("GetAgentConfiguration"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetCameraStream"), SystemInterface::Command (100, StdString ("GetCameraStream"), StdString ("GetCameraStream"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetCameraStreamResult"), SystemInterface::Command (101, StdString ("GetCameraStreamResult"), StdString ("GetCameraStreamResult"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetCaptureImage"), SystemInterface::Command (73, StdString ("GetCaptureImage"), StdString ("GetCaptureImage"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetCaptureVideo"), SystemInterface::Command (235, StdString ("GetCaptureVideo"), StdString ("GetCaptureVideo"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetDashMpd"), SystemInterface::Command (67, StdString ("GetDashMpd"), StdString ("GetDashMpd"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetDashSegment"), SystemInterface::Command (68, StdString ("GetDashSegment"), StdString ("GetDashSegment"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetHlsManifest"), SystemInterface::Command (23, StdString ("GetHlsManifest"), StdString ("GetHlsManifest"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetHlsSegment"), SystemInterface::Command (24, StdString ("GetHlsSegment"), StdString ("GetHlsSegment"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetMedia"), SystemInterface::Command (15, StdString ("GetMedia"), StdString ("GetMedia"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetStatus"), SystemInterface::Command (8, StdString ("GetStatus"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetStreamItem"), SystemInterface::Command (25, StdString ("GetStreamItem"), StdString ("GetStreamItem"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetThumbnailImage"), SystemInterface::Command (5, StdString ("GetThumbnailImage"), StdString ("GetThumbnailImage"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("IntentState"), SystemInterface::Command (36, StdString ("IntentState"), StdString ("IntentState"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("LinkSuccess"), SystemInterface::Command (63, StdString ("LinkSuccess"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("MediaDisplayIntentState"), SystemInterface::Command (51, StdString ("MediaDisplayIntentState"), StdString ("MediaDisplayIntentState"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("MediaItem"), SystemInterface::Command (16, StdString ("MediaItem"), StdString ("MediaItem"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("MediaServerStatus"), SystemInterface::Command (9, StdString ("MediaServerStatus"), StdString ("MediaServerStatus"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("MonitorServerStatus"), SystemInterface::Command (12, StdString ("MonitorServerStatus"), StdString ("MonitorServerStatus"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("PauseMedia"), SystemInterface::Command (98, StdString ("PauseMedia"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("PlayCacheStream"), SystemInterface::Command (57, StdString ("PlayCacheStream"), StdString ("PlayCacheStream"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("PlayCameraStream"), SystemInterface::Command (99, StdString ("PlayCameraStream"), StdString ("PlayCameraStream"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("PlayMedia"), SystemInterface::Command (30, StdString ("PlayMedia"), StdString ("PlayMedia"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ReadTasks"), SystemInterface::Command (6, StdString ("ReadTasks"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("RemoveIntent"), SystemInterface::Command (37, StdString ("RemoveIntent"), StdString ("RemoveIntent"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("RemoveMedia"), SystemInterface::Command (77, StdString ("RemoveMedia"), StdString ("RemoveMedia"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("RemoveMediaTag"), SystemInterface::Command (234, StdString ("RemoveMediaTag"), StdString ("RemoveMediaTag"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("RemoveStream"), SystemInterface::Command (29, StdString ("RemoveStream"), StdString ("RemoveStream"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ReportContact"), SystemInterface::Command (32, StdString ("ReportContact"), StdString ("ReportContact"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ReportStatus"), SystemInterface::Command (2, StdString ("ReportStatus"), StdString ("ReportStatus"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ScanMediaItems"), SystemInterface::Command (58, StdString ("ScanMediaItems"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ServerError"), SystemInterface::Command (20, StdString ("ServerError"), StdString ("ServerError"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("SetAdminSecret"), SystemInterface::Command (61, StdString ("SetAdminSecret"), StdString ("SetAdminSecret"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("SetIntentActive"), SystemInterface::Command (38, StdString ("SetIntentActive"), StdString ("SetIntentActive"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ShowAgentStatus"), SystemInterface::Command (41, StdString ("ShowAgentStatus"), StdString ("ShowAgentStatus"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ShowCameraImage"), SystemInterface::Command (105, StdString ("ShowCameraImage"), StdString ("ShowCameraImage"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ShowDesktopCountdown"), SystemInterface::Command (218, StdString ("ShowDesktopCountdown"), StdString ("ShowDesktopCountdown"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ShowWebUrl"), SystemInterface::Command (34, StdString ("ShowWebUrl"), StdString ("ShowWebUrl"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ShutdownAgent"), SystemInterface::Command (43, StdString ("ShutdownAgent"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("StartServers"), SystemInterface::Command (47, StdString ("StartServers"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("StopCapture"), SystemInterface::Command (72, StdString ("StopCapture"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("StopServers"), SystemInterface::Command (46, StdString ("StopServers"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("StreamCacheDisplayIntentState"), SystemInterface::Command (104, StdString ("StreamCacheDisplayIntentState"), StdString ("StreamCacheDisplayIntentState"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("StreamItem"), SystemInterface::Command (22, StdString ("StreamItem"), StdString ("StreamItem"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("StreamServerStatus"), SystemInterface::Command (10, StdString ("StreamServerStatus"), StdString ("StreamServerStatus"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("TaskItem"), SystemInterface::Command (26, StdString ("TaskItem"), StdString ("TaskItem"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("TimelapseCaptureIntentState"), SystemInterface::Command (71, StdString ("TimelapseCaptureIntentState"), StdString ("TimelapseCaptureIntentState"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("UpdateAgentConfiguration"), SystemInterface::Command (42, StdString ("UpdateAgentConfiguration"), StdString ("UpdateAgentConfiguration"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("UpdateIntentState"), SystemInterface::Command (39, StdString ("UpdateIntentState"), StdString ("UpdateIntentState"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("WatchStatus"), SystemInterface::Command (82, StdString ("WatchStatus"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("WatchTasks"), SystemInterface::Command (7, StdString ("WatchTasks"), StdString ("WatchTasks"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("WebDisplayIntentState"), SystemInterface::Command (49, StdString ("WebDisplayIntentState"), StdString ("WebDisplayIntentState"))));
  commandIdMap.insert (std::pair<int, StdString> (233, StdString ("AddMediaTag")));
  commandIdMap.insert (std::pair<int, StdString> (45, StdString ("AgentConfiguration")));
  commandIdMap.insert (std::pair<int, StdString> (33, StdString ("AgentContact")));
  commandIdMap.insert (std::pair<int, StdString> (1, StdString ("AgentStatus")));
  commandIdMap.insert (std::pair<int, StdString> (64, StdString ("ApplicationNews")));
  commandIdMap.insert (std::pair<int, StdString> (62, StdString ("AuthorizationRequired")));
  commandIdMap.insert (std::pair<int, StdString> (19, StdString ("Authorize")));
  commandIdMap.insert (std::pair<int, StdString> (13, StdString ("AuthorizeResult")));
  commandIdMap.insert (std::pair<int, StdString> (108, StdString ("CameraImageDisplayIntentState")));
  commandIdMap.insert (std::pair<int, StdString> (69, StdString ("CameraServerStatus")));
  commandIdMap.insert (std::pair<int, StdString> (239, StdString ("CameraStreamDisplayIntentState")));
  commandIdMap.insert (std::pair<int, StdString> (28, StdString ("CancelTask")));
  commandIdMap.insert (std::pair<int, StdString> (59, StdString ("ClearCache")));
  commandIdMap.insert (std::pair<int, StdString> (31, StdString ("ClearDisplay")));
  commandIdMap.insert (std::pair<int, StdString> (76, StdString ("ClearTimelapse")));
  commandIdMap.insert (std::pair<int, StdString> (0, StdString ("CommandResult")));
  commandIdMap.insert (std::pair<int, StdString> (109, StdString ("ConfigureCamera")));
  commandIdMap.insert (std::pair<int, StdString> (65, StdString ("ConfigureMediaStream")));
  commandIdMap.insert (std::pair<int, StdString> (60, StdString ("CreateCacheStream")));
  commandIdMap.insert (std::pair<int, StdString> (107, StdString ("CreateCameraImageDisplayIntent")));
  commandIdMap.insert (std::pair<int, StdString> (238, StdString ("CreateCameraStreamDisplayIntent")));
  commandIdMap.insert (std::pair<int, StdString> (50, StdString ("CreateMediaDisplayIntent")));
  commandIdMap.insert (std::pair<int, StdString> (14, StdString ("CreateMediaStream")));
  commandIdMap.insert (std::pair<int, StdString> (102, StdString ("CreateMonitorProgram")));
  commandIdMap.insert (std::pair<int, StdString> (103, StdString ("CreateStreamCacheDisplayIntent")));
  commandIdMap.insert (std::pair<int, StdString> (70, StdString ("CreateTimelapseCaptureIntent")));
  commandIdMap.insert (std::pair<int, StdString> (35, StdString ("CreateWebDisplayIntent")));
  commandIdMap.insert (std::pair<int, StdString> (21, StdString ("EndSet")));
  commandIdMap.insert (std::pair<int, StdString> (74, StdString ("FindCaptureImages")));
  commandIdMap.insert (std::pair<int, StdString> (75, StdString ("FindCaptureImagesResult")));
  commandIdMap.insert (std::pair<int, StdString> (3, StdString ("FindMediaItems")));
  commandIdMap.insert (std::pair<int, StdString> (48, StdString ("FindMediaItemsResult")));
  commandIdMap.insert (std::pair<int, StdString> (78, StdString ("FindMediaStreams")));
  commandIdMap.insert (std::pair<int, StdString> (79, StdString ("FindMediaStreamsResult")));
  commandIdMap.insert (std::pair<int, StdString> (211, StdString ("FindStreamItems")));
  commandIdMap.insert (std::pair<int, StdString> (4, StdString ("FindStreamItemsResult")));
  commandIdMap.insert (std::pair<int, StdString> (44, StdString ("GetAgentConfiguration")));
  commandIdMap.insert (std::pair<int, StdString> (100, StdString ("GetCameraStream")));
  commandIdMap.insert (std::pair<int, StdString> (101, StdString ("GetCameraStreamResult")));
  commandIdMap.insert (std::pair<int, StdString> (73, StdString ("GetCaptureImage")));
  commandIdMap.insert (std::pair<int, StdString> (235, StdString ("GetCaptureVideo")));
  commandIdMap.insert (std::pair<int, StdString> (67, StdString ("GetDashMpd")));
  commandIdMap.insert (std::pair<int, StdString> (68, StdString ("GetDashSegment")));
  commandIdMap.insert (std::pair<int, StdString> (23, StdString ("GetHlsManifest")));
  commandIdMap.insert (std::pair<int, StdString> (24, StdString ("GetHlsSegment")));
  commandIdMap.insert (std::pair<int, StdString> (15, StdString ("GetMedia")));
  commandIdMap.insert (std::pair<int, StdString> (8, StdString ("GetStatus")));
  commandIdMap.insert (std::pair<int, StdString> (25, StdString ("GetStreamItem")));
  commandIdMap.insert (std::pair<int, StdString> (5, StdString ("GetThumbnailImage")));
  commandIdMap.insert (std::pair<int, StdString> (36, StdString ("IntentState")));
  commandIdMap.insert (std::pair<int, StdString> (63, StdString ("LinkSuccess")));
  commandIdMap.insert (std::pair<int, StdString> (51, StdString ("MediaDisplayIntentState")));
  commandIdMap.insert (std::pair<int, StdString> (16, StdString ("MediaItem")));
  commandIdMap.insert (std::pair<int, StdString> (9, StdString ("MediaServerStatus")));
  commandIdMap.insert (std::pair<int, StdString> (12, StdString ("MonitorServerStatus")));
  commandIdMap.insert (std::pair<int, StdString> (98, StdString ("PauseMedia")));
  commandIdMap.insert (std::pair<int, StdString> (57, StdString ("PlayCacheStream")));
  commandIdMap.insert (std::pair<int, StdString> (99, StdString ("PlayCameraStream")));
  commandIdMap.insert (std::pair<int, StdString> (30, StdString ("PlayMedia")));
  commandIdMap.insert (std::pair<int, StdString> (6, StdString ("ReadTasks")));
  commandIdMap.insert (std::pair<int, StdString> (37, StdString ("RemoveIntent")));
  commandIdMap.insert (std::pair<int, StdString> (77, StdString ("RemoveMedia")));
  commandIdMap.insert (std::pair<int, StdString> (234, StdString ("RemoveMediaTag")));
  commandIdMap.insert (std::pair<int, StdString> (29, StdString ("RemoveStream")));
  commandIdMap.insert (std::pair<int, StdString> (32, StdString ("ReportContact")));
  commandIdMap.insert (std::pair<int, StdString> (2, StdString ("ReportStatus")));
  commandIdMap.insert (std::pair<int, StdString> (58, StdString ("ScanMediaItems")));
  commandIdMap.insert (std::pair<int, StdString> (20, StdString ("ServerError")));
  commandIdMap.insert (std::pair<int, StdString> (61, StdString ("SetAdminSecret")));
  commandIdMap.insert (std::pair<int, StdString> (38, StdString ("SetIntentActive")));
  commandIdMap.insert (std::pair<int, StdString> (41, StdString ("ShowAgentStatus")));
  commandIdMap.insert (std::pair<int, StdString> (105, StdString ("ShowCameraImage")));
  commandIdMap.insert (std::pair<int, StdString> (218, StdString ("ShowDesktopCountdown")));
  commandIdMap.insert (std::pair<int, StdString> (34, StdString ("ShowWebUrl")));
  commandIdMap.insert (std::pair<int, StdString> (43, StdString ("ShutdownAgent")));
  commandIdMap.insert (std::pair<int, StdString> (47, StdString ("StartServers")));
  commandIdMap.insert (std::pair<int, StdString> (72, StdString ("StopCapture")));
  commandIdMap.insert (std::pair<int, StdString> (46, StdString ("StopServers")));
  commandIdMap.insert (std::pair<int, StdString> (104, StdString ("StreamCacheDisplayIntentState")));
  commandIdMap.insert (std::pair<int, StdString> (22, StdString ("StreamItem")));
  commandIdMap.insert (std::pair<int, StdString> (10, StdString ("StreamServerStatus")));
  commandIdMap.insert (std::pair<int, StdString> (26, StdString ("TaskItem")));
  commandIdMap.insert (std::pair<int, StdString> (71, StdString ("TimelapseCaptureIntentState")));
  commandIdMap.insert (std::pair<int, StdString> (42, StdString ("UpdateAgentConfiguration")));
  commandIdMap.insert (std::pair<int, StdString> (39, StdString ("UpdateIntentState")));
  commandIdMap.insert (std::pair<int, StdString> (82, StdString ("WatchStatus")));
  commandIdMap.insert (std::pair<int, StdString> (7, StdString ("WatchTasks")));
  commandIdMap.insert (std::pair<int, StdString> (49, StdString ("WebDisplayIntentState")));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("AddMediaTag"), SystemInterface::getParams_AddMediaTag));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("AgentConfiguration"), SystemInterface::getParams_AgentConfiguration));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("AgentContact"), SystemInterface::getParams_AgentContact));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("AgentHost"), SystemInterface::getParams_AgentHost));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("AgentStatus"), SystemInterface::getParams_AgentStatus));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("ApplicationNews"), SystemInterface::getParams_ApplicationNews));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("ApplicationNewsItem"), SystemInterface::getParams_ApplicationNewsItem));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("Authorize"), SystemInterface::getParams_Authorize));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("AuthorizeResult"), SystemInterface::getParams_AuthorizeResult));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CameraImageDisplayIntentState"), SystemInterface::getParams_CameraImageDisplayIntentState));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CameraSensor"), SystemInterface::getParams_CameraSensor));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CameraServerConfiguration"), SystemInterface::getParams_CameraServerConfiguration));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CameraServerStatus"), SystemInterface::getParams_CameraServerStatus));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CameraStreamDisplayIntentState"), SystemInterface::getParams_CameraStreamDisplayIntentState));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CancelTask"), SystemInterface::getParams_CancelTask));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CommandResult"), SystemInterface::getParams_CommandResult));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("ConfigureCamera"), SystemInterface::getParams_ConfigureCamera));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("ConfigureMediaStream"), SystemInterface::getParams_ConfigureMediaStream));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CreateCacheStream"), SystemInterface::getParams_CreateCacheStream));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CreateCameraImageDisplayIntent"), SystemInterface::getParams_CreateCameraImageDisplayIntent));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CreateCameraStreamDisplayIntent"), SystemInterface::getParams_CreateCameraStreamDisplayIntent));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CreateMediaDisplayIntent"), SystemInterface::getParams_CreateMediaDisplayIntent));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CreateMediaStream"), SystemInterface::getParams_CreateMediaStream));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CreateMonitorProgram"), SystemInterface::getParams_CreateMonitorProgram));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CreateStreamCacheDisplayIntent"), SystemInterface::getParams_CreateStreamCacheDisplayIntent));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CreateTimelapseCaptureIntent"), SystemInterface::getParams_CreateTimelapseCaptureIntent));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CreateWebDisplayIntent"), SystemInterface::getParams_CreateWebDisplayIntent));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("EmptyObject"), SystemInterface::getParams_EmptyObject));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("FindCaptureImages"), SystemInterface::getParams_FindCaptureImages));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("FindCaptureImagesResult"), SystemInterface::getParams_FindCaptureImagesResult));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("FindMediaItems"), SystemInterface::getParams_FindMediaItems));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("FindMediaItemsResult"), SystemInterface::getParams_FindMediaItemsResult));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("FindMediaStreams"), SystemInterface::getParams_FindMediaStreams));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("FindMediaStreamsResult"), SystemInterface::getParams_FindMediaStreamsResult));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("FindStreamItems"), SystemInterface::getParams_FindStreamItems));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("FindStreamItemsResult"), SystemInterface::getParams_FindStreamItemsResult));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("GetCameraStream"), SystemInterface::getParams_GetCameraStream));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("GetCameraStreamResult"), SystemInterface::getParams_GetCameraStreamResult));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("GetCaptureImage"), SystemInterface::getParams_GetCaptureImage));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("GetCaptureVideo"), SystemInterface::getParams_GetCaptureVideo));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("GetDashMpd"), SystemInterface::getParams_GetDashMpd));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("GetDashSegment"), SystemInterface::getParams_GetDashSegment));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("GetHlsManifest"), SystemInterface::getParams_GetHlsManifest));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("GetHlsSegment"), SystemInterface::getParams_GetHlsSegment));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("GetMedia"), SystemInterface::getParams_GetMedia));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("GetStreamItem"), SystemInterface::getParams_GetStreamItem));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("GetThumbnailImage"), SystemInterface::getParams_GetThumbnailImage));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("IntentState"), SystemInterface::getParams_IntentState));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("MediaDisplayIntentState"), SystemInterface::getParams_MediaDisplayIntentState));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("MediaDisplayItem"), SystemInterface::getParams_MediaDisplayItem));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("MediaItem"), SystemInterface::getParams_MediaItem));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("MediaServerConfiguration"), SystemInterface::getParams_MediaServerConfiguration));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("MediaServerStatus"), SystemInterface::getParams_MediaServerStatus));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("MonitorProgramCondition"), SystemInterface::getParams_MonitorProgramCondition));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("MonitorProgramDirective"), SystemInterface::getParams_MonitorProgramDirective));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("MonitorProgramTimeCondition"), SystemInterface::getParams_MonitorProgramTimeCondition));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("MonitorServerConfiguration"), SystemInterface::getParams_MonitorServerConfiguration));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("MonitorServerStatus"), SystemInterface::getParams_MonitorServerStatus));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("PlayCacheStream"), SystemInterface::getParams_PlayCacheStream));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("PlayCameraStream"), SystemInterface::getParams_PlayCameraStream));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("PlayMedia"), SystemInterface::getParams_PlayMedia));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("RemoveIntent"), SystemInterface::getParams_RemoveIntent));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("RemoveMedia"), SystemInterface::getParams_RemoveMedia));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("RemoveMediaTag"), SystemInterface::getParams_RemoveMediaTag));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("RemoveStream"), SystemInterface::getParams_RemoveStream));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("ReportContact"), SystemInterface::getParams_ReportContact));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("ReportStatus"), SystemInterface::getParams_ReportStatus));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("ServerError"), SystemInterface::getParams_ServerError));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("SetAdminSecret"), SystemInterface::getParams_SetAdminSecret));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("SetIntentActive"), SystemInterface::getParams_SetIntentActive));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("ShowAgentStatus"), SystemInterface::getParams_ShowAgentStatus));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("ShowCameraImage"), SystemInterface::getParams_ShowCameraImage));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("ShowDesktopCountdown"), SystemInterface::getParams_ShowDesktopCountdown));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("ShowWebUrl"), SystemInterface::getParams_ShowWebUrl));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("StreamCacheDisplayIntentState"), SystemInterface::getParams_StreamCacheDisplayIntentState));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("StreamItem"), SystemInterface::getParams_StreamItem));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("StreamServerConfiguration"), SystemInterface::getParams_StreamServerConfiguration));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("StreamServerStatus"), SystemInterface::getParams_StreamServerStatus));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("StreamSummary"), SystemInterface::getParams_StreamSummary));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("TaskItem"), SystemInterface::getParams_TaskItem));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("TimelapseCaptureIntentState"), SystemInterface::getParams_TimelapseCaptureIntentState));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("UpdateAgentConfiguration"), SystemInterface::getParams_UpdateAgentConfiguration));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("UpdateIntentState"), SystemInterface::getParams_UpdateIntentState));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("WatchTasks"), SystemInterface::getParams_WatchTasks));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("WebDisplayIntentState"), SystemInterface::getParams_WebDisplayIntentState));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("AddMediaTag"), SystemInterface::populateDefaultFields_AddMediaTag));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("AgentConfiguration"), SystemInterface::populateDefaultFields_AgentConfiguration));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("AgentContact"), SystemInterface::populateDefaultFields_AgentContact));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("AgentHost"), SystemInterface::populateDefaultFields_AgentHost));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("AgentStatus"), SystemInterface::populateDefaultFields_AgentStatus));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("ApplicationNews"), SystemInterface::populateDefaultFields_ApplicationNews));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("ApplicationNewsItem"), SystemInterface::populateDefaultFields_ApplicationNewsItem));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("Authorize"), SystemInterface::populateDefaultFields_Authorize));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("AuthorizeResult"), SystemInterface::populateDefaultFields_AuthorizeResult));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CameraImageDisplayIntentState"), SystemInterface::populateDefaultFields_CameraImageDisplayIntentState));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CameraSensor"), SystemInterface::populateDefaultFields_CameraSensor));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CameraServerConfiguration"), SystemInterface::populateDefaultFields_CameraServerConfiguration));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CameraServerStatus"), SystemInterface::populateDefaultFields_CameraServerStatus));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CameraStreamDisplayIntentState"), SystemInterface::populateDefaultFields_CameraStreamDisplayIntentState));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CancelTask"), SystemInterface::populateDefaultFields_CancelTask));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CommandResult"), SystemInterface::populateDefaultFields_CommandResult));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("ConfigureCamera"), SystemInterface::populateDefaultFields_ConfigureCamera));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("ConfigureMediaStream"), SystemInterface::populateDefaultFields_ConfigureMediaStream));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CreateCacheStream"), SystemInterface::populateDefaultFields_CreateCacheStream));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CreateCameraImageDisplayIntent"), SystemInterface::populateDefaultFields_CreateCameraImageDisplayIntent));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CreateCameraStreamDisplayIntent"), SystemInterface::populateDefaultFields_CreateCameraStreamDisplayIntent));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CreateMediaDisplayIntent"), SystemInterface::populateDefaultFields_CreateMediaDisplayIntent));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CreateMediaStream"), SystemInterface::populateDefaultFields_CreateMediaStream));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CreateMonitorProgram"), SystemInterface::populateDefaultFields_CreateMonitorProgram));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CreateStreamCacheDisplayIntent"), SystemInterface::populateDefaultFields_CreateStreamCacheDisplayIntent));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CreateTimelapseCaptureIntent"), SystemInterface::populateDefaultFields_CreateTimelapseCaptureIntent));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CreateWebDisplayIntent"), SystemInterface::populateDefaultFields_CreateWebDisplayIntent));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("EmptyObject"), SystemInterface::populateDefaultFields_EmptyObject));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("FindCaptureImages"), SystemInterface::populateDefaultFields_FindCaptureImages));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("FindCaptureImagesResult"), SystemInterface::populateDefaultFields_FindCaptureImagesResult));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("FindMediaItems"), SystemInterface::populateDefaultFields_FindMediaItems));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("FindMediaItemsResult"), SystemInterface::populateDefaultFields_FindMediaItemsResult));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("FindMediaStreams"), SystemInterface::populateDefaultFields_FindMediaStreams));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("FindMediaStreamsResult"), SystemInterface::populateDefaultFields_FindMediaStreamsResult));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("FindStreamItems"), SystemInterface::populateDefaultFields_FindStreamItems));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("FindStreamItemsResult"), SystemInterface::populateDefaultFields_FindStreamItemsResult));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetCameraStream"), SystemInterface::populateDefaultFields_GetCameraStream));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetCameraStreamResult"), SystemInterface::populateDefaultFields_GetCameraStreamResult));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetCaptureImage"), SystemInterface::populateDefaultFields_GetCaptureImage));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetCaptureVideo"), SystemInterface::populateDefaultFields_GetCaptureVideo));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetDashMpd"), SystemInterface::populateDefaultFields_GetDashMpd));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetDashSegment"), SystemInterface::populateDefaultFields_GetDashSegment));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetHlsManifest"), SystemInterface::populateDefaultFields_GetHlsManifest));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetHlsSegment"), SystemInterface::populateDefaultFields_GetHlsSegment));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetMedia"), SystemInterface::populateDefaultFields_GetMedia));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetStreamItem"), SystemInterface::populateDefaultFields_GetStreamItem));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetThumbnailImage"), SystemInterface::populateDefaultFields_GetThumbnailImage));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("IntentState"), SystemInterface::populateDefaultFields_IntentState));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MediaDisplayIntentState"), SystemInterface::populateDefaultFields_MediaDisplayIntentState));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MediaDisplayItem"), SystemInterface::populateDefaultFields_MediaDisplayItem));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MediaItem"), SystemInterface::populateDefaultFields_MediaItem));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MediaServerConfiguration"), SystemInterface::populateDefaultFields_MediaServerConfiguration));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MediaServerStatus"), SystemInterface::populateDefaultFields_MediaServerStatus));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MonitorProgramCondition"), SystemInterface::populateDefaultFields_MonitorProgramCondition));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MonitorProgramDirective"), SystemInterface::populateDefaultFields_MonitorProgramDirective));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MonitorProgramTimeCondition"), SystemInterface::populateDefaultFields_MonitorProgramTimeCondition));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MonitorServerConfiguration"), SystemInterface::populateDefaultFields_MonitorServerConfiguration));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MonitorServerStatus"), SystemInterface::populateDefaultFields_MonitorServerStatus));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("PlayCacheStream"), SystemInterface::populateDefaultFields_PlayCacheStream));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("PlayCameraStream"), SystemInterface::populateDefaultFields_PlayCameraStream));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("PlayMedia"), SystemInterface::populateDefaultFields_PlayMedia));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("RemoveIntent"), SystemInterface::populateDefaultFields_RemoveIntent));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("RemoveMedia"), SystemInterface::populateDefaultFields_RemoveMedia));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("RemoveMediaTag"), SystemInterface::populateDefaultFields_RemoveMediaTag));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("RemoveStream"), SystemInterface::populateDefaultFields_RemoveStream));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("ReportContact"), SystemInterface::populateDefaultFields_ReportContact));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("ReportStatus"), SystemInterface::populateDefaultFields_ReportStatus));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("ServerError"), SystemInterface::populateDefaultFields_ServerError));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("SetAdminSecret"), SystemInterface::populateDefaultFields_SetAdminSecret));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("SetIntentActive"), SystemInterface::populateDefaultFields_SetIntentActive));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("ShowAgentStatus"), SystemInterface::populateDefaultFields_ShowAgentStatus));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("ShowCameraImage"), SystemInterface::populateDefaultFields_ShowCameraImage));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("ShowDesktopCountdown"), SystemInterface::populateDefaultFields_ShowDesktopCountdown));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("ShowWebUrl"), SystemInterface::populateDefaultFields_ShowWebUrl));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("StreamCacheDisplayIntentState"), SystemInterface::populateDefaultFields_StreamCacheDisplayIntentState));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("StreamItem"), SystemInterface::populateDefaultFields_StreamItem));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("StreamServerConfiguration"), SystemInterface::populateDefaultFields_StreamServerConfiguration));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("StreamServerStatus"), SystemInterface::populateDefaultFields_StreamServerStatus));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("StreamSummary"), SystemInterface::populateDefaultFields_StreamSummary));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("TaskItem"), SystemInterface::populateDefaultFields_TaskItem));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("TimelapseCaptureIntentState"), SystemInterface::populateDefaultFields_TimelapseCaptureIntentState));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("UpdateAgentConfiguration"), SystemInterface::populateDefaultFields_UpdateAgentConfiguration));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("UpdateIntentState"), SystemInterface::populateDefaultFields_UpdateIntentState));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("WatchTasks"), SystemInterface::populateDefaultFields_WatchTasks));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("WebDisplayIntentState"), SystemInterface::populateDefaultFields_WebDisplayIntentState));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("AddMediaTag"), SystemInterface::hashFields_AddMediaTag));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("AgentConfiguration"), SystemInterface::hashFields_AgentConfiguration));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("AgentContact"), SystemInterface::hashFields_AgentContact));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("AgentHost"), SystemInterface::hashFields_AgentHost));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("AgentStatus"), SystemInterface::hashFields_AgentStatus));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("ApplicationNews"), SystemInterface::hashFields_ApplicationNews));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("ApplicationNewsItem"), SystemInterface::hashFields_ApplicationNewsItem));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("Authorize"), SystemInterface::hashFields_Authorize));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("AuthorizeResult"), SystemInterface::hashFields_AuthorizeResult));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("CameraImageDisplayIntentState"), SystemInterface::hashFields_CameraImageDisplayIntentState));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("CameraSensor"), SystemInterface::hashFields_CameraSensor));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("CameraServerConfiguration"), SystemInterface::hashFields_CameraServerConfiguration));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("CameraServerStatus"), SystemInterface::hashFields_CameraServerStatus));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("CameraStreamDisplayIntentState"), SystemInterface::hashFields_CameraStreamDisplayIntentState));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("CancelTask"), SystemInterface::hashFields_CancelTask));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("CommandResult"), SystemInterface::hashFields_CommandResult));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("ConfigureCamera"), SystemInterface::hashFields_ConfigureCamera));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("ConfigureMediaStream"), SystemInterface::hashFields_ConfigureMediaStream));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("CreateCacheStream"), SystemInterface::hashFields_CreateCacheStream));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("CreateCameraImageDisplayIntent"), SystemInterface::hashFields_CreateCameraImageDisplayIntent));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("CreateCameraStreamDisplayIntent"), SystemInterface::hashFields_CreateCameraStreamDisplayIntent));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("CreateMediaDisplayIntent"), SystemInterface::hashFields_CreateMediaDisplayIntent));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("CreateMediaStream"), SystemInterface::hashFields_CreateMediaStream));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("CreateMonitorProgram"), SystemInterface::hashFields_CreateMonitorProgram));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("CreateStreamCacheDisplayIntent"), SystemInterface::hashFields_CreateStreamCacheDisplayIntent));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("CreateTimelapseCaptureIntent"), SystemInterface::hashFields_CreateTimelapseCaptureIntent));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("CreateWebDisplayIntent"), SystemInterface::hashFields_CreateWebDisplayIntent));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("EmptyObject"), SystemInterface::hashFields_EmptyObject));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("FindCaptureImages"), SystemInterface::hashFields_FindCaptureImages));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("FindCaptureImagesResult"), SystemInterface::hashFields_FindCaptureImagesResult));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("FindMediaItems"), SystemInterface::hashFields_FindMediaItems));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("FindMediaItemsResult"), SystemInterface::hashFields_FindMediaItemsResult));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("FindMediaStreams"), SystemInterface::hashFields_FindMediaStreams));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("FindMediaStreamsResult"), SystemInterface::hashFields_FindMediaStreamsResult));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("FindStreamItems"), SystemInterface::hashFields_FindStreamItems));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("FindStreamItemsResult"), SystemInterface::hashFields_FindStreamItemsResult));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("GetCameraStream"), SystemInterface::hashFields_GetCameraStream));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("GetCameraStreamResult"), SystemInterface::hashFields_GetCameraStreamResult));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("GetCaptureImage"), SystemInterface::hashFields_GetCaptureImage));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("GetCaptureVideo"), SystemInterface::hashFields_GetCaptureVideo));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("GetDashMpd"), SystemInterface::hashFields_GetDashMpd));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("GetDashSegment"), SystemInterface::hashFields_GetDashSegment));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("GetHlsManifest"), SystemInterface::hashFields_GetHlsManifest));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("GetHlsSegment"), SystemInterface::hashFields_GetHlsSegment));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("GetMedia"), SystemInterface::hashFields_GetMedia));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("GetStreamItem"), SystemInterface::hashFields_GetStreamItem));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("GetThumbnailImage"), SystemInterface::hashFields_GetThumbnailImage));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("IntentState"), SystemInterface::hashFields_IntentState));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("MediaDisplayIntentState"), SystemInterface::hashFields_MediaDisplayIntentState));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("MediaDisplayItem"), SystemInterface::hashFields_MediaDisplayItem));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("MediaItem"), SystemInterface::hashFields_MediaItem));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("MediaServerConfiguration"), SystemInterface::hashFields_MediaServerConfiguration));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("MediaServerStatus"), SystemInterface::hashFields_MediaServerStatus));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("MonitorProgramCondition"), SystemInterface::hashFields_MonitorProgramCondition));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("MonitorProgramDirective"), SystemInterface::hashFields_MonitorProgramDirective));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("MonitorProgramTimeCondition"), SystemInterface::hashFields_MonitorProgramTimeCondition));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("MonitorServerConfiguration"), SystemInterface::hashFields_MonitorServerConfiguration));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("MonitorServerStatus"), SystemInterface::hashFields_MonitorServerStatus));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("PlayCacheStream"), SystemInterface::hashFields_PlayCacheStream));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("PlayCameraStream"), SystemInterface::hashFields_PlayCameraStream));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("PlayMedia"), SystemInterface::hashFields_PlayMedia));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("RemoveIntent"), SystemInterface::hashFields_RemoveIntent));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("RemoveMedia"), SystemInterface::hashFields_RemoveMedia));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("RemoveMediaTag"), SystemInterface::hashFields_RemoveMediaTag));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("RemoveStream"), SystemInterface::hashFields_RemoveStream));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("ReportContact"), SystemInterface::hashFields_ReportContact));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("ReportStatus"), SystemInterface::hashFields_ReportStatus));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("ServerError"), SystemInterface::hashFields_ServerError));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("SetAdminSecret"), SystemInterface::hashFields_SetAdminSecret));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("SetIntentActive"), SystemInterface::hashFields_SetIntentActive));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("ShowAgentStatus"), SystemInterface::hashFields_ShowAgentStatus));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("ShowCameraImage"), SystemInterface::hashFields_ShowCameraImage));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("ShowDesktopCountdown"), SystemInterface::hashFields_ShowDesktopCountdown));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("ShowWebUrl"), SystemInterface::hashFields_ShowWebUrl));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("StreamCacheDisplayIntentState"), SystemInterface::hashFields_StreamCacheDisplayIntentState));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("StreamItem"), SystemInterface::hashFields_StreamItem));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("StreamServerConfiguration"), SystemInterface::hashFields_StreamServerConfiguration));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("StreamServerStatus"), SystemInterface::hashFields_StreamServerStatus));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("StreamSummary"), SystemInterface::hashFields_StreamSummary));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("TaskItem"), SystemInterface::hashFields_TaskItem));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("TimelapseCaptureIntentState"), SystemInterface::hashFields_TimelapseCaptureIntentState));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("UpdateAgentConfiguration"), SystemInterface::hashFields_UpdateAgentConfiguration));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("UpdateIntentState"), SystemInterface::hashFields_UpdateIntentState));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("WatchTasks"), SystemInterface::hashFields_WatchTasks));
  hashFieldsMap.insert (std::pair<StdString, SystemInterface::HashFieldsFunction> (StdString ("WebDisplayIntentState"), SystemInterface::hashFields_WebDisplayIntentState));
}

void SystemInterface::getParams_AddMediaTag (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("mediaId"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("tag"), StdString ("string"), StdString (""), 3));
}

void SystemInterface::getParams_AgentConfiguration (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("isEnabled"), StdString ("boolean"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("displayName"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("mediaServerConfiguration"), StdString ("MediaServerConfiguration"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("streamServerConfiguration"), StdString ("StreamServerConfiguration"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("monitorServerConfiguration"), StdString ("MonitorServerConfiguration"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("cameraServerConfiguration"), StdString ("CameraServerConfiguration"), StdString (""), 0));
}

void SystemInterface::getParams_AgentContact (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("urlHostname"), StdString ("string"), StdString (""), 5));
  destList->push_back (SystemInterface::Param (StdString ("tcpPort1"), StdString ("number"), 129, (double) 0, (double) 65535));
  destList->push_back (SystemInterface::Param (StdString ("tcpPort2"), StdString ("number"), 129, (double) 0, (double) 65535));
  destList->push_back (SystemInterface::Param (StdString ("udpPort"), StdString ("number"), 129, (double) 0, (double) 65535));
  destList->push_back (SystemInterface::Param (StdString ("version"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("nodeVersion"), StdString ("string"), StdString (""), 0));
}

void SystemInterface::getParams_AgentHost (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("hostname"), StdString ("string"), StdString (""), 7));
  destList->push_back (SystemInterface::Param (StdString ("authorizePath"), StdString ("string"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("authorizeSecret"), StdString ("string"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("authorizeToken"), StdString ("string"), StdString (""), 0));
}

void SystemInterface::getParams_AgentStatus (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("displayName"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("applicationName"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("urlHostname"), StdString ("string"), StdString (""), 5));
  destList->push_back (SystemInterface::Param (StdString ("tcpPort1"), StdString ("number"), 129, (double) 0, (double) 65535));
  destList->push_back (SystemInterface::Param (StdString ("tcpPort2"), StdString ("number"), 129, (double) 0, (double) 65535));
  destList->push_back (SystemInterface::Param (StdString ("udpPort"), StdString ("number"), 129, (double) 0, (double) 65535));
  destList->push_back (SystemInterface::Param (StdString ("linkPath"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("uptime"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("startTime"), StdString ("number"), StdString (""), 16));
  destList->push_back (SystemInterface::Param (StdString ("runDuration"), StdString ("number"), StdString (""), 16));
  destList->push_back (SystemInterface::Param (StdString ("version"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("nodeVersion"), StdString ("string"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("platform"), StdString ("string"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("isEnabled"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("taskCount"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("runTaskName"), StdString ("string"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("runTaskSubtitle"), StdString ("string"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("runTaskPercentComplete"), StdString ("number"), 128, (double) 0, (double) 100));
  destList->push_back (SystemInterface::Param (StdString ("runCount"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("maxRunCount"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("mediaServerStatus"), StdString ("MediaServerStatus"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("streamServerStatus"), StdString ("StreamServerStatus"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("monitorServerStatus"), StdString ("MonitorServerStatus"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("cameraServerStatus"), StdString ("CameraServerStatus"), StdString (""), 0));
}

void SystemInterface::getParams_ApplicationNews (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("items"), StdString ("array"), StdString ("ApplicationNewsItem"), 1));
}

void SystemInterface::getParams_ApplicationNewsItem (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("message"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("iconType"), StdString ("string"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("actionText"), StdString ("string"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("actionType"), StdString ("string"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("actionTarget"), StdString ("string"), StdString (""), 0));
}

void SystemInterface::getParams_Authorize (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("token"), StdString ("string"), StdString (""), 3));
}

void SystemInterface::getParams_AuthorizeResult (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("token"), StdString ("string"), StdString (""), 3));
}

void SystemInterface::getParams_CameraImageDisplayIntentState (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("host"), StdString ("AgentHost"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("sensor"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_CameraSensor (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("isCapturing"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("videoMonitor"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("capturePeriod"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("imageProfile"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("flip"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("minCaptureTime"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("lastCaptureTime"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("lastCaptureWidth"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("lastCaptureHeight"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_CameraServerConfiguration (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
}

void SystemInterface::getParams_CameraServerStatus (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("isReady"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("freeStorage"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("totalStorage"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("captureImagePath"), StdString ("string"), StdString (""), 65));
  destList->push_back (SystemInterface::Param (StdString ("captureVideoPath"), StdString ("string"), StdString (""), 64));
  destList->push_back (SystemInterface::Param (StdString ("sensors"), StdString ("array"), StdString ("CameraSensor"), 1));
}

void SystemInterface::getParams_CameraStreamDisplayIntentState (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("host"), StdString ("AgentHost"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("sensor"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("streamProfile"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("flip"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_CancelTask (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("taskId"), StdString ("string"), StdString (""), 35));
}

void SystemInterface::getParams_CommandResult (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("success"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("error"), StdString ("string"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("itemId"), StdString ("string"), StdString (""), 32));
  destList->push_back (SystemInterface::Param (StdString ("item"), StdString ("object"), StdString (""), 256));
  destList->push_back (SystemInterface::Param (StdString ("taskId"), StdString ("string"), StdString (""), 32));
  destList->push_back (SystemInterface::Param (StdString ("stringResult"), StdString ("string"), StdString (""), 0));
}

void SystemInterface::getParams_ConfigureCamera (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("sensor"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("isCaptureEnabled"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("capturePeriod"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("imageProfile"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("flip"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_ConfigureMediaStream (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("mediaId"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("mediaServerAgentId"), StdString ("string"), StdString (""), 34));
  destList->push_back (SystemInterface::Param (StdString ("mediaUrl"), StdString ("string"), StdString (""), 65));
  destList->push_back (SystemInterface::Param (StdString ("streamName"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("mediaWidth"), StdString ("number"), StdString (""), 8));
  destList->push_back (SystemInterface::Param (StdString ("mediaHeight"), StdString ("number"), StdString (""), 8));
  destList->push_back (SystemInterface::Param (StdString ("profile"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_CreateCacheStream (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("streamUrl"), StdString ("string"), StdString (""), 67));
  destList->push_back (SystemInterface::Param (StdString ("thumbnailUrl"), StdString ("string"), StdString (""), 67));
  destList->push_back (SystemInterface::Param (StdString ("streamId"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("streamName"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("duration"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("width"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("height"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("bitrate"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("frameRate"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_CreateCameraImageDisplayIntent (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("displayName"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("host"), StdString ("AgentHost"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("sensor"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_CreateCameraStreamDisplayIntent (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("displayName"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("host"), StdString ("AgentHost"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("sensor"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("streamProfile"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("flip"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_CreateMediaDisplayIntent (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("displayName"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("items"), StdString ("array"), StdString ("MediaDisplayItem"), 1));
  destList->push_back (SystemInterface::Param (StdString ("isShuffle"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("minStartPositionDelta"), StdString ("number"), 129, (double) 0, (double) 100));
  destList->push_back (SystemInterface::Param (StdString ("maxStartPositionDelta"), StdString ("number"), 129, (double) 0, (double) 100));
  destList->push_back (SystemInterface::Param (StdString ("minItemDisplayDuration"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("maxItemDisplayDuration"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_CreateMediaStream (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("name"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("mediaServerAgentId"), StdString ("string"), StdString (""), 34));
  destList->push_back (SystemInterface::Param (StdString ("mediaId"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("mediaUrl"), StdString ("string"), StdString (""), 65));
  destList->push_back (SystemInterface::Param (StdString ("width"), StdString ("number"), StdString (""), 8));
  destList->push_back (SystemInterface::Param (StdString ("height"), StdString ("number"), StdString (""), 8));
  destList->push_back (SystemInterface::Param (StdString ("profile"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_CreateMonitorProgram (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("displayName"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("directives"), StdString ("array"), StdString ("MonitorProgramDirective"), 1));
}

void SystemInterface::getParams_CreateStreamCacheDisplayIntent (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("displayName"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("isShuffle"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("minStartPositionDelta"), StdString ("number"), 129, (double) 0, (double) 100));
  destList->push_back (SystemInterface::Param (StdString ("maxStartPositionDelta"), StdString ("number"), 129, (double) 0, (double) 100));
  destList->push_back (SystemInterface::Param (StdString ("minItemDisplayDuration"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("maxItemDisplayDuration"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_CreateTimelapseCaptureIntent (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("sensor"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("capturePeriod"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_CreateWebDisplayIntent (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("displayName"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("urls"), StdString ("array"), StdString ("string"), 3));
  destList->push_back (SystemInterface::Param (StdString ("isShuffle"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("minItemDisplayDuration"), StdString ("number"), StdString (""), 9));
  destList->push_back (SystemInterface::Param (StdString ("maxItemDisplayDuration"), StdString ("number"), StdString (""), 9));
}

void SystemInterface::getParams_EmptyObject (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
}

void SystemInterface::getParams_FindCaptureImages (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("sensor"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("minTime"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("maxTime"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("maxResults"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("isDescending"), StdString ("boolean"), StdString (""), 1));
}

void SystemInterface::getParams_FindCaptureImagesResult (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("captureTimes"), StdString ("array"), StdString ("number"), 17));
}

void SystemInterface::getParams_FindMediaItems (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("searchKey"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("resultOffset"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("maxResults"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("sortOrder"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_FindMediaItemsResult (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("searchKey"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("setSize"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("resultOffset"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_FindMediaStreams (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("sourceIds"), StdString ("array"), StdString ("string"), 35));
}

void SystemInterface::getParams_FindMediaStreamsResult (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("mediaId"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("streams"), StdString ("array"), StdString ("object"), 257));
}

void SystemInterface::getParams_FindStreamItems (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("searchKey"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("resultOffset"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("maxResults"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("sortOrder"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_FindStreamItemsResult (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("searchKey"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("setSize"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("resultOffset"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("streams"), StdString ("array"), StdString ("StreamSummary"), 0));
}

void SystemInterface::getParams_GetCameraStream (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("sensor"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("monitorName"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("streamProfile"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("flip"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_GetCameraStreamResult (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("streamUrl"), StdString ("string"), StdString (""), 67));
}

void SystemInterface::getParams_GetCaptureImage (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("sensor"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("imageTime"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_GetCaptureVideo (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("sensor"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("minTime"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("maxTime"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("isDescending"), StdString ("boolean"), StdString (""), 1));
}

void SystemInterface::getParams_GetDashMpd (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("streamId"), StdString ("string"), StdString (""), 35));
}

void SystemInterface::getParams_GetDashSegment (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("streamId"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("representationIndex"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("segmentIndex"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_GetHlsManifest (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("streamId"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("startPosition"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("minStartPositionDelta"), StdString ("number"), 128, (double) 0, (double) 100));
  destList->push_back (SystemInterface::Param (StdString ("maxStartPositionDelta"), StdString ("number"), 128, (double) 0, (double) 100));
}

void SystemInterface::getParams_GetHlsSegment (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("streamId"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("segmentIndex"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_GetMedia (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 35));
}

void SystemInterface::getParams_GetStreamItem (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("streamId"), StdString ("string"), StdString (""), 35));
}

void SystemInterface::getParams_GetThumbnailImage (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("thumbnailIndex"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_IntentState (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("name"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("groupName"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("displayName"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("isActive"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("conditions"), StdString ("array"), StdString ("object"), 0));
  destList->push_back (SystemInterface::Param (StdString ("state"), StdString ("object"), StdString (""), 1));
}

void SystemInterface::getParams_MediaDisplayIntentState (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("items"), StdString ("array"), StdString ("MediaDisplayItem"), 3));
  destList->push_back (SystemInterface::Param (StdString ("itemChoices"), StdString ("array"), StdString ("number"), 3));
  destList->push_back (SystemInterface::Param (StdString ("isShuffle"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("minStartPositionDelta"), StdString ("number"), 129, (double) 0, (double) 100));
  destList->push_back (SystemInterface::Param (StdString ("maxStartPositionDelta"), StdString ("number"), 129, (double) 0, (double) 100));
  destList->push_back (SystemInterface::Param (StdString ("minItemDisplayDuration"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("maxItemDisplayDuration"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_MediaDisplayItem (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("mediaName"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("streamUrl"), StdString ("string"), StdString (""), 65));
  destList->push_back (SystemInterface::Param (StdString ("streamId"), StdString ("string"), StdString (""), 33));
  destList->push_back (SystemInterface::Param (StdString ("startPosition"), StdString ("number"), StdString (""), 16));
  destList->push_back (SystemInterface::Param (StdString ("thumbnailUrl"), StdString ("string"), StdString (""), 66));
  destList->push_back (SystemInterface::Param (StdString ("thumbnailIndex"), StdString ("number"), StdString (""), 16));
}

void SystemInterface::getParams_MediaItem (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("name"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("mediaPath"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("mtime"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("duration"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("frameRate"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("width"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("height"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("size"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("bitrate"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("isCreateStreamAvailable"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("tags"), StdString ("array"), StdString ("string"), 2));
  destList->push_back (SystemInterface::Param (StdString ("sortKey"), StdString ("string"), StdString (""), 0));
}

void SystemInterface::getParams_MediaServerConfiguration (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("mediaPath"), StdString ("string"), StdString (""), 2));
  destList->push_back (SystemInterface::Param (StdString ("dataPath"), StdString ("string"), StdString (""), 2));
  destList->push_back (SystemInterface::Param (StdString ("scanPeriod"), StdString ("number"), StdString (""), 16));
}

void SystemInterface::getParams_MediaServerStatus (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("isReady"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("mediaCount"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("mediaPath"), StdString ("string"), StdString (""), 65));
  destList->push_back (SystemInterface::Param (StdString ("thumbnailPath"), StdString ("string"), StdString (""), 65));
  destList->push_back (SystemInterface::Param (StdString ("thumbnailCount"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_MonitorProgramCondition (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("name"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("priority"), StdString ("number"), StdString (""), 9));
  destList->push_back (SystemInterface::Param (StdString ("clockTime"), StdString ("array"), StdString ("MonitorProgramTimeCondition"), 0));
  destList->push_back (SystemInterface::Param (StdString ("activeTime"), StdString ("number"), StdString (""), 16));
  destList->push_back (SystemInterface::Param (StdString ("idleTime"), StdString ("number"), StdString (""), 16));
}

void SystemInterface::getParams_MonitorProgramDirective (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("conditions"), StdString ("array"), StdString ("MonitorProgramCondition"), 1));
  destList->push_back (SystemInterface::Param (StdString ("webDisplayIntent"), StdString ("CreateWebDisplayIntent"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("mediaDisplayIntent"), StdString ("CreateMediaDisplayIntent"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("streamCacheDisplayIntent"), StdString ("CreateStreamCacheDisplayIntent"), StdString (""), 0));
}

void SystemInterface::getParams_MonitorProgramTimeCondition (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("weekdays"), StdString ("array"), StdString ("number"), 0));
  destList->push_back (SystemInterface::Param (StdString ("startHour"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("startMinute"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("endHour"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("endMinute"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_MonitorServerConfiguration (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
}

void SystemInterface::getParams_MonitorServerStatus (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("freeStorage"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("totalStorage"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("streamCount"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("cacheMtime"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("thumbnailPath"), StdString ("string"), StdString (""), 65));
  destList->push_back (SystemInterface::Param (StdString ("screenshotPath"), StdString ("string"), StdString (""), 65));
  destList->push_back (SystemInterface::Param (StdString ("screenshotTime"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("displayState"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("displayTarget"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("isPlayPaused"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("isShowUrlAvailable"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("intentName"), StdString ("string"), StdString (""), 1));
}

void SystemInterface::getParams_PlayCacheStream (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("streamId"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("startPosition"), StdString ("number"), StdString (""), 16));
  destList->push_back (SystemInterface::Param (StdString ("minStartPositionDelta"), StdString ("number"), 128, (double) 0, (double) 100));
  destList->push_back (SystemInterface::Param (StdString ("maxStartPositionDelta"), StdString ("number"), 128, (double) 0, (double) 100));
}

void SystemInterface::getParams_PlayCameraStream (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("host"), StdString ("AgentHost"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("sensor"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("streamProfile"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("flip"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_PlayMedia (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("mediaName"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("streamUrl"), StdString ("string"), StdString (""), 66));
  destList->push_back (SystemInterface::Param (StdString ("streamId"), StdString ("string"), StdString (""), 34));
  destList->push_back (SystemInterface::Param (StdString ("startPosition"), StdString ("number"), StdString (""), 16));
  destList->push_back (SystemInterface::Param (StdString ("minStartPositionDelta"), StdString ("number"), 128, (double) 0, (double) 100));
  destList->push_back (SystemInterface::Param (StdString ("maxStartPositionDelta"), StdString ("number"), 128, (double) 0, (double) 100));
  destList->push_back (SystemInterface::Param (StdString ("thumbnailUrl"), StdString ("string"), StdString (""), 66));
}

void SystemInterface::getParams_RemoveIntent (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 35));
}

void SystemInterface::getParams_RemoveMedia (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 35));
}

void SystemInterface::getParams_RemoveMediaTag (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("mediaId"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("tag"), StdString ("string"), StdString (""), 3));
}

void SystemInterface::getParams_RemoveStream (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 35));
}

void SystemInterface::getParams_ReportContact (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("destination"), StdString ("string"), StdString (""), 65));
}

void SystemInterface::getParams_ReportStatus (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("destination"), StdString ("string"), StdString (""), 65));
}

void SystemInterface::getParams_ServerError (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("error"), StdString ("string"), StdString (""), 0));
}

void SystemInterface::getParams_SetAdminSecret (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("secret"), StdString ("string"), StdString (""), 1));
}

void SystemInterface::getParams_SetIntentActive (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("isActive"), StdString ("boolean"), StdString (""), 1));
}

void SystemInterface::getParams_ShowAgentStatus (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("hostname"), StdString ("string"), StdString (""), 7));
}

void SystemInterface::getParams_ShowCameraImage (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("host"), StdString ("AgentHost"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("sensor"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("imageTime"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("displayTimestamp"), StdString ("number"), StdString (""), 16));
}

void SystemInterface::getParams_ShowDesktopCountdown (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("countdownTime"), StdString ("number"), StdString (""), 9));
}

void SystemInterface::getParams_ShowWebUrl (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("url"), StdString ("string"), StdString (""), 67));
}

void SystemInterface::getParams_StreamCacheDisplayIntentState (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("isShuffle"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("minStartPositionDelta"), StdString ("number"), 129, (double) 0, (double) 100));
  destList->push_back (SystemInterface::Param (StdString ("maxStartPositionDelta"), StdString ("number"), 129, (double) 0, (double) 100));
  destList->push_back (SystemInterface::Param (StdString ("minItemDisplayDuration"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("maxItemDisplayDuration"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_StreamItem (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("name"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("sourceId"), StdString ("string"), StdString (""), 33));
  destList->push_back (SystemInterface::Param (StdString ("duration"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("width"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("height"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("size"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("bitrate"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("frameRate"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("profile"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("hlsTargetDuration"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("segmentCount"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("segmentFilenames"), StdString ("array"), StdString ("string"), 1));
  destList->push_back (SystemInterface::Param (StdString ("segmentLengths"), StdString ("array"), StdString ("number"), 17));
  destList->push_back (SystemInterface::Param (StdString ("segmentPositions"), StdString ("array"), StdString ("number"), 17));
  destList->push_back (SystemInterface::Param (StdString ("tags"), StdString ("array"), StdString ("string"), 2));
}

void SystemInterface::getParams_StreamServerConfiguration (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("dataPath"), StdString ("string"), StdString (""), 2));
}

void SystemInterface::getParams_StreamServerStatus (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("isReady"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("streamCount"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("freeStorage"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("totalStorage"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("hlsStreamPath"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("thumbnailPath"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("htmlPlayerPath"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("htmlCatalogPath"), StdString ("string"), StdString (""), 1));
}

void SystemInterface::getParams_StreamSummary (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("name"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("duration"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("width"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("height"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("size"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("bitrate"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("frameRate"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("profile"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("segmentCount"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_TaskItem (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 33));
  destList->push_back (SystemInterface::Param (StdString ("name"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("subtitle"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("isRunning"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("percentComplete"), StdString ("number"), 129, (double) 0, (double) 100));
  destList->push_back (SystemInterface::Param (StdString ("createTime"), StdString ("number"), StdString (""), 9));
  destList->push_back (SystemInterface::Param (StdString ("endTime"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_TimelapseCaptureIntentState (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("sensor"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("capturePeriod"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("nextCaptureTime"), StdString ("number"), StdString (""), 17));
}

void SystemInterface::getParams_UpdateAgentConfiguration (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("agentConfiguration"), StdString ("AgentConfiguration"), StdString (""), 1));
}

void SystemInterface::getParams_UpdateIntentState (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("state"), StdString ("object"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("isReplace"), StdString ("boolean"), StdString (""), 1));
}

void SystemInterface::getParams_WatchTasks (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("taskIds"), StdString ("array"), StdString ("string"), 35));
}

void SystemInterface::getParams_WebDisplayIntentState (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("urls"), StdString ("array"), StdString ("string"), 3));
  destList->push_back (SystemInterface::Param (StdString ("urlChoices"), StdString ("array"), StdString ("number"), 3));
  destList->push_back (SystemInterface::Param (StdString ("isShuffle"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("minItemDisplayDuration"), StdString ("number"), StdString (""), 9));
  destList->push_back (SystemInterface::Param (StdString ("maxItemDisplayDuration"), StdString ("number"), StdString (""), 9));
}

void SystemInterface::populateDefaultFields_AddMediaTag (Json *destObject) {
}

void SystemInterface::populateDefaultFields_AgentConfiguration (Json *destObject) {
}

void SystemInterface::populateDefaultFields_AgentContact (Json *destObject) {
  if (! destObject->exists ("nodeVersion")) {
    destObject->set ("nodeVersion", "");
  }
}

void SystemInterface::populateDefaultFields_AgentHost (Json *destObject) {
}

void SystemInterface::populateDefaultFields_AgentStatus (Json *destObject) {
  if (! destObject->exists ("linkPath")) {
    destObject->set ("linkPath", "");
  }
  if (! destObject->exists ("uptime")) {
    destObject->set ("uptime", "");
  }
  if (! destObject->exists ("nodeVersion")) {
    destObject->set ("nodeVersion", "");
  }
  if (! destObject->exists ("platform")) {
    destObject->set ("platform", "");
  }
}

void SystemInterface::populateDefaultFields_ApplicationNews (Json *destObject) {
}

void SystemInterface::populateDefaultFields_ApplicationNewsItem (Json *destObject) {
}

void SystemInterface::populateDefaultFields_Authorize (Json *destObject) {
}

void SystemInterface::populateDefaultFields_AuthorizeResult (Json *destObject) {
}

void SystemInterface::populateDefaultFields_CameraImageDisplayIntentState (Json *destObject) {
}

void SystemInterface::populateDefaultFields_CameraSensor (Json *destObject) {
  if (! destObject->exists ("capturePeriod")) {
    destObject->set ("capturePeriod", 0);
  }
  if (! destObject->exists ("imageProfile")) {
    destObject->set ("imageProfile", 0);
  }
  if (! destObject->exists ("flip")) {
    destObject->set ("flip", 0);
  }
  if (! destObject->exists ("minCaptureTime")) {
    destObject->set ("minCaptureTime", 0);
  }
  if (! destObject->exists ("lastCaptureTime")) {
    destObject->set ("lastCaptureTime", 0);
  }
  if (! destObject->exists ("lastCaptureWidth")) {
    destObject->set ("lastCaptureWidth", 0);
  }
  if (! destObject->exists ("lastCaptureHeight")) {
    destObject->set ("lastCaptureHeight", 0);
  }
}

void SystemInterface::populateDefaultFields_CameraServerConfiguration (Json *destObject) {
}

void SystemInterface::populateDefaultFields_CameraServerStatus (Json *destObject) {
  if (! destObject->exists ("captureImagePath")) {
    destObject->set ("captureImagePath", "");
  }
}

void SystemInterface::populateDefaultFields_CameraStreamDisplayIntentState (Json *destObject) {
  if (! destObject->exists ("streamProfile")) {
    destObject->set ("streamProfile", 0);
  }
  if (! destObject->exists ("flip")) {
    destObject->set ("flip", 0);
  }
}

void SystemInterface::populateDefaultFields_CancelTask (Json *destObject) {
}

void SystemInterface::populateDefaultFields_CommandResult (Json *destObject) {
}

void SystemInterface::populateDefaultFields_ConfigureCamera (Json *destObject) {
  if (! destObject->exists ("capturePeriod")) {
    destObject->set ("capturePeriod", 900);
  }
  if (! destObject->exists ("imageProfile")) {
    destObject->set ("imageProfile", 0);
  }
  if (! destObject->exists ("flip")) {
    destObject->set ("flip", 0);
  }
}

void SystemInterface::populateDefaultFields_ConfigureMediaStream (Json *destObject) {
  if (! destObject->exists ("mediaUrl")) {
    destObject->set ("mediaUrl", "");
  }
  if (! destObject->exists ("profile")) {
    destObject->set ("profile", 0);
  }
}

void SystemInterface::populateDefaultFields_CreateCacheStream (Json *destObject) {
}

void SystemInterface::populateDefaultFields_CreateCameraImageDisplayIntent (Json *destObject) {
}

void SystemInterface::populateDefaultFields_CreateCameraStreamDisplayIntent (Json *destObject) {
  if (! destObject->exists ("streamProfile")) {
    destObject->set ("streamProfile", 0);
  }
  if (! destObject->exists ("flip")) {
    destObject->set ("flip", 0);
  }
}

void SystemInterface::populateDefaultFields_CreateMediaDisplayIntent (Json *destObject) {
  if (! destObject->exists ("minStartPositionDelta")) {
    destObject->set ("minStartPositionDelta", 0);
  }
  if (! destObject->exists ("maxStartPositionDelta")) {
    destObject->set ("maxStartPositionDelta", 0);
  }
  if (! destObject->exists ("minItemDisplayDuration")) {
    destObject->set ("minItemDisplayDuration", 300);
  }
  if (! destObject->exists ("maxItemDisplayDuration")) {
    destObject->set ("maxItemDisplayDuration", 900);
  }
}

void SystemInterface::populateDefaultFields_CreateMediaStream (Json *destObject) {
  if (! destObject->exists ("mediaUrl")) {
    destObject->set ("mediaUrl", "");
  }
  if (! destObject->exists ("profile")) {
    destObject->set ("profile", 0);
  }
}

void SystemInterface::populateDefaultFields_CreateMonitorProgram (Json *destObject) {
}

void SystemInterface::populateDefaultFields_CreateStreamCacheDisplayIntent (Json *destObject) {
  if (! destObject->exists ("minStartPositionDelta")) {
    destObject->set ("minStartPositionDelta", 0);
  }
  if (! destObject->exists ("maxStartPositionDelta")) {
    destObject->set ("maxStartPositionDelta", 0);
  }
  if (! destObject->exists ("minItemDisplayDuration")) {
    destObject->set ("minItemDisplayDuration", 300);
  }
  if (! destObject->exists ("maxItemDisplayDuration")) {
    destObject->set ("maxItemDisplayDuration", 900);
  }
}

void SystemInterface::populateDefaultFields_CreateTimelapseCaptureIntent (Json *destObject) {
  if (! destObject->exists ("capturePeriod")) {
    destObject->set ("capturePeriod", 900);
  }
}

void SystemInterface::populateDefaultFields_CreateWebDisplayIntent (Json *destObject) {
  if (! destObject->exists ("minItemDisplayDuration")) {
    destObject->set ("minItemDisplayDuration", 300);
  }
  if (! destObject->exists ("maxItemDisplayDuration")) {
    destObject->set ("maxItemDisplayDuration", 900);
  }
}

void SystemInterface::populateDefaultFields_EmptyObject (Json *destObject) {
}

void SystemInterface::populateDefaultFields_FindCaptureImages (Json *destObject) {
  if (! destObject->exists ("minTime")) {
    destObject->set ("minTime", 0);
  }
  if (! destObject->exists ("maxTime")) {
    destObject->set ("maxTime", 0);
  }
  if (! destObject->exists ("maxResults")) {
    destObject->set ("maxResults", 0);
  }
  if (! destObject->exists ("isDescending")) {
    destObject->set ("isDescending", false);
  }
}

void SystemInterface::populateDefaultFields_FindCaptureImagesResult (Json *destObject) {
}

void SystemInterface::populateDefaultFields_FindMediaItems (Json *destObject) {
  if (! destObject->exists ("searchKey")) {
    destObject->set ("searchKey", "*");
  }
  if (! destObject->exists ("resultOffset")) {
    destObject->set ("resultOffset", 0);
  }
  if (! destObject->exists ("maxResults")) {
    destObject->set ("maxResults", 0);
  }
  if (! destObject->exists ("sortOrder")) {
    destObject->set ("sortOrder", 0);
  }
}

void SystemInterface::populateDefaultFields_FindMediaItemsResult (Json *destObject) {
  if (! destObject->exists ("setSize")) {
    destObject->set ("setSize", 0);
  }
  if (! destObject->exists ("resultOffset")) {
    destObject->set ("resultOffset", 0);
  }
}

void SystemInterface::populateDefaultFields_FindMediaStreams (Json *destObject) {
}

void SystemInterface::populateDefaultFields_FindMediaStreamsResult (Json *destObject) {
}

void SystemInterface::populateDefaultFields_FindStreamItems (Json *destObject) {
  if (! destObject->exists ("searchKey")) {
    destObject->set ("searchKey", "*");
  }
  if (! destObject->exists ("resultOffset")) {
    destObject->set ("resultOffset", 0);
  }
  if (! destObject->exists ("maxResults")) {
    destObject->set ("maxResults", 0);
  }
  if (! destObject->exists ("sortOrder")) {
    destObject->set ("sortOrder", 0);
  }
}

void SystemInterface::populateDefaultFields_FindStreamItemsResult (Json *destObject) {
  if (! destObject->exists ("setSize")) {
    destObject->set ("setSize", 0);
  }
  if (! destObject->exists ("resultOffset")) {
    destObject->set ("resultOffset", 0);
  }
}

void SystemInterface::populateDefaultFields_GetCameraStream (Json *destObject) {
  if (! destObject->exists ("monitorName")) {
    destObject->set ("monitorName", "");
  }
  if (! destObject->exists ("streamProfile")) {
    destObject->set ("streamProfile", 0);
  }
  if (! destObject->exists ("flip")) {
    destObject->set ("flip", 0);
  }
}

void SystemInterface::populateDefaultFields_GetCameraStreamResult (Json *destObject) {
}

void SystemInterface::populateDefaultFields_GetCaptureImage (Json *destObject) {
  if (! destObject->exists ("imageTime")) {
    destObject->set ("imageTime", 0);
  }
}

void SystemInterface::populateDefaultFields_GetCaptureVideo (Json *destObject) {
  if (! destObject->exists ("minTime")) {
    destObject->set ("minTime", 0);
  }
  if (! destObject->exists ("maxTime")) {
    destObject->set ("maxTime", 0);
  }
  if (! destObject->exists ("isDescending")) {
    destObject->set ("isDescending", false);
  }
}

void SystemInterface::populateDefaultFields_GetDashMpd (Json *destObject) {
}

void SystemInterface::populateDefaultFields_GetDashSegment (Json *destObject) {
}

void SystemInterface::populateDefaultFields_GetHlsManifest (Json *destObject) {
  if (! destObject->exists ("startPosition")) {
    destObject->set ("startPosition", 0);
  }
}

void SystemInterface::populateDefaultFields_GetHlsSegment (Json *destObject) {
  if (! destObject->exists ("segmentIndex")) {
    destObject->set ("segmentIndex", 0);
  }
}

void SystemInterface::populateDefaultFields_GetMedia (Json *destObject) {
}

void SystemInterface::populateDefaultFields_GetStreamItem (Json *destObject) {
}

void SystemInterface::populateDefaultFields_GetThumbnailImage (Json *destObject) {
  if (! destObject->exists ("thumbnailIndex")) {
    destObject->set ("thumbnailIndex", 0);
  }
}

void SystemInterface::populateDefaultFields_IntentState (Json *destObject) {
  if (! destObject->exists ("groupName")) {
    destObject->set ("groupName", "");
  }
  if (! destObject->exists ("displayName")) {
    destObject->set ("displayName", "");
  }
}

void SystemInterface::populateDefaultFields_MediaDisplayIntentState (Json *destObject) {
  if (! destObject->exists ("minStartPositionDelta")) {
    destObject->set ("minStartPositionDelta", 0);
  }
  if (! destObject->exists ("maxStartPositionDelta")) {
    destObject->set ("maxStartPositionDelta", 0);
  }
  if (! destObject->exists ("minItemDisplayDuration")) {
    destObject->set ("minItemDisplayDuration", 300);
  }
  if (! destObject->exists ("maxItemDisplayDuration")) {
    destObject->set ("maxItemDisplayDuration", 900);
  }
}

void SystemInterface::populateDefaultFields_MediaDisplayItem (Json *destObject) {
  if (! destObject->exists ("mediaName")) {
    destObject->set ("mediaName", "");
  }
  if (! destObject->exists ("streamUrl")) {
    destObject->set ("streamUrl", "");
  }
  if (! destObject->exists ("streamId")) {
    destObject->set ("streamId", "");
  }
}

void SystemInterface::populateDefaultFields_MediaItem (Json *destObject) {
  if (! destObject->exists ("mtime")) {
    destObject->set ("mtime", 0);
  }
  if (! destObject->exists ("isCreateStreamAvailable")) {
    destObject->set ("isCreateStreamAvailable", true);
  }
}

void SystemInterface::populateDefaultFields_MediaServerConfiguration (Json *destObject) {
}

void SystemInterface::populateDefaultFields_MediaServerStatus (Json *destObject) {
  if (! destObject->exists ("thumbnailPath")) {
    destObject->set ("thumbnailPath", "");
  }
  if (! destObject->exists ("thumbnailCount")) {
    destObject->set ("thumbnailCount", 0);
  }
}

void SystemInterface::populateDefaultFields_MonitorProgramCondition (Json *destObject) {
  if (! destObject->exists ("priority")) {
    destObject->set ("priority", 1);
  }
}

void SystemInterface::populateDefaultFields_MonitorProgramDirective (Json *destObject) {
}

void SystemInterface::populateDefaultFields_MonitorProgramTimeCondition (Json *destObject) {
  if (! destObject->exists ("startHour")) {
    destObject->set ("startHour", 0);
  }
  if (! destObject->exists ("startMinute")) {
    destObject->set ("startMinute", 0);
  }
  if (! destObject->exists ("endHour")) {
    destObject->set ("endHour", 23);
  }
  if (! destObject->exists ("endMinute")) {
    destObject->set ("endMinute", 59);
  }
}

void SystemInterface::populateDefaultFields_MonitorServerConfiguration (Json *destObject) {
}

void SystemInterface::populateDefaultFields_MonitorServerStatus (Json *destObject) {
  if (! destObject->exists ("thumbnailPath")) {
    destObject->set ("thumbnailPath", "");
  }
  if (! destObject->exists ("screenshotPath")) {
    destObject->set ("screenshotPath", "");
  }
  if (! destObject->exists ("screenshotTime")) {
    destObject->set ("screenshotTime", 0);
  }
  if (! destObject->exists ("displayState")) {
    destObject->set ("displayState", 0);
  }
  if (! destObject->exists ("displayTarget")) {
    destObject->set ("displayTarget", "");
  }
  if (! destObject->exists ("intentName")) {
    destObject->set ("intentName", "");
  }
}

void SystemInterface::populateDefaultFields_PlayCacheStream (Json *destObject) {
}

void SystemInterface::populateDefaultFields_PlayCameraStream (Json *destObject) {
  if (! destObject->exists ("streamProfile")) {
    destObject->set ("streamProfile", 0);
  }
  if (! destObject->exists ("flip")) {
    destObject->set ("flip", 0);
  }
}

void SystemInterface::populateDefaultFields_PlayMedia (Json *destObject) {
  if (! destObject->exists ("mediaName")) {
    destObject->set ("mediaName", "");
  }
}

void SystemInterface::populateDefaultFields_RemoveIntent (Json *destObject) {
}

void SystemInterface::populateDefaultFields_RemoveMedia (Json *destObject) {
}

void SystemInterface::populateDefaultFields_RemoveMediaTag (Json *destObject) {
}

void SystemInterface::populateDefaultFields_RemoveStream (Json *destObject) {
}

void SystemInterface::populateDefaultFields_ReportContact (Json *destObject) {
}

void SystemInterface::populateDefaultFields_ReportStatus (Json *destObject) {
}

void SystemInterface::populateDefaultFields_ServerError (Json *destObject) {
  if (! destObject->exists ("error")) {
    destObject->set ("error", "");
  }
}

void SystemInterface::populateDefaultFields_SetAdminSecret (Json *destObject) {
}

void SystemInterface::populateDefaultFields_SetIntentActive (Json *destObject) {
}

void SystemInterface::populateDefaultFields_ShowAgentStatus (Json *destObject) {
}

void SystemInterface::populateDefaultFields_ShowCameraImage (Json *destObject) {
  if (! destObject->exists ("imageTime")) {
    destObject->set ("imageTime", 0);
  }
}

void SystemInterface::populateDefaultFields_ShowDesktopCountdown (Json *destObject) {
  if (! destObject->exists ("countdownTime")) {
    destObject->set ("countdownTime", 20000);
  }
}

void SystemInterface::populateDefaultFields_ShowWebUrl (Json *destObject) {
}

void SystemInterface::populateDefaultFields_StreamCacheDisplayIntentState (Json *destObject) {
  if (! destObject->exists ("minStartPositionDelta")) {
    destObject->set ("minStartPositionDelta", 0);
  }
  if (! destObject->exists ("maxStartPositionDelta")) {
    destObject->set ("maxStartPositionDelta", 0);
  }
  if (! destObject->exists ("minItemDisplayDuration")) {
    destObject->set ("minItemDisplayDuration", 300);
  }
  if (! destObject->exists ("maxItemDisplayDuration")) {
    destObject->set ("maxItemDisplayDuration", 900);
  }
}

void SystemInterface::populateDefaultFields_StreamItem (Json *destObject) {
  if (! destObject->exists ("sourceId")) {
    destObject->set ("sourceId", "");
  }
  if (! destObject->exists ("profile")) {
    destObject->set ("profile", 0);
  }
}

void SystemInterface::populateDefaultFields_StreamServerConfiguration (Json *destObject) {
}

void SystemInterface::populateDefaultFields_StreamServerStatus (Json *destObject) {
}

void SystemInterface::populateDefaultFields_StreamSummary (Json *destObject) {
  if (! destObject->exists ("profile")) {
    destObject->set ("profile", 0);
  }
}

void SystemInterface::populateDefaultFields_TaskItem (Json *destObject) {
  if (! destObject->exists ("subtitle")) {
    destObject->set ("subtitle", "");
  }
  if (! destObject->exists ("percentComplete")) {
    destObject->set ("percentComplete", 0);
  }
}

void SystemInterface::populateDefaultFields_TimelapseCaptureIntentState (Json *destObject) {
  if (! destObject->exists ("capturePeriod")) {
    destObject->set ("capturePeriod", 900);
  }
  if (! destObject->exists ("nextCaptureTime")) {
    destObject->set ("nextCaptureTime", 0);
  }
}

void SystemInterface::populateDefaultFields_UpdateAgentConfiguration (Json *destObject) {
}

void SystemInterface::populateDefaultFields_UpdateIntentState (Json *destObject) {
  if (! destObject->exists ("isReplace")) {
    destObject->set ("isReplace", false);
  }
}

void SystemInterface::populateDefaultFields_WatchTasks (Json *destObject) {
}

void SystemInterface::populateDefaultFields_WebDisplayIntentState (Json *destObject) {
  if (! destObject->exists ("minItemDisplayDuration")) {
    destObject->set ("minItemDisplayDuration", 300);
  }
  if (! destObject->exists ("maxItemDisplayDuration")) {
    destObject->set ("maxItemDisplayDuration", 900);
  }
}

void SystemInterface::hashFields_AddMediaTag (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("mediaId", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("tag", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_AgentConfiguration (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  Json obj;
  StdString s;

  if (commandParams->exists ("cameraServerConfiguration")) {
    if (commandParams->getObject ("cameraServerConfiguration", &obj)) {
      SystemInterface::hashFields_CameraServerConfiguration (&obj, hashUpdateFn, hashContextPtr);
    }
  }
  s = commandParams->getString ("displayName", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("isEnabled")) {
    s.sprintf ("%s", commandParams->getBoolean ("isEnabled", false) ? "true" : "false");
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("mediaServerConfiguration")) {
    if (commandParams->getObject ("mediaServerConfiguration", &obj)) {
      SystemInterface::hashFields_MediaServerConfiguration (&obj, hashUpdateFn, hashContextPtr);
    }
  }
  if (commandParams->exists ("monitorServerConfiguration")) {
    if (commandParams->getObject ("monitorServerConfiguration", &obj)) {
      SystemInterface::hashFields_MonitorServerConfiguration (&obj, hashUpdateFn, hashContextPtr);
    }
  }
  if (commandParams->exists ("streamServerConfiguration")) {
    if (commandParams->getObject ("streamServerConfiguration", &obj)) {
      SystemInterface::hashFields_StreamServerConfiguration (&obj, hashUpdateFn, hashContextPtr);
    }
  }
}

void SystemInterface::hashFields_AgentContact (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("id", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("nodeVersion")) {
    s = commandParams->getString ("nodeVersion", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("tcpPort1", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("tcpPort2", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("udpPort", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("urlHostname", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("version", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_AgentHost (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  if (commandParams->exists ("authorizePath")) {
    s = commandParams->getString ("authorizePath", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  if (commandParams->exists ("authorizeSecret")) {
    s = commandParams->getString ("authorizeSecret", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  if (commandParams->exists ("authorizeToken")) {
    s = commandParams->getString ("authorizeToken", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  s = commandParams->getString ("hostname", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_AgentStatus (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  Json obj;
  StdString s;

  s = commandParams->getString ("applicationName", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("cameraServerStatus")) {
    if (commandParams->getObject ("cameraServerStatus", &obj)) {
      SystemInterface::hashFields_CameraServerStatus (&obj, hashUpdateFn, hashContextPtr);
    }
  }
  s = commandParams->getString ("displayName", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("id", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%s", commandParams->getBoolean ("isEnabled", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("linkPath", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxRunCount", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  if (commandParams->exists ("mediaServerStatus")) {
    if (commandParams->getObject ("mediaServerStatus", &obj)) {
      SystemInterface::hashFields_MediaServerStatus (&obj, hashUpdateFn, hashContextPtr);
    }
  }
  if (commandParams->exists ("monitorServerStatus")) {
    if (commandParams->getObject ("monitorServerStatus", &obj)) {
      SystemInterface::hashFields_MonitorServerStatus (&obj, hashUpdateFn, hashContextPtr);
    }
  }
  if (commandParams->exists ("nodeVersion")) {
    s = commandParams->getString ("nodeVersion", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  if (commandParams->exists ("platform")) {
    s = commandParams->getString ("platform", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("runCount", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  if (commandParams->exists ("runDuration")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("runDuration", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("runTaskName")) {
    s = commandParams->getString ("runTaskName", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  if (commandParams->exists ("runTaskPercentComplete")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("runTaskPercentComplete", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("runTaskSubtitle")) {
    s = commandParams->getString ("runTaskSubtitle", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  if (commandParams->exists ("startTime")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("startTime", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("streamServerStatus")) {
    if (commandParams->getObject ("streamServerStatus", &obj)) {
      SystemInterface::hashFields_StreamServerStatus (&obj, hashUpdateFn, hashContextPtr);
    }
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("taskCount", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("tcpPort1", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("tcpPort2", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("udpPort", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("uptime", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("urlHostname", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("version", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_ApplicationNews (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  Json obj;
  int i, len;

  len = commandParams->getArrayLength ("items");
  for (i = 0; i < len; ++i) {
    if (commandParams->getArrayObject ("items", i, &obj)) {
      SystemInterface::hashFields_ApplicationNewsItem (&obj, hashUpdateFn, hashContextPtr);
    }
  }
}

void SystemInterface::hashFields_ApplicationNewsItem (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  if (commandParams->exists ("actionTarget")) {
    s = commandParams->getString ("actionTarget", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  if (commandParams->exists ("actionText")) {
    s = commandParams->getString ("actionText", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  if (commandParams->exists ("actionType")) {
    s = commandParams->getString ("actionType", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  if (commandParams->exists ("iconType")) {
    s = commandParams->getString ("iconType", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  s = commandParams->getString ("message", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_Authorize (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("token", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_AuthorizeResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("token", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_CameraImageDisplayIntentState (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  Json obj;
  StdString s;

  if (commandParams->getObject ("host", &obj)) {
    SystemInterface::hashFields_AgentHost (&obj, hashUpdateFn, hashContextPtr);
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("sensor", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_CameraSensor (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("capturePeriod", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("flip", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("imageProfile", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%s", commandParams->getBoolean ("isCapturing", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("lastCaptureHeight", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("lastCaptureTime", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("lastCaptureWidth", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("minCaptureTime", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("videoMonitor", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_CameraServerConfiguration (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {

}

void SystemInterface::hashFields_CameraServerStatus (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  Json obj;
  StdString s;
  int i, len;

  s = commandParams->getString ("captureImagePath", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("captureVideoPath")) {
    s = commandParams->getString ("captureVideoPath", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("freeStorage", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%s", commandParams->getBoolean ("isReady", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  len = commandParams->getArrayLength ("sensors");
  for (i = 0; i < len; ++i) {
    if (commandParams->getArrayObject ("sensors", i, &obj)) {
      SystemInterface::hashFields_CameraSensor (&obj, hashUpdateFn, hashContextPtr);
    }
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("totalStorage", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_CameraStreamDisplayIntentState (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  Json obj;
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("flip", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  if (commandParams->getObject ("host", &obj)) {
    SystemInterface::hashFields_AgentHost (&obj, hashUpdateFn, hashContextPtr);
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("sensor", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("streamProfile", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_CancelTask (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("taskId", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_CommandResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  if (commandParams->exists ("error")) {
    s = commandParams->getString ("error", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  if (commandParams->exists ("itemId")) {
    s = commandParams->getString ("itemId", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  if (commandParams->exists ("stringResult")) {
    s = commandParams->getString ("stringResult", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  s.sprintf ("%s", commandParams->getBoolean ("success", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  if (commandParams->exists ("taskId")) {
    s = commandParams->getString ("taskId", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
}

void SystemInterface::hashFields_ConfigureCamera (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("capturePeriod", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("flip", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("imageProfile", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%s", commandParams->getBoolean ("isCaptureEnabled", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("sensor", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_ConfigureMediaStream (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  if (commandParams->exists ("mediaHeight")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("mediaHeight", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("mediaId", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("mediaServerAgentId")) {
    s = commandParams->getString ("mediaServerAgentId", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  s = commandParams->getString ("mediaUrl", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("mediaWidth")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("mediaWidth", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("profile", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("streamName", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_CreateCacheStream (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("bitrate", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("duration", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("frameRate", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("height", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("streamId", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("streamName", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("streamUrl", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("thumbnailUrl", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("width", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_CreateCameraImageDisplayIntent (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  Json obj;
  StdString s;

  s = commandParams->getString ("displayName", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->getObject ("host", &obj)) {
    SystemInterface::hashFields_AgentHost (&obj, hashUpdateFn, hashContextPtr);
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("sensor", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_CreateCameraStreamDisplayIntent (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  Json obj;
  StdString s;

  s = commandParams->getString ("displayName", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("flip", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  if (commandParams->getObject ("host", &obj)) {
    SystemInterface::hashFields_AgentHost (&obj, hashUpdateFn, hashContextPtr);
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("sensor", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("streamProfile", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_CreateMediaDisplayIntent (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  Json obj;
  StdString s;
  int i, len;

  s = commandParams->getString ("displayName", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%s", commandParams->getBoolean ("isShuffle", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  len = commandParams->getArrayLength ("items");
  for (i = 0; i < len; ++i) {
    if (commandParams->getArrayObject ("items", i, &obj)) {
      SystemInterface::hashFields_MediaDisplayItem (&obj, hashUpdateFn, hashContextPtr);
    }
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxItemDisplayDuration", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxStartPositionDelta", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("minItemDisplayDuration", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("minStartPositionDelta", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_CreateMediaStream (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  if (commandParams->exists ("height")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("height", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("mediaId", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("mediaServerAgentId")) {
    s = commandParams->getString ("mediaServerAgentId", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  s = commandParams->getString ("mediaUrl", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("name", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("profile", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  if (commandParams->exists ("width")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("width", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_CreateMonitorProgram (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  Json obj;
  StdString s;
  int i, len;

  len = commandParams->getArrayLength ("directives");
  for (i = 0; i < len; ++i) {
    if (commandParams->getArrayObject ("directives", i, &obj)) {
      SystemInterface::hashFields_MonitorProgramDirective (&obj, hashUpdateFn, hashContextPtr);
    }
  }
  s = commandParams->getString ("displayName", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_CreateStreamCacheDisplayIntent (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("displayName", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%s", commandParams->getBoolean ("isShuffle", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxItemDisplayDuration", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxStartPositionDelta", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("minItemDisplayDuration", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("minStartPositionDelta", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_CreateTimelapseCaptureIntent (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("capturePeriod", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("sensor", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_CreateWebDisplayIntent (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;
  int i, len;

  s = commandParams->getString ("displayName", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%s", commandParams->getBoolean ("isShuffle", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxItemDisplayDuration", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("minItemDisplayDuration", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  len = commandParams->getArrayLength ("urls");
  for (i = 0; i < len; ++i) {
    s = commandParams->getArrayString ("urls", i, StdString (""));
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
}

void SystemInterface::hashFields_EmptyObject (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {

}

void SystemInterface::hashFields_FindCaptureImages (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%s", commandParams->getBoolean ("isDescending", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxResults", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxTime", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("minTime", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("sensor", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_FindCaptureImagesResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;
  int i, len;

  len = commandParams->getArrayLength ("captureTimes");
  for (i = 0; i < len; ++i) {
    s.sprintf ("%lli", (long long int) commandParams->getArrayNumber ("captureTimes", i, (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_FindMediaItems (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxResults", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("resultOffset", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("searchKey", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("sortOrder", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_FindMediaItemsResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("resultOffset", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("searchKey", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("setSize", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_FindMediaStreams (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;
  int i, len;

  len = commandParams->getArrayLength ("sourceIds");
  for (i = 0; i < len; ++i) {
    s = commandParams->getArrayString ("sourceIds", i, StdString (""));
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
}

void SystemInterface::hashFields_FindMediaStreamsResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;
  int i, len;

  s = commandParams->getString ("mediaId", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  len = commandParams->getArrayLength ("streams");
  for (i = 0; i < len; ++i) {
  }
}

void SystemInterface::hashFields_FindStreamItems (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxResults", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("resultOffset", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("searchKey", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("sortOrder", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_FindStreamItemsResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  Json obj;
  StdString s;
  int i, len;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("resultOffset", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("searchKey", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("setSize", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  if (commandParams->exists ("streams")) {
    len = commandParams->getArrayLength ("streams");
    for (i = 0; i < len; ++i) {
      if (commandParams->getArrayObject ("streams", i, &obj)) {
        SystemInterface::hashFields_StreamSummary (&obj, hashUpdateFn, hashContextPtr);
      }
    }
  }
}

void SystemInterface::hashFields_GetCameraStream (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("flip", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("monitorName", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("sensor", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("streamProfile", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_GetCameraStreamResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("streamUrl", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_GetCaptureImage (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("imageTime", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("sensor", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_GetCaptureVideo (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%s", commandParams->getBoolean ("isDescending", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxTime", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("minTime", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("sensor", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_GetDashMpd (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("streamId", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_GetDashSegment (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("representationIndex", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("segmentIndex", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("streamId", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_GetHlsManifest (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  if (commandParams->exists ("maxStartPositionDelta")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxStartPositionDelta", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("minStartPositionDelta")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("minStartPositionDelta", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("startPosition", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("streamId", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_GetHlsSegment (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("segmentIndex", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("streamId", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_GetMedia (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("id", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_GetStreamItem (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("streamId", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_GetThumbnailImage (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("id", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("thumbnailIndex", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_IntentState (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;
  int i, len;

  if (commandParams->exists ("conditions")) {
    len = commandParams->getArrayLength ("conditions");
    for (i = 0; i < len; ++i) {
    }
  }
  s = commandParams->getString ("displayName", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("groupName", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("id", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%s", commandParams->getBoolean ("isActive", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("name", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_MediaDisplayIntentState (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  Json obj;
  StdString s;
  int i, len;

  s.sprintf ("%s", commandParams->getBoolean ("isShuffle", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  len = commandParams->getArrayLength ("itemChoices");
  for (i = 0; i < len; ++i) {
    s.sprintf ("%lli", (long long int) commandParams->getArrayNumber ("itemChoices", i, (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  len = commandParams->getArrayLength ("items");
  for (i = 0; i < len; ++i) {
    if (commandParams->getArrayObject ("items", i, &obj)) {
      SystemInterface::hashFields_MediaDisplayItem (&obj, hashUpdateFn, hashContextPtr);
    }
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxItemDisplayDuration", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxStartPositionDelta", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("minItemDisplayDuration", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("minStartPositionDelta", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_MediaDisplayItem (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("mediaName", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("startPosition")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("startPosition", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("streamId", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("streamUrl", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("thumbnailIndex")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("thumbnailIndex", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("thumbnailUrl")) {
    s = commandParams->getString ("thumbnailUrl", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
}

void SystemInterface::hashFields_MediaItem (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;
  int i, len;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("bitrate", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("duration", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("frameRate", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("height", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("id", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%s", commandParams->getBoolean ("isCreateStreamAvailable", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("mediaPath", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("mtime", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("name", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("size", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  if (commandParams->exists ("sortKey")) {
    s = commandParams->getString ("sortKey", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  if (commandParams->exists ("tags")) {
    len = commandParams->getArrayLength ("tags");
    for (i = 0; i < len; ++i) {
      s = commandParams->getArrayString ("tags", i, StdString (""));
      if (! s.empty ()) {
        hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
      }
    }
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("width", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_MediaServerConfiguration (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  if (commandParams->exists ("dataPath")) {
    s = commandParams->getString ("dataPath", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  if (commandParams->exists ("mediaPath")) {
    s = commandParams->getString ("mediaPath", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  if (commandParams->exists ("scanPeriod")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("scanPeriod", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_MediaServerStatus (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%s", commandParams->getBoolean ("isReady", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("mediaCount", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("mediaPath", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("thumbnailCount", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("thumbnailPath", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_MonitorProgramCondition (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  Json obj;
  StdString s;
  int i, len;

  if (commandParams->exists ("activeTime")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("activeTime", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("clockTime")) {
    len = commandParams->getArrayLength ("clockTime");
    for (i = 0; i < len; ++i) {
      if (commandParams->getArrayObject ("clockTime", i, &obj)) {
        SystemInterface::hashFields_MonitorProgramTimeCondition (&obj, hashUpdateFn, hashContextPtr);
      }
    }
  }
  if (commandParams->exists ("idleTime")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("idleTime", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("name", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("priority", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_MonitorProgramDirective (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  Json obj;
  int i, len;

  len = commandParams->getArrayLength ("conditions");
  for (i = 0; i < len; ++i) {
    if (commandParams->getArrayObject ("conditions", i, &obj)) {
      SystemInterface::hashFields_MonitorProgramCondition (&obj, hashUpdateFn, hashContextPtr);
    }
  }
  if (commandParams->exists ("mediaDisplayIntent")) {
    if (commandParams->getObject ("mediaDisplayIntent", &obj)) {
      SystemInterface::hashFields_CreateMediaDisplayIntent (&obj, hashUpdateFn, hashContextPtr);
    }
  }
  if (commandParams->exists ("streamCacheDisplayIntent")) {
    if (commandParams->getObject ("streamCacheDisplayIntent", &obj)) {
      SystemInterface::hashFields_CreateStreamCacheDisplayIntent (&obj, hashUpdateFn, hashContextPtr);
    }
  }
  if (commandParams->exists ("webDisplayIntent")) {
    if (commandParams->getObject ("webDisplayIntent", &obj)) {
      SystemInterface::hashFields_CreateWebDisplayIntent (&obj, hashUpdateFn, hashContextPtr);
    }
  }
}

void SystemInterface::hashFields_MonitorProgramTimeCondition (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;
  int i, len;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("endHour", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("endMinute", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("startHour", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("startMinute", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  if (commandParams->exists ("weekdays")) {
    len = commandParams->getArrayLength ("weekdays");
    for (i = 0; i < len; ++i) {
      s.sprintf ("%lli", (long long int) commandParams->getArrayNumber ("weekdays", i, (int64_t) 0));
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
}

void SystemInterface::hashFields_MonitorServerConfiguration (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {

}

void SystemInterface::hashFields_MonitorServerStatus (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("cacheMtime", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("displayState", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("displayTarget", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("freeStorage", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("intentName", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%s", commandParams->getBoolean ("isPlayPaused", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%s", commandParams->getBoolean ("isShowUrlAvailable", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("screenshotPath", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("screenshotTime", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("streamCount", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("thumbnailPath", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("totalStorage", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_PlayCacheStream (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  if (commandParams->exists ("maxStartPositionDelta")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxStartPositionDelta", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("minStartPositionDelta")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("minStartPositionDelta", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("startPosition")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("startPosition", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("streamId", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_PlayCameraStream (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  Json obj;
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("flip", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  if (commandParams->getObject ("host", &obj)) {
    SystemInterface::hashFields_AgentHost (&obj, hashUpdateFn, hashContextPtr);
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("sensor", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("streamProfile", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_PlayMedia (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  if (commandParams->exists ("maxStartPositionDelta")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxStartPositionDelta", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("mediaName", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("minStartPositionDelta")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("minStartPositionDelta", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("startPosition")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("startPosition", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("streamId")) {
    s = commandParams->getString ("streamId", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  if (commandParams->exists ("streamUrl")) {
    s = commandParams->getString ("streamUrl", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  if (commandParams->exists ("thumbnailUrl")) {
    s = commandParams->getString ("thumbnailUrl", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
}

void SystemInterface::hashFields_RemoveIntent (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("id", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_RemoveMedia (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("id", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_RemoveMediaTag (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("mediaId", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("tag", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_RemoveStream (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("id", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_ReportContact (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("destination", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_ReportStatus (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("destination", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_ServerError (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  if (commandParams->exists ("error")) {
    s = commandParams->getString ("error", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
}

void SystemInterface::hashFields_SetAdminSecret (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("secret", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_SetIntentActive (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("id", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%s", commandParams->getBoolean ("isActive", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_ShowAgentStatus (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("hostname", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_ShowCameraImage (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  Json obj;
  StdString s;

  if (commandParams->exists ("displayTimestamp")) {
    s.sprintf ("%lli", (long long int) commandParams->getNumber ("displayTimestamp", (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->getObject ("host", &obj)) {
    SystemInterface::hashFields_AgentHost (&obj, hashUpdateFn, hashContextPtr);
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("imageTime", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("sensor", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_ShowDesktopCountdown (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("countdownTime", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_ShowWebUrl (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("url", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_StreamCacheDisplayIntentState (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%s", commandParams->getBoolean ("isShuffle", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxItemDisplayDuration", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxStartPositionDelta", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("minItemDisplayDuration", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("minStartPositionDelta", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_StreamItem (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;
  int i, len;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("bitrate", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("duration", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("frameRate", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("height", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("hlsTargetDuration", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("id", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("name", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("profile", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("segmentCount", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  len = commandParams->getArrayLength ("segmentFilenames");
  for (i = 0; i < len; ++i) {
    s = commandParams->getArrayString ("segmentFilenames", i, StdString (""));
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
  len = commandParams->getArrayLength ("segmentLengths");
  for (i = 0; i < len; ++i) {
    s.sprintf ("%lli", (long long int) commandParams->getArrayNumber ("segmentLengths", i, (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  len = commandParams->getArrayLength ("segmentPositions");
  for (i = 0; i < len; ++i) {
    s.sprintf ("%lli", (long long int) commandParams->getArrayNumber ("segmentPositions", i, (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("size", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("sourceId", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  if (commandParams->exists ("tags")) {
    len = commandParams->getArrayLength ("tags");
    for (i = 0; i < len; ++i) {
      s = commandParams->getArrayString ("tags", i, StdString (""));
      if (! s.empty ()) {
        hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
      }
    }
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("width", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_StreamServerConfiguration (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  if (commandParams->exists ("dataPath")) {
    s = commandParams->getString ("dataPath", "");
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
}

void SystemInterface::hashFields_StreamServerStatus (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("freeStorage", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("hlsStreamPath", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("htmlCatalogPath", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("htmlPlayerPath", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%s", commandParams->getBoolean ("isReady", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("streamCount", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("thumbnailPath", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("totalStorage", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_StreamSummary (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("bitrate", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("duration", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("frameRate", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("height", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("id", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s = commandParams->getString ("name", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("profile", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("segmentCount", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("size", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("width", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_TaskItem (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("createTime", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("endTime", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("id", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%s", commandParams->getBoolean ("isRunning", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("name", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("percentComplete", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s = commandParams->getString ("subtitle", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
}

void SystemInterface::hashFields_TimelapseCaptureIntentState (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s.sprintf ("%lli", (long long int) commandParams->getNumber ("capturePeriod", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("nextCaptureTime", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("sensor", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_UpdateAgentConfiguration (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  Json obj;

  if (commandParams->getObject ("agentConfiguration", &obj)) {
    SystemInterface::hashFields_AgentConfiguration (&obj, hashUpdateFn, hashContextPtr);
  }
}

void SystemInterface::hashFields_UpdateIntentState (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;

  s = commandParams->getString ("id", "");
  if (! s.empty ()) {
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  s.sprintf ("%s", commandParams->getBoolean ("isReplace", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
}

void SystemInterface::hashFields_WatchTasks (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;
  int i, len;

  len = commandParams->getArrayLength ("taskIds");
  for (i = 0; i < len; ++i) {
    s = commandParams->getArrayString ("taskIds", i, StdString (""));
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
}

void SystemInterface::hashFields_WebDisplayIntentState (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
  StdString s;
  int i, len;

  s.sprintf ("%s", commandParams->getBoolean ("isShuffle", false) ? "true" : "false");
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("maxItemDisplayDuration", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  s.sprintf ("%lli", (long long int) commandParams->getNumber ("minItemDisplayDuration", (int64_t) 0));
  hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  len = commandParams->getArrayLength ("urlChoices");
  for (i = 0; i < len; ++i) {
    s.sprintf ("%lli", (long long int) commandParams->getArrayNumber ("urlChoices", i, (int64_t) 0));
    hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
  }
  len = commandParams->getArrayLength ("urls");
  for (i = 0; i < len; ++i) {
    s = commandParams->getArrayString ("urls", i, StdString (""));
    if (! s.empty ()) {
      hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
    }
  }
}
SystemInterface *SystemInterface::instance = NULL;

SystemInterface::SystemInterface ()
: lastError ("")
{
	populate ();
}

SystemInterface::~SystemInterface () {

}

Json *SystemInterface::createCommand (const SystemInterface::Prefix &prefix, const char *commandName, Json *commandParams) {
	Json *cmd, *cmdprefix;
	SystemInterface::Command command;

	if (! commandParams) {
		commandParams = new Json ();
		commandParams->setEmpty ();
	}
	if (! getCommand (commandName, &command)) {
		lastError.sprintf ("Unknown command name \"%s\"", commandName);
		delete (commandParams);
		return (NULL);
	}
	if (! populateDefaultFields (command.paramType, commandParams)) {
		lastError.sprintf ("Command \"%s\" failed to populate fields for param type \"%s\"", commandName, command.paramType.c_str ());
		delete (commandParams);
		return (NULL);
	}

	cmd = new Json ();
	cmd->set ("command", command.id);
	cmd->set ("commandName", commandName);

	cmdprefix = new Json ();
	if (prefix.createTime > 0) {
		cmdprefix->set (SystemInterface::Constant_CreateTimePrefixField, prefix.createTime);
	}
	if (! prefix.agentId.empty ()) {
		cmdprefix->set (SystemInterface::Constant_AgentIdPrefixField, prefix.agentId);
	}
	if (! prefix.userId.empty ()) {
		cmdprefix->set (SystemInterface::Constant_UserIdPrefixField, prefix.userId);
	}
	if (prefix.priority > 0) {
		cmdprefix->set (SystemInterface::Constant_PriorityPrefixField, prefix.priority);
	}
	if (prefix.startTime > 0) {
		cmdprefix->set (SystemInterface::Constant_StartTimePrefixField, prefix.startTime);
	}
	if (prefix.duration > 0) {
		cmdprefix->set (SystemInterface::Constant_DurationPrefixField, prefix.duration);
	}
	cmd->set ("prefix", cmdprefix);

	cmd->set ("params", commandParams);

	return (cmd);
}

Json *SystemInterface::createCommand (const SystemInterface::Prefix &prefix, int commandId, Json *commandParams) {
	std::map<int, StdString>::iterator i;

	i = commandIdMap.find (commandId);
	if (i == commandIdMap.end ()) {
		lastError.sprintf ("Unknown command ID %i", commandId);
		return (NULL);
	}
	return (createCommand (prefix, i->second.c_str (), commandParams));
}

bool SystemInterface::setCommandAuthorization (Json *command, const StdString &authSecret, const StdString &authToken, SystemInterface::HashUpdateFunction hashUpdateFn, SystemInterface::HashDigestFunction hashDigestFn, void *hashContextPtr) {
	StdString hash;
	Json prefix;
	bool result;

	result = false;
	hash = getCommandAuthorizationHash (command, authSecret, authToken, hashUpdateFn, hashDigestFn, hashContextPtr);
	if (! hash.empty ()) {
		if (command->getObject ("prefix", &prefix)) {
			prefix.set (SystemInterface::Constant_AuthorizationHashPrefixField, hash);
			if ((! authToken.empty ()) && (! prefix.exists (SystemInterface::Constant_AuthorizationTokenPrefixField))) {
				prefix.set (SystemInterface::Constant_AuthorizationTokenPrefixField, authToken);
			}
			result = true;
		}
	}
	return (result);
}

StdString SystemInterface::getCommandAuthorizationHash (Json *command, const StdString &authSecret, const StdString &authToken, SystemInterface::HashUpdateFunction hashUpdateFn, SystemInterface::HashDigestFunction hashDigestFn, void *hashContextPtr) {
	SystemInterface::Command cmd;
	StdString token, cmdname, s;
	Json prefix, params;

	if (! (hashUpdateFn && hashDigestFn)) {
		return (StdString (""));
	}
	cmdname = command->getString ("commandName", "");
	if (! getCommand (cmdname, &cmd)) {
		return (StdString (""));
	}
	if (! command->getObject ("prefix", &prefix)) {
		return (StdString (""));
	}

	if (! authToken.empty ()) {
		token.assign (authToken);
	}
	else {
		token = prefix.getString (SystemInterface::Constant_AuthorizationTokenPrefixField, "");
	}

	hashUpdateFn (hashContextPtr, (unsigned char *) authSecret.c_str (), authSecret.length ());
	hashUpdateFn (hashContextPtr, (unsigned char *) token.c_str (), token.length ());
	hashUpdateFn (hashContextPtr, (unsigned char *) cmdname.c_str (), cmdname.length ());

	if (prefix.isNumber (SystemInterface::Constant_CreateTimePrefixField)) {
		s.sprintf ("%lli", (long long int) prefix.getNumber (SystemInterface::Constant_CreateTimePrefixField, (int64_t) 0));
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}

	s = prefix.getString (SystemInterface::Constant_AgentIdPrefixField, "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}

	s = prefix.getString (SystemInterface::Constant_UserIdPrefixField, "");
	if (! s.empty ()) {
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}

	if (prefix.isNumber (SystemInterface::Constant_PriorityPrefixField)) {
		s.sprintf ("%lli", (long long int) prefix.getNumber (SystemInterface::Constant_PriorityPrefixField, (int64_t) 0));
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}

	if (prefix.isNumber (SystemInterface::Constant_StartTimePrefixField)) {
		s.sprintf ("%lli", (long long int) prefix.getNumber (SystemInterface::Constant_StartTimePrefixField, (int64_t) 0));
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}

	if (prefix.isNumber (SystemInterface::Constant_DurationPrefixField)) {
		s.sprintf ("%lli", (long long int) prefix.getNumber (SystemInterface::Constant_DurationPrefixField, (int64_t) 0));
		hashUpdateFn (hashContextPtr, (unsigned char *) s.c_str (), s.length ());
	}

	if (command->getObject ("params", &params)) {
		hashFields (cmd.paramType, &params, hashUpdateFn, hashContextPtr);
	}

	return (hashDigestFn (hashContextPtr));
}

bool SystemInterface::getCommand (const StdString &name, SystemInterface::Command *command) {
	std::map<StdString, SystemInterface::Command>::iterator i;

	i = commandMap.find (name);
	if (i == commandMap.end ()) {
		return (false);
	}
	*command = i->second;
	return (true);
}

bool SystemInterface::getType (const StdString &name, std::list<SystemInterface::Param> *destList) {
	std::map<StdString, SystemInterface::GetParamsFunction>::iterator i;

	i = getParamsMap.find (name);
	if (i == getParamsMap.end ()) {
		return (false);
	}
	i->second (destList);
	return (true);
}

bool SystemInterface::populateDefaultFields (const StdString &typeName, Json *destObject) {
	std::map<StdString, SystemInterface::PopulateDefaultFieldsFunction>::iterator i;

	i = populateDefaultFieldsMap.find (typeName);
	if (i == populateDefaultFieldsMap.end ()) {
		return (false);
	}
	i->second (destObject);
	return (true);
}

void SystemInterface::hashFields (const StdString &typeName, Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr) {
	std::map<StdString, SystemInterface::HashFieldsFunction>::iterator i;

	if (! hashUpdateFn) {
		return;
	}
	i = hashFieldsMap.find (typeName);
	if (i == hashFieldsMap.end ()) {
		return;
	}
	i->second (commandParams, hashUpdateFn, hashContextPtr);
}

bool SystemInterface::fieldsValid (Json *fields, std::list<SystemInterface::Param> *paramList) {
	std::list<SystemInterface::Param>::iterator i, end;
	double numbervalue;
	StdString stringvalue;

	// TODO: Check for unknown field keys
	// TODO: Possibly allow validation to succeed if unknown field keys are present

	i = paramList->begin ();
	end = paramList->end ();
	while (i != end) {
		if (i->flags & SystemInterface::ParamFlag_Required) {
			if (! fields->exists (i->name)) {
				lastError.sprintf ("Missing required parameter field \"%s\"", i->name.c_str ());
				return (false);
			}
		}

		if (i->type.equals ("number")) {
			if (! fields->isNumber (i->name)) {
				lastError.sprintf ("Parameter field \"%s\" has incorrect type (expecting number)", i->name.c_str ());
				return (false);
			}

			numbervalue = fields->getNumber (i->name, 0);
			if (i->flags & SystemInterface::ParamFlag_GreaterThanZero) {
				if (numbervalue <= 0.0f) {
					lastError.sprintf ("Parameter field \"%s\" must be a number greater than zero", i->name.c_str ());
					return (false);
				}
			}

			if (i->flags & SystemInterface::ParamFlag_ZeroOrGreater) {
				if (numbervalue < 0.0f) {
					lastError.sprintf ("Parameter field \"%s\" must be a number greater than or equal to zero", i->name.c_str ());
					return (false);
				}
			}

			if (i->flags & SystemInterface::ParamFlag_RangedNumber) {
				if ((numbervalue < i->rangeMin) || (numbervalue > i->rangeMax)) {
					lastError.sprintf ("Parameter field \"%s\" must be a number in the range [%f..%f]", i->name.c_str (), i->rangeMin, i->rangeMax);
					return (false);
				}
			}

			if (i->flags & SystemInterface::ParamFlag_EnumValue) {
				// TODO: Implement this
			}
		}
		else if (i->type.equals ("boolean")) {
			if (! fields->isBoolean (i->name)) {
				lastError.sprintf ("Parameter field \"%s\" has incorrect type (expecting boolean)", i->name.c_str ());
				return (false);
			}
		}
		else if (i->type.equals ("string")) {
			if (! fields->isString (i->name)) {
				lastError.sprintf ("Parameter field \"%s\" has incorrect type (expecting string)", i->name.c_str ());
				return (false);
			}

			stringvalue = fields->getString (i->name, "");
			if (i->flags & SystemInterface::ParamFlag_NotEmpty) {
				if (stringvalue.empty ()) {
					lastError.sprintf ("Parameter field \"%s\" cannot contain an empty string", i->name.c_str ());
					return (false);
				}
			}

			if ((i->flags & SystemInterface::ParamFlag_Hostname) && (! stringvalue.empty ())) {
				// TODO: Implement this
				/*
				if (value.search (/^[a-zA-Z][a-zA-Z0-9-\.]*(:[0-9]+){0,1}$/) != 0) {
					return ("Parameter field \"" + param.name + "\" must contain a hostname string");
				}
				*/
			}

			if ((i->flags & SystemInterface::ParamFlag_Uuid) && (! stringvalue.empty ())) {
				// TODO: Implement this
				/*
				if (value.search (/^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$/) != 0) {
					return ("Parameter field \"" + param.name + "\" must contain a UUID string");
				}
				*/
			}

			if ((i->flags & SystemInterface::ParamFlag_Url) && (! stringvalue.empty ())) {
				// TODO: Implement this
				/*
				if (value.search (/[^A-Za-z0-9\$\-_\.\+\!\*\?\(\),\/:;=&]/) != -1) {
					return ("Parameter field \"" + param.name + "\" must contain a URL string");
				}
				*/
			}

			if (i->flags & SystemInterface::ParamFlag_EnumValue) {
				// TODO: Implement this
			}
		}
		else if (i->type.equals ("array")) {
			// TODO: Implement this
		}
		else if (i->type.equals ("map")) {
			// TODO: Implement this
		}
		else if (i->type.equals ("object")) {
			// TODO: Implement this
		}

		++i;
	}

	return (true);
}

bool SystemInterface::parseCommand (const StdString &commandString, Json **commandJson) {
	Json *json;

	json = new Json ();
	if (! json->parse (commandString.c_str (), commandString.length ())) {
		lastError.assign ("JSON parse failed");
		delete (json);
		return (false);
	}

	// TODO: Validate fields here

	if (commandJson) {
		*commandJson = json;
	}
	else {
		delete (json);
	}
	return (true);
}

int SystemInterface::getCommandId (Json *command) {
	return (command->getNumber ("command", -1));
}

StdString SystemInterface::getCommandName (Json *command) {
	return (command->getString ("commandName", ""));
}

StdString SystemInterface::getCommandAgentId (Json *command) {
	Json prefix;

	if (! command->getObject ("prefix", &prefix)) {
		return (StdString (""));
	}

	return (prefix.getString (SystemInterface::Constant_AgentIdPrefixField, ""));
}

StdString SystemInterface::getCommandRecordId (Json *command) {
	return (getCommandStringParam (command, "id", ""));
}

bool SystemInterface::isWindowsPlatform (const StdString &platform) {
	return (platform.startsWith ("windows") || platform.equals ("win32") || platform.equals ("win64"));
}

SystemInterface::Prefix SystemInterface::getCommandPrefix (Json *command) {
	SystemInterface::Prefix result;
	Json prefix;

	if (! command->getObject ("prefix", &prefix)) {
		return (result);
	}
	result.agentId = prefix.getString (SystemInterface::Constant_AgentIdPrefixField, "");
	result.userId = prefix.getString (SystemInterface::Constant_UserIdPrefixField, "");
	result.priority = prefix.getNumber (SystemInterface::Constant_PriorityPrefixField, (int) 0);
	result.createTime = prefix.getNumber (SystemInterface::Constant_CreateTimePrefixField, (int64_t) 0);
	result.startTime = prefix.getNumber (SystemInterface::Constant_StartTimePrefixField, (int64_t) 0);
	result.duration = prefix.getNumber (SystemInterface::Constant_DurationPrefixField, (int64_t) 0);

	return (result);
}

bool SystemInterface::getCommandParams (Json *command, Json *params) {
	return (command->getObject ("params", params));
}

StdString SystemInterface::getCommandStringParam (Json *command, const StdString &paramName, const StdString &defaultValue) {
	Json params;

	if (! command->getObject ("params", &params)) {
		return (defaultValue);
	}
	return (params.getString (paramName, defaultValue));
}

StdString SystemInterface::getCommandStringParam (Json *command, const char *paramName, const char *defaultValue) {
	return (getCommandStringParam (command, StdString (paramName), StdString (defaultValue)));
}

bool SystemInterface::getCommandBooleanParam (Json *command, const StdString &paramName, bool defaultValue) {
	Json params;

	if (! command->getObject ("params", &params)) {
		return (defaultValue);
	}
	return (params.getBoolean (paramName, defaultValue));
}

bool SystemInterface::getCommandBooleanParam (Json *command, const char *paramName, bool defaultValue) {
	return (getCommandBooleanParam (command, StdString (paramName), defaultValue));
}

int SystemInterface::getCommandNumberParam (Json *command, const StdString &paramName, const int defaultValue) {
	Json params;

	if (! command->getObject ("params", &params)) {
		return (defaultValue);
	}
	return (params.getNumber (paramName, defaultValue));
}

int SystemInterface::getCommandNumberParam (Json *command, const char *paramName, const int defaultValue) {
	return (getCommandNumberParam (command, StdString (paramName), defaultValue));
}

int64_t SystemInterface::getCommandNumberParam (Json *command, const StdString &paramName, const int64_t defaultValue) {
	Json params;

	if (! command->getObject ("params", &params)) {
		return (defaultValue);
	}
	return (params.getNumber (paramName, defaultValue));
}

int64_t SystemInterface::getCommandNumberParam (Json *command, const char *paramName, const int64_t defaultValue) {
	return (getCommandNumberParam (command, StdString (paramName), defaultValue));
}

double SystemInterface::getCommandNumberParam (Json *command, const StdString &paramName, const double defaultValue) {
	Json params;

	if (! command->getObject ("params", &params)) {
		return (defaultValue);
	}
	return (params.getNumber (paramName, defaultValue));
}

double SystemInterface::getCommandNumberParam (Json *command, const char *paramName, const double defaultValue) {
	return (getCommandNumberParam (command, StdString (paramName), defaultValue));
}

float SystemInterface::getCommandNumberParam (Json *command, const StdString &paramName, const float defaultValue) {
	Json params;

	if (! command->getObject ("params", &params)) {
		return (defaultValue);
	}
	return (params.getNumber (paramName, defaultValue));
}

float SystemInterface::getCommandNumberParam (Json *command, const char *paramName, const float defaultValue) {
	return (getCommandNumberParam (command, StdString (paramName), defaultValue));
}

bool SystemInterface::getCommandObjectParam (Json *command, const StdString &paramName, Json *destJson) {
	Json params;

	if (! command->getObject ("params", &params)) {
		return (false);
	}
	return (params.getObject (paramName, destJson));
}

bool SystemInterface::getCommandObjectParam (Json *command, const char *paramName, Json *destJson) {
	return (getCommandObjectParam (command, StdString (paramName), destJson));
}

bool SystemInterface::getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<int> *destList, bool shouldClear) {
	Json params;
	int i, len;

	if (! command->getObject ("params", &params)) {
		return (false);
	}
	if (! params.isArray (paramName)) {
		return (false);
	}

	if (shouldClear) {
		destList->clear ();
	}
	len = params.getArrayLength (paramName);
	for (i = 0; i < len; ++i) {
		destList->push_back (params.getArrayNumber (paramName, i, (int) 0));
	}
	return (true);
}

bool SystemInterface::getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<int> *destList, bool shouldClear) {
	return (getCommandNumberArrayParam (command, StdString (paramName), destList, shouldClear));
}

bool SystemInterface::getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<int64_t> *destList, bool shouldClear) {
	Json params;
	int i, len;

	if (! command->getObject ("params", &params)) {
		return (false);
	}
	if (! params.isArray (paramName)) {
		return (false);
	}

	if (shouldClear) {
		destList->clear ();
	}
	len = params.getArrayLength (paramName);
	for (i = 0; i < len; ++i) {
		destList->push_back (params.getArrayNumber (paramName, i, (int64_t) 0));
	}
	return (true);
}

bool SystemInterface::getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<int64_t> *destList, bool shouldClear) {
	return (getCommandNumberArrayParam (command, StdString (paramName), destList, shouldClear));
}

bool SystemInterface::getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<double> *destList, bool shouldClear) {
	Json params;
	int i, len;

	if (! command->getObject ("params", &params)) {
		return (false);
	}
	if (! params.isArray (paramName)) {
		return (false);
	}

	if (shouldClear) {
		destList->clear ();
	}
	len = params.getArrayLength (paramName);
	for (i = 0; i < len; ++i) {
		destList->push_back (params.getArrayNumber (paramName, i, (double) 0.0f));
	}
	return (true);
}

bool SystemInterface::getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<double> *destList, bool shouldClear) {
	return (getCommandNumberArrayParam (command, StdString (paramName), destList, shouldClear));
}

bool SystemInterface::getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<float> *destList, bool shouldClear) {
	Json params;
	int i, len;

	if (! command->getObject ("params", &params)) {
		return (false);
	}
	if (! params.isArray (paramName)) {
		return (false);
	}

	if (shouldClear) {
		destList->clear ();
	}
	len = params.getArrayLength (paramName);
	for (i = 0; i < len; ++i) {
		destList->push_back (params.getArrayNumber (paramName, i, (float) 0.0f));
	}
	return (true);
}

bool SystemInterface::getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<float> *destList, bool shouldClear) {
	return (getCommandNumberArrayParam (command, StdString (paramName), destList, shouldClear));
}

int SystemInterface::getCommandArrayLength (Json *command, const StdString &paramName) {
	Json params;

	if (! command->getObject ("params", &params)) {
		return (0);
	}
	return (params.getArrayLength (paramName));
}

int SystemInterface::getCommandArrayLength (Json *command, const char *paramName) {
	return (getCommandArrayLength (command, StdString (paramName)));
}

int SystemInterface::getCommandNumberArrayItem (Json *command, const StdString &paramName, int index, int defaultValue) {
	Json params;

	if (! command->getObject ("params", &params)) {
		return (0);
	}
	return (params.getArrayNumber (paramName, index, defaultValue));
}

int64_t SystemInterface::getCommandNumberArrayItem (Json *command, const StdString &paramName, int index, int64_t defaultValue) {
	Json params;

	if (! command->getObject ("params", &params)) {
		return (0);
	}
	return (params.getArrayNumber (paramName, index, defaultValue));
}

double SystemInterface::getCommandNumberArrayItem (Json *command, const StdString &paramName, int index, double defaultValue) {
	Json params;

	if (! command->getObject ("params", &params)) {
		return (0);
	}
	return (params.getArrayNumber (paramName, index, defaultValue));
}

float SystemInterface::getCommandNumberArrayItem (Json *command, const StdString &paramName, int index, float defaultValue) {
	Json params;

	if (! command->getObject ("params", &params)) {
		return (0);
	}
	return (params.getArrayNumber (paramName, index, defaultValue));
}

StdString SystemInterface::getCommandStringArrayItem (Json *command, const StdString &paramName, int index, const StdString &defaultValue) {
	Json params;

	if (! command->getObject ("params", &params)) {
		return (0);
	}
	return (params.getArrayString (paramName, index, defaultValue));
}

StdString SystemInterface::getCommandStringArrayItem (Json *command, const char *paramName, int index, const StdString &defaultValue) {
	return (getCommandStringArrayItem (command, StdString (paramName), index, defaultValue));
}

bool SystemInterface::getCommandObjectArrayItem (Json *command, const StdString &paramName, int index, Json *destJson) {
	Json params;

	if (! command->getObject ("params", &params)) {
		return (0);
	}
	return (params.getArrayObject (paramName, index, destJson));
}

bool SystemInterface::getCommandObjectArrayItem (Json *command, const char *paramName, int index, Json *destJson) {
	return (getCommandObjectArrayItem (command, StdString (paramName), index, destJson));
}
