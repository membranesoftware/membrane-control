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
#ifndef SYSTEM_INTERFACE_H
#define SYSTEM_INTERFACE_H

#include <map>
#include <list>
#include <vector>
#include "Result.h"
#include "StdString.h"
#include "Json.h"

class SystemInterface {
public:
  static const char *version;
  static const int Command_AgentConfiguration = 45;
  static const int Command_AgentContact = 33;
  static const int Command_AgentStatus = 1;
  static const int Command_Authorize = 19;
  static const int Command_AuthorizeResult = 13;
  static const int Command_CancelTask = 28;
  static const int Command_ClearCache = 59;
  static const int Command_ClearDisplay = 31;
  static const int Command_CommandResult = 0;
  static const int Command_CreateCacheStream = 60;
  static const int Command_CreateMediaDisplayIntent = 50;
  static const int Command_CreateMediaStream = 14;
  static const int Command_CreateWebDisplayIntent = 35;
  static const int Command_EndSet = 21;
  static const int Command_EventRecord = 40;
  static const int Command_FindItems = 3;
  static const int Command_FindMediaResult = 48;
  static const int Command_FindStreamsResult = 4;
  static const int Command_GetAgentConfiguration = 44;
  static const int Command_GetHlsHtml5Interface = 25;
  static const int Command_GetHlsManifest = 23;
  static const int Command_GetHlsSegment = 24;
  static const int Command_GetMedia = 15;
  static const int Command_GetStatus = 8;
  static const int Command_GetThumbnailImage = 5;
  static const int Command_IntentState = 36;
  static const int Command_MediaDisplayIntentState = 51;
  static const int Command_MediaItem = 16;
  static const int Command_MediaServerStatus = 9;
  static const int Command_MonitorServerStatus = 12;
  static const int Command_PlayCacheStream = 57;
  static const int Command_PlayMedia = 30;
  static const int Command_ReadEvents = 18;
  static const int Command_ReadTasks = 6;
  static const int Command_RemoveIntent = 37;
  static const int Command_RemoveStream = 29;
  static const int Command_ReportContact = 32;
  static const int Command_ReportStatus = 2;
  static const int Command_ScanMediaItems = 58;
  static const int Command_ServerError = 20;
  static const int Command_SetIntentActive = 38;
  static const int Command_ShowWebUrl = 34;
  static const int Command_ShutdownAgent = 43;
  static const int Command_StartServers = 47;
  static const int Command_StopServers = 46;
  static const int Command_StreamItem = 22;
  static const int Command_StreamServerStatus = 10;
  static const int Command_TaskItem = 26;
  static const int Command_UpdateAgentConfiguration = 42;
  static const int Command_UpdateIntentState = 39;
  static const int Command_WatchEvents = 27;
  static const int Command_WatchTasks = 7;
  static const int Command_WebDisplayIntentState = 49;
  static const int ParamFlag_Required = 1;
  static const int ParamFlag_NotEmpty = 2;
  static const int ParamFlag_Hostname = 4;
  static const int ParamFlag_GreaterThanZero = 8;
  static const int ParamFlag_ZeroOrGreater = 16;
  static const int ParamFlag_Uuid = 32;
  static const int ParamFlag_Url = 64;
  static const int ParamFlag_RangedNumber = 128;
  static const int ParamFlag_Command = 256;
  static const int Constant_MaxCommandPriority;
  static const char *Constant_CreateTimePrefixField;
  static const char *Constant_AgentIdPrefixField;
  static const char *Constant_UserIdPrefixField;
  static const char *Constant_PriorityPrefixField;
  static const char *Constant_StartTimePrefixField;
  static const char *Constant_DurationPrefixField;
  static const char *Constant_AuthorizationHashPrefixField;
  static const char *Constant_AuthorizationTokenPrefixField;
  static const char *Constant_AuthorizationHashAlgorithm;
  static const char *Constant_WebSocketEvent;
  static const char *Constant_UrlQueryParameter;
  static const int Constant_DefaultTcpPort1;
  static const int Constant_DefaultTcpPort2;
  static const int Constant_DefaultUdpPort;
  static const char *Constant_DefaultInvokePath;
  static const char *Constant_DefaultAuthorizePath;
  static const char *Constant_DefaultLinkPath;
  static const int Constant_DefaultCommandType;
  static const int Constant_Stream;
  static const int Constant_Media;
  static const int Constant_Monitor;
  static const int Constant_Event;
  static const int Constant_Master;
  static const int Constant_Admin;
  static const int Constant_CommandTypeCount;
  void populate ();
	SystemInterface ();
	~SystemInterface ();

