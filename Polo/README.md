# Polo

Polo is a native C++ terminal-style app inspired by Windows Terminal. The current desktop build uses Qt 6 for the cross-platform window, text input, tabs, and drawing. On Unix-like systems it uses a real PTY shell backend.

## Features

- Black terminal surface with white text
- Directory path shown above the command area
- Custom top bar with `>_` tab logo, current running command, close bubble, and `+` new-tab button
- Multiple tabs
- Shell commands run through a real Unix PTY session
- Polo shell helpers: `say`, `clip3d`, `3d`, `clear`, `cls`, `exit`
- UTF-8 input/output support, including symbols and emojis when the OS font supports them
- Command names are highlighted yellow at the prompt
- Invalid commands and built-in errors are shown in red
- ANSI color escape output is parsed for external applications
- 3D mode clears terminal text and shows a colored rotating cube scene
- Cross-platform UI source with Qt 6
- Unix PTY support now; Windows still needs a ConPTY backend
- No welcome message

## Desktop Requirements

To build and run the current Polo desktop app, the host system needs:

- C++17 compiler such as `g++`, `clang++`, MSVC, or MinGW
- Qt 6 development libraries
- Qt modules: `Core`, `Gui`, `Widgets`, `OpenGL`, and `OpenGLWidgets`
- `qmake6`
- A desktop window system
- Keyboard and mouse input
- Font rendering with UTF-8 support
- A filesystem with current-directory support
- Process launching support for external commands
- Unix-like builds need PTY headers such as `pty.h`

Linux packages normally include Qt headers, Qt libraries, `qmake6`, and a build tool such as `make`.

Windows builds need Qt 6 for MSVC or MinGW plus the matching compiler toolchain. Use `nmake` for MSVC builds or `mingw32-make` for MinGW builds.

Windows note: the UI builds with Qt, but real terminal behavior requires a ConPTY implementation. The current PTY backend is Unix-focused.

## Build

Linux:

```bash
qmake6 Polo.pro
make
./Polo
```

Windows with Qt:

```bat
qmake6 Polo.pro
nmake
Polo.exe
```

MinGW builds can use `mingw32-make` instead of `nmake`.

## Commands

```text
say hello
cd ..
pwd
clip3d
clear
exit
```

Press `Esc` while viewing `clip3d` to return to the terminal prompt.

## Terminal Backend

Polo now starts a shell inside each tab through a PTY on Unix-like systems. Commands stream output live and interactive programs get a real terminal-style input/output channel.

The Unix startup path uses `bash` with `assets/polo_bashrc` when `/bin/bash` is available. That file defines Polo's `say` command, red invalid-command errors, the `> ` prompt, and path updates for the top directory label.

Known remaining terminal-emulator gaps:

- Windows ConPTY backend
- Full ANSI cursor movement and alternate-screen support
- Perfect behavior for programs like `vim`, `top`, and complex TUIs
- Rich copy/paste and selection behavior

## Color System

Polo uses a small color language for terminal clarity:

- The first command word after `>` is yellow.
- Normal output is white.
- Errors are red.
- External programs can print ANSI SGR colors such as `\x1b[31mred\x1b[0m`.

Supported ANSI foreground colors include normal colors, bright colors, 256-color foreground codes, and truecolor foreground codes.

## OS Port Requirements

Polo can be ported into a custom C++ OS, but the current Qt UI will not run there unless the OS can provide Qt or a compatible desktop layer.

For a native OS port, the OS should provide:

- A framebuffer, compositor, or window API
- 2D drawing primitives for rectangles, text, and polygons
- Keyboard input with text events
- A font renderer
- UTF-8 text handling
- A filesystem API with absolute paths and directory changes
- A process/task launcher if external commands should run
- A timer API for animations
- Memory allocation and basic C++ runtime support
- Optional clipboard support
- Optional GPU/OpenGL support if future 3D rendering moves back to hardware acceleration

Reusable Polo pieces:

- `src/core/CommandParser.*`
- `src/core/History.*`
- The command/session design from `src/core/ShellSession.*`
- The cube projection math from `src/render/Clip3DWidget.*`

Parts that must be replaced for a custom OS:

- Qt widgets and layouts in `src/ui`
- `QProcess` external command launching
- Qt filesystem/path helpers if the OS has its own filesystem API
- Qt resource loading for icons and themes

A good port structure would be:

```text
polo-core      command parsing, history, sessions, built-ins
polo-ui-qt     current desktop UI
polo-ui-os     native UI for your OS framebuffer/window system
```
