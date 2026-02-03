# SerEngine

This is a personal game engine project I'm building from the ground up using C++, SDL2, and OpenGL. The goal is to understand how the low-level systems of an engine actually work by implementing things like a custom event system, renderer abstractions, and a layer-based architecture.

## How it's structured

- **Core**: Handles the app loop, windowing (via SDL), and basic input polling.
- **Events**: A generic event dispatcher used for everything from window resizing to key presses.
- **Renderer**: An abstraction over OpenGL 3.3. It handles shaders, vertex buffers, and has a 2D orthographic camera system.
- **Layers**: You can push game logic or UI overlays onto a stack. Events propagate down, allowing overlays to "block" input.

## Dependencies

- **SDL2**: Windowing and input context.
- **libepoxy**: Handles OpenGL function loading (much easier than glad).
- **GLM**: Math (vectors/matrices).
- **spdlog**: Fast logging.

## Build and Run

I'm using CMake. The engine builds as a shared library, and there's a Sandbox project that links against it.

### Build
```bash
mkdir build && cd build
cmake ..
make
```

### Run
Since it uses a shared library, you need to tell the system where to find it.
```bash
# from the build directory
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/lib
./bin/Sandbox
```

## Current State
Right now it just renders a single triangle that you can move around with the arrow keys. It's not much yet, but the plumbing (Shaders -> Buffers -> Renderer -> Window) is solid.
