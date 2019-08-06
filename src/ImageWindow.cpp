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
#include <math.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "Result.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "Sprite.h"
#include "Image.h"
#include "ImageWindow.h"

ImageWindow::ImageWindow (Image *image)
: Panel ()
, image (image)
, isWindowSizeEnabled (false)
, windowWidth (0.0f)
, windowHeight (0.0f)
, isLoadResizeEnabled (false)
, loadWidth (0.0f)
, loadSprite (NULL)
, isImageResourceLoaded (false)
, isLoadingImageResource (false)
, imageResourceData (NULL)
, isImageUrlLoaded (false)
, isLoadingImageUrl (false)
, isImageUrlLoadDisabled (false)
, shouldInvokeLoadCallback (false)
, loadCallback (NULL)
, loadCallbackData (NULL)
{
	if (image) {
		addWidget (image);
	}
}

ImageWindow::~ImageWindow () {
	if (imageResourceData) {
		imageResourceData->release ();
		imageResourceData = NULL;
	}
}

StdString ImageWindow::toStringDetail () {
	StdString s;

	s.assign (" ImageWindow");
	if (! imageResourcePath.empty ()) {
		s.appendSprintf (" imageResourcePath=\"%s\"", imageResourcePath.c_str ());
	}
	if (! imageUrl.empty ()) {
		s.appendSprintf (" imageUrl=\"%s\"", imageUrl.c_str ());
	}

	return (s);
}

void ImageWindow::setLoadCallback (Widget::EventCallback callback, void *callbackData) {
	loadCallback = callback;
	loadCallbackData = callbackData;
}

void ImageWindow::setPadding (float widthPadding, float heightPadding) {
	Panel::setPadding (widthPadding, heightPadding);
	refreshLayout ();
}

bool ImageWindow::isLoaded () {
	if (! image) {
		return (false);
	}

	return (isImageResourceLoaded || isImageUrlLoaded);
}

bool ImageWindow::shouldShowUrlImage () {
	float w, h;

	if (imageUrl.empty () || isImageUrlLoadDisabled || (! isVisible)) {
		return (false);
	}

	// TODO: Possibly use a different draw boundary here (currently using 2x the app window dimensions)
	w = ((float) App::instance->windowWidth) * 2.0f;
	h = ((float) App::instance->windowHeight) * 2.0f;
	if ((screenX >= -w) && (screenX <= w)) {
		if ((screenY >= -h) && (screenY <= h)) {
			return (true);
		}
	}

	return (false);
}

void ImageWindow::setWindowSize (float windowSizeWidth, float windowSizeHeight) {
	isWindowSizeEnabled = true;
	windowWidth = windowSizeWidth;
	windowHeight = windowSizeHeight;
	setFixedSize (true, windowWidth, windowHeight);
	refreshLayout ();
}

void ImageWindow::setImage (Image *nextImage) {
	if (image) {
		image->zLevel = -1;
		image->setDestroyDelay (1);
	}
	image = nextImage;
	if (image) {
		addWidget (image);
	}
	refreshLayout ();
}

void ImageWindow::setFrame (int frame) {
	if (! image) {
		return;
	}

	image->setFrame (frame);
	refreshLayout ();
}

void ImageWindow::setScale (float scale) {
	if (! image) {
		return;
	}

	image->setScale (scale);
	refreshLayout ();
}

void ImageWindow::setLoadSprite (Sprite *sprite) {
	loadSprite = sprite;
	if (loadSprite && (! isImageUrlLoaded) && isLoadingImageUrl) {
		setImage (new Image (loadSprite));
	}
}

void ImageWindow::setLoadUrl (const StdString &loadUrl) {
	if (imageUrl.equals (loadUrl)) {
		return;
	}
	imageUrl.assign (loadUrl);
	if (loadSprite) {
		setImage (new Image (loadSprite));
	}
	isImageUrlLoaded = false;
	isLoadingImageUrl = false;
	isImageUrlLoadDisabled = false;
}

void ImageWindow::setLoadResize (bool enable, float loadWidthValue) {
	if (enable && (loadWidthValue >= 1.0f)) {
		isLoadResizeEnabled = true;
		loadWidth = loadWidthValue;
	}
	else {
		isLoadResizeEnabled = false;
	}
}

