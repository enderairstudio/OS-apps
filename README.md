# OS Apps

A collection of native desktop utility applications and tools designed to run within custom operating system environments or native desktop configurations (built primarily using C++, Qt 6, and the Win32 API).

All applications in this repository are licensed under the **Apache License 2.0**.

---

## Applications Directory

### 1. [Camera](file:///C:/Users/Liam/Desktop/OS%20apps/Camera)
A native C++/Qt 6 camera utility designed to interface with multimedia input devices.
- **Key Features**:
  - Live video streaming and capture.
  - Image filters (Grayscale, Sepia, Invert, Warm, Cool, etc.).
  - Countdown timer for snapping photos.
  - Scan and view thumbnails of recently captured photos.
- **Technology**: C++17, Qt 6 (`Widgets` and `Multimedia`).

### 2. [DaCoder](file:///C:/Users/Liam/Desktop/OS%20apps/DaCoder)
A command-line image processing and encoding tool that handles a custom image file format called **PFF** (Pixel File/Filter Format).
- **Key Features**:
  - Info extraction, encoding, and decoding of `.pff` files.
  - Standard image conversion support (PNG, JPG, BMP) using OpenCV.
  - Compression support using Run-Length Encoding (RLE).
  - Pre-encoding filter application (Grayscale, Sepia, Invert, Warm, Cool).
- **Technology**: C++17, OpenCV, and a shared library parser (`pff`).

### 3. [File manger](file:///C:/Users/Liam/Desktop/OS%20apps/File%20manger)
A sandboxed graphical file explorer for browsing, managing, and editing files.
- **Key Features**:
  - Sandboxed folder navigation, file deletion, and file/folder creation.
  - Clipboard integration (Copy/Paste operations).
  - Integrated text editor for reading and writing files.
  - Integrated image viewer supporting standard image media.
- **Technology**: C++17, Qt 6 (`Core`, `Gui`, `Widgets`).

### 4. [My Images](file:///C:/Users/Liam/Desktop/OS%20apps/My%20Images)
A native image gallery and photo viewing GUI companion application.
- **Key Features**:
  - Grid-based thumbnail gallery.
  - Interactive selection and viewing modes.
  - Custom parser integration to natively render and display `.pff` images alongside standard image formats.
- **Technology**: C++17, Qt 6, and the shared `pff` decoder library.

### 5. [PaperPad](file:///C:/Users/Liam/Desktop/OS%20apps/PaperPad)
A lightweight, high-performance text editor built from scratch using raw Win32 APIs for maximum performance and low memory overhead.
- **Key Features**:
  - Lightweight edit control (`EditorCtrl`) with file logging systems.
  - Sidebar panel (`SidebarCtrl`) for file-explorer navigation.
  - Native Windows UI menus (`MenuBar`), keyboard shortcuts, and modeless Find/Replace dialogs (`FindDialog`).
- **Technology**: C++17, Win32 API (`comctl32`, `comdlg32`, `user32`, `gdi32`, `shell32`, `shlwapi`).

### 6. [Polo](file:///C:/Users/Liam/Desktop/OS%20apps/Polo)
A native desktop terminal emulator inspired by Windows Terminal.
- **Key Features**:
  - Multiple tab interfaces and current running command indicators.
  - Real Unix PTY shell backend integration with custom shell hooks.
  - Integrated C++ command parsing logic, terminal command history, and custom built-in utilities (`say`, `clip3d`, `3d`, `clear`, `cls`).
  - True 3D mode (wireframe rotating cube projections via `Clip3DWidget`).
  - Terminal syntax highlighting, ANSI SGR color code parser, and ANSI escape support.
- **Technology**: C++17, Qt 6, OpenGL, Unix PTY APIs.

### 7. [Task monitor](file:///C:/Users/Liam/Desktop/OS%20apps/Task%20monitor)
A system activity and hardware resource utilization monitor.
- **Key Features**:
  - Beautiful circular progress bars tracking hardware performance metrics.
  - Live system resource collection (CPU load averages, GPU usage for Intel/AMD/Nvidia hardware, RAM/Swap usage).
  - Storage drive inspection showing mount points, usage distribution, and partition capacity.
  - Active network statistics including bandwidth traffic speeds (upload/download), active interfaces, local IP mapping, and Wi-Fi SSID.
- **Technology**: C++17, Qt 6 (`Core`, `Gui`, `Widgets`).

---

## License

This project and all contained subfolders are licensed under the Apache License 2.0.

```text
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```