	struct Prefix {
		StdString agentId;
		StdString userId;
		int priority;
		int64_t startTime;
		int64_t duration;
		Prefix (): priority (0), startTime (0), duration (0) { }
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

	StdString lastError;
	std::map<StdString, SystemInterface::Command> commandMap;
	std::map<StdString, SystemInterface::GetParamsFunction> getParamsMap;
	std::map<StdString, SystemInterface::PopulateDefaultFieldsFunction> populateDefaultFieldsMap;

	// Return a newly created Json object containing a command item, or NULL if the command could not be created. commandParams can be NULL if not needed, causing the resulting command to contain empty parameter fields. If commandParams is not NULL, this method becomes responsible for freeing the object when it's no longer needed.
	Json *createCommand (const SystemInterface::Prefix &prefix, const char *commandName, int commandType, Json *commandParams);

	// Find command data for the specified name and store fields into the provided struct. Returns a boolean value indicating if the command was found.
	bool getCommand (const StdString &name, SystemInterface::Command *command);

	// Find type data for the specified name and store Param structs into the provided list. Returns a boolean value indicating if the type was found.
	bool getType (const StdString &name, std::list<SystemInterface::Param> *destList);

	// Populate default fields in an object, as appropriate for the specified type name. Returns a boolean value indicating if the type was found.
	bool populateDefaultFields (const StdString &typeName, Json *destObject);

	// Return a boolean value indicating if the provided fields are valid according to rules appearing in a Param list. If the fields are found to be invalid, this method sets the lastError value.
	bool fieldsValid (Json *fields, std::list<SystemInterface::Param> *paramList);

	// Parse a command JSON string and store the resulting Json object using the provided pointer. Returns a boolean value indicating if the parse was successful. If the parse fails, this method sets the lastError value.
	bool parseCommand (const StdString &commandString, Json **commandJson);

	// Return the command ID value appearing in the provided command object, or -1 if no such ID was found
	int getCommandId (Json *command);

	// Return the prefix.agentId value appearing in the provided command object, or an empty string if no such value was found
	StdString getCommandAgentId (Json *command);

	// Return the params.id value appearing in the provided command object, or an empty string if no such value was found
	StdString getCommandRecordId (Json *command);

	// Return the agent name value appearing in the provided AgentStatus command object, or an empty string if no such value was found
	StdString getCommandAgentName (Json *command);

	// Return the agent address value appearing in the provided AgentStatus command object, or an empty string if no such value was found
	StdString getCommandAgentAddress (Json *command);

	// Return a boolean value indicating if the provided command object is a record that holds the closed state
	bool isRecordClosed (Json *command);

	// Return a boolean value indicating if the provided string matches a Windows platform identifier
	bool isWindowsPlatform (const StdString &platform);

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
	bool getCommandNumberArrayParam (Json *command, const StdString &paramName, std::vector<double> *destList, bool shouldClear = false);
	bool getCommandNumberArrayParam (Json *command, const char *paramName, std::vector<double> *destList, bool shouldClear = false);

	// Return the length of the specified array, or 0 if the array was empty or non-existent
	int getCommandArrayLength (Json *command, const StdString &paramName);
	int getCommandArrayLength (Json *command, const char *paramName);

