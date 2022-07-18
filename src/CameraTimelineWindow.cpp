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
#include "Config.h"
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <list>
#include "SDL2/SDL.h"
#include "App.h"
#include "StdString.h"
#include "OsUtil.h"
#include "Widget.h"
#include "Color.h"
#include "Label.h"
#include "LabelWindow.h"
#include "Panel.h"
#include "UiConfiguration.h"
#include "CameraTimelineUi.h"
#include "CameraTimelineWindow.h"
#include "Log.h"

CameraTimelineWindow::CameraTimelineWindow (float barWidth)
: Panel ()
, isDisabled (false)
, startTime (0)
, endTime (0)
, highlightTime (-1)
, selectTime (-1)
, timeRate (1)
, hoverPosition (0.0f)
, clickPosition (0.0f)
, barWidth (0.0f)
, barHeight (0.0f)
, barState (NULL)
, barStateSize (0)
, shouldRefreshBarTexture (false)
, barPixels (NULL)
, barSprite (NULL)
, startTimeLabel (NULL)
, endTimeLabel (NULL)
, highlightTimeLabel (NULL)
, highlightMarker (NULL)
, selectTimeLabel (NULL)
, selectMarker (NULL)
{
	startTimeLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->darkBackgroundColor)));
	startTimeLabel->setPadding (UiConfiguration::instance->paddingSize / 2.0f, UiConfiguration::instance->textLineHeightMargin);
	startTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	startTimeLabel->isInputSuspended = true;
	startTimeLabel->zLevel = 3;
	startTimeLabel->isVisible = false;

	endTimeLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->darkBackgroundColor)));
	endTimeLabel->setPadding (UiConfiguration::instance->paddingSize / 2.0, UiConfiguration::instance->textLineHeightMargin);
	endTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	endTimeLabel->isInputSuspended = true;
	endTimeLabel->zLevel = 3;
	endTimeLabel->isVisible = false;

	highlightTimeLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->mediumSecondaryColor)));
	highlightTimeLabel->setPadding (UiConfiguration::instance->paddingSize / 2.0, UiConfiguration::instance->textLineHeightMargin);
	highlightTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	highlightTimeLabel->isInputSuspended = true;
	highlightTimeLabel->zLevel = 3;
	highlightTimeLabel->isVisible = false;

	selectTimeLabel = (LabelWindow *) addWidget (new LabelWindow (new Label (StdString (""), UiConfiguration::CaptionFont, UiConfiguration::instance->darkBackgroundColor)));
	selectTimeLabel->setPadding (UiConfiguration::instance->paddingSize / 2.0, UiConfiguration::instance->textLineHeightMargin);
	selectTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
	selectTimeLabel->isInputSuspended = true;
	selectTimeLabel->zLevel = 3;
	selectTimeLabel->isVisible = false;

	selectMarker = (Panel *) addWidget (new Panel ());
	selectMarker->setFillBg (true, UiConfiguration::instance->darkSecondaryColor);
	selectMarker->isInputSuspended = true;
	selectMarker->zLevel = 1;
	selectMarker->isVisible = false;

	setBarWidth (barWidth);
}

CameraTimelineWindow::~CameraTimelineWindow () {
	if (barState) {
		free (barState);
		barState = NULL;
	}
	if (barPixels) {
		free (barPixels);
		barPixels = NULL;
	}
	if (barSprite) {
		barSprite->unload ();
		delete (barSprite);
		barSprite = NULL;
	}
}

void CameraTimelineWindow::setBarWidth (float barWidthValue) {
	if ((barWidthValue < 1.0f) || FLOAT_EQUALS (barWidth, barWidthValue)) {
		return;
	}
	barWidth = barWidthValue;
	barStateSize = (int) barWidth;
	barState = (uint8_t *) malloc (barStateSize);
	if (! barState) {
		Log::err ("Out of memory in CameraTimelineWindow::setBarWidth; len=%i", barStateSize);
	}
	clearTimeline ();
	refreshLayout ();
}

