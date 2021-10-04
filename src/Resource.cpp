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
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <map>
#include <vector>
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include "App.h"
#include "Font.h"
#include "OsUtil.h"
#include "Log.h"
#include "StdString.h"
#include "Resource.h"

Resource::Resource ()
: dataPath ("")
, freetype (NULL)
, isBundleFile (false)
, isOpen (false)
, fileMapMutex (NULL)
, textureMapMutex (NULL)
, fontMapMutex (NULL)
{
	fileMapMutex = SDL_CreateMutex ();
	textureMapMutex = SDL_CreateMutex ();
	fontMapMutex = SDL_CreateMutex ();
}

Resource::~Resource () {
	close ();
	clearFontMap ();
	clearFileMap ();
	clearTextureMap ();

	if (fontMapMutex) {
		SDL_DestroyMutex (fontMapMutex);
		fontMapMutex = NULL;
	}
	if (fileMapMutex) {
		SDL_DestroyMutex (fileMapMutex);
		fileMapMutex = NULL;
	}
	if (textureMapMutex) {
		SDL_DestroyMutex (textureMapMutex);
		textureMapMutex = NULL;
	}
}

void Resource::clearFileMap () {
	std::map<StdString, Resource::FileData>::iterator i, end;

	SDL_LockMutex (fileMapMutex);
	i = fileMap.begin ();
	end = fileMap.end ();
	while (i != end) {
		if (i->second.data) {
			delete (i->second.data);
			i->second.data = NULL;
		}
		++i;
	}
	fileMap.clear ();
	SDL_UnlockMutex (fileMapMutex);
}

void Resource::clearTextureMap () {
	std::map<StdString, Resource::TextureData>::iterator i, end;

	SDL_LockMutex (textureMapMutex);
	i = textureMap.begin ();
	end = textureMap.end ();
	while (i != end) {
		if (i->second.texture) {
			SDL_DestroyTexture (i->second.texture);
			i->second.texture = NULL;
		}
		++i;
	}
	textureMap.clear ();
	SDL_UnlockMutex (textureMapMutex);
}

void Resource::clearFontMap () {
	std::map<StdString, Resource::FontData>::iterator i, end;

	SDL_LockMutex (fontMapMutex);
	i = fontMap.begin ();
	end = fontMap.end ();
	while (i != end) {
		if (i->second.font) {
			delete (i->second.font);
			i->second.font = NULL;
		}
		++i;
	}
	fontMap.clear ();
	SDL_UnlockMutex (fontMapMutex);
}

void Resource::setSource (const StdString &path) {
	dataPath.assign (path);
	isBundleFile = (dataPath.find (".dat") == (dataPath.length () - 4));
}

OsUtil::Result Resource::open () {
	struct stat st;
	SDL_RWops *rw;
	Resource::ArchiveEntry ae;
	uint64_t id;
	OsUtil::Result result;

	if (FT_Init_FreeType (&(freetype))) {
		Log::err ("Failed to initialize freetype library");
		return (OsUtil::Result::FreetypeOperationFailedError);
	}
	if (dataPath.empty ()) {
		return (OsUtil::Result::InvalidConfigurationError);
	}
	if (stat (dataPath.c_str (), &st) != 0) {
		Log::err ("stat failed; path=\"%s\" err=\"%s\"", dataPath.c_str (), strerror (errno));
		return (OsUtil::Result::FileOperationFailedError);
	}

	if (! isBundleFile) {
		if (! S_ISDIR (st.st_mode)) {
			Log::err ("Failed to open resources; path=\"%s\" err=\"Invalid resource path\"", dataPath.c_str ());
			return (OsUtil::Result::MismatchedTypeError);
		}
		isOpen = true;
		return (OsUtil::Result::Success);
	}

	rw = SDL_RWFromFile (dataPath.c_str (), "r");
	if (! rw) {
		Log::err ("Failed to open resource bundle file; path=\"%s\" error=\"%s\"", dataPath.c_str (), SDL_GetError ());
		return (OsUtil::Result::FileOperationFailedError);
	}

	result = OsUtil::Result::Success;
	archiveEntryMap.clear ();
	while (true) {
		result = Resource::readUint64 (rw, &id);
		if (result != OsUtil::Result::Success) {
			break;
		}
		if (id == 0) {
			break;
		}
		result = Resource::readUint64 (rw, &(ae.position));
		if (result != OsUtil::Result::Success) {
			break;
		}
		result = Resource::readUint64 (rw, &(ae.length));
		if (result != OsUtil::Result::Success) {
			break;
		}
		archiveEntryMap.insert (std::pair<uint64_t, Resource::ArchiveEntry> (id, ae));
	}

	SDL_RWclose (rw);
	if (result == OsUtil::Result::Success) {
		isOpen = true;
	}
	return (result);
}

