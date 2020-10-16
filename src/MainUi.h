/*
* Copyright 2018-2020 Membrane Software <author@membranesoftware.com> https://membranesoftware.com
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
// UI that shows controls for launching other interfaces

#ifndef MAIN_UI_H
#define MAIN_UI_H

#include <map>
#include <vector>
#include "StdString.h"
#include "SharedBuffer.h"
#include "HashMap.h"
#include "SequenceList.h"
#include "WidgetHandle.h"
#include "HelpWindow.h"
#include "Ui.h"

class MainUi : public Ui {
public:
	static const int UiLaunchWindowTypes[];
	static const StdString AnnouncementIconType;
	static const StdString UpdateIconType;
	static const StdString TextMessageIconType;
	static const StdString VideoMessageIconType;
	static const StdString OpenUrlActionType;
	static const StdString HelpActionType;

	// Sprite indexes
	enum {
		UiIconSprite = 0,
		ConnectionIconSprite = 1,
		TextMessageIconSprite = 2,
		VideoMessageIconSprite = 3,
		AnnouncementIconSprite = 4,
		NextItemButtonSprite = 5,
		UpdateIconSprite = 6
	};

	// Card view row numbers
	enum {
		UnexpandedUiRow = 0,
		ExpandedUiRow = 1
	};

	// Prefs keys
	static const char *ExpandedUiTypesKey;
	static const char *ShortcutUiTypeKey;
	static const char *ApplicationNewsItemsKey;

	MainUi ();
	~MainUi ();

	// Set fields in the provided HelpWindow widget as appropriate for the UI's help content
	void setHelpWindowContent (HelpWindow *helpWindow);

protected:
	// Return a resource path containing images to be loaded into the sprites object, or an empty string to disable sprite loading
	StdString getSpritePath ();

	// Load subclass-specific resources and return a result value
	int doLoad ();

	// Unload subclass-specific resources
	void doUnload ();

	// Add subclass-specific items to the provided main toolbar object
	void doAddMainToolbarItems (Toolbar *toolbar);

	// Add subclass-specific items to the provided secondary toolbar object
	void doAddSecondaryToolbarItems (Toolbar *toolbar);

	// Remove and destroy any subclass-specific popup widgets that have been created by the UI
	void doClearPopupWidgets ();

	// Update subclass-specific interface state as appropriate when the Ui becomes inactive
	void doPause ();

	// Update subclass-specific interface state as appropriate when the Ui becomes active
	void doResume ();

	// Update subclass-specific interface state as appropriate for an elapsed millisecond time period
	void doUpdate (int msElapsed);

	// Reload subclass-specific interface resources as needed to account for a new application window size
	void doResize ();

	// Execute subclass-specific operations to sync state with records present in the application's RecordStore object, which has been locked prior to invocation
	void doSyncRecordStore ();

private:
	// Callback functions
	static void appendExpandedUiType (void *stringListPtr, Widget *widgetPtr);
	static void helpActionClicked (void *uiPtr, Widget *widgetPtr);
	static void uiExpandStateChanged (void *uiPtr, Widget *widgetPtr);
	static void uiOpenClicked (void *uiPtr, Widget *widgetPtr);
	static void nextBannerButtonClicked (void *uiPtr, Widget *widgetPtr);
	static void getApplicationNewsComplete (void *uiPtr, const StdString &targetUrl, int statusCode, SharedBuffer *responseData);
	static void openUrlActionClicked (void *uiPtr, Widget *widgetPtr);

	struct Banner {
		StdString messageText;
		StdString iconType;
		StdString actionText;
		StdString actionType;
		StdString actionTarget;
	};

	// Update the banner window with content from the next available item
	void showNextBanner ();

	// Reset the contents of bannerList to contain all appropriate items (static items and items loaded from preferences)
	void resetBanners ();

	HashMap bannerIconTypeMap;
	std::map<StdString, Widget::EventCallback> bannerActionCallbackMap;
	SequenceList<MainUi::Banner> bannerList;
	MainUi::Banner activeBanner;
	WidgetHandle bannerWindow;
	WidgetHandle bannerActionButton;
	int bannerClock;
	int agentCount;
};

#endif
