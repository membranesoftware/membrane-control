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
#include <stdlib.h>
#include "Config.h"
#include "SystemInterface.h"
const char *SystemInterface::version = "6-stable-bf255788";
const int SystemInterface::Constant_MaxCommandPriority = 100;
const char *SystemInterface::Constant_CreateTimePrefixField = "a";
const char *SystemInterface::Constant_AgentIdPrefixField = "b";
const char *SystemInterface::Constant_UserIdPrefixField = "c";
const char *SystemInterface::Constant_PriorityPrefixField = "d";
const char *SystemInterface::Constant_StartTimePrefixField = "e";
const char *SystemInterface::Constant_DurationPrefixField = "f";
const char *SystemInterface::Constant_AuthorizationHashPrefixField = "g";
const char *SystemInterface::Constant_AuthorizationTokenPrefixField = "h";
const char *SystemInterface::Constant_AuthorizationHashAlgorithm = "sha256";
const char *SystemInterface::Constant_WebSocketEvent = "SystemInterface";
const char *SystemInterface::Constant_UrlQueryParameter = "c";
const int SystemInterface::Constant_DefaultTcpPort1 = 63738;
const int SystemInterface::Constant_DefaultTcpPort2 = 63739;
const int SystemInterface::Constant_DefaultUdpPort = 63738;
const char *SystemInterface::Constant_DefaultInvokePath = "/";
const char *SystemInterface::Constant_DefaultAuthorizePath = "C18HZb3wsXQoMQN6Laz8S5Lq";
const char *SystemInterface::Constant_DefaultLinkPath = "mNODP0RPYCLhTiPGiCifPJA9";
const int SystemInterface::Constant_DefaultCommandType = 0;
const int SystemInterface::Constant_Stream = 1;
const int SystemInterface::Constant_Media = 2;
const int SystemInterface::Constant_Monitor = 3;
const int SystemInterface::Constant_Event = 4;
const int SystemInterface::Constant_Master = 5;
const int SystemInterface::Constant_Admin = 6;
const int SystemInterface::Constant_CommandTypeCount = 7;
void SystemInterface::populate () {
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("AgentConfiguration"), SystemInterface::Command (45, StdString ("AgentConfiguration"), StdString ("AgentConfiguration"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("AgentContact"), SystemInterface::Command (33, StdString ("AgentContact"), StdString ("AgentContact"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("AgentStatus"), SystemInterface::Command (1, StdString ("AgentStatus"), StdString ("AgentStatus"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("Authorize"), SystemInterface::Command (19, StdString ("Authorize"), StdString ("Authorize"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("AuthorizeResult"), SystemInterface::Command (13, StdString ("AuthorizeResult"), StdString ("AuthorizeResult"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CancelTask"), SystemInterface::Command (28, StdString ("CancelTask"), StdString ("CancelTask"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ClearDisplay"), SystemInterface::Command (31, StdString ("ClearDisplay"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CommandResult"), SystemInterface::Command (0, StdString ("CommandResult"), StdString ("CommandResult"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CreateMediaDisplayIntent"), SystemInterface::Command (50, StdString ("CreateMediaDisplayIntent"), StdString ("CreateMediaDisplayIntent"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CreateMediaStream"), SystemInterface::Command (14, StdString ("CreateMediaStream"), StdString ("CreateMediaStream"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("CreateWebDisplayIntent"), SystemInterface::Command (35, StdString ("CreateWebDisplayIntent"), StdString ("CreateWebDisplayIntent"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("EndSet"), SystemInterface::Command (21, StdString ("EndSet"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("EventRecord"), SystemInterface::Command (40, StdString ("EventRecord"), StdString ("EventRecord"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("FindItems"), SystemInterface::Command (3, StdString ("FindItems"), StdString ("FindItems"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("FindMediaResult"), SystemInterface::Command (48, StdString ("FindMediaResult"), StdString ("FindMediaResult"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("FindStreamsResult"), SystemInterface::Command (4, StdString ("FindStreamsResult"), StdString ("FindStreamsResult"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetAgentConfiguration"), SystemInterface::Command (44, StdString ("GetAgentConfiguration"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetHlsHtml5Interface"), SystemInterface::Command (25, StdString ("GetHlsHtml5Interface"), StdString ("GetHlsHtml5Interface"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetHlsManifest"), SystemInterface::Command (23, StdString ("GetHlsManifest"), StdString ("GetHlsManifest"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetHlsSegment"), SystemInterface::Command (24, StdString ("GetHlsSegment"), StdString ("GetHlsSegment"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetMedia"), SystemInterface::Command (15, StdString ("GetMedia"), StdString ("GetMedia"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetStatus"), SystemInterface::Command (8, StdString ("GetStatus"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("GetThumbnailImage"), SystemInterface::Command (5, StdString ("GetThumbnailImage"), StdString ("GetThumbnailImage"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("IntentState"), SystemInterface::Command (36, StdString ("IntentState"), StdString ("IntentState"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("MediaDisplayIntentState"), SystemInterface::Command (51, StdString ("MediaDisplayIntentState"), StdString ("MediaDisplayIntentState"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("MediaItem"), SystemInterface::Command (16, StdString ("MediaItem"), StdString ("MediaItem"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("MediaServerStatus"), SystemInterface::Command (9, StdString ("MediaServerStatus"), StdString ("MediaServerStatus"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("MonitorServerStatus"), SystemInterface::Command (12, StdString ("MonitorServerStatus"), StdString ("MonitorServerStatus"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("PlayMedia"), SystemInterface::Command (30, StdString ("PlayMedia"), StdString ("PlayMedia"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ReadEvents"), SystemInterface::Command (18, StdString ("ReadEvents"), StdString ("ReadEvents"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ReadTasks"), SystemInterface::Command (6, StdString ("ReadTasks"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("RemoveIntent"), SystemInterface::Command (37, StdString ("RemoveIntent"), StdString ("RemoveIntent"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("RemoveStream"), SystemInterface::Command (29, StdString ("RemoveStream"), StdString ("RemoveStream"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ReportContact"), SystemInterface::Command (32, StdString ("ReportContact"), StdString ("ReportContact"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ReportStatus"), SystemInterface::Command (2, StdString ("ReportStatus"), StdString ("ReportStatus"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ServerError"), SystemInterface::Command (20, StdString ("ServerError"), StdString ("ServerError"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("SetIntentActive"), SystemInterface::Command (38, StdString ("SetIntentActive"), StdString ("SetIntentActive"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ShowWebUrl"), SystemInterface::Command (34, StdString ("ShowWebUrl"), StdString ("ShowWebUrl"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("ShutdownAgent"), SystemInterface::Command (43, StdString ("ShutdownAgent"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("StartServers"), SystemInterface::Command (47, StdString ("StartServers"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("StopServers"), SystemInterface::Command (46, StdString ("StopServers"), StdString ("EmptyObject"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("StreamItem"), SystemInterface::Command (22, StdString ("StreamItem"), StdString ("StreamItem"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("StreamServerStatus"), SystemInterface::Command (10, StdString ("StreamServerStatus"), StdString ("StreamServerStatus"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("TaskItem"), SystemInterface::Command (26, StdString ("TaskItem"), StdString ("TaskItem"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("UpdateAgentConfiguration"), SystemInterface::Command (42, StdString ("UpdateAgentConfiguration"), StdString ("UpdateAgentConfiguration"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("UpdateIntentState"), SystemInterface::Command (39, StdString ("UpdateIntentState"), StdString ("UpdateIntentState"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("WatchEvents"), SystemInterface::Command (27, StdString ("WatchEvents"), StdString ("WatchEvents"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("WatchTasks"), SystemInterface::Command (7, StdString ("WatchTasks"), StdString ("WatchTasks"))));
  commandMap.insert (std::pair<StdString, SystemInterface::Command> (StdString ("WebDisplayIntentState"), SystemInterface::Command (49, StdString ("WebDisplayIntentState"), StdString ("WebDisplayIntentState"))));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("AgentConfiguration"), SystemInterface::getParams_AgentConfiguration));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("AgentContact"), SystemInterface::getParams_AgentContact));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("AgentStatus"), SystemInterface::getParams_AgentStatus));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("Authorize"), SystemInterface::getParams_Authorize));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("AuthorizeResult"), SystemInterface::getParams_AuthorizeResult));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CancelTask"), SystemInterface::getParams_CancelTask));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CommandResult"), SystemInterface::getParams_CommandResult));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CreateMediaDisplayIntent"), SystemInterface::getParams_CreateMediaDisplayIntent));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CreateMediaStream"), SystemInterface::getParams_CreateMediaStream));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("CreateWebDisplayIntent"), SystemInterface::getParams_CreateWebDisplayIntent));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("EmptyObject"), SystemInterface::getParams_EmptyObject));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("EventRecord"), SystemInterface::getParams_EventRecord));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("FindItems"), SystemInterface::getParams_FindItems));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("FindMediaResult"), SystemInterface::getParams_FindMediaResult));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("FindStreamsResult"), SystemInterface::getParams_FindStreamsResult));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("GetHlsHtml5Interface"), SystemInterface::getParams_GetHlsHtml5Interface));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("GetHlsManifest"), SystemInterface::getParams_GetHlsManifest));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("GetHlsSegment"), SystemInterface::getParams_GetHlsSegment));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("GetMedia"), SystemInterface::getParams_GetMedia));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("GetThumbnailImage"), SystemInterface::getParams_GetThumbnailImage));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("IntentState"), SystemInterface::getParams_IntentState));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("MediaDisplayIntentState"), SystemInterface::getParams_MediaDisplayIntentState));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("MediaDisplayItem"), SystemInterface::getParams_MediaDisplayItem));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("MediaItem"), SystemInterface::getParams_MediaItem));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("MediaServerConfiguration"), SystemInterface::getParams_MediaServerConfiguration));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("MediaServerStatus"), SystemInterface::getParams_MediaServerStatus));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("MonitorServerConfiguration"), SystemInterface::getParams_MonitorServerConfiguration));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("MonitorServerStatus"), SystemInterface::getParams_MonitorServerStatus));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("PlayMedia"), SystemInterface::getParams_PlayMedia));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("ReadEvents"), SystemInterface::getParams_ReadEvents));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("RemoveIntent"), SystemInterface::getParams_RemoveIntent));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("RemoveStream"), SystemInterface::getParams_RemoveStream));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("ReportContact"), SystemInterface::getParams_ReportContact));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("ReportStatus"), SystemInterface::getParams_ReportStatus));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("ServerError"), SystemInterface::getParams_ServerError));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("SetIntentActive"), SystemInterface::getParams_SetIntentActive));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("ShowWebUrl"), SystemInterface::getParams_ShowWebUrl));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("StreamItem"), SystemInterface::getParams_StreamItem));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("StreamServerConfiguration"), SystemInterface::getParams_StreamServerConfiguration));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("StreamServerStatus"), SystemInterface::getParams_StreamServerStatus));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("TaskItem"), SystemInterface::getParams_TaskItem));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("UpdateAgentConfiguration"), SystemInterface::getParams_UpdateAgentConfiguration));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("UpdateIntentState"), SystemInterface::getParams_UpdateIntentState));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("WatchEvents"), SystemInterface::getParams_WatchEvents));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("WatchTasks"), SystemInterface::getParams_WatchTasks));
  getParamsMap.insert (std::pair<StdString, SystemInterface::GetParamsFunction> (StdString ("WebDisplayIntentState"), SystemInterface::getParams_WebDisplayIntentState));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("AgentConfiguration"), SystemInterface::populateDefaultFields_AgentConfiguration));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("AgentContact"), SystemInterface::populateDefaultFields_AgentContact));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("AgentStatus"), SystemInterface::populateDefaultFields_AgentStatus));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("Authorize"), SystemInterface::populateDefaultFields_Authorize));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("AuthorizeResult"), SystemInterface::populateDefaultFields_AuthorizeResult));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CancelTask"), SystemInterface::populateDefaultFields_CancelTask));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CommandResult"), SystemInterface::populateDefaultFields_CommandResult));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CreateMediaDisplayIntent"), SystemInterface::populateDefaultFields_CreateMediaDisplayIntent));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CreateMediaStream"), SystemInterface::populateDefaultFields_CreateMediaStream));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("CreateWebDisplayIntent"), SystemInterface::populateDefaultFields_CreateWebDisplayIntent));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("EmptyObject"), SystemInterface::populateDefaultFields_EmptyObject));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("EventRecord"), SystemInterface::populateDefaultFields_EventRecord));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("FindItems"), SystemInterface::populateDefaultFields_FindItems));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("FindMediaResult"), SystemInterface::populateDefaultFields_FindMediaResult));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("FindStreamsResult"), SystemInterface::populateDefaultFields_FindStreamsResult));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetHlsHtml5Interface"), SystemInterface::populateDefaultFields_GetHlsHtml5Interface));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetHlsManifest"), SystemInterface::populateDefaultFields_GetHlsManifest));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetHlsSegment"), SystemInterface::populateDefaultFields_GetHlsSegment));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetMedia"), SystemInterface::populateDefaultFields_GetMedia));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("GetThumbnailImage"), SystemInterface::populateDefaultFields_GetThumbnailImage));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("IntentState"), SystemInterface::populateDefaultFields_IntentState));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MediaDisplayIntentState"), SystemInterface::populateDefaultFields_MediaDisplayIntentState));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MediaDisplayItem"), SystemInterface::populateDefaultFields_MediaDisplayItem));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MediaItem"), SystemInterface::populateDefaultFields_MediaItem));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MediaServerConfiguration"), SystemInterface::populateDefaultFields_MediaServerConfiguration));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MediaServerStatus"), SystemInterface::populateDefaultFields_MediaServerStatus));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MonitorServerConfiguration"), SystemInterface::populateDefaultFields_MonitorServerConfiguration));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("MonitorServerStatus"), SystemInterface::populateDefaultFields_MonitorServerStatus));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("PlayMedia"), SystemInterface::populateDefaultFields_PlayMedia));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("ReadEvents"), SystemInterface::populateDefaultFields_ReadEvents));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("RemoveIntent"), SystemInterface::populateDefaultFields_RemoveIntent));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("RemoveStream"), SystemInterface::populateDefaultFields_RemoveStream));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("ReportContact"), SystemInterface::populateDefaultFields_ReportContact));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("ReportStatus"), SystemInterface::populateDefaultFields_ReportStatus));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("ServerError"), SystemInterface::populateDefaultFields_ServerError));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("SetIntentActive"), SystemInterface::populateDefaultFields_SetIntentActive));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("ShowWebUrl"), SystemInterface::populateDefaultFields_ShowWebUrl));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("StreamItem"), SystemInterface::populateDefaultFields_StreamItem));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("StreamServerConfiguration"), SystemInterface::populateDefaultFields_StreamServerConfiguration));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("StreamServerStatus"), SystemInterface::populateDefaultFields_StreamServerStatus));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("TaskItem"), SystemInterface::populateDefaultFields_TaskItem));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("UpdateAgentConfiguration"), SystemInterface::populateDefaultFields_UpdateAgentConfiguration));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("UpdateIntentState"), SystemInterface::populateDefaultFields_UpdateIntentState));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("WatchEvents"), SystemInterface::populateDefaultFields_WatchEvents));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("WatchTasks"), SystemInterface::populateDefaultFields_WatchTasks));
  populateDefaultFieldsMap.insert (std::pair<StdString, SystemInterface::PopulateDefaultFieldsFunction> (StdString ("WebDisplayIntentState"), SystemInterface::populateDefaultFields_WebDisplayIntentState));
}
void SystemInterface::getParams_AgentConfiguration (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("isEnabled"), StdString ("boolean"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("displayName"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("mediaServerConfiguration"), StdString ("MediaServerConfiguration"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("streamServerConfiguration"), StdString ("StreamServerConfiguration"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("monitorServerConfiguration"), StdString ("MonitorServerConfiguration"), StdString (""), 0));
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
  destList->push_back (SystemInterface::Param (StdString ("version"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("nodeVersion"), StdString ("string"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("platform"), StdString ("string"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("isEnabled"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("taskCount"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("runCount"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("maxRunCount"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("mediaServerStatus"), StdString ("MediaServerStatus"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("streamServerStatus"), StdString ("StreamServerStatus"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("monitorServerStatus"), StdString ("MonitorServerStatus"), StdString (""), 0));
}
void SystemInterface::getParams_Authorize (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("token"), StdString ("string"), StdString (""), 3));
}
void SystemInterface::getParams_AuthorizeResult (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("token"), StdString ("string"), StdString (""), 3));
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
}
void SystemInterface::getParams_CreateMediaDisplayIntent (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("displayName"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("items"), StdString ("array"), StdString ("MediaDisplayItem"), 3));
  destList->push_back (SystemInterface::Param (StdString ("isShuffle"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("minItemDisplayDuration"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("maxItemDisplayDuration"), StdString ("number"), StdString (""), 17));
}
void SystemInterface::getParams_CreateMediaStream (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("name"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("mediaServerAgentId"), StdString ("string"), StdString (""), 34));
  destList->push_back (SystemInterface::Param (StdString ("mediaId"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("mediaUrl"), StdString ("string"), StdString (""), 65));
}
void SystemInterface::getParams_CreateWebDisplayIntent (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("displayName"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("urls"), StdString ("array"), StdString ("string"), 3));
  destList->push_back (SystemInterface::Param (StdString ("isShuffle"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("minItemDisplayDuration"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("maxItemDisplayDuration"), StdString ("number"), StdString (""), 17));
}
void SystemInterface::getParams_EmptyObject (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
}
void SystemInterface::getParams_EventRecord (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("record"), StdString ("object"), StdString (""), 257));
}
void SystemInterface::getParams_FindItems (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("searchKey"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("resultOffset"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("maxResults"), StdString ("number"), StdString (""), 17));
}
void SystemInterface::getParams_FindMediaResult (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("searchKey"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("setSize"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("resultOffset"), StdString ("number"), StdString (""), 17));
}
void SystemInterface::getParams_FindStreamsResult (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("searchKey"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("setSize"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("resultOffset"), StdString ("number"), StdString (""), 17));
}
void SystemInterface::getParams_GetHlsHtml5Interface (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("streamId"), StdString ("string"), StdString (""), 35));
}
void SystemInterface::getParams_GetHlsManifest (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("streamId"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("startPosition"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("minStartPositionDelta"), StdString ("number"), 129, (double) 0, (double) 100));
  destList->push_back (SystemInterface::Param (StdString ("maxStartPositionDelta"), StdString ("number"), 129, (double) 0, (double) 100));
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
  destList->push_back (SystemInterface::Param (StdString ("state"), StdString ("object"), StdString (""), 1));
}
void SystemInterface::getParams_MediaDisplayIntentState (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("items"), StdString ("array"), StdString ("MediaDisplayItem"), 3));
  destList->push_back (SystemInterface::Param (StdString ("itemChoices"), StdString ("array"), StdString ("number"), 3));
  destList->push_back (SystemInterface::Param (StdString ("agentMap"), StdString ("object"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("isShuffle"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("minItemDisplayDuration"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("maxItemDisplayDuration"), StdString ("number"), StdString (""), 17));
}
void SystemInterface::getParams_MediaDisplayItem (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("mediaName"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("streamUrl"), StdString ("string"), StdString (""), 67));
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
void SystemInterface::getParams_MonitorServerConfiguration (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
}
void SystemInterface::getParams_MonitorServerStatus (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("isPlaying"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("mediaName"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("isShowingUrl"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("showUrl"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("intentName"), StdString ("string"), StdString (""), 1));
}
void SystemInterface::getParams_PlayMedia (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("mediaName"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("streamUrl"), StdString ("string"), StdString (""), 67));
}
void SystemInterface::getParams_ReadEvents (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
}
void SystemInterface::getParams_RemoveIntent (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 35));
}
void SystemInterface::getParams_RemoveStream (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 35));
}
void SystemInterface::getParams_ReportContact (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("destination"), StdString ("string"), StdString (""), 65));
  destList->push_back (SystemInterface::Param (StdString ("reportCommandType"), StdString ("number"), StdString (""), 1));
}
void SystemInterface::getParams_ReportStatus (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("destination"), StdString ("string"), StdString (""), 65));
  destList->push_back (SystemInterface::Param (StdString ("reportCommandType"), StdString ("number"), StdString (""), 1));
}
void SystemInterface::getParams_ServerError (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("error"), StdString ("string"), StdString (""), 0));
}
void SystemInterface::getParams_SetIntentActive (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("isActive"), StdString ("boolean"), StdString (""), 1));
}
void SystemInterface::getParams_ShowWebUrl (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("url"), StdString ("string"), StdString (""), 67));
}
void SystemInterface::getParams_StreamItem (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("name"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("sourceId"), StdString ("string"), StdString (""), 35));
  destList->push_back (SystemInterface::Param (StdString ("duration"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("width"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("height"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("bitrate"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("frameRate"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("hlsTargetDuration"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("segmentCount"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("segmentFilenames"), StdString ("array"), StdString ("string"), 1));
  destList->push_back (SystemInterface::Param (StdString ("segmentLengths"), StdString ("array"), StdString ("number"), 17));
  destList->push_back (SystemInterface::Param (StdString ("segmentPositions"), StdString ("array"), StdString ("number"), 17));
}
void SystemInterface::getParams_StreamServerConfiguration (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("dataPath"), StdString ("string"), StdString (""), 2));
}
void SystemInterface::getParams_StreamServerStatus (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("isReady"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("streamCount"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("freeSpace"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("totalSpace"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("hlsStreamPath"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("hlsHtml5Path"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("thumbnailPath"), StdString ("string"), StdString (""), 1));
}
void SystemInterface::getParams_TaskItem (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("id"), StdString ("string"), StdString (""), 33));
  destList->push_back (SystemInterface::Param (StdString ("name"), StdString ("string"), StdString (""), 3));
  destList->push_back (SystemInterface::Param (StdString ("subtitle"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("tags"), StdString ("array"), StdString ("string"), 3));
  destList->push_back (SystemInterface::Param (StdString ("description"), StdString ("string"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("isRunning"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("percentComplete"), StdString ("number"), 129, (double) 0, (double) 100));
  destList->push_back (SystemInterface::Param (StdString ("createTime"), StdString ("number"), StdString (""), 9));
  destList->push_back (SystemInterface::Param (StdString ("endTime"), StdString ("number"), StdString (""), 17));
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
void SystemInterface::getParams_WatchEvents (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
}
void SystemInterface::getParams_WatchTasks (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("taskIds"), StdString ("array"), StdString ("string"), 3));
}
void SystemInterface::getParams_WebDisplayIntentState (std::list<SystemInterface::Param> *destList) {
  destList->clear ();
  destList->push_back (SystemInterface::Param (StdString ("urls"), StdString ("array"), StdString ("string"), 3));
  destList->push_back (SystemInterface::Param (StdString ("urlChoices"), StdString ("array"), StdString ("number"), 3));
  destList->push_back (SystemInterface::Param (StdString ("agentMap"), StdString ("object"), StdString (""), 0));
  destList->push_back (SystemInterface::Param (StdString ("isShuffle"), StdString ("boolean"), StdString (""), 1));
  destList->push_back (SystemInterface::Param (StdString ("minItemDisplayDuration"), StdString ("number"), StdString (""), 17));
  destList->push_back (SystemInterface::Param (StdString ("maxItemDisplayDuration"), StdString ("number"), StdString (""), 17));
}
void SystemInterface::populateDefaultFields_AgentConfiguration (Json *destObject) {
}
void SystemInterface::populateDefaultFields_AgentContact (Json *destObject) {
  if (! destObject->exists ("nodeVersion")) {
    destObject->set ("nodeVersion", "");
  }
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
void SystemInterface::populateDefaultFields_Authorize (Json *destObject) {
}
void SystemInterface::populateDefaultFields_AuthorizeResult (Json *destObject) {
}
void SystemInterface::populateDefaultFields_CancelTask (Json *destObject) {
}
void SystemInterface::populateDefaultFields_CommandResult (Json *destObject) {
}
void SystemInterface::populateDefaultFields_CreateMediaDisplayIntent (Json *destObject) {
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
void SystemInterface::populateDefaultFields_EventRecord (Json *destObject) {
}
void SystemInterface::populateDefaultFields_FindItems (Json *destObject) {
  if (! destObject->exists ("searchKey")) {
    destObject->set ("searchKey", "*");
  }
  if (! destObject->exists ("resultOffset")) {
    destObject->set ("resultOffset", 0);
  }
  if (! destObject->exists ("maxResults")) {
    destObject->set ("maxResults", 0);
  }
}
void SystemInterface::populateDefaultFields_FindMediaResult (Json *destObject) {
  if (! destObject->exists ("setSize")) {
    destObject->set ("setSize", 0);
  }
  if (! destObject->exists ("resultOffset")) {
    destObject->set ("resultOffset", 0);
  }
}
void SystemInterface::populateDefaultFields_FindStreamsResult (Json *destObject) {
  if (! destObject->exists ("setSize")) {
    destObject->set ("setSize", 0);
  }
  if (! destObject->exists ("resultOffset")) {
    destObject->set ("resultOffset", 0);
  }
}
void SystemInterface::populateDefaultFields_GetHlsHtml5Interface (Json *destObject) {
}
void SystemInterface::populateDefaultFields_GetHlsManifest (Json *destObject) {
  if (! destObject->exists ("startPosition")) {
    destObject->set ("startPosition", 0);
  }
  if (! destObject->exists ("minStartPositionDelta")) {
    destObject->set ("minStartPositionDelta", 0);
  }
  if (! destObject->exists ("maxStartPositionDelta")) {
    destObject->set ("maxStartPositionDelta", 0);
  }
}
void SystemInterface::populateDefaultFields_GetHlsSegment (Json *destObject) {
  if (! destObject->exists ("segmentIndex")) {
    destObject->set ("segmentIndex", 0);
  }
}
void SystemInterface::populateDefaultFields_GetMedia (Json *destObject) {
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
}
void SystemInterface::populateDefaultFields_MediaItem (Json *destObject) {
  if (! destObject->exists ("mtime")) {
    destObject->set ("mtime", 0);
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
void SystemInterface::populateDefaultFields_MonitorServerConfiguration (Json *destObject) {
}
void SystemInterface::populateDefaultFields_MonitorServerStatus (Json *destObject) {
  if (! destObject->exists ("mediaName")) {
    destObject->set ("mediaName", "");
  }
  if (! destObject->exists ("showUrl")) {
    destObject->set ("showUrl", "");
  }
  if (! destObject->exists ("intentName")) {
    destObject->set ("intentName", "");
  }
}
void SystemInterface::populateDefaultFields_PlayMedia (Json *destObject) {
  if (! destObject->exists ("mediaName")) {
    destObject->set ("mediaName", "");
  }
}
void SystemInterface::populateDefaultFields_ReadEvents (Json *destObject) {
}
void SystemInterface::populateDefaultFields_RemoveIntent (Json *destObject) {
}
void SystemInterface::populateDefaultFields_RemoveStream (Json *destObject) {
}
void SystemInterface::populateDefaultFields_ReportContact (Json *destObject) {
  if (! destObject->exists ("reportCommandType")) {
    destObject->set ("reportCommandType", 0);
  }
}
void SystemInterface::populateDefaultFields_ReportStatus (Json *destObject) {
  if (! destObject->exists ("reportCommandType")) {
    destObject->set ("reportCommandType", 0);
  }
}
void SystemInterface::populateDefaultFields_ServerError (Json *destObject) {
  if (! destObject->exists ("error")) {
    destObject->set ("error", "");
  }
}
void SystemInterface::populateDefaultFields_SetIntentActive (Json *destObject) {
}
void SystemInterface::populateDefaultFields_ShowWebUrl (Json *destObject) {
}
void SystemInterface::populateDefaultFields_StreamItem (Json *destObject) {
}
void SystemInterface::populateDefaultFields_StreamServerConfiguration (Json *destObject) {
}
void SystemInterface::populateDefaultFields_StreamServerStatus (Json *destObject) {
}
void SystemInterface::populateDefaultFields_TaskItem (Json *destObject) {
  if (! destObject->exists ("subtitle")) {
    destObject->set ("subtitle", "");
  }
  if (! destObject->exists ("description")) {
    destObject->set ("description", "");
  }
  if (! destObject->exists ("percentComplete")) {
    destObject->set ("percentComplete", 0);
  }
}
void SystemInterface::populateDefaultFields_UpdateAgentConfiguration (Json *destObject) {
}
void SystemInterface::populateDefaultFields_UpdateIntentState (Json *destObject) {
  if (! destObject->exists ("isReplace")) {
    destObject->set ("isReplace", false);
  }
}
void SystemInterface::populateDefaultFields_WatchEvents (Json *destObject) {
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
SystemInterface::SystemInterface ()
: lastError ("")
{
	populate ();
}

SystemInterface::~SystemInterface () {

}

Json *SystemInterface::createCommand (const SystemInterface::Prefix &prefix, const char *commandName, int commandType, Json *commandParams) {
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
	cmd->set ("commandType", commandType);

	cmdprefix = new Json ();
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
	int result;

	json = new Json ();
	result = json->parse (commandString.c_str (), commandString.length ());
	if (result != Result::SUCCESS) {
		lastError.sprintf ("JSON parse failed, %i", result);
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

StdString SystemInterface::getCommandAgentId (Json *command) {
	Json prefix;

	if (! command->getObject ("prefix", &prefix)) {
		return (StdString (""));
	}

	return (prefix.getString (SystemInterface::Constant_AgentIdPrefixField, ""));
}

StdString SystemInterface::getCommandAgentName (Json *command) {
	StdString name;

	if (getCommandId (command) != SystemInterface::Command_AgentStatus) {
		return (StdString (""));
	}

	name = getCommandStringParam (command, "displayName", "");
	if (! name.empty ()) {
		return (name);
	}

	name = getCommandStringParam (command, "urlHostname", "");
	if (! name.empty ()) {
		return (name);
	}

	name = getCommandAgentId (command);
	if (! name.empty ()) {
		return (name);
	}

	return (StdString (""));
}

StdString SystemInterface::getCommandAgentAddress (Json *command) {
	StdString hostname;
	int port;

	if (getCommandId (command) != SystemInterface::Command_AgentStatus) {
		return (StdString (""));
	}

	hostname = getCommandStringParam (command, "urlHostname", "");
	if (hostname.empty ()) {
		return (StdString (""));
	}

	port = getCommandNumberParam (command, "tcpPort1", SystemInterface::Constant_DefaultTcpPort1);
	if ((port < 0) || (port > 65535)) {
		return (StdString (""));
	}

	return (StdString::createSprintf ("%s:%i", hostname.c_str (), port));
}

StdString SystemInterface::getCommandRecordId (Json *command) {
	return (getCommandStringParam (command, "id", ""));
}

bool SystemInterface::isRecordClosed (Json *command) {
	Json prefix;

	if (! command->getObject ("prefix", &prefix)) {
		return (false);
	}

	return (prefix.getBoolean ("isRecordClosed", false));
}

bool SystemInterface::isWindowsPlatform (const StdString &platform) {
	return (platform.equals ("win32") || platform.equals ("win64"));
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

bool SystemInterface::getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<double> *destList, bool shouldClear) {
	Json params;
	int i, len;

	if (! command->getObject ("params", &params)) {
		return (false);
	}

	if (shouldClear) {
		destList->clear ();
	}
	len = params.getArrayLength (paramName);
	for (i = 0; i < len; ++i) {
		destList->push_back (params.getArrayNumber (paramName, i, (double) 0));
	}

	return (true);
}

bool SystemInterface::getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<double> *destList, bool shouldClear) {
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
