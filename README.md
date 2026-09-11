Emulation of 8080 CPU and space invaders cabinet
Status:
1. All main game functionality working (Sound, 2player) - driver.c
2. Emulator works entirely for Space Invaders with all opcodes
correct
3. CPU Tests work (see cpu_tests folder)

## Build

Requires SDL2, SDL2_mixer, CMake, and pkg-config (macOS: `brew install sdl2 sdl2_mixer cmake pkg-config`).

```bash
mkdir build && cd build
cmake ..
make
```

This produces two binaries in `build/bin/`:
- `SpaceInvadersTest` — the Space Invaders game (driver.c + full machine/graphics/audio).
- `Diag8080` — a standalone, SDL-free 8080 core runner for exercising the CPU against CP/M-style test binaries (see Testing below).

## Run the game

The game's sound loading uses a path relative to the current working directory (`../../sounds/...`), so run it from `build/bin/`:

```bash
cd build/bin
./SpaceInvadersTest ../../invaders.h ../../invaders.g ../../invaders.f ../../invaders.e
```

Controls: Left/Right arrows to move, Space to shoot, C to insert coin, Return (P1 start) / P (P2 start), T to tilt.
