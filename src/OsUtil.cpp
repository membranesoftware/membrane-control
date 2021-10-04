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
#include "Config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#if PLATFORM_LINUX || PLATFORM_MACOS
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#endif
#if PLATFORM_WINDOWS
#include <time.h>
#include <windows.h>
#include <processthreadsapi.h>
#include <ShellAPI.h>
#endif
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include "StdString.h"
#include "StringList.h"
#include "OsUtil.h"
#include "Log.h"

const char *OsUtil::MonthNames[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

int64_t OsUtil::getTime () {
	int64_t t;
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct timeval now;

	gettimeofday (&now, NULL);
	t = ((int64_t) now.tv_sec) * 1000;
	t += (now.tv_usec / 1000);
#endif
#if PLATFORM_WINDOWS
	FILETIME ft;

	// Windows file times represent the number of 100-nanosecond intervals that
	// have elapsed since 12:00 A.M. January 1, 1601 Coordinated Universal Time
	// See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms724290(v=vs.85).aspx
	GetSystemTimeAsFileTime (&ft);
	t = ft.dwHighDateTime;
	t <<= 32;
	t |= ft.dwLowDateTime;
	t /= 10000;
	t -= 11644473600000ULL;
#endif
	return (t);
}

int OsUtil::getProcessId () {
#if PLATFORM_WINDOWS
	return ((int) GetCurrentProcessId ());
#else
	return ((int) getpid ());
#endif
}

StdString OsUtil::getDurationString (int64_t duration, OsUtil::TimeUnit minUnitType) {
	StdString s;
	char separator[2];
	int days, hours, minutes, seconds, ms, mintype;

	days = (int) (duration / (86400 * 1000));
	duration %= (86400 * 1000);
	hours = (int) (duration / (3600 * 1000));
	duration %= (3600 * 1000);
	minutes = (int) (duration / (60 * 1000));
	duration %= (60 * 1000);
	seconds = (int) (duration / 1000);
	ms = (int) (duration % 1000);

	mintype = OsUtil::MillisecondsUnit;
	if (seconds > 0) {
		mintype = OsUtil::SecondsUnit;
	}
	if (minutes > 0) {
		mintype = OsUtil::MinutesUnit;
	}
	if (hours > 0) {
		mintype = OsUtil::HoursUnit;
	}
	if (days > 0) {
		mintype = OsUtil::DaysUnit;
	}
	if (mintype < minUnitType) {
		mintype = minUnitType;
	}

	separator[0] = '\0';
	separator[1] = '\0';
	if (mintype >= OsUtil::DaysUnit) {
		s.appendSprintf ("%id", days);
	}
	if (mintype >= OsUtil::HoursUnit) {
		s.appendSprintf ("%02i", hours);
	}
	if (mintype >= OsUtil::MinutesUnit) {
		separator[0] = '\0';
		if (! s.empty ()) {
			separator[0] = ':';
		}
		s.appendSprintf ("%s%02i", separator, minutes);
	}
	if (mintype >= OsUtil::SecondsUnit) {
		separator[0] = '\0';
		if (! s.empty ()) {
			separator[0] = ':';
		}
		s.appendSprintf ("%s%02i", separator, seconds);
	}
	if (mintype >= OsUtil::MillisecondsUnit) {
		s.appendSprintf (".%03i", ms);
	}
	return (s);
}

StdString OsUtil::getDurationDisplayString (int64_t duration) {
	StdString result;
	int64_t t;
	int unit, d, h, m, s;

	unit = OsUtil::getDurationMinUnitType (duration);
	t = duration;
	t /= 1000;
	if (unit >= OsUtil::DaysUnit) {
		t /= 3600;
		h = (int) t;
		d = h / 24;
		h %= 24;
		result.sprintf ("%id", d);
		if (h > 0) {
			result.appendSprintf ("%ih", h);
		}
		return (result);
	}
	if (unit >= OsUtil::HoursUnit) {
		t /= 60;
		h = (int) (t / 60);
		t %= 60;
		m = (int) t;
		result.sprintf ("%ih", h);
		if (m > 0) {
			result.appendSprintf ("%im", m);
		}
		return (result);
	}
	if (unit >= OsUtil::MinutesUnit) {
		m = (int) (t / 60);
		s = (int) (t % 60);
		result.sprintf ("%im", m);
		if (s > 0) {
			result.appendSprintf ("%is", s);
		}
		return (result);
	}
	return (StdString::createSprintf ("%is", (int) t));
}

OsUtil::TimeUnit OsUtil::getDurationMinUnitType (int64_t duration) {
	if (duration >= (72 * 3600 * 1000)) {
		return (OsUtil::DaysUnit);
	}
	if (duration >= (3600 * 1000)) {
		return (OsUtil::HoursUnit);
	}
	if (duration >= (60 * 1000)) {
		return (OsUtil::MinutesUnit);
	}
	return (OsUtil::SecondsUnit);
}

StdString OsUtil::getTimestampString (int64_t timestamp, bool isTimezoneEnabled) {
	StdString s;
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct tm tv;
	time_t now;
	int ms;
#endif

	if (timestamp <= 0) {
		timestamp = OsUtil::getTime ();
	}
#if PLATFORM_LINUX || PLATFORM_MACOS
	ms = (int) (timestamp % 1000);
	now = (time_t) (timestamp / 1000);
	localtime_r (&now, &tv);
	s.sprintf ("%02d/%s/%04d %02d:%02d:%02d.%03d", tv.tm_mday, OsUtil::MonthNames[tv.tm_mon], tv.tm_year + 1900, tv.tm_hour, tv.tm_min, tv.tm_sec, ms);
	if (isTimezoneEnabled) {
		s.appendSprintf (" %+.2ld00", tv.tm_gmtoff / 3600);
	}
#endif
#if PLATFORM_WINDOWS
	FILETIME ft;
	SYSTEMTIME st, stlocal;
	TIME_ZONE_INFORMATION tz;
	DWORD result;

	timestamp += 11644473600000ULL;
	timestamp *= 10000;
	ft.dwLowDateTime = (timestamp & 0xFFFFFFFF);
	timestamp >>= 32;
	ft.dwHighDateTime = (timestamp & 0xFFFFFFFF);
	if (FileTimeToSystemTime (&ft, &st)) {
		if (SystemTimeToTzSpecificLocalTime (NULL, &st, &stlocal) != 0) {
			s.sprintf ("%02d/%s/%04d %02d:%02d:%02d.%03d", stlocal.wDay, OsUtil::MonthNames[stlocal.wMonth - 1], stlocal.wYear, stlocal.wHour, stlocal.wMinute, stlocal.wSecond, stlocal.wMilliseconds);

			if (isTimezoneEnabled) {
				result = GetTimeZoneInformation (&tz);
				if (result != TIME_ZONE_ID_INVALID) {
					s.appendSprintf (" %+.2ld00", -(tz.Bias / 60));
				}
			}
		}
	}
#endif
	return (s);
}

StdString OsUtil::getTimestampDisplayString (int64_t timestamp) {
	if (timestamp <= 0) {
		timestamp = OsUtil::getTime ();
	}
	return (StdString::createSprintf ("%s %s", OsUtil::getDateString (timestamp).c_str (), OsUtil::getTimeString (timestamp).c_str ()));
}

StdString OsUtil::getDateString (int64_t timestamp) {
	StdString s;
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct tm tv;
	time_t now;
#endif

	if (timestamp <= 0) {
		timestamp = OsUtil::getTime ();
	}
#if PLATFORM_LINUX || PLATFORM_MACOS
	now = (time_t) (timestamp / 1000);
	localtime_r (&now, &tv);
	s.sprintf ("%02d/%s/%04d", tv.tm_mday, OsUtil::MonthNames[tv.tm_mon], tv.tm_year + 1900);
#endif
#if PLATFORM_WINDOWS
	FILETIME ft;
	SYSTEMTIME st, stlocal;

	timestamp += 11644473600000ULL;
	timestamp *= 10000;
	ft.dwLowDateTime = (timestamp & 0xFFFFFFFF);
	timestamp >>= 32;
	ft.dwHighDateTime = (timestamp & 0xFFFFFFFF);
	if (FileTimeToSystemTime (&ft, &st)) {
		if (SystemTimeToTzSpecificLocalTime (NULL, &st, &stlocal) != 0) {
			s.sprintf ("%02d/%s/%04d", stlocal.wDay, OsUtil::MonthNames[stlocal.wMonth - 1], stlocal.wYear);
		}
	}
#endif
	return (s);
}

StdString OsUtil::getTimeString (int64_t timestamp) {
	StdString s;
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct tm tv;
	time_t now;
#endif

	if (timestamp <= 0) {
		timestamp = OsUtil::getTime ();
	}
#if PLATFORM_LINUX || PLATFORM_MACOS
	now = (time_t) (timestamp / 1000);
	localtime_r (&now, &tv);
	s.sprintf ("%02d:%02d:%02d", tv.tm_hour, tv.tm_min, tv.tm_sec);
#endif
#if PLATFORM_WINDOWS
	FILETIME ft;
	SYSTEMTIME st, stlocal;
	DWORD result;

	timestamp += 11644473600000ULL;
	timestamp *= 10000;
	ft.dwLowDateTime = (timestamp & 0xFFFFFFFF);
	timestamp >>= 32;
	ft.dwHighDateTime = (timestamp & 0xFFFFFFFF);
	if (FileTimeToSystemTime (&ft, &st)) {
		if (SystemTimeToTzSpecificLocalTime (NULL, &st, &stlocal) != 0) {
			s.sprintf ("%02d:%02d:%02d", stlocal.wHour, stlocal.wMinute, stlocal.wSecond);
		}
	}
#endif
	return (s);
}

StdString OsUtil::getByteCountDisplayString (int64_t bytes) {
	float n;

	if (bytes <= 0) {
		return (StdString ("0B"));
	}
	if (bytes >= 1099511627776L) {
		n = (float) bytes;
		n /= (float) 1099511627776L;
		return (StdString::createSprintf ("%.2fTB", n));
	}
	if (bytes >= 1073741824L) {
		n = (float) bytes;
		n /= (float) 1073741824L;
		return (StdString::createSprintf ("%.2fGB", n));
	}
	if (bytes >= 1048576) {
		n = (float) bytes;
		n /= (float) 1048576;
		return (StdString::createSprintf ("%.2fMB", n));
	}
	if (bytes >= 1024) {
		n = (float) bytes;
		n /= (float) 1024;
		return (StdString::createSprintf ("%ikB", (int) n));
	}

	n = (float) bytes;
	n /= (float) 1024;
	if (n < 0.01f) {
		n = 0.01f;
	}
	return (StdString::createSprintf ("%.2fkB", n));
}

StdString OsUtil::getStorageAmountDisplayString (int64_t bytesFree, int64_t bytesTotal) {
	float pct;

	if ((bytesFree > bytesTotal) || (bytesTotal <= 0)) {
		return (StdString ("0B"));
	}
	pct = (float) bytesFree;
	pct /= (float) bytesTotal;
	pct *= 100.0f;
	return (StdString::createSprintf ("%s / %s (%i%%)", OsUtil::getByteCountDisplayString (bytesFree).c_str (), OsUtil::getByteCountDisplayString (bytesTotal).c_str (), (int) pct));
}

StdString OsUtil::getAppendPath (const StdString &basePath, const StdString &appendName) {
	StdString s;

	s.assign (basePath);
#if PLATFORM_WINDOWS
	s.appendSprintf ("\\%s", appendName.c_str ());
#else
	s.appendSprintf ("/%s", appendName.c_str ());
#endif
	return (s);
}

StdString OsUtil::getUserDataPath () {
	StdString path;

#if PLATFORM_LINUX
	path = OsUtil::getEnvValue ("HOME", "");
	if (! path.empty ()) {
		path = OsUtil::getAppendPath (path, CONFIG_APPDATA_DIRNAME);
	}
#endif
#if PLATFORM_MACOS
	path = OsUtil::getEnvValue ("HOME", "");
	if (! path.empty ()) {
		path = OsUtil::getAppendPath (path, "Library");
		path = OsUtil::getAppendPath (path, "Application Support");
		path = OsUtil::getAppendPath (path, CONFIG_APPDATA_DIRNAME);
	}
#endif
#if PLATFORM_WINDOWS
	path = OsUtil::getEnvValue ("LOCALAPPDATA", "");
	if (! path.empty ()) {
		path = OsUtil::getAppendPath (path, CONFIG_APPDATA_DIRNAME);
	}
#endif
	return (path);
}

OsUtil::Result OsUtil::createDirectory (const StdString &path) {
	OsUtil::Result result;
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct stat st;
	int cresult;

	cresult = stat (path.c_str (), &st);
	if (cresult != 0) {
		if (errno != ENOENT) {
			return (OsUtil::Result::SystemOperationFailedError);
		}
	}
	if ((cresult == 0) && (st.st_mode & S_IFDIR)) {
		return (OsUtil::Result::Success);
	}
	cresult = mkdir (path.c_str (), S_IRWXU);
	if (cresult != 0) {
		return (OsUtil::Result::SystemOperationFailedError);
	}

	result = OsUtil::Result::Success;
#endif
#if PLATFORM_WINDOWS
	DWORD a;

	a = GetFileAttributes (path.c_str ());
	if (a != INVALID_FILE_ATTRIBUTES) {
		if (a & FILE_ATTRIBUTE_DIRECTORY) {
			return (OsUtil::Result::Success);
		}
		else {
			return (OsUtil::Result::SystemOperationFailedError);
		}
	}
	if (! CreateDirectory (path.c_str (), NULL)) {
		return (OsUtil::Result::SystemOperationFailedError);
	}

	result = OsUtil::Result::Success;
#endif
	return (result);
}

bool OsUtil::fileExists (const StdString &path) {
	struct stat st;
	int result;

	result = stat (path.c_str (), &st);
	if (result != 0) {
		return (false);
	}
	return (true);
}

OsUtil::Result OsUtil::readFile (const StdString &path, StdString *destString) {
	FILE *fp;
	char data[8192];

	fp = fopen (path.c_str (), "rb");
	if (! fp) {
		return (OsUtil::Result::FileOpenFailedError);
	}
	destString->assign ("");
	while (1) {
		if (! fgets (data, sizeof (data), fp)) {
			break;
		}
		destString->append (data);
	}

	fclose (fp);
	return (OsUtil::Result::Success);
}

StdString OsUtil::getEnvValue (const StdString &key, const StdString &defaultValue) {
#if PLATFORM_LINUX || PLATFORM_MACOS
	char *val;

	val = getenv (key.c_str ());
	if (! val) {
		return (defaultValue);
	}
	return (StdString (val));
#endif
#if PLATFORM_WINDOWS
	DWORD result, bufsize;
	LPTSTR buf;
	StdString val;
	std::basic_string<TCHAR> k;

	bufsize = 4096;
	buf = (LPTSTR) malloc (bufsize * sizeof (TCHAR));
	if (! buf) {
		return (defaultValue);
	}

	val.assign (defaultValue);
	k.assign (key);
	result = GetEnvironmentVariable (k.c_str (), buf, bufsize);
	if (result > bufsize) {
		bufsize = result;
		buf = (LPTSTR) realloc (buf, bufsize * sizeof (TCHAR));
		if (! buf) {
			return (defaultValue);
		}
		result = GetEnvironmentVariable (k.c_str (), buf, bufsize);
	}
	if ((result != 0) && (result <= bufsize)) {
		k.assign (buf);
		val.assign (k);
	}

	free (buf);
	return (val);
#endif
}

StdString OsUtil::getEnvValue (const StdString &key, const char *defaultValue) {
	return (OsUtil::getEnvValue (key, StdString (defaultValue)));
}

bool OsUtil::getEnvValue (const StdString &key, bool defaultValue) {
	StdString val;

	val = OsUtil::getEnvValue (key, defaultValue ? "true" : "false");
	return (val.equals ("true"));
}

int OsUtil::getEnvValue (const StdString &key, int defaultValue) {
	StdString val;
	int result;

	val = OsUtil::getEnvValue (key, "");
	if (val.empty () || (! val.parseInt (&result))) {
		return (defaultValue);
	}
	return (result);
}

StdString OsUtil::getEnvLanguage (const StdString &defaultValue) {
#if PLATFORM_LINUX || PLATFORM_MACOS
	StdString lang;
	size_t pos;

	lang = OsUtil::getEnvValue (StdString ("LANG"), StdString (""));
	if (! lang.empty ()) {
		pos = lang.find ('_');
		if (pos != StdString::npos) {
			return (lang.substr (0, pos));
		}
		pos = lang.find ('-');
		if (pos != StdString::npos) {
			return (lang.substr (0, pos));
		}
		return (lang);
	}
#endif
#if PLATFORM_WINDOWS
	LANGID lang;
	unsigned char id;

	lang = GetUserDefaultUILanguage ();
	id = (unsigned char) (lang & 0xFF);
	switch (id) {
		case 0x36: { return (StdString ("af")); }
		case 0x1C: { return (StdString ("sq")); }
		case 0x84: { return (StdString ("gsw")); }
		case 0x5E: { return (StdString ("am")); }
		case 0x01: { return (StdString ("ar")); }
		case 0x2B: { return (StdString ("hy")); }
		case 0x4D: { return (StdString ("as")); }
		case 0x2C: { return (StdString ("az")); }
		case 0x45: { return (StdString ("bn")); }
		case 0x6D: { return (StdString ("ba")); }
		case 0x2D: { return (StdString ("eu")); }
		case 0x23: { return (StdString ("be")); }
		case 0x7E: { return (StdString ("br")); }
		case 0x02: { return (StdString ("bg")); }
		case 0x55: { return (StdString ("my")); }
		case 0x03: { return (StdString ("ca")); }
		case 0x5F: { return (StdString ("tzm")); }
		case 0x92: { return (StdString ("ku")); }
		case 0x5C: { return (StdString ("chr")); }
		case 0x04: { return (StdString ("zh")); }
		case 0x83: { return (StdString ("co")); }
		case 0x1A: { return (StdString ("hr")); }
		case 0x05: { return (StdString ("cs")); }
		case 0x06: { return (StdString ("da")); }
		case 0x8C: { return (StdString ("prs")); }
		case 0x65: { return (StdString ("dv")); }
		case 0x13: { return (StdString ("nl")); }
		case 0x09: { return (StdString ("en")); }
		case 0x25: { return (StdString ("et")); }
		case 0x38: { return (StdString ("fo")); }
		case 0x64: { return (StdString ("fil")); }
		case 0x0B: { return (StdString ("fi")); }
		case 0x0C: { return (StdString ("fr")); }
		case 0x62: { return (StdString ("fy")); }
		case 0x67: { return (StdString ("ff")); }
		case 0x56: { return (StdString ("gl")); }
		case 0x37: { return (StdString ("ka")); }
		case 0x07: { return (StdString ("de")); }
		case 0x08: { return (StdString ("el")); }
		case 0x6F: { return (StdString ("kl")); }
		case 0x74: { return (StdString ("gn")); }
		case 0x47: { return (StdString ("gu")); }
		case 0x68: { return (StdString ("ha")); }
		case 0x75: { return (StdString ("haw")); }
		case 0x0D: { return (StdString ("he")); }
		case 0x39: { return (StdString ("hi")); }
		case 0x0E: { return (StdString ("hu")); }
		case 0x0F: { return (StdString ("is")); }
		case 0x70: { return (StdString ("ig")); }
		case 0x21: { return (StdString ("id")); }
		case 0x5D: { return (StdString ("iu")); }
		case 0x3C: { return (StdString ("ga")); }
		case 0x10: { return (StdString ("it")); }
		case 0x11: { return (StdString ("ja")); }
		case 0x4B: { return (StdString ("kn")); }
		case 0x71: { return (StdString ("kr")); }
		case 0x60: { return (StdString ("ks")); }
		case 0x3F: { return (StdString ("kk")); }
		case 0x53: { return (StdString ("km")); }
		case 0x86: { return (StdString ("quc")); }
		case 0x87: { return (StdString ("rw")); }
		case 0x41: { return (StdString ("sw")); }
		case 0x57: { return (StdString ("kok")); }
		case 0x12: { return (StdString ("ko")); }
		case 0x40: { return (StdString ("ky")); }
		case 0x54: { return (StdString ("lo")); }
		case 0x76: { return (StdString ("la")); }
		case 0x26: { return (StdString ("lv")); }
		case 0x27: { return (StdString ("lt")); }
		case 0x6E: { return (StdString ("lb")); }
		case 0x2F: { return (StdString ("mk")); }
		case 0x3E: { return (StdString ("ms")); }
		case 0x4C: { return (StdString ("ml")); }
		case 0x3A: { return (StdString ("mt")); }
		case 0x81: { return (StdString ("mi")); }
		case 0x7A: { return (StdString ("arn")); }
		case 0x4E: { return (StdString ("mr")); }
		case 0x7C: { return (StdString ("moh")); }
		case 0x50: { return (StdString ("mn")); }
		case 0x61: { return (StdString ("ne")); }
		case 0x14: { return (StdString ("no")); }
		case 0x82: { return (StdString ("oc")); }
		case 0x48: { return (StdString ("or")); }
		case 0x72: { return (StdString ("om")); }
		case 0x63: { return (StdString ("ps")); }
		case 0x29: { return (StdString ("fa")); }
		case 0x15: { return (StdString ("pl")); }
		case 0x16: { return (StdString ("pt")); }
		case 0x46: { return (StdString ("pa")); }
		case 0x6B: { return (StdString ("quz")); }
		case 0x18: { return (StdString ("ro")); }
		case 0x17: { return (StdString ("rm")); }
		case 0x19: { return (StdString ("ru")); }
		case 0x85: { return (StdString ("sah")); }
		case 0x3B: { return (StdString ("smn")); }
		case 0x4F: { return (StdString ("sa")); }
		case 0x91: { return (StdString ("gd")); }
		case 0x6C: { return (StdString ("nso")); }
		case 0x32: { return (StdString ("tn")); }
		case 0x59: { return (StdString ("sd")); }
		case 0x5B: { return (StdString ("si")); }
		case 0x1B: { return (StdString ("sk")); }
		case 0x24: { return (StdString ("sl")); }
		case 0x77: { return (StdString ("so")); }
		case 0x30: { return (StdString ("st")); }
		case 0x0A: { return (StdString ("es")); }
		case 0x1D: { return (StdString ("sv")); }
		case 0x5A: { return (StdString ("syr")); }
		case 0x28: { return (StdString ("tg")); }
		case 0x49: { return (StdString ("ta")); }
		case 0x44: { return (StdString ("tt")); }
		case 0x4A: { return (StdString ("te")); }
		case 0x1E: { return (StdString ("th")); }
		case 0x51: { return (StdString ("bo")); }
		case 0x73: { return (StdString ("ti")); }
		case 0x31: { return (StdString ("ts")); }
		case 0x1F: { return (StdString ("tr")); }
		case 0x42: { return (StdString ("tk")); }
		case 0x22: { return (StdString ("uk")); }
		case 0x2E: { return (StdString ("hsb")); }
		case 0x20: { return (StdString ("ur")); }
		case 0x80: { return (StdString ("ug")); }
		case 0x43: { return (StdString ("uz")); }
		case 0x33: { return (StdString ("ve")); }
		case 0x2A: { return (StdString ("vi")); }
		case 0x52: { return (StdString ("cy")); }
		case 0x88: { return (StdString ("wo")); }
		case 0x34: { return (StdString ("xh")); }
		case 0x78: { return (StdString ("ii")); }
		case 0x3D: { return (StdString ("yi")); }
		case 0x6A: { return (StdString ("yo")); }
		case 0x35: { return (StdString ("zu")); }
	}
#endif
	return (defaultValue);
}

StdString OsUtil::getEnvLanguage (const char *defaultValue) {
	return (OsUtil::getEnvLanguage (StdString (defaultValue)));
}

StdString OsUtil::getProtocolString (const StdString &sourceText, const StdString &protocol) {
	StdString text;

	text.assign (sourceText);
	if (! text.contains ("://")) {
		text.insert (0, StdString::createSprintf ("%s://", protocol.c_str ()));
	}
	return (text);
}

StdString OsUtil::getAddressDisplayString (const StdString &address, int defaultPort) {
	StdString s, suffix;

	suffix.sprintf (":%i", defaultPort);
	s.assign (address);
	if (s.endsWith (suffix)) {
		s.assign (s.substr (0, s.length () - suffix.length ()));
	}
	return (s);
}

OsUtil::Result OsUtil::openUrl (const StdString &url) {
	OsUtil::Result result;
#if PLATFORM_LINUX
	StdString execfile, execarg, path;
	StringList parts, execnames;
	StringList::iterator i, iend, j, jend;
#endif

	result = OsUtil::Result::NotImplementedError;
#if PLATFORM_LINUX
	execfile = OsUtil::getEnvValue ("BROWSER", "");
	if (! execfile.empty ()) {
		execarg.assign (execfile);
	}
	if (execfile.empty ()) {
		execnames.push_back (StdString ("xdg-open"));
		execnames.push_back (StdString ("firefox"));
		execnames.push_back (StdString ("google-chrome"));
		execnames.push_back (StdString ("chromium"));
		execnames.push_back (StdString ("mozilla"));

		path = OsUtil::getEnvValue (StdString ("PATH"), "");
		path.split (":", &parts);

		i = parts.begin ();
		iend = parts.end ();
		while (i != iend) {
			j = execnames.begin ();
			jend = execnames.end ();
			while (j != jend) {
				path.sprintf ("%s/%s", i->c_str (), j->c_str ());
				if (OsUtil::fileExists (path)) {
					execfile.assign (path);
					execarg.assign (j->c_str ());
					break;
				}
				++j;
			}
			if (! execfile.empty ()) {
				break;
			}

			++i;
		}
	}
	if (execfile.empty ()) {
		return (OsUtil::Result::ProgramNotFoundError);
	}

	if (!(fork ())) {
		execlp (execfile.c_str (), execarg.c_str (), url.c_str (), NULL);
		_Exit (1);
	}
	result = OsUtil::Result::Success;
#endif
#if PLATFORM_MACOS
	if (!(fork ())) {
		execlp ("open", "open", url.c_str (), NULL);
		_Exit (1);
	}
	result = OsUtil::Result::Success;
#endif
#if PLATFORM_WINDOWS
	HINSTANCE h;

	result = OsUtil::Result::Success;
	h = ShellExecute (NULL, "open", url.c_str (), NULL, NULL, SW_SHOWNORMAL);
	if (((int) h) <= 32) {
		result = OsUtil::Result::SystemOperationFailedError;
	}
#endif
	return (result);
}

void *OsUtil::executeProcess (const StdString &execPath, const StdString &arg1, const StdString &arg2) {
#if PLATFORM_LINUX || PLATFORM_MACOS
	char *path;
	char *argv[4];
	pid_t *pid;

	memset (argv, 0, sizeof (argv));
	path = (char *) execPath.c_str ();
	argv[0] = (char *) execPath.c_str ();
	if (! arg1.empty ()) {
		argv[1] = (char *) arg1.c_str ();
		if (! arg2.empty ()) {
			argv[2] = (char *) arg2.c_str ();
		}
	}

	pid = (pid_t *) malloc (sizeof (pid_t));
	if (! pid) {
		return (NULL);
	}
	*pid = fork ();
	if (*pid) {
		return (pid);
	}
	execv (path, argv);
	_Exit (1);
#endif
#if PLATFORM_WINDOWS
	STARTUPINFO si;
	PROCESS_INFORMATION *pi;
	StdString path, args;

	path.assign (execPath);
	if (! path.endsWith (".exe")) {
		path.append (".exe");
	}
	args.assign (path);
	if (! arg1.empty ()) {
		args.appendSprintf (" %s", arg1.c_str ());
		if (! arg2.empty ()) {
			args.appendSprintf (" %s", arg2.c_str ());
		}
	}

	ZeroMemory (&si, sizeof (si));
	si.cb = sizeof (si);
	pi = (PROCESS_INFORMATION *) malloc (sizeof (PROCESS_INFORMATION));
	ZeroMemory (pi, sizeof (PROCESS_INFORMATION));
	if (CreateProcess (const_cast<char *> (path.c_str ()), const_cast<char *> (args.c_str ()), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, pi)) {
		return (pi);
	}
	delete (pi);
	Log::debug ("OsUtil::executeProcess CreateProcess failed; error=%d", GetLastError ());
#endif
	return (NULL);
}

void OsUtil::waitProcess (void *processPtr) {
#if PLATFORM_LINUX || PLATFORM_MACOS
	pid_t *pid;
	int wstatus;

	pid = (pid_t *) processPtr;
	waitpid (*pid, &wstatus, 0);
#endif
#if PLATFORM_WINDOWS
	PROCESS_INFORMATION *pi;

	pi = (PROCESS_INFORMATION *) processPtr;
	WaitForSingleObject (pi->hProcess, INFINITE);
	CloseHandle (pi->hProcess);
	CloseHandle (pi->hThread);
#endif
}

void OsUtil::terminateProcess (void *processPtr) {
#if PLATFORM_LINUX || PLATFORM_MACOS
	pid_t *pid;

	pid = (pid_t *) processPtr;
	kill (*pid, SIGTERM);
#endif
#if PLATFORM_WINDOWS
	PROCESS_INFORMATION *pi;

	pi = (PROCESS_INFORMATION *) processPtr;
	TerminateProcess (pi->hProcess, 0);
#endif
}

void OsUtil::freeProcessData (void *processPtr) {
	free (processPtr);
}
