# In order to build with Emscripten when using MinGW, we have to call emcc,
# which is not available through MinGW itself. As such, we need to have a path
# to emcc, which will then work fine as a regular windows executable.

# Set EMCC to the path to emcc. If the emsdk is stored right in
# the C directory, you can simply use the commented out example path.

# This file must be renamed to 'config.mak' in order to configure the makefile
# properly.

EMCC=
#EMCC=/c/emsdk/upstream/emscripten/emcc

EMAR=