void ImageWindow::setLoadResourcePath (const StdString &loadPath, bool shouldLoadNow) {
	SDL_Surface *surface;
	SDL_Texture *texture;
	Sprite *sprite;

	if (imageResourcePath.equals (loadPath)) {
		return;
	}
	imageResourcePath.assign (loadPath);
	if (! shouldLoadNow) {
		isImageResourceLoaded = false;
		return;
	}

	surface = App::instance->resource.loadSurface (imageResourcePath);
	if (! surface) {
		imageResourcePath.assign ("");
		return;
	}

	texture = App::instance->resource.createTexture (imageResourcePath, surface);
	SDL_FreeSurface (surface);
	if (! texture) {
		imageResourcePath.assign ("");
		return;
	}

	sprite = new Sprite ();
	sprite->addTexture (texture, imageResourcePath);
	setImage (new Image (sprite, 0, true));
	isImageResourceLoaded = true;
	isLoadingImageResource = false;
	refreshLayout ();

	if (loadCallback) {
		shouldInvokeLoadCallback = true;
	}
}

bool ImageWindow::isLoadUrlEmpty () {
	return (imageUrl.empty ());
}

void ImageWindow::reload () {
	if (isImageUrlLoaded) {
		if (loadSprite) {
			setImage (new Image (loadSprite));
		}
		isImageUrlLoaded = false;
	}
}

void ImageWindow::refreshLayout () {
	if (isWindowSizeEnabled) {
		if (image) {
			image->position.assign ((windowWidth / 2.0f) - (image->width / 2.0f), (windowHeight / 2.0f) - (image->height / 2.0f));
		}
	}
	else {
		if (isLoadResizeEnabled) {
			if (image && loadSprite && image->hasSprite (loadSprite)) {
				image->position.assign ((width / 2.0f) - (image->width / 2.0f), (height / 2.0f) - (image->height / 2.0f));
			}
		}
		else {
			if (image) {
				image->position.assign (widthPadding, heightPadding);
			}
			resetSize ();
		}
	}
}

void ImageWindow::doUpdate (int msElapsed) {
	bool shouldload;

	Panel::doUpdate (msElapsed);
	if (! imageResourcePath.empty ()) {
		shouldload = false;
		if ((! isImageResourceLoaded) && (! isLoadingImageResource)) {
			shouldload = true;
		}

		if (shouldload) {
			loadImageResource ();
		}
	}
	else if (! imageUrl.empty ()) {
		shouldload = shouldShowUrlImage ();
		if (shouldload) {
			if ((! isImageUrlLoaded) && (! isLoadingImageUrl)) {
				requestImage ();
			}
		}
		else {
			if (isImageUrlLoaded && (! isLoadingImageUrl)) {
				if (loadSprite) {
					setImage (new Image (loadSprite));
				}
				isImageUrlLoaded = false;
			}
		}
	}

	if (shouldInvokeLoadCallback) {
		shouldInvokeLoadCallback = false;
		if (loadCallback) {
			loadCallback (loadCallbackData, this);
		}
	}
}

void ImageWindow::loadImageResource () {
	if (isLoadingImageResource || imageResourcePath.empty ()) {
		return;
	}
	isLoadingImageResource = true;
	retain ();

	App::instance->addRenderTask (ImageWindow::createResourcePathTexture, this);
}

void ImageWindow::endLoadImageResource (bool clearResourcePath) {
	if (clearResourcePath) {
		imageResourcePath.assign ("");
	}
	isLoadingImageResource = false;
	if (loadCallback && isLoaded ()) {
		shouldInvokeLoadCallback = true;
	}
	release ();
}

void ImageWindow::createResourcePathTexture (void *windowPtr) {
	ImageWindow *window;
	SDL_Surface *surface;
	SDL_Texture *texture;
	Sprite *sprite;

	window = (ImageWindow *) windowPtr;
	if (window->isDestroyed) {
		window->endLoadImageResource ();
		return;
	}

	surface = App::instance->resource.loadSurface (window->imageResourcePath);
	if (! surface) {
		window->endLoadImageResource (true);
		return;
	}

	texture = App::instance->resource.createTexture (window->imageResourcePath, surface);
	SDL_FreeSurface (surface);
	if (! texture) {
		window->endLoadImageResource (true);
		return;
	}

	sprite = new Sprite ();
	sprite->addTexture (texture, window->imageResourcePath);
	window->setImage (new Image (sprite, 0, true));
	window->isImageResourceLoaded = true;
	window->refreshLayout ();
	window->endLoadImageResource ();
}

