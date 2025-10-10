# QGroundControl CMake Build System

This directory contains the CMake build system configuration for QGroundControl.

## Requirements

- **CMake 3.25+**: Modern CMake features including presets and improved Qt integration
- **Qt 6.8.3+**: Qt6 with QML, Quick, Multimedia, and other required components
- **C++20 Compiler**: GCC 10+, Clang 12+, or MSVC 2019+
- **Ninja** (recommended): Fast build system

## Quick Start

### Using CMake Presets (Recommended)

CMake presets provide pre-configured build configurations:

```bash
# List available presets
cmake --list-presets

# Configure with a preset
cmake --preset linux-debug
# or
cmake --preset macos-release
# or
cmake --preset windows-debug

# Build
cmake --build --preset linux-debug

# Run tests
ctest --preset linux-debug

# Full workflow (configure + build + test)
cmake --workflow --preset linux-full
```

### Traditional CMake

```bash
# Configure
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build

# Install
cmake --install build
```

## Directory Structure

```
cmake/
├── CMakeLists.txt              # Main build configuration (in parent dir)
├── CMakePresets.json           # Build presets (in parent dir)
├── CMakeUserPresets.json.example  # User customization template
├── CustomOptions.cmake         # Build options and feature flags
├── Helpers.cmake               # Helper functions for build system
├── PrintSummary.cmake          # Build configuration summary
├── Toolchain.cmake             # Compiler and toolchain settings
├── CPack/                      # Packaging configurations
├── find-modules/               # Custom Find modules
│   └── FindGStreamer.cmake
├── install/                    # Installation scripts
│   ├── Install.cmake
│   ├── CreateAppImage.cmake    # Linux AppImage generation
│   ├── CreateMacDMG.cmake      # macOS DMG creation
│   └── CreateWinInstaller.cmake # Windows installer
├── modules/                    # Additional CMake modules
│   ├── Git.cmake               # Git version extraction
│   ├── CPM.cmake               # CPM dependency manager
│   └── vulkan.cmake            # Vulkan configuration
└── platform/                   # Platform-specific configurations
    ├── Android.cmake
    ├── Apple.cmake
    ├── Linux.cmake
    └── Windows.cmake
```

## Build Options

Key CMake cache variables that control the build:

### Core Build Options

- `CMAKE_BUILD_TYPE`: Build type (Debug, Release, RelWithDebInfo, MinSizeRel)
- `QGC_STABLE_BUILD`: Enable stable build mode (OFF for daily builds)
- `QGC_USE_CACHE`: Use ccache/sccache for faster rebuilds (default: ON)
- `QGC_BUILD_TESTING`: Build unit tests (default: ON for Debug)
- `QGC_DEBUG_QML`: Enable QML debugging/profiling (default: ON for Debug)

### Feature Flags

- `QGC_VIEWER3D`: Enable 3D viewer (requires Qt6Quick3D)
- `QGC_UTM_ADAPTER`: Enable UTM adapter support
- `QGC_ENABLE_BLUETOOTH`: Enable Bluetooth links
- `QGC_ENABLE_GST_VIDEOSTREAMING`: Enable GStreamer video backend
- `QGC_ENABLE_QT_VIDEOSTREAMING`: Enable Qt Multimedia video backend
- `QGC_NO_SERIAL_LINK`: Disable serial port support

### Plugin Options

- `QGC_DISABLE_APM_MAVLINK`: Disable ArduPilot MAVLink dialect
- `QGC_DISABLE_APM_PLUGIN`: Disable ArduPilot plugin
- `QGC_DISABLE_PX4_PLUGIN`: Disable PX4 plugin

### Platform-Specific Options

#### Linux
- `QGC_CREATE_APPIMAGE`: Create AppImage after build (default: ON)

#### macOS
- `QGC_MACOS_UNIVERSAL_BUILD`: Build Universal binary (x86_64 + arm64)
- `CMAKE_OSX_DEPLOYMENT_TARGET`: Minimum macOS version (default: 12.0)

#### Android
- `QGC_QT_ANDROID_TARGET_SDK_VERSION`: Target Android SDK version
- `QT_ANDROID_SIGN_APK`: Enable APK signing

## CMake Modules

### Helpers.cmake

