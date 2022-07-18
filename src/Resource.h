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
// Class that manages media resources loaded from assets

#ifndef RESOURCE_H
#define RESOURCE_H

#include <map>
#include <vector>
#include "SDL2/SDL.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include "StdString.h"
#include "OsUtil.h"
#include "Buffer.h"
#include "Font.h"

class Resource {
public:
	Resource ();
	~Resource ();
	static Resource *instance;

	// Set the source path that should be used for loading file assets. If the path ends in ".dat", it is opened as a bundle file; otherwise, the path is treated as a directory prefix for direct file access.
	void setSource (const StdString &path);

	// Prepare the Resource object to execute file operations. Returns a Result value.
	OsUtil::Result open ();

	// Close the resource object and free all assets
	void close ();

	// Free objects associated with resources that are no longer referenced
	void compact ();

	// Return a boolean value indicating whether a resource file exists at the specified path
	bool fileExists (const StdString &path);

	// Open resource data at the specified path and return the resulting SDL_RWops object, or NULL if the file could not be opened. The caller is responsible for closing the SDL_RWops object when it's no longer needed. If fileSize is non-NULL, its value is set to the size of the opened file.
	SDL_RWops *openFile (const StdString &path, uint64_t *fileSize = NULL);

	// Load file data from the specified resource path. Returns a pointer to the resulting Buffer object, or NULL if the file load failed. If a pointer is returned by this method, the referenced path must be unloaded with the unloadFile method when the Buffer is no longer needed.
	Buffer *loadFile (const StdString &path);

	// Unload previously acquired file resources from the specified path
	void unloadFile (const StdString &path);

	// Load an SDL_surface asset from an image file at the specified resource path. Returns a pointer to the resulting SDL_surface, or NULL if the surface could not be loaded. If an SDL_surface is returned by this method, the caller is responsible for freeing it with SDL_FreeSurface when it's no longer needed.
	SDL_Surface *loadSurface (const StdString &path);

	// Load an SDL_Texture asset from an image file at the specified resource path. Returns a pointer to the resulting SDL_Texture, or NULL if the texture could not be loaded. This method must be invoked only from the application's main thread.
	SDL_Texture *loadTexture (const StdString &path);

	// Create a texture from a surface and associate it with a path. Returns a pointer to the resulting SDL_Texture, or NULL if the texture could not be created. The surface object is not modified or freed by this method. This method must be invoked only from the application's main thread.
	SDL_Texture *createTexture (const StdString &path, SDL_Surface *surface);

	// Create a render target texture of the specified size and associate it with a path. Returns a pointer to the resulting SDL_Texture, or NULL if the texture could not be created. This method must be invoked only from the application's main thread.
	SDL_Texture *createTexture (const StdString &path, int textureWidth, int textureHeight);

	// Unload previously acquired texture resources from the specified path
	void unloadTexture (const StdString &path);

	// Load a Font asset from a ttf file at the specified resource path. Returns a pointer to the resulting Font, or NULL if the font could not be loaded. This method must be invoked only from the application's main thread.
	Font *loadFont (const StdString &path, int pointSize);

	// Unload previously acquired font resources for the specified path and point size
	void unloadFont (const StdString &path, int pointSize);

	// Read a value from an SDL_RWops object and store it in the provided pointer. Returns a Result value.
	static OsUtil::Result readUint64 (SDL_RWops *src, Uint64 *value);

	// Interface functions for use in an SDL_RWops struct
	static Sint64 rwopsSize (SDL_RWops *rw);
	static Sint64 rwopsSeek (SDL_RWops *rw, Sint64 offset, int whence);
	static size_t rwopsRead (SDL_RWops *rw, void *ptr, size_t size, size_t maxnum);
	static size_t rwopsWrite (SDL_RWops *rw, const void *ptr, size_t size, size_t num);
	static int rwopsClose (SDL_RWops *rw);

private:
	struct ArchiveEntry {
		uint64_t position;
		uint64_t length;
	};

	struct FileData {
		Buffer *data;
		int refcount;
	};

	struct TextureData {
		SDL_Texture *texture;
		int refcount;
	};

	struct FontData {
		Font *font;
		int refcount;
	};

	StdString dataPath;
	FT_Library freetype;
	bool isBundleFile;
	bool isOpen;

	// A map of resource paths to FileData objects
	std::map<StdString, Resource::FileData> fileMap;
	std::vector<StdString> fileCompactList;
	SDL_mutex *fileMapMutex;

	// A map of resource paths to TextureData objects
	std::map<StdString, Resource::TextureData> textureMap;
	std::vector<StdString> textureCompactList;
	SDL_mutex *textureMapMutex;

	// A map of font keys to FontData objects
	std::map<StdString, Resource::FontData> fontMap;
	std::vector<StdString> fontCompactList;
	SDL_mutex *fontMapMutex;

	// A map of entry ID values to ArchiveEntry structs
	std::map<uint64_t, Resource::ArchiveEntry> archiveEntryMap;

	// Clear the file map
	void clearFileMap ();

	// Clear the texture map
	void clearTextureMap ();

	// Clear the font map
	void clearFontMap ();

	// Remove unreferenced items from the file map
	void compactFileMap ();

	// Remove unreferenced items from the texture map
	void compactTextureMap ();

	// Remove unreferenced items from the font map
	void compactFontMap ();

	// Return the key that should be used to store the specified font in the font map
	static StdString getFontKey (const StdString &key, int pointSize);

	// Return an ID value computed from the provided path, using the djb2 algorithm
	static uint64_t getPathId (const StdString &path);
};

#endif
