PLATFORM=$(shell uname)
PLATFORM_CFLAGS=
ifeq ($(PLATFORM), Linux)
PLATFORM_ID=linux
PLATFORM_CFLAGS=-DPLATFORM_LINUX=1
CC=g++
endif

ifndef BUILD_ID
BUILD_ID=0-ffffffff
endif
ifndef PLATFORM_ID
PLATFORM_ID=unknown
endif

SDL2_PREFIX?= /usr
JPEG_PREFIX?= /usr
LIBPNG_PREFIX?= /usr
SDL2_IMAGE_PREFIX?= /usr
ZLIB_PREFIX?= /usr
FREETYPE_PREFIX?= /usr
OPENSSL_PREFIX?= /usr
CURL_PREFIX?= /usr
LIBWEBSOCKETS_PREFIX?= /usr

SRCPATH=$(PWD)/src
O=json-parser.o json-builder.o Main.o App.o StdString.o Util.o Log.o Input.o Buffer.o SharedBuffer.o Widget.o Image.o Resource.o Sprite.o SpriteGroup.o SpriteHandle.o Panel.o Label.o Font.o Ui.o UiConfiguration.o Color.o Position.o ComboBox.o Prng.o HashMap.o Network.o AgentControl.o SystemInterface.o Json.o MediaUi.o ImageWindow.o LabelWindow.o MediaWindow.o StringList.o TooltipWindow.o Button.o LinkClient.o RecordStore.o ServerUi.o Toggle.o UiText.o TextField.o Toolbar.o WidgetHandle.o ProgressBar.o Menu.o TextFieldWindow.o ScrollView.o CardView.o ActionWindow.o TextArea.o ToggleWindow.o Chip.o MediaItemUi.o ThumbnailWindow.o StreamWindow.o StreamItemUi.o TimelineBar.o NewsWindow.o IconCardWindow.o ListView.o SettingsWindow.o Slider.o SliderWindow.o RowView.o IconLabelWindow.o MainUi.o UiLaunchWindow.o LogoWindow.o HelpWindow.o NewsItemWindow.o TaskWindow.o ScrollBar.o StatsWindow.o SnackbarWindow.o HelpActionWindow.o WebKioskUi.o ServerWindow.o WebPlaylistWindow.o Ipv4Address.o HyperlinkWindow.o Agent.o MediaLibraryWindow.o ServerAdminUi.o MediaDetailWindow.o StreamDetailWindow.o MonitorUi.o MonitorWindow.o StreamPlaylistWindow.o AgentConfigurationWindow.o MonitorCacheUi.o AdminSecretWindow.o ServerContactWindow.o CommandList.o
VPATH=$(SRCPATH)
CFLAGS=$(PLATFORM_CFLAGS) -I$(SDL2_PREFIX)/include -I$(SDL2_PREFIX)/include/SDL2 -I$(JPEG_PREFIX)/include -I$(LIBPNG_PREFIX)/include -I$(SDL2_IMAGE_PREFIX)/include -I$(ZLIB_PREFIX)/include -I$(FREETYPE_PREFIX)/include -I$(FREETYPE_PREFIX)/include/freetype2 -I$(OPENSSL_PREFIX)/include -I$(CURL_PREFIX)/include -I$(LIBWEBSOCKETS_PREFIX)/include -I$(SRCPATH) $(EXTRA_CFLAGS)
LDFLAGS=-L$(SDL2_PREFIX)/lib -L$(JPEG_PREFIX)/lib -L$(LIBPNG_PREFIX)/lib -L$(SDL2_IMAGE_PREFIX)/lib -L$(ZLIB_PREFIX)/lib -L$(FREETYPE_PREFIX)/lib -L$(OPENSSL_PREFIX)/lib -L$(CURL_PREFIX)/lib -L$(LIBWEBSOCKETS_PREFIX)/lib $(EXTRA_LDFLAGS)
LD_DYNAMIC_LIBS=-lSDL2 -lSDL2_image -lfreetype -ldl -lm -lpthread -lcurl -lwebsockets -lssl -lcrypto -lpng -ljpeg -lz

all: membrane-control

clean:
	rm -f $(O) membrane-control $(SRCPATH)/BuildConfig.h

$(SRCPATH)/BuildConfig.h:
	@echo "#ifndef BUILD_CONFIG_H" > $@
	@echo "#define BUILD_CONFIG_H" >> $@
	@echo "#ifndef BUILD_ID" >> $@
	@echo "#define BUILD_ID \"$(BUILD_ID)\"" >> $@
	@echo "#endif" >> $@
	@echo "#ifndef PLATFORM_ID" >> $@
	@echo "#define PLATFORM_ID \"$(PLATFORM_ID)\"" >> $@
	@echo "#endif" >> $@
	@echo "#endif" >> $@

membrane-control: $(SRCPATH)/BuildConfig.h $(O)
	$(CC) -o $@ $(O) $(LDFLAGS) $(LD_DYNAMIC_LIBS)

.SECONDARY: $(O)

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

json-parser.o: json-parser.c
	$(CC) -o $@ -g -c $<

json-builder.o: json-builder.c
	$(CC) -o $@ -g -c $<
