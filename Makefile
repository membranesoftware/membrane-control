PROJECT_NAME=membrane-control
APPLICATION_NAME="Membrane Control"
APPLICATION_PACKAGE_NAME=MembraneControl
PLATFORM=$(shell uname)
PLATFORM_CFLAGS=
ifeq ($(PLATFORM), Linux)
PLATFORM_ID=linux
PLATFORM_CFLAGS=-DPLATFORM_LINUX=1
CC=g++
endif
ifeq ($(PLATFORM), Darwin)
PLATFORM_ID=macos
PLATFORM_CFLAGS=-DPLATFORM_MACOS=1 -std=c++11
CC=g++
endif

ifndef BUILD_ID
BUILD_ID=0-ffffffff
endif
ifndef PLATFORM_ID
PLATFORM_ID=unknown
endif

CURL_PREFIX?= /usr
FREETYPE_PREFIX?= /usr
JPEG_PREFIX?= /usr
LIBPNG_PREFIX?= /usr
LIBWEBSOCKETS_PREFIX?= /usr
LUA_PREFIX?= /usr
OPENSSL_PREFIX?= /usr
SDL2_PREFIX?= /usr
SDL2IMAGE_PREFIX?= /usr
ZLIB_PREFIX?= /usr

SRC_PATH=src
O=ActionWindow.o \
	AdminSecretWindow.o \
	Agent.o \
	AgentConfigurationWindow.o \
	AgentControl.o \
	AgentTaskWindow.o \
	App.o \
	AsyncCommand.o \
	BannerWindow.o \
	Buffer.o \
	Button.o \
	CameraDetailWindow.o \
	CameraThumbnailWindow.o \
	CameraTimelineUi.o \
	CameraTimelineWindow.o \
	CameraUi.o \
	CameraWindow.o \
	CardView.o \
	Chip.o \
	Color.o \
	ComboBox.o \
	CommandHistory.o \
	CommandList.o \
	CommandListener.o \
	ConsoleWindow.o \
	Font.o \
	HashMap.o \
	HelpActionWindow.o \
	HelpWindow.o \
	HistoryWindow.o \
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
	LuaScript.o \
	Main.o \
	MainUi.o \
	MathUtil.o \
	MediaItemUi.o \
	MediaLibraryWindow.o \
	MediaThumbnailWindow.o \
	MediaTimelineWindow.o \
	MediaUi.o \
	MediaUtil.o \
	MediaWindow.o \
	Menu.o \
	MonitorCacheUi.o \
	MonitorWindow.o \
	Network.o \
	NumberSpace.o \
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
	ServerAttachWindow.o \
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
	StreamItemUi.o \
	StreamPlaylistWindow.o \
	StreamWindow.o \
	StringList.o \
	SystemInterface.o \
	TagWindow.o \
	TaskGroup.o \
	TaskWindow.o \
	TextArea.o \
	TextField.o \
	TextFieldWindow.o \
	TextFlow.o \
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

VPATH=$(SRC_PATH)
CFLAGS=$(PLATFORM_CFLAGS) \
	-I$(CURL_PREFIX)/include \
	-I$(FREETYPE_PREFIX)/include \
	-I$(FREETYPE_PREFIX)/include/freetype2 \
	-I$(JPEG_PREFIX)/include \
	-I$(LIBPNG_PREFIX)/include \
	-I$(LIBWEBSOCKETS_PREFIX)/include \
	-I$(LUA_PREFIX)/include \
	-I$(OPENSSL_PREFIX)/include \
	-I$(SDL2_PREFIX)/include \
	-I$(SDL2_PREFIX)/include/SDL2 \
	-I$(SDL2IMAGE_PREFIX)/include \
	-I$(ZLIB_PREFIX)/include \
	-I$(SRC_PATH) $(EXTRA_CFLAGS)
LDFLAGS=-L$(CURL_PREFIX)/lib \
	-L$(FREETYPE_PREFIX)/lib \
	-L$(JPEG_PREFIX)/lib \
	-L$(LIBPNG_PREFIX)/lib \
	-L$(LIBWEBSOCKETS_PREFIX)/lib \
	-L$(LUA_PREFIX)/lib \
	-L$(OPENSSL_PREFIX)/lib \
	-L$(SDL2_PREFIX)/lib \
	-L$(SDL2IMAGE_PREFIX)/lib \
	-L$(ZLIB_PREFIX)/lib \
	$(EXTRA_LDFLAGS)
LD_STATIC_LIBS=$(LUA_PREFIX)/lib/liblua.a
LD_DYNAMIC_LIBS=-lSDL2 \
	-lSDL2_image \
	-lfreetype \
	-ldl \
	-lm \
	-lpthread \
	-lcurl \
	-lwebsockets \
	-lssl \
	-lcrypto \
	-lpng \
	-ljpeg \
	-lz

all: $(PROJECT_NAME)

clean:
	rm -f $(O) $(PROJECT_NAME) $(SRC_PATH)/BuildConfig.h

$(SRC_PATH)/BuildConfig.h:
	@echo "#ifndef BUILD_CONFIG_H" > $@
	@echo "#define BUILD_CONFIG_H" >> $@
	@echo "#ifndef BUILD_ID" >> $@
	@echo "#define BUILD_ID \"$(BUILD_ID)\"" >> $@
	@echo "#endif" >> $@
	@echo "#ifndef PLATFORM_ID" >> $@
	@echo "#define PLATFORM_ID \"$(PLATFORM_ID)\"" >> $@
	@echo "#endif" >> $@
	@echo "#ifndef APPLICATION_NAME" >> $@
	@echo "#define APPLICATION_NAME \"$(APPLICATION_NAME)\"" >> $@
	@echo "#endif" >> $@
	@echo "#ifndef APPLICATION_PACKAGE_NAME" >> $@
	@echo "#define APPLICATION_PACKAGE_NAME \"$(APPLICATION_PACKAGE_NAME)\"" >> $@
	@echo "#endif" >> $@
	@echo "#endif" >> $@

$(PROJECT_NAME): $(SRC_PATH)/BuildConfig.h $(O)
	$(CC) -o $@ $(O) $(LD_STATIC_LIBS) $(LDFLAGS) $(LD_DYNAMIC_LIBS)

.SECONDARY: $(O)

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

json-parser.o: json-parser.c
	$(CC) -o $@ -g -c $<

json-builder.o: json-builder.c
	$(CC) -o $@ -g -c $<
