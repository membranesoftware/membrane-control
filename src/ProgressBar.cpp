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
#include <math.h>
#include "SDL2/SDL.h"
#include "App.h"
#include "Log.h"
#include "StdString.h"
#include "Ui.h"
#include "Input.h"
#include "Sprite.h"
#include "Widget.h"
#include "UiConfiguration.h"
#include "ProgressBar.h"

const float ProgressBar::AnimationFactor = 2.0f; // milliseconds per pixel

ProgressBar::ProgressBar (float barWidth, float barHeight)
: Widget ()
, isIndeterminate (false)
, progressValue (0.0f)
, targetProgressValue (100.0f)
, fillStage (0)
, fillStart (0.0f)
, fillStartTarget (0.0f)
, fillEnd (0.0f)
, fillEndTarget (0.0f)
{
	width = barWidth;
	height = barHeight;
	bgColor.assign (UiConfiguration::instance->lightPrimaryColor);
	fillColor.animate (UiConfiguration::instance->darkPrimaryColor, UiConfiguration::instance->mediumSecondaryColor, UiConfiguration::instance->longColorAnimateDuration, UiConfiguration::instance->longColorAnimateDuration * 2);
}

ProgressBar::~ProgressBar () {

}

void ProgressBar::setSize (float barWidth, float barHeight) {
	if (FLOAT_EQUALS (width, barWidth) && FLOAT_EQUALS (height, barHeight)) {
		return;
	}
	width = barWidth;
	height = barHeight;
	refreshLayout ();
}

void ProgressBar::setProgress (float value) {
	if (FLOAT_EQUALS (progressValue, value)) {
		return;
	}
	if (value < 0.0f) {
		value = 0.0f;
	}
	progressValue = value;
	refreshLayout ();
}

void ProgressBar::setProgress (float value, float targetValue) {
	if (FLOAT_EQUALS (progressValue, value) && FLOAT_EQUALS (targetProgressValue, targetValue)) {
		return;
	}
	if (value < 0.0f) {
		value = 0.0f;
	}
	if (targetValue < 1.0f) {
		targetValue = 1.0f;
	}
	progressValue = value;
	targetProgressValue = targetValue;
	refreshLayout ();
}

void ProgressBar::setIndeterminate (bool indeterminate) {
	if (indeterminate == isIndeterminate) {
		return;
	}
	isIndeterminate = indeterminate;
	if (isIndeterminate) {
		fillStage = 0;
	}
	refreshLayout ();
}

void ProgressBar::doUpdate (int msElapsed) {
	if (isIndeterminate) {
		switch (fillStage) {
			case 0: {
				fillStart = 0.0f;
				fillStartTarget = 0.0f;
				fillEnd = 0.0f;
				fillEndTarget = (width / 2.0f);
				fillStage = 1;
				break;
			}
			case 1: {
				if (fillEnd < fillEndTarget) {
					fillEnd += ((float) msElapsed) / ProgressBar::AnimationFactor;
					if (fillEnd >= fillEndTarget) {
						fillEnd = fillEndTarget;
					}
				}
				if (fillEnd >= fillEndTarget) {
					fillStartTarget = (width / 2.0f);
					fillEndTarget = width;
					fillStage = 2;
				}
				break;
			}
			case 2: {
				if (fillEnd < fillEndTarget) {
					fillEnd += ((float) msElapsed) / ProgressBar::AnimationFactor;
					if (fillEnd >= fillEndTarget) {
						fillEnd = fillEndTarget;
					}
				}
				if (fillStart < fillStartTarget) {
					fillStart += ((float) msElapsed) / ProgressBar::AnimationFactor;
					if (fillStart >= fillStartTarget) {
						fillStart = fillStartTarget;
					}
				}
				if ((fillEnd >= fillEndTarget) && (fillStart >= fillStartTarget)) {
					fillEnd = width;
					fillStartTarget = width;
					fillStage = 3;
				}
				break;
			}
			case 3: {
				if (fillStart < fillStartTarget) {
					fillStart += ((float) msElapsed) / ProgressBar::AnimationFactor;
					if (fillStart >= fillStartTarget) {
						fillStart = fillStartTarget;
					}
				}
				if (fillStart >= fillStartTarget) {
					fillStage = 4;
				}
				break;
			}
			default: {
				fillStage = 0;
				break;
			}
		}
	}

	fillColor.update (msElapsed);
}

void ProgressBar::doDraw (SDL_Texture *targetTexture, float originX, float originY) {
	SDL_Renderer *render;
	SDL_Rect rect;
	float x1, x2, w;

	render = App::instance->render;
	rect.x = (int) (originX + position.x);
	rect.y = (int) (originY + position.y);
	rect.w = (int) width;
	rect.h = (int) height;
	SDL_SetRenderDrawColor (render, bgColor.rByte, bgColor.gByte, bgColor.bByte, 255);
	SDL_RenderFillRect (render, &rect);

	x1 = floorf (fillStart);
	x2 = floorf (fillEnd);
	w = x2 - x1;
	if (w > 0.0f) {
		rect.x = (int) (originX + position.x + x1);
		rect.w = (int) w;
		SDL_SetRenderDrawColor (render, fillColor.rByte, fillColor.gByte, fillColor.bByte, 255);
		SDL_RenderFillRect (render, &rect);
	}

	SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor (render, 0, 0, 0, 128);
	rect.x = (int) (originX + position.x);
	rect.y = (int) (originY + position.y);
	rect.w = (int) width;
	rect.h = 1;
	SDL_RenderFillRect (render, &rect);
	++(rect.y);
	rect.w = 1;
	rect.h = ((int) height) - 1;
	SDL_RenderFillRect (render, &rect);
	++(rect.x);
	rect.y = (int) (originY + position.y) + ((int) height) - 1;
	rect.w = ((int) width) - 1;
	rect.h = 1;
	SDL_RenderFillRect (render, &rect);
	rect.x = (int) (originX + position.x) + ((int) width) - 1;
	rect.y = (int) (originY + position.y + 1);
	rect.w = 1;
	rect.h = ((int) height) - 2;
	SDL_RenderFillRect (render, &rect);
	SDL_SetRenderDrawBlendMode (render, SDL_BLENDMODE_NONE);

	SDL_SetRenderDrawColor (render, 0, 0, 0, 0);
}

void ProgressBar::refreshLayout () {
	if ((! isIndeterminate) && (targetProgressValue > 0.0f)) {
		fillStart = 0.0f;
		fillEnd = (width * progressValue) / targetProgressValue;
		if (fillEnd > width) {
			fillEnd = width;
		}
	}
}

void ProgressBar::doRefresh () {
	refreshLayout ();
}
