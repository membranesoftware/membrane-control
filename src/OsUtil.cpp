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
#include "Config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#if PLATFORM_LINUX || PLATFORM_MACOS
#include <sys/time.h>
#include <unistd.h>
#endif
#if PLATFORM_WINDOWS
#include <time.h>
#include <windows.h>
#include <ShellAPI.h>
#endif
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include "Result.h"
#include "StdString.h"
#include "StringList.h"
#include "OsUtil.h"

const char *OsUtil::monthNames[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

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

StdString OsUtil::getDurationString (int64_t duration, int minUnitType) {
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
	int64_t t;
	int unit, d, h, m;

	unit = OsUtil::getDurationMinUnitType (duration);
	t = duration;
	t /= 1000;
	if (unit >= OsUtil::DaysUnit) {
		t /= 3600;
		h = (int) t;
		d = h / 24;
		h %= 24;
		return (StdString::createSprintf ("%id%ih", d, h));
	}
	if (unit >= OsUtil::HoursUnit) {
		t /= 60;
		h = (int) (t / 60);
		t %= 60;
		m = (int) t;
		return (StdString::createSprintf ("%ih%im", h, m));
	}
	if (unit >= OsUtil::MinutesUnit) {
		m = (int) (t / 60);
		return (StdString::createSprintf ("%im%is", m, (int) (t % 60)));
	}

	return (StdString::createSprintf ("%is", (int) t));
}

int OsUtil::getDurationMinUnitType (int64_t duration) {
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
	s.sprintf ("%02d/%s/%04d %02d:%02d:%02d.%03d", tv.tm_mday, OsUtil::monthNames[tv.tm_mon], tv.tm_year + 1900, tv.tm_hour, tv.tm_min, tv.tm_sec, ms);
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
			s.sprintf ("%02d/%s/%04d %02d:%02d:%02d.%03d", stlocal.wDay, OsUtil::monthNames[stlocal.wMonth - 1], stlocal.wYear, stlocal.wHour, stlocal.wMinute, stlocal.wSecond, stlocal.wMilliseconds);

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
	s.sprintf ("%02d/%s/%04d", tv.tm_mday, OsUtil::monthNames[tv.tm_mon], tv.tm_year + 1900);
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
			s.sprintf ("%02d/%s/%04d", stlocal.wDay, OsUtil::monthNames[stlocal.wMonth - 1], stlocal.wYear);
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

int OsUtil::createDirectory (const StdString &path) {
	int result;
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct stat st;

	result = stat (path.c_str (), &st);
	if (result != 0) {
		if (errno != ENOENT) {
			return (Result::SystemOperationFailedError);
		}
	}

	if ((result == 0) && (st.st_mode & S_IFDIR)) {
		return (Result::Success);
	}

	result = mkdir (path.c_str (), S_IRWXU);
	if (result != 0) {
		return (Result::SystemOperationFailedError);
	}

	result = Result::Success;
#endif
#if PLATFORM_WINDOWS
	DWORD a;

	a = GetFileAttributes (path.c_str ());
	if (a != INVALID_FILE_ATTRIBUTES) {
		if (a & FILE_ATTRIBUTE_DIRECTORY) {
			return (Result::Success);
		}
		else {
			return (Result::SystemOperationFailedError);
		}
	}

	if (! CreateDirectory (path.c_str (), NULL)) {
		return (Result::SystemOperationFailedError);
	}

	result = Result::Success;
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

int OsUtil::readFile (const StdString &path, StdString *destString) {
	FILE *fp;
	char data[8192];

	fp = fopen (path.c_str (), "rb");
	if (! fp) {
		return (Result::FileOpenFailedError);
	}

	destString->assign ("");
	while (1) {
		if (! fgets (data, sizeof (data), fp)) {
			break;
		}
		destString->append (data);
	}

	fclose (fp);
	return (Result::Success);
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
	// TODO: Implement this
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

int OsUtil::openUrl (const StdString &url) {
	int result;
#if PLATFORM_LINUX
	StdString execfile, execarg, path;
	StringList parts, execnames;
	StringList::iterator i, iend, j, jend;
#endif

	result = Result::NotImplementedError;
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
		return (Result::ProgramNotFoundError);
	}

	if (!(fork ())) {
		execlp (execfile.c_str (), execarg.c_str (), url.c_str (), NULL);
		exit (1);
	}
	result = Result::Success;
#endif
#if PLATFORM_MACOS
	if (!(fork ())) {
		execlp ("open", "open", url.c_str (), NULL);
		exit (-1);
	}
	result = Result::Success;
#endif
#if PLATFORM_WINDOWS
	HINSTANCE h;

	result = Result::Success;
	h = ShellExecute (NULL, "open", url.c_str (), NULL, NULL, SW_SHOWNORMAL);
	if (((int) h) <= 32) {
		result = Result::SystemOperationFailedError;
	}
#endif
	return (result);
}
