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
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "StdString.h"
#include "App.h"
#include "Network.h"
#include "Resource.h"
#include "Widget.h"
#include "Color.h"
#include "Panel.h"
#include "Label.h"
#include "Sprite.h"
#include "Image.h"
#include "ImageWindow.h"
#include "Log.h"

const float ImageWindow::UrlImageShowAreaMultiplier = 2.0f;

ImageWindow::ImageWindow (Image *image)
: Panel ()
, imageLoadSourceWidth (0.0f)
, imageLoadSourceHeight (0.0f)
, image (image)
, isWindowSizeEnabled (false)
, windowWidth (0.0f)
, windowHeight (0.0f)
, loadingSprite (NULL)
, loadingWidth (0.0f)
, loadingHeight (0.0f)
, isImageFileExternal (false)
, isImageFileLoaded (false)
, isLoadingImageFile (false)
, imageUrlData (NULL)
, isImageUrlLoaded (false)
, isLoadingImageUrl (false)
, isImageUrlLoadDisabled (false)
, shouldInvokeLoadCallback (false)
, onLoadResizeType (0)
, onLoadWidth (0.0f)
, onLoadHeight (0.0f)
{
	if (image) {
		addWidget (image);
	}
}

ImageWindow::~ImageWindow () {
	if (imageUrlData) {
		imageUrlData->release ();
		imageUrlData = NULL;
	}
}

StdString ImageWindow::toStringDetail () {
	StdString s;

	s.assign (" ImageWindow");
	if (! imageFilePath.empty ()) {
		s.appendSprintf (" imageFilePath=\"%s\"", imageFilePath.c_str ());
	}
	if (! imageUrl.empty ()) {
		s.appendSprintf (" imageUrl=\"%s\"", imageUrl.c_str ());
	}
	return (s);
}

void ImageWindow::setPadding (float widthPadding, float heightPadding) {
	Panel::setPadding (widthPadding, heightPadding);
	refreshLayout ();
}

void ImageWindow::setWindowSize (bool enable, float windowSizeWidth, float windowSizeHeight) {
	if (! enable) {
		isWindowSizeEnabled = false;
	}
	else {
		if ((windowSizeWidth >= 1.0f) && (windowSizeHeight >= 1.0f)) {
			isWindowSizeEnabled = true;
			windowWidth = windowSizeWidth;
			windowHeight = windowSizeHeight;
		}
	}
	refreshLayout ();
}

bool ImageWindow::isLoaded () {
	if (! image) {
		return (false);
	}
	return (isImageFileLoaded || isImageUrlLoaded);
}

