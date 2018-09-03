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
// UI for browsing and manipulating display agents

#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include "StdString.h"
#include "Json.h"
#include "RecordStore.h"
#include "CardView.h"
#include "WidgetHandle.h"
#include "Toolbar.h"
#include "Ui.h"

class DisplayUi : public Ui {
public:
	DisplayUi ();
	~DisplayUi ();

	// Constants to use for sprite indexes
	enum {
		DISPLAY_ICON = 0,
		BREADCRUMB_ICON = 1,
		LOADING_IMAGE_ICON = 2,
		VIEW_BUTTON = 3,
		LARGE_THUMBNAILS_ICON = 4,
		MEDIUM_THUMBNAILS_ICON = 5,
		SMALL_THUMBNAILS_ICON = 6,
		COMMAND_ICON = 7,
		PLAY_BUTTON = 8,
		STOP_BUTTON = 9,
		ADDRESS_BUTTON = 10,
		EMPTY_INTERFACE_ICON = 11
	};

	// Return text that should be used to identify the UI in a set of breadcrumb actions, or an empty string if no such title exists
	StdString getBreadcrumbTitle ();

	// Return a newly created Sprite object that should be used to identify the UI in a set of breadcrumb actions, or NULL if no such Sprite exists. If a Sprite is returned by this method, the caller becomes responsible for destroying it when no longer needed.
	Sprite *getBreadcrumbSprite ();

	// Callback functions
	static void processDisplayServerRecord (void *uiPtr, Json *record, const StdString &agentId);
	static void processStreamItemRecord (void *uiPtr, Json *record, const StdString &recordId);
	static void viewButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void viewSmallActionClicked (void *uiPtr, Widget *widgetPtr);
	static void viewMediumActionClicked (void *uiPtr, Widget *widgetPtr);
	static void viewLargeActionClicked (void *uiPtr, Widget *widgetPtr);
	static void resetStreamCardLayout (void *uiPtr, Widget *widgetPtr);
	static bool sortCards (Widget *a, Widget *b);
	static void streamCardClicked (void *uiPtr, Widget *widgetPtr);
	static void displayCardSelectToggleStateChanged (void *uiPtr, Widget *widgetPtr);
	static void appendDisplayCardName (void *textStringPtr, Widget *widgetPtr);
	static void playButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void stopButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void streamUiThumbnailClicked (void *uiPtr, Widget *widgetPtr);
	static void appendDisplayCardAgentId (void *stringListPtr, Widget *widgetPtr);
	static void addressToggleStateChanged (void *uiPtr, Widget *widgetPtr);
	static void addressTextFieldEdited (void *uiPtr, Widget *widgetPtr);

protected:
	// Return a resource path containing images to be loaded into the sprites object, or an empty string to disable sprite loading
	StdString getSpritePath ();

	// Load subclass-specific resources and return a result value
	int doLoad ();

	// Unload subclass-specific resources
	void doUnload ();

	// Add subclass-specific items to the provided main toolbar object
	void doResetMainToolbar (Toolbar *toolbar);

	// Add subclass-specific items to the provided secondary toolbar object
	void doResetSecondaryToolbar (Toolbar *toolbar);

	// Remove and destroy any subclass-specific popup widgets that have been created by the UI
	void doClearPopupWidgets ();

	// Execute subclass-specific operations to refresh the interface's layout as appropriate for the current set of UiConfiguration values
	void doRefresh ();

	// Update subclass-specific interface state as appropriate when the Ui becomes active
	void doResume ();

	// Update subclass-specific interface state as appropriate when the Ui becomes inactive
	void doPause ();

	// Update subclass-specific interface state as appropriate for an elapsed millisecond time period
	void doUpdate (int msElapsed);

	// Reload subclass-specific interface resources as needed to account for a new application window size
	void doResize ();

	// Execute subclass-specific operations to sync state with records present in the provided RecordStore object, which has been locked prior to invocation
	void doSyncRecordStore (RecordStore *store);

private:
	// Execute actions appropriate when the view button is clicked
	void handleViewButtonClick (Widget *buttonWidget);

	// Execute actions appropriate when a display card select toggle is checked
	void handleDisplayCardSelectToggleStateChanged (Widget *cardWidget);

	// Execute actions appropriate when a thumbnail card has been selected on a StreamItemUi
	void handleStreamUiThumbnailClicked (Widget *cardWidget);

	static const int readEventsPeriod;
	static const float smallImageScale;
	static const float mediumImageScale;
	static const float largeImageScale;

	CardView *cardView;
	WidgetHandle viewMenu;
	WidgetHandle agentNameChip;
	WidgetHandle playTargetChip;
	WidgetHandle selectedStreamCard;
	WidgetHandle addressToggle;
	WidgetHandle addressTextFieldWindow;
	float selectedPlayPosition;
	int streamCardLayout;
	float streamCardMaxImageWidth;
	int64_t lastReadEventsTime;
};

#endif
