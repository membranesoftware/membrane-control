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
, imageLoadingSprite (NULL)
, isImageResourceLoaded (false)
, isLoadingImageResource (false)
, isImageUrlLoaded (false)
, isLoadingImageUrl (false)
, isImageUrlLoadDisabled (false)
, loadCallback (NULL)
, loadCallbackData (NULL)
{
	if (image) {
		addWidget (image);
	}
}

ImageWindow::~ImageWindow () {

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

void ImageWindow::setPadding (float widthPadding, float heightPadding) {
	Panel::setPadding (widthPadding, heightPadding);
	resetLayout ();
}

bool ImageWindow::isLoaded () {
	if (! image) {
		return (false);
	}

	return (isImageResourceLoaded || isImageUrlLoaded);
}

void ImageWindow::setLoadCallback (Widget::EventCallback callback, void *callbackData) {
	loadCallback = callback;
	loadCallbackData = callbackData;
}

void ImageWindow::setWindowSize (float windowSizeWidth, float windowSizeHeight) {
	isWindowSizeEnabled = true;
	windowWidth = windowSizeWidth;
	windowHeight = windowSizeHeight;
	setFixedSize (true, windowWidth, windowHeight);
	resetLayout ();
}

void ImageWindow::setImage (Image *nextImage) {
	if (image) {
		image->isDestroyed = true;
	}
	image = nextImage;
	if (image) {
		addWidget (image);
	}
	resetLayout ();
}

void ImageWindow::setFrame (int frame) {
	if (! image) {
		return;
	}

	image->setFrame (frame);
	resetLayout ();
}

void ImageWindow::setScale (float scale) {
	if (! image) {
		return;
	}

	image->setScale (scale);
	resetLayout ();
}

void ImageWindow::setLoadUrl (const StdString &loadUrl, Sprite *loadingSprite) {
	if (imageUrl.equals (loadUrl)) {
		return;
	}
	imageUrl.assign (loadUrl);
	imageLoadingSprite = loadingSprite;
	if (imageLoadingSprite) {
		setImage (new Image (imageLoadingSprite));
	}
	else {
		setImage (NULL);
	}
	isImageUrlLoaded = false;
	isLoadingImageUrl = false;
	isImageUrlLoadDisabled = false;
}

void ImageWindow::setLoadResourcePath (const StdString &loadPath) {
	if (imageResourcePath.equals (loadPath)) {
		return;
	}
	imageResourcePath.assign (loadPath);
	isImageResourceLoaded = false;
}

bool ImageWindow::isLoadUrlEmpty () {
	return (imageUrl.empty ());
}

void ImageWindow::reload () {
	if (isImageUrlLoaded) {
		if (imageLoadingSprite) {
			setImage (new Image (imageLoadingSprite));
		}
		isImageUrlLoaded = false;
	}
}

void ImageWindow::resetLayout () {
	if (isWindowSizeEnabled) {
		if (image) {
			image->position.assign ((windowWidth / 2.0f) - (image->width / 2.0f), (windowHeight / 2.0f) - (image->height / 2.0f));
		}
	}
	else {
		if (image) {
			image->position.assign (widthPadding, heightPadding);
		}
		resetSize ();
	}
}

void ImageWindow::doUpdate (int msElapsed, float originX, float originY) {
	App *app;
	SDL_Surface *surface;
	float w, h;
	bool shouldload;

	Panel::doUpdate (msElapsed, originX, originY);

	app = App::getInstance ();
	if (! imageResourcePath.empty ()) {
		shouldload = false;
		if ((! isImageResourceLoaded) && (! isLoadingImageResource)) {
			shouldload = true;
		}

		if (shouldload) {
			surface = app->resource.loadSurface (imageResourcePath);
			if (! surface) {
				imageResourcePath.assign ("");
			}
			else {
				isLoadingImageResource = true;
				retain ();
				app->createResourceTexture (imageResourcePath, surface, ImageWindow::createResourceTextureComplete, this);
			}
		}
	}
	else if (! imageUrl.empty ()) {
		shouldload = false;
		if (isVisible && (! isImageUrlLoadDisabled)) {
			// TODO: Possibly use a different draw boundary here (currently using 2x the app window dimensions)
			w = ((float) app->windowWidth) * 2.0f;
			h = ((float) app->windowHeight) * 2.0f;
			if ((drawX >= -w) && (drawX <= w)) {
				if ((drawY >= -h) && (drawY <= h)) {
					shouldload = true;
				}
			}
		}

		if (shouldload) {
			if ((! isImageUrlLoaded) && (! isLoadingImageUrl)) {
				requestImage ();
			}
		}
		else {
			if (isImageUrlLoaded) {
				if (imageLoadingSprite) {
					setImage (new Image (imageLoadingSprite));
				}
				isImageUrlLoaded = false;
			}
		}
	}
}

void ImageWindow::createResourceTextureComplete (void *windowPtr, const StdString &path, SDL_Surface *surface, SDL_Texture *texture) {
	ImageWindow *window;
	Sprite *sprite;

	window = (ImageWindow *) windowPtr;
	if (window->isDestroyed) {
		window->isLoadingImageResource = false;
		window->release ();
		return;
	}
	if (texture) {
		sprite = new Sprite ();
		sprite->addTexture (texture, path);
		window->setImage (new Image (sprite, 0, true));
		window->isImageResourceLoaded = true;
		window->resetLayout ();
		if (window->loadCallback) {
			window->loadCallback (window->loadCallbackData, window);
		}
	}
	if (surface) {
		SDL_FreeSurface (surface);
	}

	window->isLoadingImageResource = false;
	window->release ();
}

void ImageWindow::requestImage () {
	App *app;

	if (isLoadingImageUrl) {
		return;
	}

	app = App::getInstance ();
	isLoadingImageUrl = true;
	retain ();
	app->network.sendHttpGet (imageUrl, ImageWindow::getImageComplete, this);
}

void ImageWindow::getImageComplete (void *windowPtr, const StdString &targetUrl, int statusCode, Buffer *responseData) {
	App *app;
	ImageWindow *window;
	SDL_RWops *rw;
	SDL_Surface *surface, *scaledsurface;
	uint8_t *data;
	int datalen;

	window = (ImageWindow *) windowPtr;
	if (window->isDestroyed) {
		window->isLoadingImageUrl = false;
		window->release ();
		return;
	}

	app = App::getInstance ();
	data = NULL;
	datalen = -1;
	if (responseData) {
		responseData->getData (&data, &datalen);
	}

	if ((! data) || (datalen <= 0)) {
		Log::write (Log::WARNING, "Failed to get thumbnail image; targetUrl=\"%s\" statusCode=%i err=\"No response data\"", targetUrl.c_str (), statusCode);
		window->isImageUrlLoadDisabled = true;
		window->isLoadingImageUrl = false;
		window->release ();
		return;
	}

	rw = SDL_RWFromConstMem (data, datalen);
	if (! rw) {
		Log::write (Log::WARNING, "Failed to get thumbnail image; targetUrl=\"%s\" statusCode=%i err=\"SDL_RWFromConstMem: %s\"", targetUrl.c_str (), statusCode, SDL_GetError ());
		window->isImageUrlLoadDisabled = true;
		window->isLoadingImageUrl = false;
		window->release ();
		return;
	}
	surface = IMG_Load_RW (rw, 1);
	if (! surface) {
		Log::write (Log::WARNING, "Failed to get thumbnail image; targetUrl=\"%s\" statusCode=%i err=\"IMG_Load_RW: %s\"", targetUrl.c_str (), statusCode, IMG_GetError ());
		window->isImageUrlLoadDisabled = true;
		window->isLoadingImageUrl = false;
		window->release ();
		return;
	}

	scaledsurface = SDL_CreateRGBSurface (0, window->width, window->height, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);
	if (scaledsurface) {
		SDL_BlitScaled (surface, NULL, scaledsurface, NULL);
	}
	SDL_FreeSurface (surface);
	if (! scaledsurface) {
		Log::write (Log::WARNING, "Failed to get thumbnail image; targetUrl=\"%s\" statusCode=%i err=\"Failed to resize surface: %s\"", targetUrl.c_str (), statusCode, SDL_GetError ());
		window->isImageUrlLoadDisabled = true;
		window->isLoadingImageUrl = false;
		window->release ();
		return;
	}

	app->createResourceTexture (StdString::createSprintf ("*_ImageWindow_%llx_%llx", (long long int) window->id, (long long int) app->getUniqueId ()), scaledsurface, ImageWindow::createUrlTextureComplete, window);
}

void ImageWindow::createUrlTextureComplete (void *windowPtr, const StdString &path, SDL_Surface *surface, SDL_Texture *texture) {
	ImageWindow *window;
	Sprite *sprite;

	window = (ImageWindow *) windowPtr;
	if (window->isDestroyed) {
		window->isLoadingImageUrl = false;
		window->release ();
		return;
	}
	if (texture) {
		sprite = new Sprite ();
		sprite->addTexture (texture, path);
		window->setImage (new Image (sprite, 0, true));
		window->isImageUrlLoaded = true;
		window->resetLayout ();
		if (window->loadCallback) {
			window->loadCallback (window->loadCallbackData, window);
		}
	}
	if (surface) {
		SDL_FreeSurface (surface);
	}

	window->isLoadingImageUrl = false;
	window->release ();
}