bool ImageWindow::shouldShowUrlImage () {
	float w, h;

	if (imageUrl.empty () || isImageUrlLoadDisabled || (! isVisible)) {
		return (false);
	}
	w = ((float) App::instance->windowWidth) * ImageWindow::UrlImageShowAreaMultiplier;
	h = ((float) App::instance->windowHeight) * ImageWindow::UrlImageShowAreaMultiplier;
	if ((screenX >= -w) && (screenX <= w)) {
		if ((screenY >= -h) && (screenY <= h)) {
			return (true);
		}
	}
	return (false);
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

void ImageWindow::setLoadingSprite (Sprite *sprite, float loadingWidthValue, float loadingHeightValue) {
	loadingSprite = sprite;
	loadingWidth = loadingWidthValue;
	loadingHeight = loadingHeightValue;
	if (loadingSprite && (! isImageUrlLoaded) && isLoadingImageUrl) {
		setImage (new Image (loadingSprite));
	}
}

void ImageWindow::showLoadingSprite () {
	if (loadingSprite) {
		setImage (new Image (loadingSprite));
	}
}

void ImageWindow::setImageUrl (const StdString &loadUrl) {
	if (imageUrl.equals (loadUrl)) {
		return;
	}
	if (isLoadingImageUrl) {
		nextImageUrl.assign (loadUrl);
		return;
	}
	imageUrl.assign (loadUrl);
	if (loadingSprite) {
		setImage (new Image (loadingSprite));
	}
	isImageUrlLoaded = false;
	isLoadingImageUrl = false;
	isImageUrlLoadDisabled = false;
	imageLoadSourceWidth = 0.0f;
	imageLoadSourceHeight = 0.0f;
}

void ImageWindow::onLoadScale (float scaleWidth, float scaleHeight) {
	onLoadResizeType = ImageWindow::Scale;
	onLoadWidth = scaleWidth;
	onLoadHeight = scaleHeight;
}

void ImageWindow::onLoadFit (float targetWidth, float targetHeight) {
	if ((targetWidth <= 0.0f) || (targetHeight <= 0.0f)) {
		return;
	}
	onLoadResizeType = ImageWindow::Fit;
	onLoadWidth = targetWidth;
	onLoadHeight = targetHeight;
}

void ImageWindow::setImageFilePath (const StdString &filePath, bool isExternalPath, bool shouldLoadNow) {
	SDL_RWops *rw;
	SDL_Surface *surface;
	SDL_Texture *texture;
	Sprite *sprite;
	StdString path;

	if (imageFilePath.equals (filePath) && (isImageFileExternal == isExternalPath)) {
		return;
	}
	imageFilePath.assign (filePath);
	isImageFileExternal = isExternalPath;
	imageLoadSourceWidth = 0.0f;
	imageLoadSourceHeight = 0.0f;
	if (! shouldLoadNow) {
		isImageFileLoaded = false;
		return;
	}

	if (loadCallback.callback) {
		shouldInvokeLoadCallback = true;
	}
	surface = NULL;
	if (isImageFileExternal) {
		rw = SDL_RWFromFile (imageFilePath.c_str (), "r");
		if (rw) {
			surface = IMG_Load_RW (rw, 1);
		}
	}
	else {
		surface = Resource::instance->loadSurface (imageFilePath);
	}
	if (! surface) {
		imageFilePath.assign ("");
		return;
	}

	imageLoadSourceWidth = (float) surface->w;
	imageLoadSourceHeight = (float) surface->h;
	path.sprintf ("*_ImageWindow_%llx_%llx", (long long int) id, (long long int) App::instance->getUniqueId ());
	texture = Resource::instance->createTexture (path, surface);
	SDL_FreeSurface (surface);
	if (! texture) {
		imageFilePath.assign ("");
		return;
	}

	sprite = new Sprite ();
	sprite->addTexture (texture, path);
	setImage (new Image (sprite, 0, true));
	isImageFileLoaded = true;
	isLoadingImageFile = false;
	refreshLayout ();
}

bool ImageWindow::isImageUrlEmpty () {
	return (imageUrl.empty ());
}

void ImageWindow::reload () {
	if (isLoadingImageUrl) {
		return;
	}
	if (isImageUrlLoaded) {
		if (loadingSprite) {
			setImage (new Image (loadingSprite));
		}
		isImageUrlLoaded = false;
	}
}

void ImageWindow::refreshLayout () {
	float w, h;

	if (isWindowSizeEnabled) {
		setFixedSize (true, windowWidth, windowHeight);
		if (image) {
			image->position.assign ((width / 2.0f) - (image->width / 2.0f), (height / 2.0f) - (image->height / 2.0f));
		}
	}
	else if (loadingSprite && image && image->hasSprite (loadingSprite) && (loadingWidth >= 1.0f) && (loadingHeight >= 1.0f)) {
		setFixedSize (true, loadingWidth, loadingHeight);
		image->position.assign ((width / 2.0f) - (image->width / 2.0f), (height / 2.0f) - (image->height / 2.0f));
	}
	else {
		w = widthPadding * 2.0f;
		h = heightPadding * 2.0f;
		if (image) {
			image->position.assign (widthPadding, heightPadding);
			w += image->width;
			h += image->height;
		}
		setFixedSize (true, w, h);
	}
}

void ImageWindow::doUpdate (int msElapsed) {
	bool shouldload;

	Panel::doUpdate (msElapsed);
	if (! imageFilePath.empty ()) {
		shouldload = false;
		if ((! isImageFileLoaded) && (! isLoadingImageFile)) {
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
				if (loadingSprite) {
					setImage (new Image (loadingSprite));
				}
				isImageUrlLoaded = false;
			}
		}
	}

	if (shouldInvokeLoadCallback) {
		shouldInvokeLoadCallback = false;
		eventCallback (loadCallback);
	}
}

