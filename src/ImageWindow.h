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
// Panel that holds an Image widget and provides operations for loading image content from outside resources

#ifndef IMAGE_WINDOW_H
#define IMAGE_WINDOW_H

#include "StdString.h"
#include "SharedBuffer.h"
#include "Image.h"
#include "Sprite.h"
#include "Widget.h"
#include "Panel.h"

class ImageWindow : public Panel {
public:
	ImageWindow (Image *image = NULL);
	virtual ~ImageWindow ();

	static const float UrlImageShowAreaMultiplier;

	// Read-write data members
	Widget::EventCallbackContext loadCallback;

	// Read-only data members
	StdString imageUrl;
	float imageLoadSourceWidth, imageLoadSourceHeight;

	// Set the amount of size padding that should be applied to the window
	void setPadding (float widthPadding, float heightPadding);

	// Set window's fixed size option. If window size is enabled and the display image does not fill the provided size, it is centered inside a larger background space.
	void setWindowSize (bool enable, float windowSizeWidth = 0.0f, float windowSizeHeight = 0.0f);

	// Clear the window's content and replace it with the provided image
	void setImage (Image *nextImage);

	// Set a sprite that should be shown while image content loads from a URL, or disable any existing loading sprite if sprite is NULL. If loadingWidthValue and loadingHeightValue are provided, set window size to those values while the load sprite is displayed.
	void setLoadingSprite (Sprite *sprite, float loadingWidthValue = 0.0f, float loadingHeightValue = 0.0f);

	// Set a source URL that should be used to load the image window's content
	void setImageUrl (const StdString &loadUrl);

	// Set a path that should be used to load the image window's content from file data. If isExternalPath is true, read data from a filesystem path instead of application resources. If shouldLoadNow is true, load the image resource immediately. Otherwise, queue a task to load the resource on the next render cycle.
	void setImageFilePath (const StdString &filePath, bool isExternalPath = false, bool shouldLoadNow = false);

	// Return a boolean value indicating if the image window's load URL is empty
	bool isImageUrlEmpty ();

	// Return a boolean value indicating if the image window is loaded with content
	bool isLoaded ();

	// Return a boolean value indicating if the image window is configured with a source URL and holds state indicating that it should show content
	bool shouldShowUrlImage ();

	// Reload image content from the window's source URL
	void reload ();

	// Set the image content to the loading sprite if previously configured by setLoadingSprite
	void showLoadingSprite ();

	// Set the sprite frame for use in drawing the image
	void setFrame (int frame);

	// Set the draw scale factor for the window's image
	void setScale (float scale);

	// Set a scale size to apply after image content loads. If scaleWidth or scaleHeight are zero or less, choose a value that preserves the source aspect ratio.
	void onLoadScale (float scaleWidth = 0.0f, float scaleHeight = 0.0f);

	// Set a fit size to apply after image content loads, with the final image size computed as the largest width and height values that preserve the source aspect ratio while fitting inside targetWidth and targetHeight
	void onLoadFit (float targetWidth, float targetHeight);

protected:
	// Return a string that should be included as part of the toString method's output
	StdString toStringDetail ();

	// Execute operations to update object state as appropriate for an elapsed millisecond time period
	void doUpdate (int msElapsed);

	// Reset the panel's widget layout as appropriate for its content and configuration
	void refreshLayout ();

private:
	// Callback functions
	static void getImageComplete (void *windowPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData);
	static void createUrlDataTexture (void *windowPtr);
	static void createFileTexture (void *windowPtr);

	// onLoadResizeType values
	enum {
		Scale = 1,
		Fit = 2
	};

	// Execute operations to load content using the value stored in imageUrl
	void requestImage ();

	// Execute operations appropriate after an image request completes, optionally disabling subsequent load attempts
	void endRequestImage (bool disableLoad = false);

	// Execute operations to load content using the value stored in imageResourcePath
	void loadImageResource ();

	// Execute operations appropriate after an image load from resources completes, optionally clearing the image resource path
	void endLoadImageResource (bool clearResourcePath = false);

	// Assign destWidth and destHeight to target size values for configured onLoad settings and return a boolean value indicating if the operation succeeded
	bool getOnLoadScaleSize (float *destWidth, float *destHeight);

	Image *image;
	bool isWindowSizeEnabled;
	float windowWidth;
	float windowHeight;
	Sprite *loadingSprite;
	float loadingWidth;
	float loadingHeight;
	StdString imageFilePath;
	bool isImageFileExternal;
	bool isImageFileLoaded;
	bool isLoadingImageFile;
	SharedBuffer *imageUrlData;
	bool isImageUrlLoaded;
	bool isLoadingImageUrl;
	bool isImageUrlLoadDisabled;
	StdString nextImageUrl;
	bool shouldInvokeLoadCallback;
	int onLoadResizeType;
	float onLoadWidth;
	float onLoadHeight;
};

#endif