void Resource::close () {
	if (! isOpen) {
		return;
	}
	clearFileMap ();
	clearTextureMap ();
	clearFontMap ();
	if (freetype) {
		FT_Done_FreeType (freetype);
		freetype = NULL;
	}
	isOpen = false;
}

void Resource::compact () {
	compactFontMap ();
	compactFileMap ();
	compactTextureMap ();
}

void Resource::compactFileMap () {
	std::vector<StdString>::iterator i, end;
	std::map<StdString, Resource::FileData>::iterator mi;

	if (fileCompactList.empty ()) {
		return;
	}
	SDL_LockMutex (fileMapMutex);
	i = fileCompactList.begin ();
	end = fileCompactList.end ();
	while (i != end) {
		mi = fileMap.find (*i);
		if (mi != fileMap.end ()) {
			if (mi->second.refcount <= 0) {
				delete (mi->second.data);
				mi->second.data = NULL;
				fileMap.erase (mi);
			}
		}
		++i;
	}
	fileCompactList.clear ();
	SDL_UnlockMutex (fileMapMutex);
}

void Resource::compactTextureMap () {
	std::vector<StdString>::iterator i, end;
	std::map<StdString, Resource::TextureData>::iterator mi;

	if (textureCompactList.empty ()) {
		return;
	}
	SDL_LockMutex (textureMapMutex);
	i = textureCompactList.begin ();
	end = textureCompactList.end ();
	while (i != end) {
		mi = textureMap.find (*i);
		if (mi != textureMap.end ()) {
			if (mi->second.refcount <= 0) {
				SDL_DestroyTexture (mi->second.texture);
				mi->second.texture = NULL;
				textureMap.erase (mi);
			}
		}
		++i;
	}
	textureCompactList.clear ();
	SDL_UnlockMutex (textureMapMutex);
}

void Resource::compactFontMap () {
	std::vector<StdString>::iterator i, end;
	std::map<StdString, Resource::FontData>::iterator mi;

	if (fontCompactList.empty ()) {
		return;
	}
	SDL_LockMutex (fontMapMutex);
	i = fontCompactList.begin ();
	end = fontCompactList.end ();
	while (i != end) {
		mi = fontMap.find (*i);
		if (mi != fontMap.end ()) {
			if (mi->second.refcount <= 0) {
				delete (mi->second.font);
				mi->second.font = NULL;
				fontMap.erase (mi);
			}
		}
		++i;
	}
	fontCompactList.clear ();
	SDL_UnlockMutex (fontMapMutex);
}

bool Resource::fileExists (const StdString &path) {
	uint64_t id;
	std::map<uint64_t, Resource::ArchiveEntry>::iterator i;
	SDL_RWops *rw;
	StdString loadpath;
	bool exists;

	exists = false;
	if (isBundleFile) {
		id = Resource::getPathId (path);
		i = archiveEntryMap.find (id);
		if (i != archiveEntryMap.end ()) {
			exists = true;
		}
	}
	else {
		loadpath.sprintf ("%s/%s", dataPath.c_str (), path.c_str ());
		rw = SDL_RWFromFile (loadpath.c_str (), "r");
		if (rw) {
			SDL_RWclose (rw);
			exists = true;
		}
	}
	return (exists);
}