void CameraTimelineWindow::setDisabled (bool disabled) {
	if (disabled == isDisabled) {
		return;
	}
	isDisabled = disabled;
	if (isDisabled) {
		setFillBg (true, UiConfiguration::instance->lightBackgroundColor.copy (UiConfiguration::instance->buttonDisabledShadeAlpha));
		startTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f));
		endTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f));
		selectTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f));
		selectMarker->setFillBg (true, UiConfiguration::instance->darkSecondaryColor.copy (UiConfiguration::instance->buttonDisabledShadeAlpha));
	}
	else {
		setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		startTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
		endTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
		selectTimeLabel->setFillBg (true, Color (0.0f, 0.0f, 0.0f, UiConfiguration::instance->scrimBackgroundAlpha));
		selectMarker->setFillBg (true, UiConfiguration::instance->darkSecondaryColor);
	}
	refreshLayout ();
}

void CameraTimelineWindow::refreshLayout () {
	barHeight = startTimeLabel->height * 2.0f;
	setFixedSize (true, barWidth, barHeight);

	if ((selectTime >= 0) && (highlightTime < 0)) {
		selectTimeLabel->isVisible = true;
	}
	else {
		selectTimeLabel->isVisible = false;
	}

	if (selectMarker->isVisible) {
		selectMarker->setFixedSize (true, UiConfiguration::instance->timelineMarkerWidth, barHeight - 2.0f);
		selectMarker->position.assignY (1.0f);
	}
	startTimeLabel->position.assign (0.0f, barHeight - startTimeLabel->height);
	endTimeLabel->position.assign (barWidth - endTimeLabel->width, barHeight - endTimeLabel->height);
}

void CameraTimelineWindow::doUpdate (int msElapsed) {
	Panel::doUpdate (msElapsed);
	if (shouldRefreshBarTexture) {
		if (! barPixels) {
			shouldRefreshBarTexture = false;
			if (createBarPixels ()) {
				retain ();
				App::instance->addRenderTask (CameraTimelineWindow::createBarTexture, this);
			}
		}
	}
}

bool CameraTimelineWindow::createBarPixels () {
	Uint32 *dest, color, r, g, b;
	uint8_t *state, level;
	int x, y, w, h;

	if (barPixels) {
		free (barPixels);
		barPixels = NULL;
	}
	w = (int) barWidth;
	h = (int) barHeight;
	barPixels = (Uint32 *) malloc (w * h * sizeof (Uint32));
	if (! barPixels) {
		Log::err ("Failed to create CameraTimelineWindow texture; err=\"Out of memory, bitmap dimensions %ix%i\"", w, h);
		return (false);
	}

	state = barState;
	x = 0;
	while (x < w) {
		level = *state;
		if (level <= 0) {
			r = 16;
			g = 16;
			b = 16;
		}
		else {
			if (level == 1) {
				r = 32;
				g = 32;
				b = 0;
			}
			else {
				r = level;
				r *= (255 - 64);
				r /= 255;
				r += 64;
				if (r > 255) {
					r = 255;
				}
				g = level;
				g *= (255 - 64);
				g /= 255;
				g += 64;
				if (g > 255) {
					g = 255;
				}
				b = level / 8;
			}
		}
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		color = 0x000000FF;
		color |= ((b << 8) & 0x0000FF00);
		color |= ((g << 16) & 0x00FF0000);
		color |= ((r << 24) & 0xFF000000);
#else
		color = 0xFF000000;
		color |= r & 0x000000FF;
		color |= ((g << 8) & 0x0000FF00);
		color |= ((b << 16) & 0x00FF0000);
#endif
		dest = barPixels + x;
		y = 0;
		while (y < h) {
			*dest = color;
			dest += w;
			++y;
		}
		++state;
		++x;
	}
	return (true);
}

