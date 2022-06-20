CC?=gcc

LINK=mingw32 SDL2main SDL2 opengl32 gdi32\
#shell32

SRC_DIR=src
OBJ_DIR?=build

BIN:=ponygame.exe

PRIMARY_SRC:=$(addprefix $(SRC_DIR)/,$(PRIMARY_SRC))

SRC=\
main.c\
pony_clib.c\
pony_data_structure.c\
pony_node_init.c\
pony_node.c\
pony_object.c\
script.c

CFLAGS:=$(CFLAGS) -Wall -g -Isrc

OBJ:=$(addsuffix .o,$(SRC))
OBJ:=$(addprefix $(OBJ_DIR)/,$(OBJ))

space:=
space+=
mkpath=rm -rf $(1) && mkdir -p $(1) && rm -rf $(1)

.PHONY: all clean unity install

all: $(BIN)
	
$(OBJ_DIR)/%.c.o: $(SRC_DIR)/%.c
	@$(call mkpath,$@)
	$(CC) -MD -c $< -o $@ $(CFLAGS) -Iinclude -O2

$(BIN): $(OBJ)
	$(CC) -o $@ $^ $(CXXFLAGS) $(addprefix -l,$(LINK))
	
clean:
	rm -f $(BIN)
	rm -rf $(OBJ_DIR)
	
install: $(BIN)
	cp $(BIN) /usr/bin/$(BIN)

-include $(OBJ:.o=.d)