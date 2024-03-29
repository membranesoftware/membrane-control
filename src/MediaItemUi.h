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
// UI that shows details and controls for a media item

#ifndef MEDIA_ITEM_UI_H
#define MEDIA_ITEM_UI_H

#include "StdString.h"
#include "Json.h"
#include "WidgetHandle.h"
#include "Button.h"
#include "Toolbar.h"
#include "HashMap.h"
#include "HelpWindow.h"
#include "Ui.h"

class MediaItemUi : public Ui {
public:
	// Sprite indexes
	enum {
		ConfigureStreamButtonSprite = 0,
		TimeIconSprite = 1,
		AttributesIconSprite = 2,
		DurationIconSprite = 3,
		CreateTagButtonSprite = 4
	};

	// Card view row numbers
	enum {
		InfoRow = 0,
		TagButtonRow = 1,
		TagRow = 2,
		ImageRow = 3
	};

	static const float TextWidthMultiplier;

	// Prefs keys
	static const char *VideoQualityKey;
	static const char *ImageSizeKey;

	MediaItemUi (const StdString &mediaId, const StdString &mediaName);
	~MediaItemUi ();

	// Read-write data members
	Widget::EventCallbackContext removeMediaCallback;

	// Read-only data members
	StdString mediaId;
	StdString mediaName;
	StdString agentId;
	int64_t duration;
	int64_t bitrate;
	int frameWidth;
	int frameHeight;
	float frameRate;
	StdString thumbnailPath;
	int thumbnailCount;
	int64_t mediaSize;

	// Set fields in the provided HelpWindow widget as appropriate for the UI's help content
	void setHelpWindowContent (HelpWindow *helpWindow);

protected:
	// Return a resource path containing images to be loaded into the sprites object, or an empty string to disable sprite loading
	StdString getSpritePath ();

	// Return a newly created widget for use as a main toolbar breadcrumb item
	Widget *createBreadcrumbWidget ();

	// Load subclass-specific resources and return a result value
	OsUtil::Result doLoad ();

	// Unload subclass-specific resources
	void doUnload ();

	// Add subclass-specific items to the provided main toolbar object
	void doAddMainToolbarItems (Toolbar *toolbar);

	// Add subclass-specific items to the provided secondary toolbar object
	void doAddSecondaryToolbarItems (Toolbar *toolbar);

	// Update subclass-specific interface state as appropriate when the Ui becomes active
	void doResume ();

	// Update subclass-specific interface state as appropriate when the Ui becomes inactive
	void doPause ();

	// Update subclass-specific interface state as appropriate for an elapsed millisecond time period
	void doUpdate (int msElapsed);

	// Execute subclass-specific operations to sync state with records present in the application's RecordStore object, which has been locked prior to invocation
	void doSyncRecordStore ();

private:
	// Callback functions
	static void imageSizeButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void smallImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void mediumImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void largeImageSizeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void thumbnailMouseEntered (void *uiPtr, Widget *widgetPtr);
	static void thumbnailMouseExited (void *uiPtr, Widget *widgetPtr);
	static void configureStreamButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void configureStreamOptionChanged (void *uiPtr, Widget *widgetPtr);
	static void configureStreamActionClosed (void *uiPtr, Widget *widgetPtr);
	static void removeMediaActionClosed (void *uiPtr, Widget *widgetPtr);
	static void removeMediaCommandComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess);
	static void createTagButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void createTagActionClosed (void *uiPtr, Widget *widgetPtr);
	static void removeTagButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void removeTagActionClosed (void *uiPtr, Widget *widgetPtr);
	static void invokeMediaCommandComplete (Ui *invokeUi, const StdString &agentId, Json *invokeCommand, Json *responseCommand, bool isResponseCommandSuccess);

	// Find the source MediaItem record and populate the interface based on its fields
	void syncMediaItem ();

	int cardDetail;
	WidgetHandle nameWindow;
	WidgetHandle attributesWindow;
	WidgetHandle mediaSizeWindow;
	WidgetHandle durationWindow;
	WidgetHandle timelineWindow;
	WidgetHandle configureStreamSizeIcon;
	HashMap streamServerAgentMap;
	bool isCreateStreamAvailable;
};

#endif
