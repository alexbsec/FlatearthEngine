# Flatearth Engine

Flatearth engine is a project to create a game engine tailored for 2D games.
The idea came from the fact that most engines focus solely on 3D rendering over 2D rendering. 
Even though most industry level game engines do have 2D rendering support, I feel that an 
engine focused solely on 2D rendering could come in handy for many developers and myself. 

## Platform Support 

The engine is going to be primarily supported for Linux, but ultimately it will
support both Windows and Linux operating systems.

## Tech Stack

The game is going to be powered by C++23 as its core language and Vulkan for rendering.

## Project Structure

We shall have the main Flatearth Engine entrypoint, which will be the main library 
of code (dll and so) which is not executable, but it is referenced by the application. 
All of the core objects are going to be stored in the Flatearth Engine. 

The Game Application will reference the Flatearth Engine library entrypoint. 
This should be a lightweight executable.

The Hot-Reloadable Code will serve as a support for the Game Application so that
we can make it lightweight. This is also going to be a library (dll and so). The 
idea is to have a lightweight Game Application running and make minor changes to our 
game code using the Hot-Reloadable Code. The Hot-Reloadable Code should be responsible to 
keep the compilation time fastar, and it also references the Flatearth Engine library.

The Testsuite is going to be an executable similar to the Game Application, except that 
it is only meant for testing specific game engine features. This also references the Flatearth 
Engine library.

Finally, the Editor is going to be an application on its own, and is going to stand
separately from the Flatearth Engine library. The editor source code is going to be shipped 
separately from the core engine. This is to make the engine even more lightweight and will 
let us open the editor and the engine at the same time as two separate applications.


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
- Networking for online multiplayer

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

