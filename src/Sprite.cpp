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
#include <vector>
#include "SDL2/SDL.h"
#include "OsUtil.h"
#include "Log.h"
#include "StdString.h"
#include "App.h"
#include "Resource.h"
#include "Sprite.h"

Sprite::Sprite ()
: frameCount (0)
, maxWidth (0)
, maxHeight (0)
{

}

Sprite::~Sprite () {
	unload ();
}

Sprite *Sprite::copy () {
	Sprite *sprite;
	Resource *resource;
	SDL_Texture *texture;
	std::vector<Sprite::TextureData>::iterator i, end;
	int result;

	resource = &(App::instance->resource);
	sprite = new Sprite ();
	i = textureList.begin ();
	end = textureList.end ();
	while (i != end) {
		texture = resource->loadTexture (i->loadPath);
		if (! texture) {
			delete (sprite);
			sprite = NULL;
			break;
		}
		result = sprite->addTexture (texture, i->loadPath);
		if (result != OsUtil::Result::Success) {
			resource->unloadTexture (i->loadPath);
			delete (sprite);
			sprite = NULL;
			break;
		}
		++i;
	}
	return (sprite);
}

OsUtil::Result Sprite::load (const StdString &path, int imageScale) {
	Resource *resource;
	StdString loadpath;
	SDL_Texture *texture;
	OsUtil::Result result;
	int i;
	bool found;

	result = OsUtil::Result::Success;
	resource = &(App::instance->resource);
	i = 0;
	maxWidth = 0;
	maxHeight = 0;
	while (true) {
		found = false;

		if (imageScale >= 0) {
			loadpath.sprintf ("%s/%03i_%i.png", path.c_str (), i, imageScale);
			if (resource->fileExists (loadpath)) {
				found = true;
			}
		}
		if (! found) {
			loadpath.sprintf ("%s/%03i.png", path.c_str (), i);
			if (resource->fileExists (loadpath)) {
				found = true;
			}
		}
		if (! found) {
			break;
		}
		texture = resource->loadTexture (loadpath);
		if (! texture) {
			result = OsUtil::Result::SdlOperationFailedError;
			break;
		}
		result = addTexture (texture, loadpath);
		if (result != OsUtil::Result::Success) {
			resource->unloadTexture (loadpath);
			break;
		}
		++i;
	}

	return (result);
}

OsUtil::Result Sprite::addTexture (SDL_Texture *texture, const StdString &loadPath) {
	Sprite::TextureData item;

	item.texture = texture;
	item.loadPath.assign (loadPath);
	if (SDL_QueryTexture (item.texture, NULL, NULL, &(item.width), &(item.height)) != 0) {
		Log::err ("Failed to query sprite texture; path=\"%s\" err=\"%s\"", item.loadPath.c_str (), SDL_GetError ());
		return (OsUtil::Result::SdlOperationFailedError);
	}
	textureList.push_back (item);
	frameCount = (int) textureList.size ();
	if (item.width > maxWidth) {
		maxWidth = item.width;
	}
	if (item.height > maxHeight) {
		maxHeight = item.height;
	}
	return (OsUtil::Result::Success);
}

void Sprite::unload () {
	Resource *resource;
	std::vector<Sprite::TextureData>::iterator i, end;

	resource = &(App::instance->resource);
	i = textureList.begin ();
	end = textureList.end ();
	while (i != end) {
		resource->unloadTexture (i->loadPath);
		i->texture = NULL;
		++i;
	}

	textureList.clear ();
	frameCount = 0;
	maxWidth = 0;
	maxHeight = 0;
}

SDL_Texture *Sprite::getTexture (int index, int *width, int *height) {
	Sprite::TextureData item;

	if ((index < 0) || (index >= (int) textureList.size ())) {
		return (NULL);
	}
	item = textureList.at (index);
	if (width) {
		*width = item.width;
	}
	if (height) {
		*height = item.height;
	}
	return (item.texture);
}

StdString Sprite::getLoadPath (int index) const {
	Sprite::TextureData item;

	if ((index < 0) || (index >= (int) textureList.size ())) {
		return (StdString (""));
	}
	item = textureList.at (index);
	return (item.loadPath);
}
