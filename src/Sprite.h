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
// Class that holds a set of frame textures loaded from application resources

#ifndef SPRITE_H
#define SPRITE_H

#include <vector>
#include "SDL2/SDL.h"
#include "StdString.h"
#include "OsUtil.h"

class Sprite {
public:
	Sprite ();
	~Sprite ();

	// Read-only data members
	int frameCount;
	int maxWidth, maxHeight;

	// Return a newly created Sprite object with its own copies of all resources referenced by this object, or NULL if the copy could not be created
	Sprite *copy ();

	// Load sprite data from png files at the specified path, which is expected to contain numbered png files named 000.png, 001.png, etc. If an image scale value is provided, the operation checks for files named with that suffix (i.e. 000_0.png, 001_0.png) and loads those if they exist. Returns a Result value.
	OsUtil::Result load (const StdString &path, int imageScale = -1);

	// Add the provided texture to the sprite's frame set. When the sprite is unloaded, release it from resources using the specified loadPath. Returns a Result value.
	OsUtil::Result addTexture (SDL_Texture *texture, const StdString &loadPath);

	// Unload previously loaded sprite data
	void unload ();

	// Return the SDL_Texture object at the specified index, or NULL if no such texture was found. If a texture is found and width and height pointers are provided, those values are filled in with texture attributes.
	SDL_Texture *getTexture (int index, int *width = NULL, int *height = NULL);

	// Return the load path for the texture at the specified index, or an empty string if no such texture was found
	StdString getLoadPath (int index) const;

private:
	struct TextureData {
		SDL_Texture *texture;
		StdString loadPath;
		int width, height;
		TextureData (): texture (NULL), loadPath (""), width (0), height (0) { }
	};
	std::vector<Sprite::TextureData> textureList;
};

#endif