Provides utility functions:

- `qgc_set_qt_resource_alias()`: Set Qt resource aliases
- `qgc_config_caching()`: Configure ccache/sccache
- `qgc_set_linker()`: Select fastest available linker (mold/lld/gold)
- `qgc_enable_pie()`: Enable Position Independent Executable
- `qgc_enable_ipo()`: Enable Link-Time Optimization

### Git.cmake

Extracts version information from Git repository:

- `QGC_GIT_BRANCH`: Current Git branch
- `QGC_GIT_HASH`: Short commit hash
- `QGC_APP_VERSION`: Application version from tags
- `QGC_APP_VERSION_STR`: Full version string
- `QGC_APP_DATE`: Build date

### Toolchain.cmake

Configures compiler and build system:

- Sets C++20 standard
- Enables Qt auto-generation tools (AUTOMOC, AUTOUIC, AUTORCC)
- Configures optimization flags
- Platform-specific compiler settings

## Custom Builds

QGroundControl supports custom builds that can override default settings.

1. Create `custom/` directory in project root
2. Add `custom/cmake/CustomOverrides.cmake` to override settings
3. Custom source files in `custom/src/`
4. CMake will automatically detect and include custom build

## Compiler Cache

The build system supports ccache and sccache for faster rebuilds:

```bash
# Install ccache (Linux)
sudo apt install ccache

# Install sccache
cargo install sccache

# Configure with caching enabled (default)
cmake --preset linux-debug -DQGC_USE_CACHE=ON
```

Cache statistics:
```bash
ccache -s           # ccache stats
sccache --show-stats  # sccache stats
```

## Testing

```bash
# Build with tests enabled
cmake --preset linux-debug -DQGC_BUILD_TESTING=ON

# Run all tests
ctest --preset linux-debug

# Run specific test
ctest --preset linux-debug -R MAVLinkTest

# Verbose output
ctest --preset linux-debug --output-on-failure -V
```

## Packaging

### Linux AppImage

```bash
cmake --preset linux-release
cmake --build --preset linux-release
cmake --install build/linux-release
# AppImage created in build directory
```

### macOS DMG

```bash
cmake --preset macos-release
cmake --build --preset macos-release
cmake --install build/macos-release
# DMG created in build directory
```

### Windows Installer

```bash
cmake --preset windows-release
cmake --build --preset windows-release
cmake --install build/windows-release
# NSIS installer created in build directory
```

## Troubleshooting

### CMake can't find Qt

```bash
# Set Qt path explicitly
cmake --preset debug -DCMAKE_PREFIX_PATH=/path/to/qt

# Or set environment variable
export CMAKE_PREFIX_PATH=/path/to/qt
```

### Slow builds

```bash
# Enable compiler cache
cmake --preset debug -DQGC_USE_CACHE=ON

# Use Ninja generator (faster than Make)
cmake -B build -G Ninja

# Enable unity builds (experimental)
cmake -B build -DCMAKE_UNITY_BUILD=ON
```

### Build errors after Git pull

```bash
# Clean build directory
rm -rf build/
cmake --preset linux-debug
```

## Advanced Configuration

### Using Different Qt Versions

```bash
# Specify Qt version
cmake --preset debug \
  -DCMAKE_PREFIX_PATH=/opt/Qt/6.8.3/gcc_64 \
  -DQt6_DIR=/opt/Qt/6.8.3/gcc_64/lib/cmake/Qt6
```

### Cross-Compilation

```bash
# Android
cmake -B build-android \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DQT_HOST_PATH=/path/to/desktop/qt

# iOS (requires macOS)
cmake -B build-ios \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_OSX_ARCHITECTURES=arm64
```

## Contributing

When modifying the build system:

1. Maintain CMake 3.25+ compatibility
2. Test on all platforms (Linux, macOS, Windows)
3. Update this documentation
4. Add comments to complex CMake code
5. Use modern CMake practices (targets, not variables)

## Resources

- [CMake Documentation](https://cmake.org/documentation/)
- [CMake Presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
- [Qt6 CMake Manual](https://doc.qt.io/qt-6/cmake-manual.html)
- [CPM Package Manager](https://github.com/cpm-cmake/CPM.cmake)