SDL_RWops *Resource::openFile (const StdString &path, uint64_t *fileSize) {
	uint64_t id;
	std::map<uint64_t, Resource::ArchiveEntry>::iterator i;
	SDL_RWops *rw, *rwbundle;
	StdString loadpath;
	Sint64 pos;

	rw = NULL;
	if (isBundleFile) {
		id = Resource::getPathId (path);
		i = archiveEntryMap.find (id);
		if (i == archiveEntryMap.end ()) {
			Log::debug3 ("Failed to open file resource; path=\"%s\" error=\"Unknown path\"", path.c_str ());
			return (NULL);
		}
		rwbundle = SDL_RWFromFile (dataPath.c_str (), "r");
		if (! rwbundle) {
			Log::debug3 ("Failed to open file resource; path=\"%s\" error=\"bundle: %s\"", dataPath.c_str (), SDL_GetError ());
			return (NULL);
		}
		pos = SDL_RWseek (rwbundle, (Sint64) i->second.position, RW_SEEK_SET);
		if (pos < 0) {
			SDL_RWclose (rwbundle);
			Log::debug3 ("Failed to open file resource; path=\"%s\" error=\"seek: %s\"", dataPath.c_str (), SDL_GetError ());
			return (NULL);
		}
		rw = SDL_AllocRW ();
		if (! rw) {
			SDL_RWclose (rwbundle);
			Log::debug3 ("Failed to open file resource; path=\"%s\" error=\"bundle: %s\"", dataPath.c_str (), SDL_GetError ());
			return (NULL);
		}

		rw->type = SDL_RWOPS_UNKNOWN;
		rw->hidden.unknown.data1 = &(i->second);
		rw->hidden.unknown.data2 = rwbundle;
		rw->size = Resource::rwopsSize;
		rw->seek = Resource::rwopsSeek;
		rw->read = Resource::rwopsRead;
		rw->write = Resource::rwopsWrite;
		rw->close = Resource::rwopsClose;

		if (fileSize) {
			*fileSize = i->second.length;
		}
	}
	else {
		loadpath.sprintf ("%s/%s", dataPath.c_str (), path.c_str ());
		rw = SDL_RWFromFile (loadpath.c_str (), "r");
		if (! rw) {
			return (NULL);
		}
		pos = SDL_RWsize (rw);
		if (pos < 0) {
			Log::debug3 ("Failed to open file resource; path=\"%s\" error=\"%s\"", loadpath.c_str (), SDL_GetError ());
			return (NULL);
		}
		if (fileSize) {
			*fileSize = (uint64_t) pos;
		}
	}
	return (rw);
}

Sint64 Resource::rwopsSize (SDL_RWops *rw) {
	Resource::ArchiveEntry *ae;

	ae = (Resource::ArchiveEntry *) rw->hidden.unknown.data1;
	return ((Sint64) ae->length);
}

Sint64 Resource::rwopsSeek (SDL_RWops *rw, Sint64 offset, int whence) {
	Resource::ArchiveEntry *ae;
	SDL_RWops *rwbundle;
	Sint64 result, pos;

	ae = (Resource::ArchiveEntry *) rw->hidden.unknown.data1;
	rwbundle = (SDL_RWops *) rw->hidden.unknown.data2;
	result = -1;
	switch (whence) {
		case RW_SEEK_SET: {
			if ((offset < 0) || (offset >= (Sint64) ae->length)) {
				break;
			}
			result = SDL_RWseek (rwbundle, ((Sint64) ae->position) + offset, RW_SEEK_SET);
			if (result >= 0) {
				result -= (Sint64) ae->position;
			}
			break;
		}
		case RW_SEEK_CUR: {
			pos = SDL_RWtell (rwbundle);
			if (pos < 0) {
				break;
			}
			if (offset == 0) {
				return (pos - (Sint64) ae->position);
			}
			pos += offset;
			pos -= (Sint64) ae->position;
			if ((pos < 0) || (pos >= (Sint64) ae->length)) {
				break;
			}
			result = SDL_RWseek (rwbundle, ((Sint64) ae->position) + pos, RW_SEEK_SET);
			if (result >= 0) {
				result -= (Sint64) ae->position;
			}
			break;
		}
		case RW_SEEK_END: {
			pos = ((Sint64) ae->length) - offset - 1;
			if ((pos < 0) || (pos >= (Sint64) ae->length)) {
				break;
			}
			result = SDL_RWseek (rwbundle, ((Sint64) ae->position) + pos, RW_SEEK_SET);
			if (result >= 0) {
				result -= (Sint64) ae->position;
			}
			break;
		}
	}
	return (result);
}

