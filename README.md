# opengl_test

## Build

This project uses CMake and the same OpenGL/GLEW/GLFW dependencies as the original shell script.

### Prerequisites

On Ubuntu or Debian, install the required dev packages:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libgl1-mesa-dev libglew-dev libglfw3-dev gdb
```

### Configure and build

From the project root:

```bash
cmake -S . -B build
cmake --build build
```

The executable is created at `build/test`.

### Run

```bash
./build/test
```

The executable expects to be launched with the workspace root as the working directory so repo-relative assets like `data/graph/...` and `data/models/...` resolve correctly.

## Debugging

### In VS Code

1. Open the Run and Debug view.
2. Select `(gdb) Launch CMake`.
3. Press `F5`.

That configuration runs the CMake build task first, then launches `build/test` under `gdb` with the workspace root as the working directory.

### Run under gdb manually

```bash
gdb --args ./build/test
```

Then inside gdb:

```gdb
run
bt
break main
break game.cpp:...   # replace with the file/line you want to inspect
next
step
print variable_name
continue
```

### Common issue

If CMake complains about missing packages, install the development libraries and re-run:

```bash
sudo apt-get install -y libgl1-mesa-dev libglew-dev libglfw3-dev
```

### Notes

- The project bundles the audio engine source in `engine/sound/miniaudio.c` and links it into the final executable.
- The CMake target is named `test` so the output matches the legacy build script.

## Text Rendering

The engine now supports bitmap-font text rendering through `Engine::PrintText(...)`.

1. Prepare a BMFont text metadata file (`.fnt`) and matching atlas texture (for example `font.png`).
2. Load metadata and texture:

```cpp
BitmapFont uiFont;
uiFont.LoadMetadata("data/graph/font.fnt");
uiFont.SetTexture(engine->LoadTexture("data/graph/font.png"));
```

3. Draw text each frame after `UpdateStart()`:

```cpp
engine->PrintText(uiFont, 16.0f, 24.0f, "Hello OpenGL", 1.0f, 1.0f, 1.0f, 0.0f);
```

`PrintText` renders each glyph via the existing `Plane` quad path, using per-glyph UV coordinates from the font atlas.

## 3D Model Demo

The engine now includes a minimal OBJ + MTL mesh loader for textured 3D geometry. The current game startup loads `data/models/test_cube.obj`, resolves its diffuse texture from `data/models/test_cube.mtl`, and renders it through the new fixed-function 3D pass before the existing 2D sprites and text.
