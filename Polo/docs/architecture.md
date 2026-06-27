# Polo Architecture

Polo is split into small components so the terminal behavior, UI, and 3D renderer can evolve independently.

- `src/app`: application entry point.
- `src/core`: command parsing, current directory handling, history, legacy command execution, and the Unix PTY session backend.
- `src/ui`: terminal surface, top bar, tab model, and main window.
- `src/render`: OpenGL 3D clip widget.
- `scripts`: helper scripts for local builds.

The desktop UI now talks to `PtySession` for Unix-like real terminal behavior. `ShellSession` remains as a simple command-runner implementation that can be useful for ports or fallback modes, but the active UI path uses PTY-backed shell tabs.