void CameraTimelineWindow::createBarTexture (void *windowPtr) {
	CameraTimelineWindow *window;
	SDL_Surface *surface;
	SDL_Texture *texture;
	Sprite *sprite, *oldsprite;
	StdString path;
	Uint32 rmask, gmask, bmask, amask;
	int w, h;

	window = (CameraTimelineWindow *) windowPtr;
	if (window->isDestroyed || (! window->barPixels)) {
		window->release ();
		return;
	}
	w = (int) window->barWidth;
	h = (int) window->barHeight;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xFF000000;
	gmask = 0x00FF0000;
	bmask = 0x0000FF00;
	amask = 0x000000FF;
#else
	rmask = 0x000000FF;
	gmask = 0x0000FF00;
	bmask = 0x00FF0000;
	amask = 0xFF000000;
#endif
	surface = SDL_CreateRGBSurfaceFrom (window->barPixels, w, h, 32, w * sizeof (Uint32), rmask, gmask, bmask, amask);
	if (! surface) {
		Log::err ("Failed to create CameraTimelineWindow texture; err=\"SDL_CreateRGBSurfaceFrom, %s\"", SDL_GetError ());
		free (window->barPixels);
		window->barPixels = NULL;
		window->release ();
		return;
	}
	path.sprintf ("*_CameraTimelineWindow_%llx_%llx", (long long int) window->id, (long long int) App::instance->getUniqueId ());
	texture = Resource::instance->createTexture (path, surface);
	SDL_FreeSurface (surface);
	free (window->barPixels);
	window->barPixels = NULL;
	if (! texture) {
		window->release ();
		return;
	}
	sprite = new Sprite ();
	sprite->addTexture (texture, path);
	oldsprite = window->barSprite;
	window->barSprite = sprite;
	if (oldsprite) {
		oldsprite->unload ();
		delete (oldsprite);
	}
	window->release ();
}

void CameraTimelineWindow::doDraw (SDL_Texture *targetTexture, float originX, float originY) {
	SDL_Rect rect;
	int x0, y0;

	if (barSprite) {
		x0 = (int) (originX + position.x);
		y0 = (int) (originY + position.y);
		rect.x = x0;
		rect.y = y0;
		rect.w = (int) barWidth;
		rect.h = (int) barHeight;
		SDL_RenderCopy (App::instance->render, barSprite->getTexture (0), NULL, &rect);
	}
	Panel::doDraw (targetTexture, originX, originY);
}

bool CameraTimelineWindow::doProcessMouseState (const Widget::MouseState &mouseState) {
	Panel::doProcessMouseState (mouseState);

	if (isDisabled) {
		return (false);
	}
	if (mouseState.isEntered) {
		if (! FLOAT_EQUALS (hoverPosition, mouseState.enterDeltaX)) {
			hoverPosition = mouseState.enterDeltaX;
			eventCallback (positionHoverCallback);
		}
		if (mouseState.isLeftClicked) {
			clickPosition = mouseState.enterDeltaX;
			eventCallback (positionClickCallback);
		}
	}
	else {
		if (hoverPosition >= 0.0f) {
			hoverPosition = -1.0f;
			eventCallback (positionHoverCallback);
		}
	}
	return (false);
}

void CameraTimelineWindow::setTimespan (int64_t spanStartTime, int64_t spanEndTime) {
	if ((spanStartTime <= 0) || (spanEndTime <= 0)) {
		return;
	}
	startTime = spanStartTime;
	endTime = spanEndTime;

	timeRate = 1;
	if (barWidth >= 1.0f) {
		timeRate = (endTime - startTime) / (int64_t) barWidth;
		if (timeRate < 1) {
			timeRate = 1;
		}
	}

	startTimeLabel->setText (OsUtil::getTimestampDisplayString (startTime));
	endTimeLabel->setText (OsUtil::getTimestampDisplayString (endTime));
	startTimeLabel->isVisible = true;
	endTimeLabel->isVisible = true;
	refreshLayout ();
}

void CameraTimelineWindow::setHighlightedTime (int64_t highlightTimeValue) {
	float x;

	if ((width <= 0.0f) || (startTime >= endTime)) {
		return;
	}
	highlightTime = highlightTimeValue;
	if (highlightMarker) {
		highlightMarker->bgColor.translate (UiConfiguration::instance->lightBackgroundColor, UiConfiguration::instance->shortColorTranslateDuration);
		highlightMarker->setDestroyDelay (UiConfiguration::instance->shortColorTranslateDuration);
		highlightMarker = NULL;
	}
	if ((highlightTime < startTime) || (highlightTime > endTime)) {
		highlightTime = -1;
		highlightTimeLabel->isVisible = false;
	}
	else {
		x = (float) (highlightTime - startTime);
		x *= width;
		x /= (float) (endTime - startTime);
		highlightMarker = (Panel *) addWidget (new Panel (), x - (UiConfiguration::instance->timelineMarkerWidth / 2.0f), 1.0f);
		highlightMarker->setFillBg (true, UiConfiguration::instance->lightBackgroundColor);
		highlightMarker->bgColor.translate (UiConfiguration::instance->darkPrimaryColor, UiConfiguration::instance->shortColorTranslateDuration);
		highlightMarker->setFixedSize (true, UiConfiguration::instance->timelineMarkerWidth, barHeight - 2.0f);
		highlightMarker->isInputSuspended = true;
		highlightMarker->zLevel = 2;

		highlightTimeLabel->setText (OsUtil::getTimestampDisplayString (highlightTime));
		highlightTimeLabel->position.assignBounded (x - (highlightTimeLabel->width / 2.0f), 0.0f, 0.0f, 0.0f, width - highlightTimeLabel->width, 0.0f);
		highlightTimeLabel->isVisible = true;
	}

	refreshLayout ();
}

