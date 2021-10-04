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
// Utility methods for operating system functions

#ifndef OS_UTIL_H
#define OS_UTIL_H

#include "StdString.h"

class OsUtil {
public:
	enum Result {
		Success = 0,
		InvalidParamError = -1,
		FileOpenFailedError = -2,
		ThreadCreateFailedError = -3,
		MalformedResponseError = -4,
		FileOperationFailedError = -5,
		MalformedDataError = -6,
		OutOfMemoryError = -7,
		FreetypeOperationFailedError = -8,
		SocketOperationFailedError = -9,
		SocketNotConnectedError = -10,
		HttpOperationFailedError = -11,
		MoreDataRequiredError = -12,
		JsonParseFailedError = -13,
		SystemOperationFailedError = -14,
		KeyNotFoundError = -15,
		MismatchedTypeError = -16,
		SdlOperationFailedError = -17,
		LibmicrohttpdOperationFailedError = -18,
		ArrayIndexOutOfBoundsError = -19,
		HttpRequestFailedError = -20,
		DuplicateIdError = -21,
		InvalidConfigurationError = -22,
		UnknownHostnameError = -23,
		NotImplementedError = -24,
		AlreadyLoadedError = -25,
		InternalApplicationFailureError = -26,
		UnknownProtocolError = -27,
		LibcurlOperationFailedError = -28,
		UnknownMethodError = -29,
		ApplicationNotInstalledError = -30,
		ProgramNotFoundError = -31,
		UnauthorizedError = -32,
		LuaOperationFailedError = -33
	};

	static const char *MonthNames[];

	// Return the current time in milliseconds since the epoch
	static int64_t getTime ();

	// Return the application's process ID
	static int getProcessId ();

	// Return a string containing a formatted value representing the specified millisecond duration
	enum TimeUnit {
		MillisecondsUnit = 0,
		SecondsUnit = 1,
		MinutesUnit = 2,
		HoursUnit = 3,
		DaysUnit = 4
	};
	static StdString getDurationString (int64_t duration, OsUtil::TimeUnit minUnitType = OsUtil::MillisecondsUnit);

	// Return a string containing a value representing the specified millisecond duration in readable format
	static StdString getDurationDisplayString (int64_t duration);

	// Return an enum value usable as a minUnitType value with the createDurationLabel method, indicating the most suitable unit type for use with the specified millisecond duration (i.e. shorter durations should be shown with smaller unit types)
	static OsUtil::TimeUnit getDurationMinUnitType (int64_t duration);

	// Return a string containing formatted text representing the time and date of the specified millisecond timestamp. A timestamp of zero or less indicates that the current timestamp should be used.
	static StdString getTimestampString (int64_t timestamp = 0, bool isTimezoneEnabled = false);

	// Return a string containing text representing the time and date of the specified millisecond timestamp in readable format. A timestamp of zero or less indicates that the current timestamp should be used.
	static StdString getTimestampDisplayString (int64_t timestamp = 0);

	// Return a string containing formatted text representing the date of the specified millisecond timestamp. A timestamp of zero or less indicates that the current timestamp should be used.
	static StdString getDateString (int64_t timestamp = 0);

	// Return a string containing formatted text representing the time of the specified millisecond timestamp. A timestamp of zero or less indicates that the current timestamp should be used.
	static StdString getTimeString (int64_t timestamp = 0);

	// Return a string containing text representing the specified number of bytes in readable format
	static StdString getByteCountDisplayString (int64_t bytes);

	// Return a string containing text representing the specified storage numbers in readable format
	static StdString getStorageAmountDisplayString (int64_t bytesFree, int64_t bytesTotal);

	// Return a string containing a base path with the provided name appended as a sub-path
	static StdString getAppendPath (const StdString &basePath, const StdString &appendName);

	// Return a string containing the system user data path, or an empty string if not known
	static StdString getUserDataPath ();

	// Create the named directory if it doesn't already exist. Returns a Result value.
	static OsUtil::Result createDirectory (const StdString &path);

	// Return a boolean value indicating if the provided path names a file that exists
	static bool fileExists (const StdString &path);

	// Read a file from the specified path and store its data in the provided string. Returns a Result value.
	static OsUtil::Result readFile (const StdString &path, StdString *destString);

	// Return a value read from the environment, or the specified default if no such value exists
	static StdString getEnvValue (const StdString &key, const StdString &defaultValue);
	static StdString getEnvValue (const StdString &key, const char *defaultValue);
	static bool getEnvValue (const StdString &key, bool defaultValue);
	static int getEnvValue (const StdString &key, int defaultValue);

	// Return the language identifier from the environment, or the specified default if no such value was found
	static StdString getEnvLanguage (const StdString &defaultValue);
	static StdString getEnvLanguage (const char *defaultValue);

	// Return a string containing the provided text, prepended with a protocol prefix "protocol://" if one isn't already present
	static StdString getProtocolString (const StdString &sourceText, const StdString &protocol = StdString ("http"));

	// Return a string containing the provided address, removing its port suffix if it matches the specified default
	static StdString getAddressDisplayString (const StdString &address, int defaultPort);

	// Launch the system's default browser to open the specified URL. Returns a Result value.
	static OsUtil::Result openUrl (const StdString &url);

	// Launch a child process and return a pointer to the resulting process data, or NULL if the process could not be launched
	static void *executeProcess (const StdString &execPath, const StdString &arg1 = StdString (""), const StdString &arg2 = StdString (""));

	// Block until a previously executed child process completes
	static void waitProcess (void *processPtr);

	// Send a terminate signal to a child process
	static void terminateProcess (void *processPtr);

	// Free process data, as created by a previous call to executeProcess
	static void freeProcessData (void *processPtr);
};

#endif