size_t Resource::rwopsRead (SDL_RWops *rw, void *ptr, size_t size, size_t maxnum) {
	Resource::ArchiveEntry *ae;
	SDL_RWops *rwbundle;
	uint64_t readbytes, filebytes;
	Sint64 pos;

	ae = (Resource::ArchiveEntry *) rw->hidden.unknown.data1;
	rwbundle = (SDL_RWops *) rw->hidden.unknown.data2;
	if ((size == 0) || (maxnum == 0)) {
		return (0);
	}
	pos = SDL_RWtell (rwbundle);
	filebytes = ae->length - (uint64_t) (pos - (Sint64) ae->position);
	readbytes = ((uint64_t) size) * maxnum;
	if (readbytes > filebytes) {
		maxnum = (filebytes / size);
	}
	return (SDL_RWread (rwbundle, ptr, size, maxnum));
}

size_t Resource::rwopsWrite (SDL_RWops *rw, const void *ptr, size_t size, size_t num) {
	// Write operations are not supported by this interface

	return (0);
}

int Resource::rwopsClose (SDL_RWops *rw) {
	SDL_RWops *rwbundle;
	int result;

	rwbundle = (SDL_RWops *) rw->hidden.unknown.data2;
	result = SDL_RWclose (rwbundle);
	if (result < 0) {
		return (result);
	}
	SDL_FreeRW (rw);
	return (0);
}

Buffer *Resource::loadFile (const StdString &path) {
	std::map<StdString, Resource::FileData>::iterator i;
	Resource::FileData data;
	Buffer *buffer;
	SDL_RWops *rw;
	uint64_t sz;
	uint8_t buf[8192];
	size_t len, rlen;

	buffer = NULL;
	SDL_LockMutex (fileMapMutex);
	i = fileMap.find (path);
	if (i != fileMap.end ()) {
		++(i->second.refcount);
		buffer = i->second.data;
	}
	SDL_UnlockMutex (fileMapMutex);
	if (buffer) {
		return (buffer);
	}
	rw = openFile (path, &sz);
	if (! rw) {
		return (NULL);
	}

	buffer = new Buffer ();
	while (sz > 0) {
		rlen = (size_t) sz;
		if (rlen > sizeof (buf)) {
			rlen = sizeof (buf);
		}
		len = SDL_RWread (rw, buf, 1, rlen);
		if (len <= 0) {
			break;
		}

		buffer->add (buf, len);
		sz -= len;
	}
	SDL_RWclose (rw);

	if (sz > 0) {
		Log::err ("Failed to load file resource; path=\"%s\" error=\"%s\"", path.c_str (), SDL_GetError ());
		delete (buffer);
		return (NULL);
	}

	data.data = buffer;
	data.refcount = 1;
	SDL_LockMutex (fileMapMutex);
	fileMap.insert (std::pair<StdString, Resource::FileData> (path, data));
	SDL_UnlockMutex (fileMapMutex);

	return (buffer);
}

void Resource::unloadFile (const StdString &path) {
	std::map<StdString, Resource::FileData>::iterator i;

	SDL_LockMutex (fileMapMutex);
	i = fileMap.find (path);
	if (i != fileMap.end ()) {
		if (i->second.refcount > 0) {
			--(i->second.refcount);
			if (i->second.refcount <= 0) {
				fileCompactList.push_back (i->first);
			}
		}
	}
	SDL_UnlockMutex (fileMapMutex);
}

SDL_Surface *Resource::loadSurface (const StdString &path) {
	StdString loadpath;
	SDL_RWops *rw;
	SDL_Surface *surface;

	surface = NULL;
	if (isBundleFile) {
		rw = openFile (path);
		if (! rw) {
			return (NULL);
		}
		surface = IMG_Load_RW (rw, 1);
		if (! surface) {
			Log::info ("bundle IMG_Load_RW failed; path=\"%s\" err=\"%s\"", path.c_str (), SDL_GetError ());
			return (NULL);
		}
	}
	else {
		loadpath.sprintf ("%s/%s", dataPath.c_str (), path.c_str ());
		surface = IMG_Load (loadpath.c_str ());
		if (! surface) {
			Log::info ("IMG_Load failed; path=\"%s\" err=\"%s\"", path.c_str (), SDL_GetError ());
			return (NULL);
		}
	}
	return (surface);
}

