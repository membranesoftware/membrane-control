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
#include "Config.h"
#include <stdlib.h>
#include <list>
#include "SDL2/SDL.h"
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Util.h"
#include "Widget.h"
#include "Color.h"
#include "Sprite.h"
#include "Label.h"
#include "Image.h"
#include "Ui.h"
#include "UiConfiguration.h"
#include "Slider.h"

Slider::Slider (float minValue, float maxValue)
: Widget ()
, value (minValue)
, hoverValue (0.0f)
, minValue (minValue)
, maxValue (maxValue)
, isHovering (false)
, isDragging (false)
, thumbSize (0.0f)
, isThumbSpriteLoaded (false)
, isThumbSpriteLoading (false)
, thumbSprite (NULL)
, thumbSpritePixels (NULL)
, trackWidth (0.0f)
, trackHeight (0.0f)
, hoverSize (0.0f)
, valueChangeCallback (NULL)
, valueChangeCallbackData (NULL)
, valueHoverCallback (NULL)
, valueHoverCallbackData (NULL)
{
	UiConfiguration *uiconfig;

	typeName.assign ("Slider");

	uiconfig = &(App::getInstance ()->uiConfig);
	if (maxValue < minValue) {
		maxValue = minValue;
	}
	thumbSize = uiconfig->sliderThumbSize;
	thumbColor.assign (uiconfig->lightPrimaryColor);
	trackWidth = uiconfig->sliderTrackWidth;
	trackHeight = uiconfig->sliderTrackHeight;
	trackColor.assign (uiconfig->darkPrimaryColor);
	hoverSize = uiconfig->sliderThumbSize;
	hoverColor.assign (uiconfig->lightPrimaryColor);
	resetLayout ();
}

Slider::~Slider () {
	if (thumbSprite) {
		thumbSprite->unload ();
		delete (thumbSprite);
		thumbSprite = NULL;
	}

	if (thumbSpritePixels) {
		free (thumbSpritePixels);
		thumbSpritePixels = NULL;
	}
}

void Slider::resetLayout () {
	width = trackWidth;
	height = thumbSize;
}

float Slider::getSnappedValue (float targetValue) {
	std::list<float>::iterator i, end;
	float dist, mindist, val;

	if (targetValue < minValue) {
		targetValue = minValue;
	}
	if (targetValue > maxValue) {
		targetValue = maxValue;
	}

	if (! snapValueList.empty ()) {
		mindist = -1.0f;
		val = 0.0f;
		i = snapValueList.begin ();
		end = snapValueList.end ();
		while (i != end) {
			dist = fabs (targetValue - *i);
			if ((mindist < 0.0f) || (dist < mindist)) {
				mindist = dist;
				val = *i;
			}
			++i;
		}
		targetValue = val;
	}

	return (targetValue);
}

void Slider::setValue (float sliderValue) {
	sliderValue = getSnappedValue (sliderValue);
	if (FLOAT_EQUALS (value, sliderValue)) {
		return;
	}
	value = sliderValue;
	if (valueChangeCallback) {
		valueChangeCallback (valueChangeCallbackData, this);
	}
}

void Slider::setValueChangeCallback (Widget::EventCallback callback, void *callbackData) {
	valueChangeCallback = callback;
	valueChangeCallbackData = callbackData;
}

void Slider::setValueHoverCallback (Widget::EventCallback callback, void *callbackData) {
	valueHoverCallback = callback;
	valueHoverCallbackData = callbackData;
}

void Slider::addSnapValue (float snapValue) {
	std::list<float>::iterator i, end, pos;

	if (snapValue < minValue) {
		snapValue = minValue;
	}
	if (snapValue > maxValue) {
		snapValue = maxValue;
	}
	i = snapValueList.begin ();
	end = snapValueList.end ();
	pos = end;
	while (i != end) {
		if (snapValue <= *i) {
			pos = i;
			break;
		}
		++i;
	}

	if (pos == end) {
		snapValueList.push_back (snapValue);
	}
	else {
		snapValueList.insert (pos, snapValue);
	}
}

