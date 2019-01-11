/*
* Copyright 2019 Membrane Software <author@membranesoftware.com>
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
// Panel that holds an Image widget and provides operations for loading image content from outside resources

#ifndef IMAGE_WINDOW_H
#define IMAGE_WINDOW_H

#include "StdString.h"
#include "SharedBuffer.h"
#include "Image.h"
#include "Sprite.h"
#include "Panel.h"

class ImageWindow : public Panel {
public:
	ImageWindow (Image *image = NULL);
	virtual ~ImageWindow ();

	// Set the amount of size padding that should be applied to the window
	void setPadding (float widthPadding, float heightPadding);

	// Set the fixed size for the window. If the window's image does not fill the provided size, it is centered inside a larger background space.
	void setWindowSize (float windowSizeWidth, float windowSizeHeight);

	// Clear the window's content and replace it with the provided image
	void setImage (Image *nextImage);

	// Set a source URL that should be used to load the image window's content, as well as an optional sprite that should be shown while the load is in progress
	void setLoadUrl (const StdString &loadUrl, Sprite *loadingSprite = NULL);

	// Set a path that should be used to load the image window's content from application resources
	void setLoadResourcePath (const StdString &loadPath);

	// Return a boolean value indicating if the image window's load URL is empty
	bool isLoadUrlEmpty ();

	// Return a boolean value indicating if the image window is loaded with content
	bool isLoaded ();

	// Return a boolean value indicating if the image window is configured with a source URL and holds state indicating that it should show content
	bool shouldShowUrlImage ();

	// Reload image content from the window's source URL
	void reload ();

	// Set the sprite frame for use in drawing the image
	void setFrame (int frame);

	// Set the draw scale factor for the window's image
	void setScale (float scale);

	// Callback functions
	static void getImageComplete (void *windowPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData);
	static void createResourceDataTexture (void *windowPtr);
	static void createResourcePathTexture (void *windowPtr);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Execute operations to update object state as appropriate for an elapsed millisecond time period and origin position
	void doUpdate (int msElapsed, float originX, float originY);

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Execute operations to load content using the value stored in imageUrl
	void requestImage ();

	// Execute operations appropriate after an image request completes, optionally disabling subsequent load attempts
	void endRequestImage (bool disableLoad = false);

	// Execute operations to load content using the value stored in imageResourcePath
	void loadImageResource ();

	// Execute operations appropriate after an image load from resources completes, optionally clearing the image resource path
	void endLoadImageResource (bool clearResourcePath = false);

	Image *image;
	bool isWindowSizeEnabled;
	float windowWidth;
	float windowHeight;
	Sprite *imageLoadingSprite;
	StdString imageResourcePath;
	bool isImageResourceLoaded;
	bool isLoadingImageResource;
	SharedBuffer *imageResourceData;
	StdString imageUrl;
	bool isImageUrlLoaded;
	bool isLoadingImageUrl;
	bool isImageUrlLoadDisabled;
};

#endif