void ImageWindow::loadImageResource () {
	if (isLoadingImageFile || imageFilePath.empty ()) {
		return;
	}
	isLoadingImageFile = true;
	retain ();
	App::instance->addRenderTask (ImageWindow::createFileTexture, this);
}

void ImageWindow::endLoadImageResource (bool clearResourcePath) {
	if (clearResourcePath) {
		imageFilePath.assign ("");
	}
	isLoadingImageFile = false;
	if (loadCallback.callback) {
		shouldInvokeLoadCallback = true;
	}
	release ();
}

void ImageWindow::createFileTexture (void *windowPtr) {
	ImageWindow *window;
	SDL_RWops *rw;
	SDL_Surface *surface, *scaledsurface;
	SDL_Texture *texture;
	Sprite *sprite;
	StdString path;
	float scaledw, scaledh;

	window = (ImageWindow *) windowPtr;
	if (window->isDestroyed) {
		window->endLoadImageResource ();
		return;
	}

	if (window->isImageFileExternal) {
		rw = SDL_RWFromFile (window->imageFilePath.c_str (), "r");
		if (! rw) {
			Log::debug ("Failed to open external image file; filePath=\"%s\"", window->imageFilePath.c_str ());
			window->endLoadImageResource (true);
			return;
		}

		surface = IMG_Load_RW (rw, 1);
		if (! surface) {
			Log::debug ("external file IMG_Load_RW failed; path=\"%s\" err=\"%s\"", window->imageFilePath.c_str (), SDL_GetError ());
			window->endLoadImageResource (true);
			return;
		}
	}
	else {
		surface = Resource::instance->loadSurface (window->imageFilePath);
		if (! surface) {
			window->endLoadImageResource (true);
			return;
		}
	}

	window->imageLoadSourceWidth = (float) surface->w;
	window->imageLoadSourceHeight = (float) surface->h;
	if (window->getOnLoadScaleSize (&scaledw, &scaledh)) {
		scaledsurface = SDL_CreateRGBSurface (0, (int) floorf (scaledw), (int) floorf (scaledh), surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
		if (scaledsurface) {
			SDL_BlitScaled (surface, NULL, scaledsurface, NULL);
			SDL_FreeSurface (surface);
			surface = scaledsurface;
		}
	}

	path.sprintf ("*_ImageWindow_%llx_%llx", (long long int) window->id, (long long int) App::instance->getUniqueId ());
	texture = Resource::instance->createTexture (path, surface);
	SDL_FreeSurface (surface);
	if (! texture) {
		window->endLoadImageResource (true);
		return;
	}

	sprite = new Sprite ();
	sprite->addTexture (texture, path);
	window->setImage (new Image (sprite, 0, true));
	window->isImageFileLoaded = true;
	window->refreshLayout ();
	window->endLoadImageResource ();
}

void ImageWindow::requestImage () {
	if (isLoadingImageUrl) {
		return;
	}
	isLoadingImageUrl = true;
	retain ();
	Network::instance->sendHttpGet (imageUrl, Network::HttpRequestCallbackContext (ImageWindow::getImageComplete, this));
}

void ImageWindow::endRequestImage (bool disableLoad) {
	isImageUrlLoadDisabled = disableLoad;
	if (imageUrlData) {
		imageUrlData->release ();
		imageUrlData = NULL;
	}
	isLoadingImageUrl = false;
	if (loadCallback.callback) {
		shouldInvokeLoadCallback = true;
	}
	if (! nextImageUrl.empty () && (! isDestroyed)) {
		setImageUrl (nextImageUrl);
	}
	nextImageUrl.assign ("");
	release ();
}

void ImageWindow::getImageComplete (void *windowPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData) {
	ImageWindow *window;

	window = (ImageWindow *) windowPtr;
	if (window->isDestroyed || (! window->shouldShowUrlImage ())) {
		window->endRequestImage ();
		return;
	}

	if (statusCode != Network::HttpOkCode) {
		Log::warning ("Failed to load image; targetUrl=\"%s\" statusCode=%i err=\"non-success response status\"", targetUrl.c_str (), statusCode);
		window->endRequestImage (true);
		return;
	}
	if ((! responseData) || responseData->empty ()) {
		Log::warning ("Failed to load image; targetUrl=\"%s\" statusCode=%i err=\"No response data\"", targetUrl.c_str (), statusCode);
		window->endRequestImage (true);
		return;
	}

	if (window->imageUrlData) {
		window->imageUrlData->release ();
	}
	window->imageUrlData = responseData;
	window->imageUrlData->retain ();
	App::instance->addRenderTask (ImageWindow::createUrlDataTexture, window);
}

void ImageWindow::createUrlDataTexture (void *windowPtr) {
	ImageWindow *window;
	SDL_RWops *rw;
	SDL_Surface *surface, *scaledsurface;
	SDL_Texture *texture;
	Sprite *sprite;
	StdString path;
	float scaledw, scaledh;

	window = (ImageWindow *) windowPtr;
	if (window->isDestroyed || (! window->shouldShowUrlImage ()) || (! window->imageUrlData)) {
		window->endRequestImage ();
		return;
	}

	rw = SDL_RWFromConstMem (window->imageUrlData->data, window->imageUrlData->length);
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

	window->imageLoadSourceWidth = (float) surface->w;
	window->imageLoadSourceHeight = (float) surface->h;
	if (window->getOnLoadScaleSize (&scaledw, &scaledh)) {
		scaledsurface = SDL_CreateRGBSurface (0, (int) floorf (scaledw), (int) floorf (scaledh), surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
		if (scaledsurface) {
			SDL_BlitScaled (surface, NULL, scaledsurface, NULL);
			SDL_FreeSurface (surface);
			surface = scaledsurface;
		}
	}

	path.sprintf ("*_ImageWindow_%llx_%llx", (long long int) window->id, (long long int) App::instance->getUniqueId ());
	texture = Resource::instance->createTexture (path, surface);
	SDL_FreeSurface (surface);
	if (! texture) {
		window->endRequestImage (true);
		return;
	}

	sprite = new Sprite ();
	sprite->addTexture (texture, path);
	window->setImage (new Image (sprite, 0, true));
	window->isImageUrlLoaded = true;
	window->refreshLayout ();
	window->endRequestImage ();
}

bool ImageWindow::getOnLoadScaleSize (float *destWidth, float *destHeight) {
	float w, h;

	if ((imageLoadSourceWidth <= 0.0f) || (imageLoadSourceHeight <= 0.0f)) {
		return (false);
	}
	switch (onLoadResizeType) {
		case ImageWindow::Scale: {
			if ((onLoadWidth <= 0.0f) && (onLoadHeight <= 0.0f)) {
				w = imageLoadSourceWidth;
				h = imageLoadSourceHeight;
			}
			else if (onLoadHeight <= 0.0f) {
				w = onLoadWidth;
				h = (imageLoadSourceHeight * onLoadWidth) / imageLoadSourceWidth;
			}
			else if (onLoadWidth <= 0.0f) {
				h = onLoadHeight;
				w = (imageLoadSourceWidth * onLoadHeight) / imageLoadSourceHeight;
			}
			else {
				w = onLoadWidth;
				h = onLoadHeight;
			}
			break;
		}
		case ImageWindow::Fit: {
			if ((onLoadWidth <= 0.0f) || (onLoadHeight <= 0.0f)) {
				return (false);
			}
			if (onLoadWidth >= onLoadHeight) {
				w = onLoadWidth;
				h = (imageLoadSourceHeight * onLoadWidth) / imageLoadSourceWidth;
				if (h > onLoadHeight) {
					h = onLoadHeight;
					w = (imageLoadSourceWidth * onLoadHeight) / imageLoadSourceHeight;
				}
			}
			else {
				h = onLoadHeight;
				w = (imageLoadSourceWidth * onLoadHeight) / imageLoadSourceHeight;
				if (w > onLoadWidth) {
					w = onLoadWidth;
					h = (imageLoadSourceHeight * onLoadWidth) / imageLoadSourceWidth;
				}
			}
			break;
		}
		default: {
			return (false);
		}
	}

	if (w < 1.0f) {
		w = 1.0f;
	}
	if (h < 1.0f) {
		h = 1.0f;
	}
	if (destWidth) {
		*destWidth = w;
	}
	if (destHeight) {
		*destHeight = h;
	}
	return (true);
}
