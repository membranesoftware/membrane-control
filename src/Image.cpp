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
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "SDL2/SDL.h"
#include "App.h"
#include "Log.h"
#include "StdString.h"
#include "Sprite.h"
#include "SpriteHandle.h"
#include "Widget.h"
#include "Image.h"

Image::Image (Sprite *sprite, int spriteFrame, bool shouldDestroySprite)
: Widget ()
, drawAlpha (1.0f)
, maxSpriteWidth (0.0f)
, maxSpriteHeight (0.0f)
, drawScale (1.0f)
, isDrawColorEnabled (false)
, spriteHandle (sprite)
, shouldDestroySprite (shouldDestroySprite)
, isMouseHighlightScaled (false)
, mouseHighlightScale (1.0f)
{
	spriteHandle.frame = spriteFrame;
	maxSpriteWidth = (float) sprite->maxWidth;
	maxSpriteHeight = (float) sprite->maxHeight;
	resetSize ();
}

Image::~Image () {
	if (shouldDestroySprite) {
		spriteHandle.sprite->unload ();
		delete (spriteHandle.sprite);
		spriteHandle.sprite = NULL;
	}
}

StdString Image::toStringDetail () {
	return (StdString::createSprintf (" x=%.2f y=%.2f w=%.2f h=%.2f", position.x, position.y, width, height));
}

bool Image::hasSprite (Sprite *sprite) const {
	return (spriteHandle.sprite == sprite);
}

void Image::setScale (float scale) {
	if (scale <= 0.0f) {
		return;
	}

	drawScale = scale;
	resetSize ();
}

void Image::setFrame (int frame) {
	if ((spriteHandle.frame == frame) || (frame < 0) || (frame >= spriteHandle.sprite->frameCount)) {
		return;
	}

	spriteHandle.frame = frame;
	resetSize ();
}

void Image::setMouseHighlightScale (bool enable, float highlightScale) {
	isMouseHighlightScaled = enable;
	if (isMouseHighlightScaled) {
		mouseHighlightScale = highlightScale;
	}
}

void Image::setSpriteDestroy (bool enable) {
	shouldDestroySprite = enable;
}

void Image::setDrawColor (bool enable, const Color &color) {
	if (isDrawColorEnabled == enable) {
		return;
	}

	isDrawColorEnabled = enable;
	if (isDrawColorEnabled) {
		drawColor.assign (color);
	}
}

void Image::translateAlpha (float startAlpha, float targetAlpha, int durationMs) {
	if (startAlpha < 0.0f) {
		startAlpha = 0.0f;
	}
	else if (startAlpha > 1.0f) {
		startAlpha = 1.0f;
	}
	if (targetAlpha < 0.0f) {
		targetAlpha = 0.0f;
	}
	else if (targetAlpha > 1.0f) {
		targetAlpha = 1.0f;
	}

	drawAlpha = startAlpha;
	translateAlphaValue.translateX (startAlpha, targetAlpha, durationMs);
}

void Image::resetSize () {
	SDL_Texture *texture;
	int tw, th;

	texture = spriteHandle.getTexture (&tw, &th);
	if (! texture) {
		width = 0.0f;
		height = 0.0f;
		return;
	}

	width = ((float) tw) * drawScale;
	height = ((float) th) * drawScale;
}

void Image::doUpdate (int msElapsed) {
	if (isMouseHighlightScaled) {
		if (isMouseEntered) {
			if (! FLOAT_EQUALS (drawScale, mouseHighlightScale)) {
				setScale (mouseHighlightScale);
			}
		}
		else {
			if (! FLOAT_EQUALS (drawScale, 1.0f)) {
				setScale (1.0f);
			}
		}
	}

	if (translateAlphaValue.isTranslating) {
		translateAlphaValue.update (msElapsed);
		drawAlpha = translateAlphaValue.x;
	}
}

void Image::doRefresh () {
	maxSpriteWidth = (float) spriteHandle.sprite->maxWidth;
	maxSpriteHeight = (float) spriteHandle.sprite->maxHeight;
	resetSize ();
}

void Image::doDraw (SDL_Texture *targetTexture, float originX, float originY) {
	SDL_Texture *texture;
	SDL_Rect rect;

	texture = spriteHandle.getTexture ();
	rect.x = (int) (originX + position.x);
	rect.y = (int) (originY + position.y);
	rect.w = (int) width;
	rect.h = (int) height;

	SDL_SetTextureAlphaMod (texture, (Uint8) (drawAlpha * 255.0f));
	if (isDrawColorEnabled) {
		SDL_SetTextureColorMod (texture, drawColor.rByte, drawColor.gByte, drawColor.bByte);
	}
	SDL_RenderCopy (App::instance->render, texture, NULL, &rect);
	if (isDrawColorEnabled) {
		SDL_SetTextureColorMod (texture, 255, 255, 255);
	}
}
