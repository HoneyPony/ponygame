-include config.mak

CC?=gcc

LINK=mingw32 SDL2main SDL2 SDL2_mixer glew32 opengl32 gdi32 

STATIC_SDL_FLAGS=-static -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -static-libgcc -lsetupapi
# Note: add -mwindows for the final build of a game probably. Not actually related
# to the static linking though.

LIB_OPTIMIZATION=-O3

SRC_DIR=src
OBJ_DIR?=build
WEB_OBJ_DIR=$(OBJ_DIR)/web

BIN:=ponygame
STATIC_BIN:=ponygame-static
TEST_BIN:=ponygame-test

PONY_BIN:=pony

PRIMARY_SRC:=$(addprefix $(SRC_DIR)/,$(PRIMARY_SRC))

SRC=\
node_sprite.c\
pony_main.c\
pony_clib.c\
pony_math.c\
pony_list.c\
pony_string.c\
pony_node_init.c\
pony_node_node.c\
pony_raw_transform.c\
pony_node.c\
pony_glm.c\
pony_fs.c\
pony_sound.c\
pony_input.c\
pony_random.c\
pony_benchtime.c\
nuklear_impl.c\
spatial_hash.c\
render/render.c\
render/shader.c\
render/gltex.c\
render/fb_renderer.c\
render/object_renderer.c\
render/sprite_renderer.c\
render/shader.c\
pony_log.c\
script.c

PONY_SRC=\
main.c\
scan.c\
directories.c\
path_list.c\
config.c\
tex_file.c\
snd_file.c\
image_pack.c\
resources.c\
dir_tree.c\
aseprite_to_tex.c\
krita_to_tex.c\
dummy_functions.c\
generated_files.c\
pony_file.c\
stb.c

SHADER_SRC=\
sprite.frag\
sprite.vert\
frame.frag\
frame.vert

TEST_SRC=\
test_runner.c\
test_list.c\
test_node.c\
test_node_ref.c\
test_transforms.c\
bench_node_construct_destruct.c

CFLAGS:=$(CFLAGS) -Wall -Wextra -g -Isrc -Ivendor

OBJ:=$(addsuffix .o,$(SRC))
OBJ:=$(addprefix $(OBJ_DIR)/,$(OBJ))

SHADER_C:=$(addsuffix .c,$(SHADER_SRC))
SHADER_C:=$(addprefix $(OBJ_DIR)/shaders/,$(SHADER_C))

SRC_C:=$(addprefix $(SRC_DIR)/,$(SRC))

SHADER_OBJ:=$(addsuffix .o,$(SHADER_C))

WEB_OBJ:=$(OBJ) $(SHADER_OBJ)
WEB_OBJ:=$(subst build/,build/web/,$(WEB_OBJ))

TEST_OBJ:=$(addsuffix .o,$(TEST_SRC))
TEST_OBJ:=$(addprefix $(OBJ_DIR)/tests/,$(TEST_OBJ))

TEST_SRC_C:=$(addprefix tests/,$(TEST_SRC))

OBJ_NO_MAIN:=$(filter-out build/pony_main.c.o,$(OBJ))

PONY_OBJ:=$(addsuffix .o,$(PONY_SRC))
PONY_OBJ:=$(addprefix $(OBJ_DIR)/pony/,$(PONY_OBJ))

.PHONY: all clean unity install lib run-tests no-test
.PRECIOUS: $(OBJ_DIR)/shaders/%.c

# In order to ensure the build directory is created, we need a way to depend
# on it.
# I used to, essentially, just run mkdir whenever building a file... but that
# measurably slows down compilation. e.g. -j8 can be especially fast once we
# remove those extra directory creations.
DIR_LOCK=build/build.lock

# Due to pony.main.h, it is currently not possible to build the previous $(BIN)
# this can be re-enabled with a little bit more configuration.
# As it stands though, it is nice to work on getting this stuff all usable
# with a real game project.
all: lib $(PONY_BIN)
no-test: $(BIN)

$(OBJ_DIR)/shaders/%.c.o: $(OBJ_DIR)/shaders/%.c $(DIR_LOCK)
	$(CC) -c $< -o $@ $(LIB_OPTIMIZATION)

