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
#include <map>
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "StringList.h"
#include "Util.h"

const char *Util::monthNames[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
const float Util::aspectRatioMatchEpsilon = 0.1f;
const StdString Util::serverUrl = StdString ("https://membranesoftware.com/");

float Util::normalizeDirection (float direction) {
	while (direction < 0.0f) {
		direction += 360.0f;
	}
	while (direction >= 360.0f) {
		direction -= 360.0f;
	}

	return (direction);
}

float Util::getDistance (float x1, float y1, float x2, float y2) {
	float dx, dy;

	dx = x2 - x1;
	dy = y2 - y1;
	return ((float) sqrt ((dx * dx) + (dy * dy)));
}

float Util::getDistance (float dx, float dy) {
	return ((float) sqrt ((dx * dx) + (dy * dy)));
}

void Util::getDirectionVector (float direction, float scale, float *dx, float *dy) {
	float rad;

	rad = direction * ((double) (3.14159265f / 180.0f));
	if (dx) {
		*dx = scale * cos (rad);
	}
	if (dy) {
		*dy = -1.0f * scale * sin (rad);
	}
}

float Util::getVectorDirection (float dx, float dy) {
	double dist, theta;

	dist = (double) ((dx * dx) + (dy * dy));
	if (fabs (dist) < CONFIG_FLOAT_EPSILON) {
		return (0);
	}

	dist = sqrt (dist);
	theta = acos (((double) fabs (dx)) / dist);
	theta *= (double) (180.0f / 3.14159265f);

	if (dx < 0) {
		if (dy < 0) {
			theta = 180 - theta;
		}
		else {
			theta += 180;
		}
	}
	else {
		if (dy > 0) {
			theta = 360 - theta;
		}
	}

	return ((float) theta);
}

int Util::parseHexString (const char *str, int *intValue) {
	int i;
	char *s, c;

	i = 0;
	s = (char *) str;

	if (! (*s)) {
		return (Result::ERROR_INVALID_PARAM);
	}

	while (1) {
		c = *s;
		if (! c) {
			break;
		}

		i <<= 4;
		if ((c >= '0') && (c <= '9')) {
			i |= ((c - '0') & 0x0F);
		}
		else if ((c >= 'a') && (c <= 'f')) {
			i |= ((c - 'a' + 10) & 0x0F);
		}
		else if ((c >= 'A') && (c <= 'F')) {
			i |= ((c - 'A' + 10) & 0x0F);
		}
		else {
			return (Result::ERROR_MALFORMED_DATA);
		}

		++s;
	}

	if (intValue) {
		*intValue = i;
	}

	return (Result::SUCCESS);
}

int64_t Util::getTime () {
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

StdString Util::getDurationString (int64_t duration, int minUnitType) {
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

	mintype = Util::MILLISECONDS;
	if (seconds > 0) {
		mintype = Util::SECONDS;
	}
	if (minutes > 0) {
		mintype = Util::MINUTES;
	}
	if (hours > 0) {
		mintype = Util::HOURS;
	}
	if (days > 0) {
		mintype = Util::DAYS;
	}
	if (mintype < minUnitType) {
		mintype = minUnitType;
	}

	separator[0] = '\0';
	separator[1] = '\0';
	if (mintype >= Util::DAYS) {
		s.appendSprintf ("%id", days);
	}
	if (mintype >= Util::HOURS) {
		s.appendSprintf ("%02i", hours);
	}
	if (mintype >= Util::MINUTES) {
		separator[0] = '\0';
		if (! s.empty ()) {
			separator[0] = ':';
		}
		s.appendSprintf ("%s%02i", separator, minutes);
	}
	if (mintype >= Util::SECONDS) {
		separator[0] = '\0';
		if (! s.empty ()) {
			separator[0] = ':';
		}
		s.appendSprintf ("%s%02i", separator, seconds);
	}
	if (mintype >= Util::MILLISECONDS) {
		s.appendSprintf (".%03i", ms);
	}

	return (s);
}

StdString Util::getDurationDisplayString (int64_t duration) {
	int64_t t;
	int unit, h, m;

	unit = Util::getDurationMinUnitType (duration);
	t = duration;
	t /= 1000;
	if (unit >= Util::HOURS) {
		t /= 60;
		h = (int) (t / 60);
		t %= 60;
		m = (int) t;
		return (StdString::createSprintf ("%ih%im", h, m));
	}
	if (unit >= Util::MINUTES) {
		m = (int) (t / 60);
		return (StdString::createSprintf ("%im%is", m, (int) (t % 60)));
	}

	return (StdString::createSprintf ("%is", (int) t));
}

int Util::getDurationMinUnitType (int64_t duration) {
	if (duration >= (3600 * 1000)) {
		return (Util::HOURS);
	}
	if (duration >= (60 * 1000)) {
		return (Util::MINUTES);
	}
	return (Util::SECONDS);
}

StdString Util::getTimestampString (int64_t timestamp, bool isTimezoneEnabled) {
	StdString s;
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct tm tv;
	time_t now;
	int ms;
#endif

	if (timestamp <= 0) {
		timestamp = Util::getTime ();
	}

#if PLATFORM_LINUX || PLATFORM_MACOS
	ms = (int) (timestamp % 1000);
	now = (time_t) (timestamp / 1000);
	localtime_r (&now, &tv);
	s.sprintf ("%02d/%s/%04d %02d:%02d:%02d.%03d", tv.tm_mday, Util::monthNames[tv.tm_mon], tv.tm_year + 1900, tv.tm_hour, tv.tm_min, tv.tm_sec, ms);
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
			s.sprintf ("%02d/%s/%04d %02d:%02d:%02d.%03d", stlocal.wDay, Util::monthNames[stlocal.wMonth - 1], stlocal.wYear, stlocal.wHour, stlocal.wMinute, stlocal.wSecond, stlocal.wMilliseconds);

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

StdString Util::getTimestampDisplayString (int64_t timestamp) {
	if (timestamp <= 0) {
		timestamp = Util::getTime ();
	}
	return (StdString::createSprintf ("%s %s", Util::getDateString (timestamp).c_str (), Util::getTimeString (timestamp).c_str ()));
}

StdString Util::getDateString (int64_t timestamp) {
	StdString s;
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct tm tv;
	time_t now;
#endif

	if (timestamp <= 0) {
		timestamp = Util::getTime ();
	}

#if PLATFORM_LINUX || PLATFORM_MACOS
	now = (time_t) (timestamp / 1000);
	localtime_r (&now, &tv);
	s.sprintf ("%02d/%s/%04d", tv.tm_mday, Util::monthNames[tv.tm_mon], tv.tm_year + 1900);
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
			s.sprintf ("%02d/%s/%04d", stlocal.wDay, Util::monthNames[stlocal.wMonth - 1], stlocal.wYear);
		}
	}
#endif

	return (s);
}

StdString Util::getTimeString (int64_t timestamp) {
	StdString s;
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct tm tv;
	time_t now;
#endif

	if (timestamp <= 0) {
		timestamp = Util::getTime ();
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

StdString Util::getByteCountDisplayString (int64_t bytes) {
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

StdString Util::getStorageAmountDisplayString (int64_t bytesFree, int64_t bytesTotal) {
	float pct;

	if ((bytesFree > bytesTotal) || (bytesTotal <= 0)) {
		return (StdString ("0B"));
	}

	pct = (float) bytesFree;
	pct /= (float) bytesTotal;
	pct *= 100.0f;
	return (StdString::createSprintf ("%s / %s (%i%%)", Util::getByteCountDisplayString (bytesFree).c_str (), Util::getByteCountDisplayString (bytesTotal).c_str (), (int) pct));
}

StdString Util::getAspectRatioDisplayString (int width, int height) {
	return (Util::getAspectRatioDisplayString ((float) width / (float) height));
}

StdString Util::getAspectRatioDisplayString (float ratio) {
	if (fabs (ratio - (16.0f / 9.0f)) <= Util::aspectRatioMatchEpsilon) {
		return (StdString ("16:9"));
	}
	if (fabs (ratio - (4.0f / 3.0f)) <= Util::aspectRatioMatchEpsilon) {
		return (StdString ("4:3"));
	}
	if (fabs (ratio - (3.0f / 2.0f)) <= Util::aspectRatioMatchEpsilon) {
		return (StdString ("3:2"));
	}
	if (fabs (ratio - (5.0f / 3.0f)) <= Util::aspectRatioMatchEpsilon) {
		return (StdString ("5:3"));
	}
	if (fabs (ratio - (5.0f / 4.0f)) <= Util::aspectRatioMatchEpsilon) {
		return (StdString ("5:4"));
	}
	if (fabs (ratio - (8.0f / 5.0f)) <= Util::aspectRatioMatchEpsilon) {
		return (StdString ("8:5"));
	}
	if (fabs (ratio - 1.0f) <= Util::aspectRatioMatchEpsilon) {
		return (StdString ("1:1"));
	}
	if (fabs (ratio - 1.85f) <= Util::aspectRatioMatchEpsilon) {
		return (StdString ("1.85:1"));
	}
	if (fabs (ratio - 3.0f) <= Util::aspectRatioMatchEpsilon) {
		return (StdString ("3:1"));
	}

	return (StdString (""));
}

StdString Util::getBitrateDisplayString (int64_t bitsPerSecond) {
	if (bitsPerSecond <= 0) {
		return (StdString ("0kbps"));
	}

	if (bitsPerSecond < 1024) {
		return (StdString::createSprintf ("%ibps", (int) bitsPerSecond));
	}

	return (StdString::createSprintf ("%llikbps", (long long int) (bitsPerSecond / 1024)));
}

StdString Util::getFrameSizeName (int width, int height) {
	if ((width == 3840) && (height == 2160)) {
		return (StdString ("4K Ultra HD 1"));
	}
	if ((width == 1920) && (height == 1280)) {
		return (StdString ("Full HD Plus"));
	}
	if ((width == 1920) && (height == 1080)) {
		return (StdString ("Full HD"));
	}
	if ((width == 1600) && (height == 1200)) {
		return (StdString ("Ultra XGA"));
	}
	if ((width == 1600) && (height == 900)) {
		return (StdString ("HD+"));
	}
	if ((width == 1280) && (height == 1024)) {
		return (StdString ("Super XGA"));
	}
	if ((width == 1280) && (height == 720)) {
		return (StdString ("720p HD"));
	}
	if ((width == 1280) && (height == 800)) {
		return (StdString ("Wide XGA"));
	}
	if ((width == 1024) && (height == 768)) {
		return (StdString ("XGA"));
	}
	if ((width == 960) && (height == 540)) {
		return (StdString ("qHD"));
	}
	if ((width == 800) && (height == 600)) {
		return (StdString ("Super VGA"));
	}
	if ((width == 640) && (height == 480)) {
		return (StdString ("VGA"));
	}
	if ((width == 432) && (height == 240)) {
		return (StdString ("Wide QVGA"));
	}
	if ((width == 320) && (height == 240)) {
		return (StdString ("QVGA"));
	}
	if ((width == 320) && (height == 200)) {
		return (StdString ("CGA"));
	}
	if ((width == 240) && (height == 160)) {
		return (StdString ("HQVGA"));
	}
	if ((width == 160) && (height == 120)) {
		return (StdString ("QQVGA"));
	}
	return (StdString (""));
}

StdString Util::getAddressDisplayString (const StdString &address, int defaultPort) {
	StdString s, suffix;

	suffix.sprintf (":%i", defaultPort);
	s.assign (address);
	if (s.endsWith (suffix)) {
		s.assign (s.substr (0, s.length () - suffix.length ()));
	}

	return (s);
}

StdString Util::getAppendPath (const StdString &basePath, const StdString &appendName) {
	StdString s;

	s.assign (basePath);
#if PLATFORM_WINDOWS
	s.appendSprintf ("\\%s", appendName.c_str ());
#else
	s.appendSprintf ("/%s", appendName.c_str ());
#endif
	return (s);
}

int Util::createDirectory (const StdString &path) {
	int result;
#if PLATFORM_LINUX || PLATFORM_MACOS
	struct stat st;

	result = stat (path.c_str (), &st);
	if (result != 0) {
		if (errno != ENOENT) {
			return (Result::ERROR_SYSTEM_OPERATION_FAILED);
		}
	}

	if ((result == 0) && (st.st_mode & S_IFDIR)) {
		return (Result::SUCCESS);
	}

	result = mkdir (path.c_str (), S_IRWXU);
	if (result != 0) {
		return (Result::ERROR_SYSTEM_OPERATION_FAILED);
	}

	result = Result::SUCCESS;
#endif
#if PLATFORM_WINDOWS
	DWORD a;

	a = GetFileAttributes (path.c_str ());
	if (a != INVALID_FILE_ATTRIBUTES) {
		if (a & FILE_ATTRIBUTE_DIRECTORY) {
			return (Result::SUCCESS);
		}
		else {
			return (Result::ERROR_SYSTEM_OPERATION_FAILED);
		}
	}

	if (! CreateDirectory (path.c_str (), NULL)) {
		return (Result::ERROR_SYSTEM_OPERATION_FAILED);
	}

	result = Result::SUCCESS;
#endif

	return (result);
}

bool Util::fileExists (const StdString &path) {
	struct stat st;
	int result;

	result = stat (path.c_str (), &st);
	if (result != 0) {
		return (false);
	}

	return (true);
}

int Util::readFile (const StdString &path, StdString *destString) {
	FILE *fp;
	char data[8192];

	fp = fopen (path.c_str (), "rb");
	if (! fp) {
		return (Result::ERROR_FILE_OPEN_FAILED);
	}

	destString->assign ("");
	while (1) {
		if (! fgets (data, sizeof (data), fp)) {
			break;
		}
		destString->append (data);
	}

	fclose (fp);
	return (Result::SUCCESS);
}

StdString Util::getEnvValue (const StdString &key, const StdString &defaultValue) {
	char *val;

	val = getenv (key.c_str ());
	if (! val) {
		return (defaultValue);
	}

	return (StdString (val));
}

StdString Util::getEnvValue (const StdString &key, const char *defaultValue) {
	char *val;

	val = getenv (key.c_str ());
	if (! val) {
		return (StdString (defaultValue));
	}

	return (StdString (val));
}

bool Util::getEnvValue (const StdString &key, bool defaultValue) {
	char *val;

	val = getenv (key.c_str ());
	if (! val) {
		return (defaultValue);
	}

	if (! strcmp (val, "true")) {
		return (true);
	}

	return (false);
}

int Util::writeValue (SDL_RWops *dest, Uint64 value) {
	char buf[8];
	size_t wlen;

	buf[7] = (char) (value & 0xFF);
	value >>= 8;
	buf[6] = (char) (value & 0xFF);
	value >>= 8;
	buf[5] = (char) (value & 0xFF);
	value >>= 8;
	buf[4] = (char) (value & 0xFF);
	value >>= 8;
	buf[3] = (char) (value & 0xFF);
	value >>= 8;
	buf[2] = (char) (value & 0xFF);
	value >>= 8;
	buf[1] = (char) (value & 0xFF);
	value >>= 8;
	buf[0] = (char) (value & 0xFF);

	wlen = SDL_RWwrite (dest, buf, 8, 1);
	if (wlen < 1) {
		return (Result::ERROR_FILE_OPERATION_FAILED);
	}

	return (Result::SUCCESS);
}

int Util::readValue (SDL_RWops *src, Uint64 *value) {
	char buf[8];
	size_t rlen;
	Uint64 val;

	rlen = SDL_RWread (src, buf, 8, 1);
	if (rlen < 1) {
		return (Result::ERROR_FILE_OPERATION_FAILED);
	}

	val = 0;
	val |= (buf[0] & 0xFF);
	val <<= 8;
	val |= (buf[1] & 0xFF);
	val <<= 8;
	val |= (buf[2] & 0xFF);
	val <<= 8;
	val |= (buf[3] & 0xFF);
	val <<= 8;
	val |= (buf[4] & 0xFF);
	val <<= 8;
	val |= (buf[5] & 0xFF);
	val <<= 8;
	val |= (buf[6] & 0xFF);
	val <<= 8;
	val |= (buf[7] & 0xFF);

	if (value) {
		*value = val;
	}
	return (Result::SUCCESS);
}

int Util::openUrl (const StdString &url) {
	int result;
#if PLATFORM_LINUX
	StdString execfile, execarg, path;
	StringList parts, execnames;
	StringList::iterator i, iend, j, jend;
#endif

	result = Result::ERROR_NOT_IMPLEMENTED;
#if PLATFORM_LINUX
	execfile = Util::getEnvValue ("BROWSER", "");
	if (! execfile.empty ()) {
		execarg.assign (execfile);
	}

	if (execfile.empty ()) {
		execnames.push_back (StdString ("xdg-open"));
		execnames.push_back (StdString ("firefox"));
		execnames.push_back (StdString ("google-chrome"));
		execnames.push_back (StdString ("chromium"));
		execnames.push_back (StdString ("mozilla"));

		path = Util::getEnvValue (StdString ("PATH"), "");
		path.split (":", &parts);

		i = parts.begin ();
		iend = parts.end ();
		while (i != iend) {
			j = execnames.begin ();
			jend = execnames.end ();
			while (j != jend) {
				path.sprintf ("%s/%s", i->c_str (), j->c_str ());
				if (Util::fileExists (path)) {
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
		return (Result::ERROR_PROGRAM_NOT_FOUND);
	}

	if (!(fork ())) {
		execlp (execfile.c_str (), execarg.c_str (), url.c_str (), NULL);
		exit (1);
	}
	result = Result::SUCCESS;
#endif
#if PLATFORM_MACOS
	if (!(fork ())) {
		execlp ("open", "open", url.c_str (), NULL);
		exit (-1);
	}
	result = Result::SUCCESS;
#endif
#if PLATFORM_WINDOWS
	HINSTANCE h;

	result = Result::SUCCESS;
	h = ShellExecute (NULL, "open", url.c_str (), NULL, NULL, SW_SHOWNORMAL);
	if (((int) h) <= 32) {
		result = Result::ERROR_SYSTEM_OPERATION_FAILED;
	}
#endif
	return (result);
}

StdString Util::getHelpUrl (const StdString &topicId) {
	return (StdString::createSprintf ("%si/%s", Util::serverUrl.c_str (), topicId.urlEncoded ().c_str ()));
}

StdString Util::getHelpUrl (const char *topicId) {
	return (Util::getHelpUrl (StdString (topicId)));
}

StdString Util::getApplicationUrl (const StdString &applicationId) {
	return (StdString::createSprintf ("%s%s", Util::serverUrl.c_str (), applicationId.urlEncoded ().c_str ()));
}

StdString Util::getFeatureUrl (const StdString &featureId) {
	return (StdString::createSprintf ("%sfeature/%s", Util::serverUrl.c_str (), featureId.urlEncoded ().c_str ()));
}

StdString Util::getFeedbackUrl (bool shouldIncludeVersion) {
	if (! shouldIncludeVersion) {
		return (StdString::createSprintf ("%scontact", Util::serverUrl.c_str ()));
	}

	return (StdString::createSprintf ("%scontact/%s", Util::serverUrl.c_str (), StdString (BUILD_ID).urlEncoded ().c_str ()));
}

StdString Util::getUpdateUrl (const StdString &applicationId) {
	if (! applicationId.empty ()) {
		return (StdString::createSprintf ("%supdate/%s", Util::serverUrl.c_str (), applicationId.c_str ()));
	}

	return (StdString::createSprintf ("%supdate/%s_%s", Util::serverUrl.c_str (), StdString (BUILD_ID).urlEncoded ().c_str (), StdString (PLATFORM_ID).urlEncoded ().c_str ()));
}

StdString Util::getProtocolString (const StdString &sourceText, const StdString &protocol) {
	StdString text;

	text.assign (sourceText);
	if (! text.contains ("://")) {
		text.insert (0, StdString::createSprintf ("%s://", protocol.c_str ()));
	}

	return (text);
}
