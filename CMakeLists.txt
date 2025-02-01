# Minimum CMake version required
cmake_minimum_required(VERSION 3.15)

# Project name and version
project(NvWinTop VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find CUDA package for NVML
find_package(CUDAToolkit REQUIRED)

# Add source files
set(SOURCES
    src/main.cpp
    src/gpu_monitor.cpp
    src/graph_renderer.cpp
    src/window.cpp
)

# Add header files
set(HEADERS
    include/gpu_monitor.hpp
    include/graph_renderer.hpp
    include/window.hpp
)

# Create executable
add_executable(NvWinTop ${SOURCES} ${HEADERS})

# Add include directories
target_include_directories(NvWinTop PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CUDAToolkit_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(NvWinTop PRIVATE
    ${CUDA_nvml_LIBRARY}  # NVML for GPU monitoring
    d2d1                  # Direct2D for graphics
    dwrite                # DirectWrite for text rendering
)

# Set Windows subsystem
set_target_properties(NvWinTop PROPERTIES
    WIN32_EXECUTABLE TRUE  # Create a Windows GUI application
)
