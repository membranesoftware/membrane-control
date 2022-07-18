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
// Class that holds a set of Sprite objects

#ifndef SPRITE_GROUP_H
#define SPRITE_GROUP_H

#include <vector>
#include "StdString.h"
#include "OsUtil.h"
#include "Sprite.h"

class SpriteGroup {
public:
	SpriteGroup ();
	~SpriteGroup ();

	// Read-only data members
	bool isLoaded;
	StdString loadPath;

	// Load sprite data from the specified path, which is expected to contain numbered directories named 000, 001, etc. Returns a Result value. If no image scale value is provided, the application image scale is used.
	OsUtil::Result load (const StdString &path, int imageScale = -1);

	// Unload previously loaded sprite data
	void unload ();

	// Reload sprite textures as needed for the specified image scale. If no image scale value is provided, the application image scale is used.
	void resize (int imageScale = -1);

	// Return the Sprite object at the specified index, or NULL if no such Sprite was found
	Sprite *getSprite (int index);

private:
	// Remove all items from the sprite list
	void clearSpriteList ();

	std::vector<Sprite *> spriteList;
};

#endif
