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
#include "OsUtil.h"
#include "Log.h"
#include "App.h"
#include "StdString.h"
#include "Resource.h"
#include "Sprite.h"
#include "SpriteGroup.h"

SpriteGroup::SpriteGroup ()
: isLoaded (false)
{

}

SpriteGroup::~SpriteGroup () {
	clearSpriteList ();
}

void SpriteGroup::clearSpriteList () {
	std::vector<Sprite *>::iterator i, end;
	Sprite *sprite;

	i = spriteList.begin ();
	end = spriteList.end ();
	while (i != end) {
		sprite = *i;
		sprite->unload ();
		delete (sprite);
		++i;
	}
	spriteList.clear ();
}

OsUtil::Result SpriteGroup::load (const StdString &path, int imageScale) {
	Resource *resource;
	Sprite *sprite;
	OsUtil::Result result;
	int i;

	if (isLoaded) {
		if (! loadPath.equals (path)) {
			return (OsUtil::Result::AlreadyLoadedError);
		}

		return (OsUtil::Result::Success);
	}

	resource = &(App::instance->resource);
	if (imageScale < 0) {
		imageScale = App::instance->imageScale;
	}
	result = OsUtil::Result::Success;
	i = 0;
	while (true) {
		if (! resource->fileExists (StdString::createSprintf ("%s/%03i/000_%i.png", path.c_str (), i, imageScale))) {
			if (! resource->fileExists (StdString::createSprintf ("%s/%03i/000.png", path.c_str (), i))) {
				break;
			}
		}

		sprite = new Sprite ();
		result = sprite->load (StdString::createSprintf ("%s/%03i", path.c_str (), i), imageScale);
		if (result != OsUtil::Result::Success) {
			delete (sprite);
			break;
		}

		spriteList.push_back (sprite);
		++i;
	}

	if (result == OsUtil::Result::Success) {
		loadPath.assign (path);
		isLoaded = true;
	}
	else {
		clearSpriteList ();
	}

	return (result);
}

void SpriteGroup::unload () {
	if (! isLoaded) {
		return;
	}

	isLoaded = false;
	clearSpriteList ();
}

void SpriteGroup::resize (int imageScale) {
	std::vector<Sprite *>::iterator i, end;
	Resource *resource;
	Sprite *sprite;
	int result, index;

	resource = &(App::instance->resource);
	if (imageScale < 0) {
		imageScale = App::instance->imageScale;
	}
	index = 0;
	i = spriteList.begin ();
	end = spriteList.end ();
	while (i != end) {
		sprite = *i;
		if (resource->fileExists (StdString::createSprintf ("%s/%03i/000_%i.png", loadPath.c_str (), index, imageScale))) {
			sprite->unload ();
			result = sprite->load (StdString::createSprintf ("%s/%03i", loadPath.c_str (), index), imageScale);
			if (result != OsUtil::Result::Success) {
				Log::err ("Failed to reload textures; path=\"%s\" index=%i err=%i", loadPath.c_str (), index, result);
			}
		}

		++index;
		++i;
	}
}

Sprite *SpriteGroup::getSprite (int index) {
	if ((! isLoaded) || (index < 0) || (index >= (int) spriteList.size ())) {
		return (NULL);
	}

	return (spriteList.at (index));
}
