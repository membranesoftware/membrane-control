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
#include <math.h>
#include "StdString.h"
#include "MediaUtil.h"

const float MediaUtil::aspectRatioMatchEpsilon = 0.1f;

StdString MediaUtil::getAspectRatioDisplayString (int width, int height) {
	return (MediaUtil::getAspectRatioDisplayString ((float) width / (float) height));
}

StdString MediaUtil::getAspectRatioDisplayString (float ratio) {
	if (fabs (ratio - (16.0f / 9.0f)) <= MediaUtil::aspectRatioMatchEpsilon) {
		return (StdString ("16:9"));
	}
	if (fabs (ratio - (4.0f / 3.0f)) <= MediaUtil::aspectRatioMatchEpsilon) {
		return (StdString ("4:3"));
	}
	if (fabs (ratio - (3.0f / 2.0f)) <= MediaUtil::aspectRatioMatchEpsilon) {
		return (StdString ("3:2"));
	}
	if (fabs (ratio - (5.0f / 3.0f)) <= MediaUtil::aspectRatioMatchEpsilon) {
		return (StdString ("5:3"));
	}
	if (fabs (ratio - (5.0f / 4.0f)) <= MediaUtil::aspectRatioMatchEpsilon) {
		return (StdString ("5:4"));
	}
	if (fabs (ratio - (8.0f / 5.0f)) <= MediaUtil::aspectRatioMatchEpsilon) {
		return (StdString ("8:5"));
	}
	if (fabs (ratio - 1.0f) <= MediaUtil::aspectRatioMatchEpsilon) {
		return (StdString ("1:1"));
	}
	if (fabs (ratio - 1.85f) <= MediaUtil::aspectRatioMatchEpsilon) {
		return (StdString ("1.85:1"));
	}
	if (fabs (ratio - 3.0f) <= MediaUtil::aspectRatioMatchEpsilon) {
		return (StdString ("3:1"));
	}

	return (StdString (""));
}

StdString MediaUtil::getBitrateDisplayString (int64_t bitsPerSecond) {
	if (bitsPerSecond <= 0) {
		return (StdString ("0kbps"));
	}

	if (bitsPerSecond < 1024) {
		return (StdString::createSprintf ("%ibps", (int) bitsPerSecond));
	}

	return (StdString::createSprintf ("%llikbps", (long long int) (bitsPerSecond / 1024)));
}

StdString MediaUtil::getFrameSizeName (int width, int height) {
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
