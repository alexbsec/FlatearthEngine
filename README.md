# Flatearth Engine

Flatearth Engine is an open-source project to create a game engine tailored for 2D games.
The idea originated from the observation that most open-source game engines focus primarily on 3D rendering, 
often treating 2D rendering as an afterthought. While many industry-standard game engines do support 
2D rendering, a dedicated 2D engine can offer simplicity, performance, and ease
of use for developers focusing on 2D games.

## Platform Support 

The engine is going to be primarily supported for Linux, but ultimately it will
support both Windows and Linux operating systems.

## Tech Stack

- Language: C++23
- Rendering: Vulkan

## Project Structure

1. Flatearth Engine Library
The core engine library (shared object/dynamic link library) is not executable but
is referenced by applications. It houses all core engine functionality, including rendering,
physics, and memory management.

2. Game Application
A lightweight executable that references the Flatearth Engine Library. This serves as the entry
point for games built on the engine.

3. Hot-Reloadable Code
A support library for the Game Application that allows lightweight runtime updates. It helps
to keep the game application small and facilitates rapid iteration by minimizing compilation time.

4. Testsuite
An executable designed to test specific features of the engine. Like the Game Application,
it references the Flatearth Engine Library.

5. Editor
A standalone application separate from the core engine. The editor is designed for creating and
managing game assets, and it can be run alongside the game application for real-time development.


## Features
- Lightweight build system using a batch and bash file.
- Low-level utilities (dynamic arrays, string handling, binary trees etc...)
- Platform layer for Windows and Linux
	- Windowing, input, console interaction, etc...
- Logger for debugging purposes
- File I/O layer
- Application layer
	- Managing the main game loop, updating the physics system, calling the renderer, etc...
- Renderer/API Abstraction layer
- Memory management (allocators, etc...)
- Scenegraph/ECS
- Profiling/Debugging utilities
- Scripting support via hot-reloading
- Physics system
- Support for online multiplayer games.

## Build requirements

### Linux

To build the project on Linux, we must have X11 and XCB, as these are the libraries
we are going to use to make the window.

#### Ubuntu/Debian

To install the required dependencies on Ubuntu/Debian-based distributions, use 
the following commands


```bash
sudo apt update
sudo apt install -y libx11-dev libxcb1-dev libxcb-keysyms1-dev libxcb-icccm4-dev \
libxcb-image0-dev libxcb-shm0-dev libxcb-xfixes0-dev libxcb-randr0-dev libxcb-render-util0-dev \
libxcb-xinerama0-dev libxcb-glx0-dev
```

#### RHEL/Fedora

For Red Hat-based distributions like RHEL or Fedora, install the required dependencies using dnf:

```bash
sudo dnf install -y libX11-devel libxcb-devel xcb-util-keysyms-devel \
xcb-util-devel xcb-util-image-devel xcb-util-wm-devel xcb-util-renderutil-devel
```

#### Arch Linux

On Arch Linux or its derivatives (e.g., Manjaro), you can use pacman to install the required libraries:

```bash
sudo pacman -Syu xorg-server xorg-xrandr libx11 libxcb xcb-util xcb-util-wm xcb-util-image
```

