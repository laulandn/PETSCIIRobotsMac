#CXXFLAGS :=-g -std=c++11 -fsigned-char -DPLATFORM_NAME=\"sdl\" -DPLATFORM_SCREEN_WIDTH=440 -DPLATFORM_SCREEN_HEIGHT=224 -DPLATFORM_MAP_WINDOW_TILES_WIDTH=16 -DPLATFORM_MAP_WINDOW_TILES_HEIGHT=8 -DPLATFORM_INTRO_OPTIONS=3 -DPLATFORM_DEFAULT_CONTROL=3 -DPLATFORM_MODULE_BASED_AUDIO -DPLATFORM_TILE_BASED_RENDERING -DPLATFORM_IMAGE_BASED_TILES -DPLATFORM_IMAGE_SUPPORT -DPLATFORM_SPRITE_SUPPORT -DPLATFORM_COLOR_SUPPORT -DPLATFORM_CURSOR_SUPPORT -DPLATFORM_CURSOR_SHAPE_SUPPORT -DPLATFORM_FADE_SUPPORT -DPLATFORM_LIVE_MAP_SUPPORT -DOPTIMIZED_MAP_RENDERING
CXXFLAGS :=-g -std=c++11 -fsigned-char 
LDFLAGS :=

#CXXFLAGS += -D_LATITUDE_ -I../more_src/mycarbon/CIncludes
#LDFLAGS += -L../more_src/mycarbon -lmycarbon_static -L../more_src/nlib -lnlib_Static
#LDFLAGS += -L/opt/local/lib -lX11

CXXFLAGS += `pkg-config --cflags sdl2` `pkg-config --cflags SDL2_image`
LDFLAGS += `pkg-config --libs sdl2` `pkg-config --libs SDL2_image`

SOURCES := petrobots.cpp Platform.cpp PlatformSDL.cpp PT2.3A_replay_cia.cpp
OBJECTS := $(SOURCES:.cpp=.o)
TARGET := petrobots

all: $(TARGET) setup

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

setup:
	SDL/setup.sh

clean:
	rm -f $(OBJECTS) $(TARGET)
