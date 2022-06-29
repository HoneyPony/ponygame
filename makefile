CC?=gcc

LINK=mingw32 SDL2main SDL2 opengl32 gdi32 glew32

#STATIC_SDL_FLAGS=-static -mwindows -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -static-libgcc -lsetupapi
#shell32

SRC_DIR=src
OBJ_DIR?=build

BIN:=ponygame

PRIMARY_SRC:=$(addprefix $(SRC_DIR)/,$(PRIMARY_SRC))

SRC=\
pony_main.c\
pony_clib.c\
pony_list.c\
pony_string.c\
pony_node_init.c\
pony_node.c\
pony_glm.c\
render/render.c\
render/shader.c\
script.c

SHADER_SRC=\
sprite.frag\
sprite.vert\
frame.frag\
frame.vert

CFLAGS:=$(CFLAGS) -Wall -g -Isrc

OBJ:=$(addsuffix .o,$(SRC))
OBJ:=$(addprefix $(OBJ_DIR)/,$(OBJ))

SHADER_C:=$(addsuffix .c,$(SHADER_SRC))
SHADER_C:=$(addprefix $(OBJ_DIR)/shaders/,$(SHADER_C))

SRC_C:=$(addprefix $(SRC_DIR)/,$(SRC))

SHADER_OBJ:=$(addsuffix .o,$(SHADER_C))

.PHONY: all clean unity install lib
.PRECIOUS: $(OBJ_DIR)/shaders/%.c

# In order to ensure the build directory is created, we need a way to depend
# on it.
# I used to, essentially, just run mkdir whenever building a file... but that
# measurably slows down compilation. e.g. -j8 can be especially fast once we
# remove those extra directory creations.
DIR_LOCK=build/build.lock

all: $(BIN)

$(OBJ_DIR)/shaders/%.c.o: $(OBJ_DIR)/shaders/%.c $(DIR_LOCK)
	$(CC) -c $< -o $@ -O2

$(OBJ_DIR)/shaders/%.c: shader_src/% shader2c $(DIR_LOCK)
	./shader2c $< $@

$(OBJ_DIR)/%.c.o: $(SRC_DIR)/%.c $(DIR_LOCK)
	$(CC) -MD -c $< -o $@ $(CFLAGS) -Iinclude -O2

$(BIN): $(OBJ) $(SHADER_OBJ)
	$(CC) -o $@ $^ $(CXXFLAGS) $(addprefix -l,$(LINK))

$(DIR_LOCK):
	mkdir -p build
	mkdir -p build/shaders
	mkdir -p build/render
	touch $@

unity: $(SHADER_C)
	$(CC) $(SRC_C) $(SHADER_C) -o $(addsuffix _unity,$(BIN)) -O2 $(addprefix -l,$(LINK))

lib: $(OBJ) $(SHADER_OBJ)
	$(CC) -o ponygame.o $^ $(addprefix -l,$(LINK))
	ar rcs libponygame.a ponygame.o 

shader2c: tools/shader2c.c
	$(CC) $< -o $@ -O2

clean:
	rm -f $(BIN)
	rm -f shader2c
	rm -rf $(OBJ_DIR)
	
install: $(BIN)
	cp $(BIN) /usr/bin/$(BIN)

web: $(SHADER_C)
	echo emcc -Isrc -sUSE_SDL=2 -o html-build/index.js $(SRC_C) $(SHADER_C) > web-build.bat
#	emcc -o html-build/index.html $(addprefix $(SRC_DIR)/,$(SRC))

-include $(OBJ:.o=.d)