void Slider::doUpdate (int msElapsed, float originX, float originY) {
	if (!(isThumbSpriteLoaded || isThumbSpriteLoading)) {
		loadThumbSprite ();
	}
}

void Slider::doDraw () {
	App *app;
	SDL_Rect rect;
	SDL_Texture *texture;
	float w, h;

	if (! isThumbSpriteLoaded) {
		return;
	}

	app = App::getInstance ();

	h = (thumbSize - trackHeight) / 2.0f;
	rect.x = (int) drawX;
	rect.y = (int) (drawY + h);
	rect.w = (int) trackWidth;
	rect.h = (int) trackHeight;
	SDL_SetRenderDrawColor (app->render, trackColor.rByte, trackColor.gByte, trackColor.bByte, 255);
	SDL_RenderFillRect (app->render, &rect);

	if (isHovering && (! isDragging) && (! FLOAT_EQUALS (hoverValue, value))) {
		rect.x = (int) drawX;
		if (maxValue > minValue) {
			w = width - hoverSize;
			rect.x += (int) (w * ((hoverValue - minValue) / (maxValue - minValue)));
		}
		rect.y = (int) (drawY + h);
		rect.w = (int) hoverSize;
		rect.h = (int) trackHeight;
		SDL_SetRenderDrawColor (app->render, hoverColor.rByte, hoverColor.gByte, hoverColor.bByte, 255);
		SDL_RenderFillRect (app->render, &rect);
	}

	rect.x = (int) drawX;
	if (maxValue > minValue) {
		w = width - thumbSize;
		rect.x += (int) (w * ((value - minValue) / (maxValue - minValue)));
	}
	rect.y = (int) drawY;
	rect.w = (int) thumbSize;
	rect.h = (int) thumbSize;
	if (thumbSprite) {
		texture = thumbSprite->getTexture (0);
		SDL_SetTextureColorMod (texture, thumbColor.rByte, thumbColor.gByte, thumbColor.bByte);
		SDL_SetTextureBlendMode (texture, SDL_BLENDMODE_BLEND);
		SDL_RenderCopy (app->render, texture, NULL, &rect);
	}
	else {
		SDL_SetRenderDrawColor (app->render, thumbColor.rByte, thumbColor.gByte, thumbColor.bByte, 255);
		SDL_RenderFillRect (app->render, &rect);
	}
}

void Slider::doProcessMouseState (const Widget::MouseState &mouseState) {
	Input *input;
	float val, dx;
	bool firsthover;

	input = &(App::getInstance ()->input);
	firsthover = false;
	if (mouseState.isEntered) {
		if (isDragging) {
			if (! input->isMouseLeftButtonDown) {
				isDragging = false;
			}
		}
		else {
			if (mouseState.isLeftClicked) {
				isDragging = true;
			}
		}

		if (! isHovering) {
			if (! isDragging) {
				isHovering = true;
				firsthover = true;
			}
		}
	}
	else {
		if (isDragging) {
			if (! input->isMouseLeftButtonDown) {
				isDragging = false;
			}
		}

		if (isHovering) {
			isHovering = false;
			if (valueHoverCallback) {
				valueHoverCallback (valueHoverCallbackData, this);
			}
		}
	}

	if (isDragging || isHovering) {
		dx = ((float) input->mouseX) - drawX;
		if (dx < 0.0f) {
			dx = 0.0f;
		}
		if (dx > width) {
			dx = width;
		}
		val = (dx / width);
		val *= (maxValue - minValue);
		val += minValue;

		if (isDragging) {
			setValue (val);
		}
		else {
			if (firsthover || (! FLOAT_EQUALS (val, hoverValue))) {
				hoverValue = getSnappedValue (val);
				if (valueHoverCallback) {
					valueHoverCallback (valueHoverCallbackData, this);
				}
			}
		}
	}
}

