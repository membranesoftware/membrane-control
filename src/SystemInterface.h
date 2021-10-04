/*
* Copyright 2018-2021 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
#ifndef SYSTEM_INTERFACE_H
#define SYSTEM_INTERFACE_H

#include <stdint.h>
#include <map>
#include <list>
#include <vector>
#include "StdString.h"
#include "Json.h"

class SystemInterface {
public:
  static const char *version;
  static const char *Command_AgentConfiguration;
  static const char *Command_AgentContact;
  static const char *Command_AgentStatus;
  static const char *Command_ApplicationNews;
  static const char *Command_AuthorizationRequired;
  static const char *Command_Authorize;
  static const char *Command_AuthorizeResult;
  static const char *Command_CameraDisplayIntentState;
  static const char *Command_CameraServerStatus;
  static const char *Command_CancelTask;
  static const char *Command_ClearCache;
  static const char *Command_ClearDisplay;
  static const char *Command_ClearTimelapse;
  static const char *Command_CommandResult;
  static const char *Command_ConfigureCamera;
  static const char *Command_ConfigureMediaStream;
  static const char *Command_CreateCacheStream;
  static const char *Command_CreateCameraDisplayIntent;
  static const char *Command_CreateMediaDisplayIntent;
  static const char *Command_CreateMediaStream;
  static const char *Command_CreateMonitorProgram;
  static const char *Command_CreateStreamCacheDisplayIntent;
  static const char *Command_CreateTimelapseCaptureIntent;
  static const char *Command_CreateWebDisplayIntent;
  static const char *Command_EndSet;
  static const char *Command_FindCaptureImages;
  static const char *Command_FindCaptureImagesResult;
  static const char *Command_FindMediaItems;
  static const char *Command_FindMediaItemsResult;
  static const char *Command_FindMediaStreams;
  static const char *Command_FindMediaStreamsResult;
  static const char *Command_FindStreamItems;
  static const char *Command_FindStreamItemsResult;
  static const char *Command_GetAgentConfiguration;
  static const char *Command_GetCameraStream;
  static const char *Command_GetCameraStreamResult;
  static const char *Command_GetCaptureImage;
  static const char *Command_GetDashMpd;
  static const char *Command_GetDashSegment;
  static const char *Command_GetHlsManifest;
  static const char *Command_GetHlsSegment;
  static const char *Command_GetMedia;
  static const char *Command_GetStatus;
  static const char *Command_GetStreamItem;
  static const char *Command_GetThumbnailImage;
  static const char *Command_IntentState;
  static const char *Command_LinkSuccess;
  static const char *Command_MediaDisplayIntentState;
  static const char *Command_MediaItem;
  static const char *Command_MediaServerStatus;
  static const char *Command_MonitorServerStatus;
  static const char *Command_PauseMedia;
  static const char *Command_PlayCacheStream;
  static const char *Command_PlayCameraStream;
  static const char *Command_PlayMedia;
  static const char *Command_ReadTasks;
  static const char *Command_RemoveIntent;
  static const char *Command_RemoveMedia;
  static const char *Command_RemoveStream;
  static const char *Command_ReportContact;
  static const char *Command_ReportStatus;
  static const char *Command_ScanMediaItems;
  static const char *Command_ServerError;
  static const char *Command_SetAdminSecret;
  static const char *Command_SetIntentActive;
  static const char *Command_ShowAgentStatus;
  static const char *Command_ShowCameraImage;
  static const char *Command_ShowDesktopCountdown;
  static const char *Command_ShowWebUrl;
  static const char *Command_ShutdownAgent;
  static const char *Command_StartServers;
  static const char *Command_StopCapture;
  static const char *Command_StopServers;
  static const char *Command_StreamCacheDisplayIntentState;
  static const char *Command_StreamItem;
  static const char *Command_StreamServerStatus;
  static const char *Command_TaskItem;
  static const char *Command_TimelapseCaptureIntentState;
  static const char *Command_UpdateAgentConfiguration;
  static const char *Command_UpdateIntentState;
  static const char *Command_WatchStatus;
  static const char *Command_WatchTasks;
  static const char *Command_WebDisplayIntentState;
  static const int CommandId_AgentConfiguration = 45;
  static const int CommandId_AgentContact = 33;
  static const int CommandId_AgentStatus = 1;
  static const int CommandId_ApplicationNews = 64;
  static const int CommandId_AuthorizationRequired = 62;
  static const int CommandId_Authorize = 19;
  static const int CommandId_AuthorizeResult = 13;
  static const int CommandId_CameraDisplayIntentState = 108;
  static const int CommandId_CameraServerStatus = 69;
  static const int CommandId_CancelTask = 28;
  static const int CommandId_ClearCache = 59;
  static const int CommandId_ClearDisplay = 31;
  static const int CommandId_ClearTimelapse = 76;
  static const int CommandId_CommandResult = 0;
  static const int CommandId_ConfigureCamera = 109;
  static const int CommandId_ConfigureMediaStream = 65;
  static const int CommandId_CreateCacheStream = 60;
  static const int CommandId_CreateCameraDisplayIntent = 107;
  static const int CommandId_CreateMediaDisplayIntent = 50;
  static const int CommandId_CreateMediaStream = 14;
  static const int CommandId_CreateMonitorProgram = 102;
  static const int CommandId_CreateStreamCacheDisplayIntent = 103;
  static const int CommandId_CreateTimelapseCaptureIntent = 70;
  static const int CommandId_CreateWebDisplayIntent = 35;
  static const int CommandId_EndSet = 21;
  static const int CommandId_FindCaptureImages = 74;
  static const int CommandId_FindCaptureImagesResult = 75;
  static const int CommandId_FindMediaItems = 3;
  static const int CommandId_FindMediaItemsResult = 48;
  static const int CommandId_FindMediaStreams = 78;
  static const int CommandId_FindMediaStreamsResult = 79;
  static const int CommandId_FindStreamItems = 211;
  static const int CommandId_FindStreamItemsResult = 4;
  static const int CommandId_GetAgentConfiguration = 44;
  static const int CommandId_GetCameraStream = 100;
  static const int CommandId_GetCameraStreamResult = 101;
  static const int CommandId_GetCaptureImage = 73;
  static const int CommandId_GetDashMpd = 67;
  static const int CommandId_GetDashSegment = 68;
  static const int CommandId_GetHlsManifest = 23;
  static const int CommandId_GetHlsSegment = 24;
  static const int CommandId_GetMedia = 15;
  static const int CommandId_GetStatus = 8;
  static const int CommandId_GetStreamItem = 25;
  static const int CommandId_GetThumbnailImage = 5;
  static const int CommandId_IntentState = 36;
  static const int CommandId_LinkSuccess = 63;
  static const int CommandId_MediaDisplayIntentState = 51;
  static const int CommandId_MediaItem = 16;
  static const int CommandId_MediaServerStatus = 9;
  static const int CommandId_MonitorServerStatus = 12;
  static const int CommandId_PauseMedia = 98;
  static const int CommandId_PlayCacheStream = 57;
  static const int CommandId_PlayCameraStream = 99;
  static const int CommandId_PlayMedia = 30;
  static const int CommandId_ReadTasks = 6;
  static const int CommandId_RemoveIntent = 37;
  static const int CommandId_RemoveMedia = 77;
  static const int CommandId_RemoveStream = 29;
  static const int CommandId_ReportContact = 32;
  static const int CommandId_ReportStatus = 2;
  static const int CommandId_ScanMediaItems = 58;
  static const int CommandId_ServerError = 20;
  static const int CommandId_SetAdminSecret = 61;
  static const int CommandId_SetIntentActive = 38;
  static const int CommandId_ShowAgentStatus = 41;
  static const int CommandId_ShowCameraImage = 105;
  static const int CommandId_ShowDesktopCountdown = 218;
  static const int CommandId_ShowWebUrl = 34;
  static const int CommandId_ShutdownAgent = 43;
  static const int CommandId_StartServers = 47;
  static const int CommandId_StopCapture = 72;
  static const int CommandId_StopServers = 46;
  static const int CommandId_StreamCacheDisplayIntentState = 104;
  static const int CommandId_StreamItem = 22;
  static const int CommandId_StreamServerStatus = 10;
  static const int CommandId_TaskItem = 26;
  static const int CommandId_TimelapseCaptureIntentState = 71;
  static const int CommandId_UpdateAgentConfiguration = 42;
  static const int CommandId_UpdateIntentState = 39;
  static const int CommandId_WatchStatus = 82;
  static const int CommandId_WatchTasks = 7;
  static const int CommandId_WebDisplayIntentState = 49;
  static const int ParamFlag_Required = 1;
  static const int ParamFlag_NotEmpty = 2;
  static const int ParamFlag_Hostname = 4;
  static const int ParamFlag_GreaterThanZero = 8;
  static const int ParamFlag_ZeroOrGreater = 16;
  static const int ParamFlag_Uuid = 32;
  static const int ParamFlag_Url = 64;
  static const int ParamFlag_RangedNumber = 128;
  static const int ParamFlag_Command = 256;
  static const int ParamFlag_EnumValue = 512;
  static const char *Constant_AgentIdPrefixField;
  static const char *Constant_AuthorizationHashAlgorithm;
  static const char *Constant_AuthorizationHashPrefixField;
  static const char *Constant_AuthorizationTokenPrefixField;
  static const int Constant_CompressedStreamProfile = 1;
  static const char *Constant_CreateTimePrefixField;
  static const char *Constant_DefaultAuthorizePath;
  static const int Constant_DefaultDisplayState = 0;
  static const int Constant_DefaultImageProfile = 0;
  static const char *Constant_DefaultInvokePath;
  static const char *Constant_DefaultLinkPath;
  static const int Constant_DefaultSortOrder = 0;
  static const int Constant_DefaultStreamProfile = 0;
  static const int Constant_DefaultTcpPort1 = 63738;
  static const int Constant_DefaultTcpPort2 = 63739;
  static const int Constant_DefaultUdpPort = 63738;
  static const char *Constant_DisplayCondition;
  static const char *Constant_DurationPrefixField;
  static const int Constant_FastPreviewStreamProfile = 10;
  static const int Constant_HighBitrateStreamProfile = 4;
  static const int Constant_HighQualityImageProfile = 1;
  static const int Constant_HorizontalAndVerticalFlip = 3;
  static const int Constant_HorizontalFlip = 1;
  static const int Constant_LowBitrateStreamProfile = 6;
  static const int Constant_LowQualityImageProfile = 2;
  static const int Constant_LowQualityStreamProfile = 2;
  static const int Constant_LowestBitrateStreamProfile = 7;
  static const int Constant_LowestQualityImageProfile = 3;
  static const int Constant_LowestQualityStreamProfile = 3;
  static const int Constant_MaxCommandPriority = 100;
  static const int Constant_MediumBitrateStreamProfile = 5;
  static const int Constant_NameSort = 0;
  static const int Constant_NewestSort = 1;
  static const int Constant_NoFlip = 0;
  static const int Constant_PlayCameraStreamDisplayState = 4;
  static const int Constant_PlayMediaDisplayState = 2;
  static const int Constant_PreviewStreamProfile = 9;
  static const char *Constant_PriorityPrefixField;
  static const int Constant_ShowImageDisplayState = 3;
  static const int Constant_ShowUrlDisplayState = 1;
  static const int Constant_SourceMatchStreamProfile = 8;
  static const char *Constant_StartTimePrefixField;
  static const char *Constant_UrlQueryParameter;
  static const char *Constant_UserIdPrefixField;
  static const int Constant_VerticalFlip = 2;
  static const char *Constant_WebSocketEvent;
  void populate ();
	SystemInterface ();
	~SystemInterface ();
	static SystemInterface *instance;

	struct Prefix {
		StdString agentId;
		StdString userId;
		int priority;
		int64_t createTime;
		int64_t startTime;
		int64_t duration;
		Prefix (): priority (0), createTime (0), startTime (0), duration (0) { }
	};

	struct Param {
		StdString name;
		StdString type;
		StdString containerType;
		int flags;
		double rangeMin, rangeMax;
		Param (): name (""), type (""), containerType (""), flags (0), rangeMin (0.0f), rangeMax (0.0f) { }
		Param (const StdString &name, const StdString &type, const StdString &containerType, int flags): name (name), type (type), containerType (containerType), flags (flags), rangeMin (0.0f), rangeMax (0.0f) { }
		Param (const StdString &name, const StdString &type, int flags, double rangeMin, double rangeMax): name (name), type (type), containerType (""), flags (flags), rangeMin (rangeMin), rangeMax (rangeMax) { }
	};

	struct Command {
		int id;
		StdString name;
		StdString paramType;
		Command (): id (0), name (""), paramType ("") { }
		Command (int id, const StdString &name, const StdString &paramType): id (id), name (name), paramType (paramType) { }
	};

	typedef void (*GetParamsFunction) (std::list<SystemInterface::Param> *destList);
	typedef void (*PopulateDefaultFieldsFunction) (Json *destObject);
	typedef void (*HashUpdateFunction) (void *contextPtr, unsigned char *data, int dataLength);
	typedef StdString (*HashDigestFunction) (void *contextPtr);
	typedef void (*HashFieldsFunction) (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);

	StdString lastError;
	std::map<StdString, SystemInterface::Command> commandMap;
	std::map<StdString, SystemInterface::GetParamsFunction> getParamsMap;
	std::map<StdString, SystemInterface::PopulateDefaultFieldsFunction> populateDefaultFieldsMap;
	std::map<StdString, SystemInterface::HashFieldsFunction> hashFieldsMap;

	// Return a newly created Json object containing a command item, or NULL if the command could not be created. commandParams can be NULL if not needed, causing the resulting command to contain empty parameter fields. If commandParams is not NULL, this method becomes responsible for freeing the object when it's no longer needed.
	Json *createCommand (const SystemInterface::Prefix &prefix, const char *commandName, Json *commandParams = NULL);

	// Populate a command's authorization prefix field using the provided values and hash functions. Returns a boolean value indicating if the field was successfully generated.
	bool setCommandAuthorization (Json *command, const StdString &authSecret, const StdString &authToken, SystemInterface::HashUpdateFunction hashUpdateFn, SystemInterface::HashDigestFunction hashDigestFn, void *hashContextPtr);

	// Return the authorization hash generated from the provided values and functions. If authToken is not provided, any available prefix auth token is used.
	StdString getCommandAuthorizationHash (Json *command, const StdString &authSecret, const StdString &authToken, SystemInterface::HashUpdateFunction hashUpdateFn, SystemInterface::HashDigestFunction hashDigestFn, void *hashContextPtr);

	// Find command data for the specified name and store fields into the provided struct. Returns a boolean value indicating if the command was found.
	bool getCommand (const StdString &name, SystemInterface::Command *command);

	// Find type data for the specified name and store Param structs into the provided list. Returns a boolean value indicating if the type was found.
	bool getType (const StdString &name, std::list<SystemInterface::Param> *destList);

	// Populate default fields in an object, as appropriate for the specified type name. Returns a boolean value indicating if the type was found.
	bool populateDefaultFields (const StdString &typeName, Json *destObject);

	// Update a hash digest using fields in an object, as appropriate for the specified type name
	void hashFields (const StdString &typeName, Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);

	// Return a boolean value indicating if the provided fields are valid according to rules appearing in a Param list. If the fields are found to be invalid, this method sets the lastError value.
	bool fieldsValid (Json *fields, std::list<SystemInterface::Param> *paramList);

	// Parse a command JSON string and store the resulting Json object using the provided pointer. Returns a boolean value indicating if the parse was successful. If the parse fails, this method sets the lastError value.
	bool parseCommand (const StdString &commandString, Json **commandJson);

	// Return the command ID value appearing in the provided command object, or -1 if no such ID was found
	int getCommandId (Json *command);

	// Return the command name value appearing in the provided command object, or an empty string if no such name was found
	StdString getCommandName (Json *command);

	// Return the prefix.agentId value appearing in the provided command object, or an empty string if no such value was found
	StdString getCommandAgentId (Json *command);

	// Return the params.id value appearing in the provided command object, or an empty string if no such value was found
	StdString getCommandRecordId (Json *command);

	// Return a boolean value indicating if the provided string matches a Windows platform identifier
	bool isWindowsPlatform (const StdString &platform);

	// Return a SystemInterface::Prefix structure containing prefix fields from the provided command
	SystemInterface::Prefix getCommandPrefix (Json *command);

	// Get the provided command's params object and store it using the provided Json pointer. Returns a boolean value indicating if the params object was found.
	bool getCommandParams (Json *command, Json *params);

	// Return a string value from params in the provided command, or the default value if the named field wasn't found
	StdString getCommandStringParam (Json *command, const StdString &paramName, const StdString &defaultValue);
	StdString getCommandStringParam (Json *command, const char *paramName, const char *defaultValue);

	// Return a bool value from params in the provided command, or the default value if the named field wasn't found
	bool getCommandBooleanParam (Json *command, const StdString &paramName, bool defaultValue);
	bool getCommandBooleanParam (Json *command, const char *paramName, bool defaultValue);

	// Return an int number value from params in the provided command, or the default value if the named field wasn't found
	int getCommandNumberParam (Json *command, const StdString &paramName, const int defaultValue);
	int getCommandNumberParam (Json *command, const char *paramName, const int defaultValue);
	int64_t getCommandNumberParam (Json *command, const StdString &paramName, const int64_t defaultValue);
	int64_t getCommandNumberParam (Json *command, const char *paramName, const int64_t defaultValue);
	double getCommandNumberParam (Json *command, const StdString &paramName, const double defaultValue);
	double getCommandNumberParam (Json *command, const char *paramName, const double defaultValue);
	float getCommandNumberParam (Json *command, const StdString &paramName, const float defaultValue);
	float getCommandNumberParam (Json *command, const char *paramName, const float defaultValue);

	// Find the specified object item and store it in the provided Json object. Returns a boolean value indicating if the item was found.
	bool getCommandObjectParam (Json *command, const StdString &paramName, Json *destJson);
	bool getCommandObjectParam (Json *command, const char *paramName, Json *destJson);

	// Fill the provided vector with items from the specified number array, optionally clearing the list before doing so. Returns a boolean value indicating if the array was found.
	bool getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<int> *destList, bool shouldClear = false);
	bool getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<int> *destList, bool shouldClear = false);
	bool getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<int64_t> *destList, bool shouldClear = false);
	bool getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<int64_t> *destList, bool shouldClear = false);
	bool getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<double> *destList, bool shouldClear = false);
	bool getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<double> *destList, bool shouldClear = false);
	bool getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<float> *destList, bool shouldClear = false);
	bool getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<float> *destList, bool shouldClear = false);

	// Return the length of the specified array, or 0 if the array was empty or non-existent
	int getCommandArrayLength (Json *command, const StdString &paramName);
	int getCommandArrayLength (Json *command, const char *paramName);

	// Return the specified number array item, or the default value if the item wasn't found
	int getCommandNumberArrayItem (Json *command, const StdString &paramName, int index, int defaultValue);
	int64_t getCommandNumberArrayItem (Json *command, const StdString &paramName, int index, int64_t defaultValue);
	double getCommandNumberArrayItem (Json *command, const StdString &paramName, int index, double defaultValue);
	float getCommandNumberArrayItem (Json *command, const StdString &paramName, int index, float defaultValue);

	// Return the specified string array item, or the default value if the item wasn't found
	StdString getCommandStringArrayItem (Json *command, const StdString &paramName, int index, const StdString &defaultValue);
	StdString getCommandStringArrayItem (Json *command, const char *paramName, int index, const StdString &defaultValue);

	// Find the specified object array item and store it in the provided Json object. Returns a boolean value indicating if the item was found.
	bool getCommandObjectArrayItem (Json *command, const StdString &paramName, int index, Json *destJson);
	bool getCommandObjectArrayItem (Json *command, const char *paramName, int index, Json *destJson);

  static void getParams_AgentConfiguration (std::list<SystemInterface::Param> *destList);
  static void getParams_AgentContact (std::list<SystemInterface::Param> *destList);
  static void getParams_AgentHost (std::list<SystemInterface::Param> *destList);
  static void getParams_AgentStatus (std::list<SystemInterface::Param> *destList);
  static void getParams_ApplicationNews (std::list<SystemInterface::Param> *destList);
  static void getParams_ApplicationNewsItem (std::list<SystemInterface::Param> *destList);
  static void getParams_Authorize (std::list<SystemInterface::Param> *destList);
  static void getParams_AuthorizeResult (std::list<SystemInterface::Param> *destList);
  static void getParams_CameraDisplayIntentState (std::list<SystemInterface::Param> *destList);
  static void getParams_CameraServerConfiguration (std::list<SystemInterface::Param> *destList);
  static void getParams_CameraServerStatus (std::list<SystemInterface::Param> *destList);
  static void getParams_CancelTask (std::list<SystemInterface::Param> *destList);
  static void getParams_CommandResult (std::list<SystemInterface::Param> *destList);
  static void getParams_ConfigureCamera (std::list<SystemInterface::Param> *destList);
  static void getParams_ConfigureMediaStream (std::list<SystemInterface::Param> *destList);
  static void getParams_CreateCacheStream (std::list<SystemInterface::Param> *destList);
  static void getParams_CreateCameraDisplayIntent (std::list<SystemInterface::Param> *destList);
  static void getParams_CreateMediaDisplayIntent (std::list<SystemInterface::Param> *destList);
  static void getParams_CreateMediaStream (std::list<SystemInterface::Param> *destList);
  static void getParams_CreateMonitorProgram (std::list<SystemInterface::Param> *destList);
  static void getParams_CreateStreamCacheDisplayIntent (std::list<SystemInterface::Param> *destList);
  static void getParams_CreateTimelapseCaptureIntent (std::list<SystemInterface::Param> *destList);
  static void getParams_CreateWebDisplayIntent (std::list<SystemInterface::Param> *destList);
  static void getParams_EmptyObject (std::list<SystemInterface::Param> *destList);
  static void getParams_FindCaptureImages (std::list<SystemInterface::Param> *destList);
  static void getParams_FindCaptureImagesResult (std::list<SystemInterface::Param> *destList);
  static void getParams_FindMediaItems (std::list<SystemInterface::Param> *destList);
  static void getParams_FindMediaItemsResult (std::list<SystemInterface::Param> *destList);
  static void getParams_FindMediaStreams (std::list<SystemInterface::Param> *destList);
  static void getParams_FindMediaStreamsResult (std::list<SystemInterface::Param> *destList);
  static void getParams_FindStreamItems (std::list<SystemInterface::Param> *destList);
  static void getParams_FindStreamItemsResult (std::list<SystemInterface::Param> *destList);
  static void getParams_GetCameraStream (std::list<SystemInterface::Param> *destList);
  static void getParams_GetCameraStreamResult (std::list<SystemInterface::Param> *destList);
  static void getParams_GetCaptureImage (std::list<SystemInterface::Param> *destList);
  static void getParams_GetDashMpd (std::list<SystemInterface::Param> *destList);
  static void getParams_GetDashSegment (std::list<SystemInterface::Param> *destList);
  static void getParams_GetHlsManifest (std::list<SystemInterface::Param> *destList);
  static void getParams_GetHlsSegment (std::list<SystemInterface::Param> *destList);
  static void getParams_GetMedia (std::list<SystemInterface::Param> *destList);
  static void getParams_GetStreamItem (std::list<SystemInterface::Param> *destList);
  static void getParams_GetThumbnailImage (std::list<SystemInterface::Param> *destList);
  static void getParams_IntentState (std::list<SystemInterface::Param> *destList);
  static void getParams_MediaDisplayIntentState (std::list<SystemInterface::Param> *destList);
  static void getParams_MediaDisplayItem (std::list<SystemInterface::Param> *destList);
  static void getParams_MediaItem (std::list<SystemInterface::Param> *destList);
  static void getParams_MediaServerConfiguration (std::list<SystemInterface::Param> *destList);
  static void getParams_MediaServerStatus (std::list<SystemInterface::Param> *destList);
  static void getParams_MonitorProgramCondition (std::list<SystemInterface::Param> *destList);
  static void getParams_MonitorProgramDirective (std::list<SystemInterface::Param> *destList);
  static void getParams_MonitorProgramTimeCondition (std::list<SystemInterface::Param> *destList);
  static void getParams_MonitorServerConfiguration (std::list<SystemInterface::Param> *destList);
  static void getParams_MonitorServerStatus (std::list<SystemInterface::Param> *destList);
  static void getParams_PlayCacheStream (std::list<SystemInterface::Param> *destList);
  static void getParams_PlayCameraStream (std::list<SystemInterface::Param> *destList);
  static void getParams_PlayMedia (std::list<SystemInterface::Param> *destList);
  static void getParams_RemoveIntent (std::list<SystemInterface::Param> *destList);
  static void getParams_RemoveMedia (std::list<SystemInterface::Param> *destList);
  static void getParams_RemoveStream (std::list<SystemInterface::Param> *destList);
  static void getParams_ReportContact (std::list<SystemInterface::Param> *destList);
  static void getParams_ReportStatus (std::list<SystemInterface::Param> *destList);
  static void getParams_ServerError (std::list<SystemInterface::Param> *destList);
  static void getParams_SetAdminSecret (std::list<SystemInterface::Param> *destList);
  static void getParams_SetIntentActive (std::list<SystemInterface::Param> *destList);
  static void getParams_ShowAgentStatus (std::list<SystemInterface::Param> *destList);
  static void getParams_ShowCameraImage (std::list<SystemInterface::Param> *destList);
  static void getParams_ShowDesktopCountdown (std::list<SystemInterface::Param> *destList);
  static void getParams_ShowWebUrl (std::list<SystemInterface::Param> *destList);
  static void getParams_StreamCacheDisplayIntentState (std::list<SystemInterface::Param> *destList);
  static void getParams_StreamItem (std::list<SystemInterface::Param> *destList);
  static void getParams_StreamServerConfiguration (std::list<SystemInterface::Param> *destList);
  static void getParams_StreamServerStatus (std::list<SystemInterface::Param> *destList);
  static void getParams_StreamSummary (std::list<SystemInterface::Param> *destList);
  static void getParams_TaskItem (std::list<SystemInterface::Param> *destList);
  static void getParams_TimelapseCaptureIntentState (std::list<SystemInterface::Param> *destList);
  static void getParams_UpdateAgentConfiguration (std::list<SystemInterface::Param> *destList);
  static void getParams_UpdateIntentState (std::list<SystemInterface::Param> *destList);
  static void getParams_WatchTasks (std::list<SystemInterface::Param> *destList);
  static void getParams_WebDisplayIntentState (std::list<SystemInterface::Param> *destList);
  static void populateDefaultFields_AgentConfiguration (Json *destObject);
  static void populateDefaultFields_AgentContact (Json *destObject);
  static void populateDefaultFields_AgentHost (Json *destObject);
  static void populateDefaultFields_AgentStatus (Json *destObject);
  static void populateDefaultFields_ApplicationNews (Json *destObject);
  static void populateDefaultFields_ApplicationNewsItem (Json *destObject);
  static void populateDefaultFields_Authorize (Json *destObject);
  static void populateDefaultFields_AuthorizeResult (Json *destObject);
  static void populateDefaultFields_CameraDisplayIntentState (Json *destObject);
  static void populateDefaultFields_CameraServerConfiguration (Json *destObject);
  static void populateDefaultFields_CameraServerStatus (Json *destObject);
  static void populateDefaultFields_CancelTask (Json *destObject);
  static void populateDefaultFields_CommandResult (Json *destObject);
  static void populateDefaultFields_ConfigureCamera (Json *destObject);
  static void populateDefaultFields_ConfigureMediaStream (Json *destObject);
  static void populateDefaultFields_CreateCacheStream (Json *destObject);
  static void populateDefaultFields_CreateCameraDisplayIntent (Json *destObject);
  static void populateDefaultFields_CreateMediaDisplayIntent (Json *destObject);
  static void populateDefaultFields_CreateMediaStream (Json *destObject);
  static void populateDefaultFields_CreateMonitorProgram (Json *destObject);
  static void populateDefaultFields_CreateStreamCacheDisplayIntent (Json *destObject);
  static void populateDefaultFields_CreateTimelapseCaptureIntent (Json *destObject);
  static void populateDefaultFields_CreateWebDisplayIntent (Json *destObject);
  static void populateDefaultFields_EmptyObject (Json *destObject);
  static void populateDefaultFields_FindCaptureImages (Json *destObject);
  static void populateDefaultFields_FindCaptureImagesResult (Json *destObject);
  static void populateDefaultFields_FindMediaItems (Json *destObject);
  static void populateDefaultFields_FindMediaItemsResult (Json *destObject);
  static void populateDefaultFields_FindMediaStreams (Json *destObject);
  static void populateDefaultFields_FindMediaStreamsResult (Json *destObject);
  static void populateDefaultFields_FindStreamItems (Json *destObject);
  static void populateDefaultFields_FindStreamItemsResult (Json *destObject);
  static void populateDefaultFields_GetCameraStream (Json *destObject);
  static void populateDefaultFields_GetCameraStreamResult (Json *destObject);
  static void populateDefaultFields_GetCaptureImage (Json *destObject);
  static void populateDefaultFields_GetDashMpd (Json *destObject);
  static void populateDefaultFields_GetDashSegment (Json *destObject);
  static void populateDefaultFields_GetHlsManifest (Json *destObject);
  static void populateDefaultFields_GetHlsSegment (Json *destObject);
  static void populateDefaultFields_GetMedia (Json *destObject);
  static void populateDefaultFields_GetStreamItem (Json *destObject);
  static void populateDefaultFields_GetThumbnailImage (Json *destObject);
  static void populateDefaultFields_IntentState (Json *destObject);
  static void populateDefaultFields_MediaDisplayIntentState (Json *destObject);
  static void populateDefaultFields_MediaDisplayItem (Json *destObject);
  static void populateDefaultFields_MediaItem (Json *destObject);
  static void populateDefaultFields_MediaServerConfiguration (Json *destObject);
  static void populateDefaultFields_MediaServerStatus (Json *destObject);
  static void populateDefaultFields_MonitorProgramCondition (Json *destObject);
  static void populateDefaultFields_MonitorProgramDirective (Json *destObject);
  static void populateDefaultFields_MonitorProgramTimeCondition (Json *destObject);
  static void populateDefaultFields_MonitorServerConfiguration (Json *destObject);
  static void populateDefaultFields_MonitorServerStatus (Json *destObject);
  static void populateDefaultFields_PlayCacheStream (Json *destObject);
  static void populateDefaultFields_PlayCameraStream (Json *destObject);
  static void populateDefaultFields_PlayMedia (Json *destObject);
  static void populateDefaultFields_RemoveIntent (Json *destObject);
  static void populateDefaultFields_RemoveMedia (Json *destObject);
  static void populateDefaultFields_RemoveStream (Json *destObject);
  static void populateDefaultFields_ReportContact (Json *destObject);
  static void populateDefaultFields_ReportStatus (Json *destObject);
  static void populateDefaultFields_ServerError (Json *destObject);
  static void populateDefaultFields_SetAdminSecret (Json *destObject);
  static void populateDefaultFields_SetIntentActive (Json *destObject);
  static void populateDefaultFields_ShowAgentStatus (Json *destObject);
  static void populateDefaultFields_ShowCameraImage (Json *destObject);
  static void populateDefaultFields_ShowDesktopCountdown (Json *destObject);
  static void populateDefaultFields_ShowWebUrl (Json *destObject);
  static void populateDefaultFields_StreamCacheDisplayIntentState (Json *destObject);
  static void populateDefaultFields_StreamItem (Json *destObject);
  static void populateDefaultFields_StreamServerConfiguration (Json *destObject);
  static void populateDefaultFields_StreamServerStatus (Json *destObject);
  static void populateDefaultFields_StreamSummary (Json *destObject);
  static void populateDefaultFields_TaskItem (Json *destObject);
  static void populateDefaultFields_TimelapseCaptureIntentState (Json *destObject);
  static void populateDefaultFields_UpdateAgentConfiguration (Json *destObject);
  static void populateDefaultFields_UpdateIntentState (Json *destObject);
  static void populateDefaultFields_WatchTasks (Json *destObject);
  static void populateDefaultFields_WebDisplayIntentState (Json *destObject);
  static void hashFields_AgentConfiguration (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_AgentContact (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_AgentHost (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_AgentStatus (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_ApplicationNews (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_ApplicationNewsItem (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_Authorize (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_AuthorizeResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_CameraDisplayIntentState (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_CameraServerConfiguration (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_CameraServerStatus (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_CancelTask (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_CommandResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_ConfigureCamera (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_ConfigureMediaStream (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_CreateCacheStream (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_CreateCameraDisplayIntent (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_CreateMediaDisplayIntent (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_CreateMediaStream (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_CreateMonitorProgram (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_CreateStreamCacheDisplayIntent (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_CreateTimelapseCaptureIntent (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_CreateWebDisplayIntent (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_EmptyObject (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_FindCaptureImages (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_FindCaptureImagesResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_FindMediaItems (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_FindMediaItemsResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_FindMediaStreams (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_FindMediaStreamsResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_FindStreamItems (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_FindStreamItemsResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_GetCameraStream (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_GetCameraStreamResult (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_GetCaptureImage (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_GetDashMpd (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_GetDashSegment (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_GetHlsManifest (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_GetHlsSegment (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_GetMedia (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_GetStreamItem (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_GetThumbnailImage (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_IntentState (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_MediaDisplayIntentState (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_MediaDisplayItem (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_MediaItem (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_MediaServerConfiguration (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_MediaServerStatus (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_MonitorProgramCondition (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_MonitorProgramDirective (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_MonitorProgramTimeCondition (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_MonitorServerConfiguration (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_MonitorServerStatus (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_PlayCacheStream (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_PlayCameraStream (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_PlayMedia (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_RemoveIntent (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_RemoveMedia (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_RemoveStream (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_ReportContact (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_ReportStatus (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_ServerError (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_SetAdminSecret (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_SetIntentActive (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_ShowAgentStatus (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_ShowCameraImage (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_ShowDesktopCountdown (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_ShowWebUrl (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_StreamCacheDisplayIntentState (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_StreamItem (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_StreamServerConfiguration (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_StreamServerStatus (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_StreamSummary (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_TaskItem (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_TimelapseCaptureIntentState (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_UpdateAgentConfiguration (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_UpdateIntentState (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_WatchTasks (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
  static void hashFields_WebDisplayIntentState (Json *commandParams, SystemInterface::HashUpdateFunction hashUpdateFn, void *hashContextPtr);
};
#endif