SDL_Texture *Resource::loadTexture (const StdString &path) {
	std::map<StdString, Resource::TextureData>::iterator i;
	Resource::TextureData data;
	StdString loadpath;
	SDL_RWops *rw;
	SDL_Surface *surface;
	SDL_Texture *texture;

	texture = NULL;
	SDL_LockMutex (textureMapMutex);
	i = textureMap.find (path);
	if (i != textureMap.end ()) {
		++(i->second.refcount);
		texture = i->second.texture;
	}
	SDL_UnlockMutex (textureMapMutex);
	if (texture) {
		return (texture);
	}

	surface = NULL;
	if (isBundleFile) {
		rw = openFile (path);
		if (! rw) {
			return (NULL);
		}
		surface = IMG_Load_RW (rw, 1);
		if (! surface) {
			Log::info ("bundle IMG_Load_RW failed; path=\"%s\" err=\"%s\"", path.c_str (), SDL_GetError ());
			return (NULL);
		}
	}
	else {
		loadpath.sprintf ("%s/%s", dataPath.c_str (), path.c_str ());
		surface = IMG_Load (loadpath.c_str ());
		if (! surface) {
			Log::info ("IMG_Load failed; path=\"%s\" err=\"%s\"", path.c_str (), SDL_GetError ());
			return (NULL);
		}
	}
	if (! surface) {
		return (NULL);
	}

	texture = SDL_CreateTextureFromSurface (App::instance->render, surface);
	SDL_FreeSurface (surface);
	if (! texture) {
		Log::err ("SDL_CreateTextureFromSurface failed; path=\"%s\" err=\"%s\"", path.c_str (), SDL_GetError ());
		return (NULL);
	}

	data.texture = texture;
	data.refcount = 1;
	SDL_LockMutex (textureMapMutex);
	textureMap.insert (std::pair<StdString, Resource::TextureData> (path, data));
	SDL_UnlockMutex (textureMapMutex);

	return (texture);
}

SDL_Texture *Resource::createTexture (const StdString &path, SDL_Surface *surface) {
	std::map<StdString, Resource::TextureData>::iterator i;
	Resource::TextureData data;
	SDL_Texture *texture;

	texture = NULL;
	SDL_LockMutex (textureMapMutex);
	i = textureMap.find (path);
	if (i != textureMap.end ()) {
		++(i->second.refcount);
		texture = i->second.texture;
	}
	SDL_UnlockMutex (textureMapMutex);
	if (texture) {
		return (texture);
	}
	texture = SDL_CreateTextureFromSurface (App::instance->render, surface);
	if (! texture) {
		Log::err ("SDL_CreateTextureFromSurface failed; path=\"%s\" err=\"%s\"", path.c_str (), SDL_GetError ());
		return (NULL);
	}

	data.texture = texture;
	data.refcount = 1;
	SDL_LockMutex (textureMapMutex);
	textureMap.insert (std::pair<StdString, Resource::TextureData> (path, data));
	SDL_UnlockMutex (textureMapMutex);

	return (texture);
}

SDL_Texture *Resource::createTexture (const StdString &path, int textureWidth, int textureHeight) {
	std::map<StdString, Resource::TextureData>::iterator i;
	Resource::TextureData data;
	SDL_Texture *texture;

	if ((textureWidth <= 0) || (textureHeight <= 0)) {
		return (NULL);
	}
	texture = NULL;
	SDL_LockMutex (textureMapMutex);
	i = textureMap.find (path);
	if (i != textureMap.end ()) {
		++(i->second.refcount);
		texture = i->second.texture;
	}
	SDL_UnlockMutex (textureMapMutex);
	if (texture) {
		return (texture);
	}
	texture = SDL_CreateTexture (App::instance->render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, textureWidth, textureHeight);
	if (! texture) {
		Log::err ("SDL_CreateTextureFromSurface failed; path=\"%s\" err=\"%s\"", path.c_str (), SDL_GetError ());
		return (NULL);
	}

	data.texture = texture;
	data.refcount = 1;
	SDL_LockMutex (textureMapMutex);
	textureMap.insert (std::pair<StdString, Resource::TextureData> (path, data));
	SDL_UnlockMutex (textureMapMutex);

	return (texture);
}