$(OBJ_DIR)/shaders/%.c: shader_src/% shader2c $(DIR_LOCK)
	./shader2c $< $@

$(OBJ_DIR)/%.c.o: $(SRC_DIR)/%.c $(DIR_LOCK)
	$(CC) -MD -c $< -o $@ $(CFLAGS) -Iinclude $(LIB_OPTIMIZATION)

$(WEB_OBJ_DIR)/shaders/%.c.o: $(OBJ_DIR)/shaders/%.c $(DIR_LOCK)
	$(EMCC) -c $< -o $@ $(LIB_OPTIMIZATION)

$(WEB_OBJ_DIR)/%.c.o: $(SRC_DIR)/%.c $(DIR_LOCK)
	$(EMCC) -MD -c $< -o $@ $(CFLAGS) -Iinclude $(LIB_OPTIMIZATION) -sUSE_SDL_MIXER=2

$(BIN): $(OBJ) $(SHADER_OBJ)
	$(CC) -o $@ $^ $(addprefix -l,$(LINK))

$(PONY_BIN): $(PONY_OBJ) $(OBJ_NO_MAIN) $(SHADER_OBJ)
# The 'pony' tool requires cJSON in order to parse aseprite files
	$(CC) -o $@ $^ $(addprefix -l,$(LINK)) -lcjson

$(OBJ_DIR)/tests/%.c.o: tests/%.c $(DIR_LOCK)
	$(CC) -MD -c $< -o $@ $(CFLAGS) -Iinclude -O0

$(OBJ_DIR)/pony/%.c.o: pony_src/%.c $(DIR_LOCK)
	$(CC) -MD -g -c $< -o $@ $(CFLAGS) -Iinclude -O0

$(TEST_BIN): $(OBJ_NO_MAIN) $(TEST_OBJ) $(SHADER_OBJ)
	$(CC) -o $@ $^ $(addprefix -l,$(LINK))

$(DIR_LOCK):
	mkdir -p build
	mkdir -p build/shaders
	mkdir -p build/render
	mkdir -p build/tests
	mkdir -p build/pony

	mkdir -p build/web
	mkdir -p build/web/shaders
	mkdir -p build/web/render
	mkdir -p build/web/tests
	touch $@

unity: $(SHADER_C)
	$(CC) $(SRC_C) $(SHADER_C) -o $(addsuffix _unity,$(BIN)) -O2 $(addprefix -l,$(LINK))

lib: $(OBJ) $(SHADER_OBJ)
	ar rcs libponygame.a $(OBJ) $(SHADER_OBJ)

shader2c: tools/shader2c.c
	$(CC) $< -o $@ -O2

clean:
	rm -f $(BIN)
	rm -f shader2c
	rm -rf $(OBJ_DIR)
	
install: $(BIN)
	cp $(BIN) /usr/bin/$(BIN)

run-tests: $(TEST_BIN)
	./$(TEST_BIN)

run-bench: $(TEST_BIN)
	./$(TEST_BIN) bench

web: $(WEB_OBJ)
	$(EMAR) rcs libponygame_web.a $(WEB_OBJ)
#	echo call $(EMSDK_ENV_BAT) > web-build.bat
#	echo @echo on >> web-build.bat
#	echo $(EMCC) --preload-file test_sprite.png -Isrc -Ivendor -sUSE_SDL=2 -o html-build/index.js $(SRC_C) $(SHADER_C) > web-build.bat
#	mkdir -p html-build
#	cp web_src/basic_shell.html html-build/index.html

#	$(EMCC) --preload-file test_sprite.png -Isrc -Ivendor -sUSE_SDL=2 -o html-build/index.js $(SRC_C) $(SHADER_C)

	

#	cmd /c web-build.bat
#	emcc -o html-build/index.html $(addprefix $(SRC_DIR)/,$(SRC))

$(STATIC_BIN): $(OBJ) $(SHADER_OBJ)
	$(CC) -o $@ $^ $(addprefix -l,$(LINK)) $(STATIC_SDL_FLAGS)

-include $(OBJ:.o=.d)
-include $(TEST_OBJ:.o=.d)
-include $(PONY_OBJ:.o=.d)