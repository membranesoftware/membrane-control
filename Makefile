PROJECT_NAME=membrane-control
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
O=ActionWindow.o \
	AdminSecretWindow.o \
	Agent.o \
	AgentConfigurationWindow.o \
	AgentControl.o \
	App.o \
	BannerWindow.o \
	Buffer.o \
	Button.o \
	CardView.o \
	Chip.o \
	Color.o \
	ComboBox.o \
	CommandList.o \
	Font.o \
	HashMap.o \
	HelpActionWindow.o \
	HelpWindow.o \
	HyperlinkWindow.o \
	IconCardWindow.o \
	IconLabelWindow.o \
	Image.o \
	ImageWindow.o \
	Input.o \
	Ipv4Address.o \
	json-builder.o \
	Json.o \
	json-parser.o \
	Label.o \
	LabelWindow.o \
	LinkClient.o \
	ListView.o \
	Log.o \
	LogoWindow.o \
	Main.o \
	MainUi.o \
	MathUtil.o \
	MediaDetailWindow.o \
	MediaItemUi.o \
	MediaLibraryWindow.o \
	MediaUi.o \
	MediaUtil.o \
	MediaWindow.o \
	Menu.o \
	MonitorCacheUi.o \
	MonitorWindow.o \
	Network.o \
	OsUtil.o \
	Panel.o \
	Position.o \
	Prng.o \
	ProgressBar.o \
	RecordStore.o \
	Resource.o \
	ScrollBar.o \
	ScrollView.o \
	ServerAdminUi.o \
	ServerContactWindow.o \
	ServerUi.o \
	ServerWindow.o \
	SettingsWindow.o \
	SharedBuffer.o \
	Slider.o \
	SliderWindow.o \
	SnackbarWindow.o \
	SpriteGroup.o \
	SpriteHandle.o \
	Sprite.o \
	StatsWindow.o \
	StdString.o \
	StreamDetailWindow.o \
	StreamItemUi.o \
	StreamPlaylistWindow.o \
	StreamWindow.o \
	StringList.o \
	SystemInterface.o \
	TaskWindow.o \
	TextArea.o \
	TextField.o \
	TextFieldWindow.o \
	ThumbnailWindow.o \
	TimelineWindow.o \
	Toggle.o \
	ToggleWindow.o \
	Toolbar.o \
	TooltipWindow.o \
	UiConfiguration.o \
	Ui.o \
	UiLaunchWindow.o \
	UiStack.o \
	UiText.o \
	WebKioskUi.o \
	WebPlaylistWindow.o \
	WidgetHandle.o \
	Widget.o

VPATH=$(SRCPATH)
CFLAGS=$(PLATFORM_CFLAGS) -I$(SDL2_PREFIX)/include -I$(SDL2_PREFIX)/include/SDL2 -I$(JPEG_PREFIX)/include -I$(LIBPNG_PREFIX)/include -I$(SDL2_IMAGE_PREFIX)/include -I$(ZLIB_PREFIX)/include -I$(FREETYPE_PREFIX)/include -I$(FREETYPE_PREFIX)/include/freetype2 -I$(OPENSSL_PREFIX)/include -I$(CURL_PREFIX)/include -I$(LIBWEBSOCKETS_PREFIX)/include -I$(SRCPATH) $(EXTRA_CFLAGS)
LDFLAGS=-L$(SDL2_PREFIX)/lib -L$(JPEG_PREFIX)/lib -L$(LIBPNG_PREFIX)/lib -L$(SDL2_IMAGE_PREFIX)/lib -L$(ZLIB_PREFIX)/lib -L$(FREETYPE_PREFIX)/lib -L$(OPENSSL_PREFIX)/lib -L$(CURL_PREFIX)/lib -L$(LIBWEBSOCKETS_PREFIX)/lib $(EXTRA_LDFLAGS)
LD_DYNAMIC_LIBS=-lSDL2 -lSDL2_image -lfreetype -ldl -lm -lpthread -lcurl -lwebsockets -lssl -lcrypto -lpng -ljpeg -lz

all: $(PROJECT_NAME)

clean:
	rm -f $(O) $(PROJECT_NAME) $(SRCPATH)/BuildConfig.h

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

$(PROJECT_NAME): $(SRCPATH)/BuildConfig.h $(O)
	$(CC) -o $@ $(O) $(LDFLAGS) $(LD_DYNAMIC_LIBS)

.SECONDARY: $(O)

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

json-parser.o: json-parser.c
	$(CC) -o $@ -g -c $<

json-builder.o: json-builder.c
	$(CC) -o $@ -g -c $<