void CameraTimelineWindow::setSelectedTime (int64_t selectTimeValue, bool isSpanDescending) {
	float x;

	if ((width <= 0.0f) || (startTime >= endTime)) {
		return;
	}
	selectTime = selectTimeValue;
	if ((selectTime < startTime) || (selectTime > endTime)) {
		selectTime = -1;
		selectMarker->isVisible = false;
	}
	else {
		x = (float) (selectTime - startTime);
		x *= width;
		x /= (float) (endTime - startTime);
		selectMarker->position.assignX (x - (selectMarker->width / 2.0f));
		selectMarker->isVisible = true;

		if (isSpanDescending) {
			selectTimeLabel->setText (StdString::createSprintf ("< %s", OsUtil::getTimestampDisplayString (selectTime).c_str ()));
		}
		else {
			selectTimeLabel->setText (StdString::createSprintf ("%s >", OsUtil::getTimestampDisplayString (selectTime).c_str ()));
		}
		selectTimeLabel->position.assignBounded (x - (selectTimeLabel->width / 2.0f), 0.0f, 0.0f, 0.0f, width - selectTimeLabel->width, 0.0f);
	}
	refreshLayout ();
}

void CameraTimelineWindow::clearTimeline () {
	int i;

	if ((! barState) || (barStateSize <= 0)) {
		return;
	}
	for (i = 0; i < barStateSize; ++i) {
		barState[i] = 0;
	}
	shouldRefreshBarTexture = true;
}

void CameraTimelineWindow::addTimelinePoint (int64_t t) {
	float delta, pos;
	uint8_t *d;

	delta = endTime - startTime;
	if ((! barState) || (barStateSize <= 0) || (delta <= 0.0f) || (t < startTime) || (t > endTime)) {
		return;
	}
	pos = ((t - startTime) / delta) * (float) barStateSize;
	if (pos < 0) {
		pos = 0;
	}
	if (pos > (barStateSize - 1)) {
		pos = barStateSize - 1;
	}

	d = barState + (int) pos;
	if (*d < 255) {
		++(*d);
		shouldRefreshBarTexture = true;
	}
}

void CameraTimelineWindow::addTimelineCoverRange (int64_t rangeMin, int64_t rangeMax) {
	float delta, pos1, pos2;
	uint8_t *d, *end;
	bool found;

	if (rangeMin < startTime) {
		rangeMin = startTime;
	}
	if (rangeMin > endTime) {
		rangeMin = endTime;
	}
	if (rangeMax < startTime) {
		rangeMax = startTime;
	}
	if (rangeMax > endTime) {
		rangeMax = endTime;
	}
	if (rangeMax < rangeMin) {
		rangeMax = rangeMin;
	}
	delta = endTime - startTime;
	if ((! barState) || (barStateSize <= 0) || (delta <= 0.0f)) {
		return;
	}
	pos1 = ((rangeMin - startTime) / delta) * (float) barStateSize;
	pos2 = ((rangeMax - startTime) / delta) * (float) barStateSize;
	if (pos1 < 0) {
		pos1 = 0;
	}
	if (pos1 > (barStateSize - 1)) {
		pos1 = barStateSize - 1;
	}
	if (pos2 < 0) {
		pos2 = 0;
	}
	if (pos2 > barStateSize) {
		pos2 = barStateSize;
	}
	found = false;
	d = barState + (int) pos1;
	if ((*d) <= 0) {
		*d = 1;
		found = true;
	}

	++d;
	end = barState + (int) pos2;
	while (d < end) {
		if ((*d) <= 0) {
			*d = 1;
			found = true;
		}
		++d;
	}
	if (found) {
		shouldRefreshBarTexture = true;
	}
}
