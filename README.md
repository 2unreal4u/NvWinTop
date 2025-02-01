# NvWinTop

```
 _   _       __        ___       _____ 
| \ | |      \ \      / (_)_ __ |_   _|__  _ __  
|  \| |_   __ \ \ /\ / /| | '_ \  | |/ _ \| '_ \ 
| |\  \ \ / /  \ V  V / | | | | | | | (_) | |_) |
|_| \_|\_/\_/   \_/\_/  |_|_| |_| |_|\___/| .__/ 
                                           |_|    
```

A modern GPU monitoring tool built with Direct2D and NVML, created in Windsurf IDE.

## Overview

NvWinTop is a lightweight, real-time GPU monitoring tool for NVIDIA graphics cards. It features a clean, dark-mode interface with color-coded graphs for easy performance monitoring.

## Features

- 🎯 Real-time GPU statistics monitoring
- 🎨 Modern dark theme with Direct2D graphics
- 📊 Color-coded utilization graphs
  - Green (0-60%): Normal usage
  - Yellow (60-80%): Moderate usage
  - Red (80-100%): High usage
- 🌡️ Temperature monitoring with accurate readings
- ⚡ Power usage tracking with dynamic scaling
- 💾 Memory utilization graphs
- 🖥️ Multi-GPU support with clear separation

## Screenshots

[Add screenshots here]

## Requirements

- Windows 10 or later
- NVIDIA GPU with updated drivers
- CUDA Toolkit

## Installation

1. Download the latest release
2. Extract to your desired location
3. Run `setup.ps1` as administrator to verify system requirements
4. Launch `NvWinTop.exe`

## Building from Source

### Prerequisites

- Visual Studio 2019 or later with C++ workload
- CMake 3.15 or later
- CUDA Toolkit
- Windows SDK 10.0 or later

### Build Steps

```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Project Structure

- `src/` - Source files
  - `main.cpp` - Application entry point and window creation
  - `gpu_monitor.cpp` - GPU monitoring using NVML
  - `graph_renderer.cpp` - Graph rendering using Direct2D
  - `window.cpp` - Window management and message handling
- `include/` - Header files
  - `gpu_monitor.hpp` - GPU monitoring class definitions
  - `graph_renderer.hpp` - Graph rendering class definitions
  - `window.hpp` - Window class definitions
- `CMakeLists.txt` - CMake build configuration
- `setup.ps1` - System requirements verification script

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Built using the NVIDIA Management Library (NVML)
- Graphics rendered with Microsoft Direct2D
- Created in Windsurf IDE
