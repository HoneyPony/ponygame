## PonyGame
PonyGame is a C game engine with a focus on pixel-perfect pixel-art games (with the eventual goal of
also supporting vector graphics and font rendering for UI elements). The engine style is based heavily
off Godot, with the primary user-facing abstraction being Nodes that can have some associated function,
and that can be composed into node trees that can be instantiated and deleted.

PonyGame uses a custom build system and custom code-generation tools in order to do more powerful things
than raw C code can do itself. For example:
- PonyGame packs all sprites into spritesheets to minimize state change during rendering
- PonyGame automatically converts sound files into .ogg format for minimal program size
- PonyGame packs all assets into the executable as binary data, making distribution simple

### Usage
PonyGame is currently not strongly recommended for general use. The only supported development environment
at the time is Windows with MinGW64. Dependencies for PonyGame include SDL2, SDL2_mixer, and GLEW. More
information on building and using PonyGame should be available ... at some point.

If you have all the dependencies installed, it should be possible to build the engine and the `pony`
executable by running `make`.

#### Making a game
Once the `pony` executable has been built, you can make a new game project by cd'ing to an empty
folder and running `pony init`. Then, to build the project, there are three steps:

1. Setup your configuration
2. Run `pony scan`
3. Run `pony build` (or `pony go` to build and run)

Right now, PonyGame unfortunately requires a lot of awkward manual configuration. Basically, you
have to edit the file `.ponygame/my.ponyconfig` after your project is generated, and specify the
include paths and library paths. If, for example, you had cloned the `ponygame` repository
to `C:\ponygame`, your `my.ponyconfig` file could look like this:
```
include_path=C:/ponygame/src
lib_path=C:/ponygame
lib_file=/ponygame/libponygame.a
```
The different file path formats are because different tools act slightly differently. Perhaps
at some point PonyGame will be able to automatically figure out the different path formats and
configure everything correctly.

However, once the project is configured, it is very straightforward to build. Running `pony scan`
causes PonyGame to look for source files and asset files, and generate some `.ninja` files based
off of what it finds. Then, running `pony build` or `pony go` will use `ninja` to build the
`build.debug.local.ninja` file, generating a working game executable.

### More to come...
Unfortunatenly, PonyGame is still in an early stage and is, at this point, intended to primarily be
used for personal use. Perhaps after Ludum Dare 51 there will be a link to an example game built
with it, though!