void Slider::doRefresh () {
	UiConfiguration *uiconfig;

	uiconfig = &(App::getInstance ()->uiConfig);
	thumbSize = uiconfig->sliderThumbSize;
	thumbColor.assign (uiconfig->lightPrimaryColor);
	trackWidth = uiconfig->sliderTrackWidth;
	trackHeight = uiconfig->sliderTrackHeight;
	trackColor.assign (uiconfig->darkPrimaryColor);
	hoverSize = uiconfig->sliderThumbSize;
	hoverColor.assign (uiconfig->lightPrimaryColor);
	resetLayout ();
}

void Slider::doResetInputState () {
	isDragging = false;
	isHovering = false;
}

void Slider::loadThumbSprite () {
	App *app;
	SDL_Surface *surface;
	Uint32 *dest, color, rmask, gmask, bmask, amask;
	int x, y, w, h;
	float dist, radius, targetalpha, minalpha, opacity;
	uint8_t alpha;

	app = App::getInstance ();
	w = (int) thumbSize;
	h = (int) thumbSize;
	if ((w <= 0) || (h <= 0)) {
		return;
	}

	isThumbSpriteLoading = true;
	if (thumbSpritePixels) {
		free (thumbSpritePixels);
	}
	thumbSpritePixels = (Uint32 *) malloc (w * h * sizeof (Uint32));
	if (! thumbSpritePixels) {
		Log::write (Log::WARNING, "Failed to create slider texture; err=\"Out of memory, bitmap dimensions %ix%i\"", w, h);
		isThumbSpriteLoaded = true;
		return;
	}

	minalpha = 0.0f;
	opacity = 3.6f;
	dest = thumbSpritePixels;
	radius = thumbSize / 2.0f;
	y = 0;
	while (y < h) {
		x = 0;
		while (x < w) {
			dist = Util::getDistance (((float) x) + 0.5f, ((float) y) + 0.5f, radius + 0.5f, radius + 0.5f);
			targetalpha = 1.0f - ((1.0f - minalpha) * (dist / radius));
			if (targetalpha <= 0.0f) {
				targetalpha = 0.0f;
			}
			else {
				targetalpha *= opacity;
				if (targetalpha > 1.0f) {
					targetalpha = 1.0f;
				}
			}
			alpha = (uint8_t) (targetalpha * 255.0f);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			color = 0xFFFFFF00 | (alpha & 0xFF);
#else
			color = 0x00FFFFFF | (((Uint32) (alpha & 0xFF)) << 24);
#endif
			*dest = color;
			++dest;
			++x;
		}

		++y;
	}

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
	surface = SDL_CreateRGBSurfaceFrom (thumbSpritePixels, w, h, 32, w * sizeof (Uint32), rmask, gmask, bmask, amask);
	if (! surface) {
		Log::write (Log::WARNING, "Failed to create slider texture; err=\"SDL_CreateRGBSurfaceFrom, %s\"", SDL_GetError ());
		free (thumbSpritePixels);
		thumbSpritePixels = NULL;
		isThumbSpriteLoaded = true;
		return;
	}

	retain ();
	app->createResourceTexture (StdString::createSprintf ("*_Slider_%llx_%llx", (long long int) id, (long long int) app->getUniqueId ()), surface, Slider::createThumbTextureComplete, this);
}

void Slider::createThumbTextureComplete (void *sliderPtr, const StdString &path, SDL_Surface *surface, SDL_Texture *texture) {
	Slider *slider;
	Sprite *sprite;

	slider = (Slider *) sliderPtr;

	sprite = new Sprite ();
	sprite->addTexture (texture, path);
	if (surface) {
		SDL_FreeSurface (surface);
	}
	if (slider->thumbSpritePixels) {
		free (slider->thumbSpritePixels);
		slider->thumbSpritePixels = NULL;
	}
	if (slider->thumbSprite) {
		slider->thumbSprite->unload ();
		delete (slider->thumbSprite);
	}
	slider->thumbSprite = sprite;
	slider->isThumbSpriteLoaded = true;
	slider->isThumbSpriteLoading = false;
	slider->release ();
}