void Resource::unloadTexture (const StdString &path) {
	std::map<StdString, Resource::TextureData>::iterator i;

	SDL_LockMutex (textureMapMutex);
	i = textureMap.find (path);
	if (i != textureMap.end ()) {
		if (i->second.refcount > 0) {
			--(i->second.refcount);

			if (i->second.refcount <= 0) {
				textureCompactList.push_back (i->first);
			}
		}
	}

	SDL_UnlockMutex (textureMapMutex);
}

Font *Resource::loadFont (const StdString &path, int pointSize) {
	std::map<StdString, Resource::FontData>::iterator i;
	Resource::FontData data;
	StdString key;
	Buffer *buffer;
	Font *font;
	int result;

	font = NULL;
	key = Resource::getFontKey (path, pointSize);
	SDL_LockMutex (fontMapMutex);
	i = fontMap.find (key);
	if (i != fontMap.end ()) {
		++(i->second.refcount);
		font = i->second.font;
	}
	SDL_UnlockMutex (fontMapMutex);
	if (font) {
		return (font);
	}
	buffer = loadFile (path);
	if (! buffer) {
		return (NULL);
	}

	font = new Font (freetype, key);
	result = font->load (buffer, pointSize);
	if (result != OsUtil::Result::Success) {
		delete (font);
		unloadFile (path);
		Log::err ("Failed to load font resource; key=\"%s\" err=%i", key.c_str (), result);
		return (NULL);
	}

	data.font = font;
	data.refcount = 1;
	SDL_LockMutex (fontMapMutex);
	fontMap.insert (std::pair<StdString, Resource::FontData> (key, data));
	SDL_UnlockMutex (fontMapMutex);

	return (font);
}

void Resource::unloadFont (const StdString &path, int pointSize) {
	std::map<StdString, Resource::FontData>::iterator i;
	StdString key;
	bool unloaded;

	unloaded = false;
	key = Resource::getFontKey (path, pointSize);
	SDL_LockMutex (fontMapMutex);
	i = fontMap.find (key);
	if (i != fontMap.end ()) {
		if (i->second.refcount > 0) {
			--(i->second.refcount);
			if (i->second.refcount <= 0) {
				fontCompactList.push_back (i->first);
				unloaded = true;
			}
		}
	}
	SDL_UnlockMutex (fontMapMutex);
	if (unloaded) {
		unloadFile (path);
	}
}

StdString Resource::getFontKey (const StdString &key, int pointSize) {
	return (StdString::createSprintf ("%s:%i", key.c_str (), pointSize));
}

uint64_t Resource::getPathId (const StdString &path) {
	uint64_t id;
	char *s, c;

	id = 5381;
	s = (char *) path.c_str ();
	while (true) {
		c = *s;
		if (! c) {
			break;
		}
		id = ((id << 5) + id) + c;
		++s;
	}
	return (id);
}

OsUtil::Result Resource::readUint64 (SDL_RWops *src, Uint64 *value) {
	char buf[8];
	size_t rlen;
	Uint64 val;

	rlen = SDL_RWread (src, buf, 8, 1);
	if (rlen < 1) {
		return (OsUtil::Result::FileOperationFailedError);
	}
	val = 0;
	val |= (buf[0] & 0xFF);
	val <<= 8;
	val |= (buf[1] & 0xFF);
	val <<= 8;
	val |= (buf[2] & 0xFF);
	val <<= 8;
	val |= (buf[3] & 0xFF);
	val <<= 8;
	val |= (buf[4] & 0xFF);
	val <<= 8;
	val |= (buf[5] & 0xFF);
	val <<= 8;
	val |= (buf[6] & 0xFF);
	val <<= 8;
	val |= (buf[7] & 0xFF);

	if (value) {
		*value = val;
	}
	return (OsUtil::Result::Success);
}