void ImageWindow::requestImage () {
	if (isLoadingImageUrl) {
		return;
	}

	isLoadingImageUrl = true;

	retain ();
	App::instance->network.sendHttpGet (imageUrl, ImageWindow::getImageComplete, this);
}

void ImageWindow::endRequestImage (bool disableLoad) {
	isImageUrlLoadDisabled = disableLoad;
	if (imageResourceData) {
		imageResourceData->release ();
		imageResourceData = NULL;
	}
	isLoadingImageUrl = false;
	if (loadCallback && isLoaded ()) {
		shouldInvokeLoadCallback = true;
	}
	release ();
}

void ImageWindow::getImageComplete (void *windowPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData) {
	ImageWindow *window;

	window = (ImageWindow *) windowPtr;
	if (window->isDestroyed || (! window->shouldShowUrlImage ())) {
		window->endRequestImage ();
		return;
	}

	if ((! responseData) || responseData->empty ()) {
		Log::warning ("Failed to get thumbnail image; targetUrl=\"%s\" statusCode=%i err=\"No response data\"", targetUrl.c_str (), statusCode);
		window->endRequestImage (true);
		return;
	}

	if (window->imageResourceData) {
		window->imageResourceData->release ();
	}
	window->imageResourceData = responseData;
	window->imageResourceData->retain ();
	App::instance->addRenderTask (ImageWindow::createResourceDataTexture, window);
}

void ImageWindow::createResourceDataTexture (void *windowPtr) {
	ImageWindow *window;
	SDL_RWops *rw;
	SDL_Surface *surface, *scaledsurface;
	SDL_Texture *texture;
	Sprite *sprite;
	StdString path;
	float scaledw, scaledh;

	window = (ImageWindow *) windowPtr;
	if (window->isDestroyed || (! window->shouldShowUrlImage ()) || (! window->imageResourceData)) {
		window->endRequestImage ();
		return;
	}

	rw = SDL_RWFromConstMem (window->imageResourceData->data, window->imageResourceData->length);
	if (! rw) {
		Log::warning ("Failed to create image window texture; err=\"SDL_RWFromConstMem: %s\"", SDL_GetError ());
		window->endRequestImage (true);
		return;
	}
	surface = IMG_Load_RW (rw, 1);
	if (! surface) {
		Log::warning ("Failed to create image window texture; err=\"IMG_Load_RW: %s\"", IMG_GetError ());
		window->endRequestImage (true);
		return;
	}

	if (window->isLoadResizeEnabled && (surface->w > 0)) {
		scaledw = window->loadWidth;
		scaledh = (float) surface->h;
		scaledh *= window->loadWidth;
		scaledh /= (float) surface->w;
	}
	else {
		scaledw = window->width;
		scaledh = window->height;
	}

	if (scaledw < 1.0f) {
		scaledw = 1.0f;
	}
	if (scaledh < 1.0f) {
		scaledh = 1.0f;
	}
	scaledsurface = SDL_CreateRGBSurface (0, (int) floorf (scaledw), (int) floorf (scaledh), surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
	if (scaledsurface) {
		SDL_BlitScaled (surface, NULL, scaledsurface, NULL);
	}
	SDL_FreeSurface (surface);
	if (! scaledsurface) {
		Log::warning ("Failed to create image window texture; err=\"SDL_BlitScaled: %s\"", SDL_GetError ());
		window->endRequestImage (true);
		return;
	}

	path.sprintf ("*_ImageWindow_%llx_%llx", (long long int) window->id, (long long int) App::instance->getUniqueId ());
	texture = App::instance->resource.createTexture (path, scaledsurface);
	SDL_FreeSurface (scaledsurface);
	if (! texture) {
		window->endRequestImage (true);
		return;
	}

	sprite = new Sprite ();
	sprite->addTexture (texture, path);
	window->setImage (new Image (sprite, 0, true));
	window->isImageUrlLoaded = true;
	if (window->isLoadResizeEnabled) {
		window->setFixedSize (true, scaledw, scaledh);
	}
	window->refreshLayout ();
	window->endRequestImage ();
}
