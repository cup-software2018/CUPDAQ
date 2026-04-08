# ------------------------------------------------------------
# CUPDAQ top-level CMakeLists.txt (Linux Optimized & Relocatable)
# ------------------------------------------------------------
cmake_minimum_required ( VERSION 3.16 )
project ( CUPDAQ VERSION 1.0.0 LANGUAGES CXX )

# ---- Compilation Database ----
# Generates compile_commands.json in the build directory,
# helping IDEs and analysis tools (like clangd/Antigravity) resolve includes.
set ( CMAKE_EXPORT_COMPILE_COMMANDS ON )

# ---- C++ Standard ----
set ( CMAKE_CXX_STANDARD 17 )
set ( CMAKE_CXX_STANDARD_REQUIRED ON )
set ( CMAKE_CXX_EXTENSIONS OFF )

# Use standard GNU installation directories
include ( GNUInstallDirs )

# ---- Relocatable RPATH Settings (Linux) ----
# This ensures that installed binaries look for libraries relative to their own location.
set ( CMAKE_SKIP_BUILD_RPATH FALSE )
set ( CMAKE_BUILD_WITH_INSTALL_RPATH FALSE )
set ( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE )

# Use $ORIGIN to make the 'install' directory independent of its absolute path.
# Binaries in 'bin/' will look for libraries in '../lib64' or '../lib'.
set ( CMAKE_INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}" )

# ---- ROOT ----
find_package ( ROOT REQUIRED COMPONENTS Core RIO RHTTP )
include ( ${ROOT_USE_FILE} )

# Prevent absolute paths from being hardcoded in ROOT dictionaries
set ( CMAKE_ROOT_DICT_ARGS --noinclude )
set ( ROOT_GENERATE_DICTIONARY_RELATIVE ON )

# ---- libusb (via pkg-config) ----
find_package ( PkgConfig REQUIRED )
pkg_check_modules ( LIBUSB REQUIRED IMPORTED_TARGET libusb-1.0 )

# ---- yaml-cpp for configuration ----
find_package ( yaml-cpp REQUIRED )

# ---- zeromq (cppzmq) for message server ----
find_package ( cppzmq REQUIRED )

# ---- Global Compiler Settings ----
# Apply global optimization flags to all sub-directories
add_compile_options ( -O2 )

# ---- HDF5 (optional) ----
option ( CUPDAQ_ENABLE_HDF5 "Enable HDF5-based utilities" ON )

if ( CUPDAQ_ENABLE_HDF5 )
  find_package ( HDF5 QUIET COMPONENTS C CXX HL )

  if ( HDF5_FOUND )
    message ( STATUS "HDF5 found (${HDF5_VERSION}): enabling HDF5Utils module" )
    add_compile_definitions ( ENABLE_HDF5 )
    add_subdirectory ( HDF5Utils )
  else ()
    message ( STATUS "HDF5 not found: skipping HDF5Utils module" )
    set ( CUPDAQ_ENABLE_HDF5 OFF CACHE BOOL "Enable HDF5-based utilities" FORCE )
  endif ()
else ()
  message ( STATUS "HDF5 disabled" )
endif ()

# ---- Submodules ----
add_subdirectory ( OnlConsts )
add_subdirectory ( DAQUtils )
add_subdirectory ( DAQConfig )
add_subdirectory ( Notice )
add_subdirectory ( DAQSystem )
add_subdirectory ( OnlObjs )
add_subdirectory ( DAQTrigger )
add_subdirectory ( OnlHistogramer )
add_subdirectory ( RawObjs )
add_subdirectory ( DAQ )

# ---- Environment Setup Script Generation (setup_cupdaq.sh) ----
# Create a robust, relocatable setup script with English comments.
# Using 'builtin cd' to avoid conflicts with user-defined 'cd' functions.
file ( WRITE ${CMAKE_BINARY_DIR}/setup_cupdaq.sh
  "#!/bin/bash
# CUPDAQ Environment Setup Script
# Generated automatically by CMake.

# Get the absolute path of the directory where this script is located.
# Using 'builtin cd' to bypass any user-defined aliases or functions.
export CUPDAQ_DIR=\"\$(builtin cd \"\$(dirname \"\${BASH_SOURCE[0]}\")\" && pwd)\"

# Update system PATH
export PATH=\"\$CUPDAQ_DIR/bin:\$PATH\"

# Update library search path
export LD_LIBRARY_PATH=\"\$CUPDAQ_DIR/${CMAKE_INSTALL_LIBDIR}:\$LD_LIBRARY_PATH\"

# Update ROOT include path for dictionary and interpreter support
export ROOT_INCLUDE_PATH=\"\$CUPDAQ_DIR/include:\$ROOT_INCLUDE_PATH\"

echo \"[CUPDAQ] Environment loaded from: \$CUPDAQ_DIR\"
" )

# Install the script to the root of the installation prefix
install ( FILES ${CMAKE_BINARY_DIR}/setup_cupdaq.sh DESTINATION . )