	// Find the specified object array item and store it in the provided Json object. Returns a boolean value indicating if the item was found.
	bool getCommandObjectArrayItem (Json *command, const StdString &paramName, int index, Json *destJson);
	bool getCommandObjectArrayItem (Json *command, const char *paramName, int index, Json *destJson);

  static void getParams_AgentConfiguration (std::list<SystemInterface::Param> *destList);
  static void getParams_AgentContact (std::list<SystemInterface::Param> *destList);
  static void getParams_AgentStatus (std::list<SystemInterface::Param> *destList);
  static void getParams_Authorize (std::list<SystemInterface::Param> *destList);
  static void getParams_AuthorizeResult (std::list<SystemInterface::Param> *destList);
  static void getParams_CancelTask (std::list<SystemInterface::Param> *destList);
  static void getParams_CommandResult (std::list<SystemInterface::Param> *destList);
  static void getParams_CreateCacheStream (std::list<SystemInterface::Param> *destList);
  static void getParams_CreateMediaDisplayIntent (std::list<SystemInterface::Param> *destList);
  static void getParams_CreateMediaStream (std::list<SystemInterface::Param> *destList);
  static void getParams_CreateWebDisplayIntent (std::list<SystemInterface::Param> *destList);
  static void getParams_EmptyObject (std::list<SystemInterface::Param> *destList);
  static void getParams_EventRecord (std::list<SystemInterface::Param> *destList);
  static void getParams_FindItems (std::list<SystemInterface::Param> *destList);
  static void getParams_FindMediaResult (std::list<SystemInterface::Param> *destList);
  static void getParams_FindStreamsResult (std::list<SystemInterface::Param> *destList);
  static void getParams_GetHlsHtml5Interface (std::list<SystemInterface::Param> *destList);
  static void getParams_GetHlsManifest (std::list<SystemInterface::Param> *destList);
  static void getParams_GetHlsSegment (std::list<SystemInterface::Param> *destList);
  static void getParams_GetMedia (std::list<SystemInterface::Param> *destList);
  static void getParams_GetThumbnailImage (std::list<SystemInterface::Param> *destList);
  static void getParams_IntentState (std::list<SystemInterface::Param> *destList);
  static void getParams_MediaDisplayIntentState (std::list<SystemInterface::Param> *destList);
  static void getParams_MediaDisplayItem (std::list<SystemInterface::Param> *destList);
  static void getParams_MediaItem (std::list<SystemInterface::Param> *destList);
  static void getParams_MediaServerConfiguration (std::list<SystemInterface::Param> *destList);
  static void getParams_MediaServerStatus (std::list<SystemInterface::Param> *destList);
  static void getParams_MonitorServerConfiguration (std::list<SystemInterface::Param> *destList);
  static void getParams_MonitorServerStatus (std::list<SystemInterface::Param> *destList);
  static void getParams_PlayCacheStream (std::list<SystemInterface::Param> *destList);
  static void getParams_PlayMedia (std::list<SystemInterface::Param> *destList);
  static void getParams_ReadEvents (std::list<SystemInterface::Param> *destList);
  static void getParams_RemoveIntent (std::list<SystemInterface::Param> *destList);
  static void getParams_RemoveStream (std::list<SystemInterface::Param> *destList);
  static void getParams_ReportContact (std::list<SystemInterface::Param> *destList);
  static void getParams_ReportStatus (std::list<SystemInterface::Param> *destList);
  static void getParams_ServerError (std::list<SystemInterface::Param> *destList);
  static void getParams_SetIntentActive (std::list<SystemInterface::Param> *destList);
  static void getParams_ShowWebUrl (std::list<SystemInterface::Param> *destList);
  static void getParams_StreamItem (std::list<SystemInterface::Param> *destList);
  static void getParams_StreamServerConfiguration (std::list<SystemInterface::Param> *destList);
  static void getParams_StreamServerStatus (std::list<SystemInterface::Param> *destList);
  static void getParams_TaskItem (std::list<SystemInterface::Param> *destList);
  static void getParams_UpdateAgentConfiguration (std::list<SystemInterface::Param> *destList);
  static void getParams_UpdateIntentState (std::list<SystemInterface::Param> *destList);
  static void getParams_WatchEvents (std::list<SystemInterface::Param> *destList);
  static void getParams_WatchTasks (std::list<SystemInterface::Param> *destList);
  static void getParams_WebDisplayIntentState (std::list<SystemInterface::Param> *destList);
  static void populateDefaultFields_AgentConfiguration (Json *destObject);
  static void populateDefaultFields_AgentContact (Json *destObject);
  static void populateDefaultFields_AgentStatus (Json *destObject);
  static void populateDefaultFields_Authorize (Json *destObject);
  static void populateDefaultFields_AuthorizeResult (Json *destObject);
  static void populateDefaultFields_CancelTask (Json *destObject);
  static void populateDefaultFields_CommandResult (Json *destObject);
  static void populateDefaultFields_CreateCacheStream (Json *destObject);
  static void populateDefaultFields_CreateMediaDisplayIntent (Json *destObject);
  static void populateDefaultFields_CreateMediaStream (Json *destObject);
  static void populateDefaultFields_CreateWebDisplayIntent (Json *destObject);
  static void populateDefaultFields_EmptyObject (Json *destObject);
  static void populateDefaultFields_EventRecord (Json *destObject);
  static void populateDefaultFields_FindItems (Json *destObject);
  static void populateDefaultFields_FindMediaResult (Json *destObject);
  static void populateDefaultFields_FindStreamsResult (Json *destObject);
  static void populateDefaultFields_GetHlsHtml5Interface (Json *destObject);
  static void populateDefaultFields_GetHlsManifest (Json *destObject);
  static void populateDefaultFields_GetHlsSegment (Json *destObject);
  static void populateDefaultFields_GetMedia (Json *destObject);
  static void populateDefaultFields_GetThumbnailImage (Json *destObject);
  static void populateDefaultFields_IntentState (Json *destObject);
  static void populateDefaultFields_MediaDisplayIntentState (Json *destObject);
  static void populateDefaultFields_MediaDisplayItem (Json *destObject);
  static void populateDefaultFields_MediaItem (Json *destObject);
  static void populateDefaultFields_MediaServerConfiguration (Json *destObject);
  static void populateDefaultFields_MediaServerStatus (Json *destObject);
  static void populateDefaultFields_MonitorServerConfiguration (Json *destObject);
  static void populateDefaultFields_MonitorServerStatus (Json *destObject);
  static void populateDefaultFields_PlayCacheStream (Json *destObject);
  static void populateDefaultFields_PlayMedia (Json *destObject);
  static void populateDefaultFields_ReadEvents (Json *destObject);
  static void populateDefaultFields_RemoveIntent (Json *destObject);
  static void populateDefaultFields_RemoveStream (Json *destObject);
  static void populateDefaultFields_ReportContact (Json *destObject);
  static void populateDefaultFields_ReportStatus (Json *destObject);
  static void populateDefaultFields_ServerError (Json *destObject);
  static void populateDefaultFields_SetIntentActive (Json *destObject);
  static void populateDefaultFields_ShowWebUrl (Json *destObject);
  static void populateDefaultFields_StreamItem (Json *destObject);
  static void populateDefaultFields_StreamServerConfiguration (Json *destObject);
  static void populateDefaultFields_StreamServerStatus (Json *destObject);
  static void populateDefaultFields_TaskItem (Json *destObject);
  static void populateDefaultFields_UpdateAgentConfiguration (Json *destObject);
  static void populateDefaultFields_UpdateIntentState (Json *destObject);
  static void populateDefaultFields_WatchEvents (Json *destObject);
  static void populateDefaultFields_WatchTasks (Json *destObject);
  static void populateDefaultFields_WebDisplayIntentState (Json *destObject);
};
#endif
