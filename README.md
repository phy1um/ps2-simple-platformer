# PS2 Simple Platformer (name WIP)

A 2D platformer with a large world and streaming assets, running on
real Playstation 2 hardware.

## Playing

Pre-requisites:
* Anything needed by `PS2SDK` _or_ Docker
* python >= 3.4

1. Install the [PS2DEV](https://github.com/ps2dev/ps2dev) toolchain + PS2SDK
1. `make wad` to build game assets
1. `make -C src` to build the main `game.elf` binary
1. Run using [wLaunchELF](https://github.com/ps2homebrew/wLaunchELF), [ps2client](https://github.com/ps2dev/ps2client), or
another homebrew launcher

## Building Levels

Levels are authored in LDTK. Any image assets referenced must be in the `assets` folder,
and saved as both a `.png` and `.tga` (for now). Look at `game_world.ldtk` for examples
of loading zones, decorations and how the tilemaps work.

Layers of type `Tiles` are decorative, layers of type `IntGrid` are used for collision.
There is currently no support for depth, this is a work in progress.

