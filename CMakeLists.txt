# ------------------------------------------------------------
# CUPDAQ top-level CMakeLists.txt (Linux Optimized & Relocatable)
# ------------------------------------------------------------
cmake_minimum_required ( VERSION 3.16 )
project ( CUPDAQ LANGUAGES CXX )

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

option ( CUPDAQ_AUTO_INSTALL_DEPENDENCIES "Automatically install missing system dependencies when running as root" ON )

# Platform detection, privilege detection, and auto-install helpers
include ( cmake/CUPDAQHelpers.cmake )

# ---- Install Prefix ----
# When running as root with no explicit prefix, install into a system path
if ( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
  if ( CUPDAQ_IS_ROOT )
    set ( CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install path prefix" FORCE )
    message ( STATUS "Running as root: defaulting install prefix to /usr" )
  else ()
    message ( STATUS "CMAKE_INSTALL_PREFIX not specified; defaulting to ${CMAKE_INSTALL_PREFIX}" )
  endif ()
else ()
  message ( STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}" )
endif ()

# ---- Build-tree header staging area ----
# All module headers are staged here so rootcling can resolve cross-module includes.
# Mirrors what ROOT does with HEADER_OUTPUT_PATH = CMAKE_BINARY_DIR/include.
set ( CUPDAQ_HEADER_OUTPUT_PATH ${CMAKE_BINARY_DIR}/include )
file ( MAKE_DIRECTORY ${CUPDAQ_HEADER_OUTPUT_PATH} )

# ---- Relocatable RPATH Settings (Linux) ----
# This ensures that installed binaries look for libraries relative to their own location.
set ( CMAKE_SKIP_BUILD_RPATH FALSE )
set ( CMAKE_BUILD_WITH_INSTALL_RPATH FALSE )
set ( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE )

# Use $ORIGIN to make the 'install' directory independent of its absolute path.
# Binaries in 'bin/' will look for libraries in '../lib64' or '../lib'.
set ( CMAKE_INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}" )

# ---- ROOT (pre-installed; verify only) ----
find_package ( ROOT REQUIRED COMPONENTS Core RIO RHTTP )
message ( STATUS "Found ROOT ${ROOT_VERSION}: ${ROOT_DIR}" )
include ( ${ROOT_USE_FILE} )

# ---- PkgConfig ----
find_package ( PkgConfig QUIET )
if ( NOT PkgConfig_FOUND AND CUPDAQ_AUTO_INSTALL_DEPENDENCIES AND CUPDAQ_IS_ROOT )
  if ( CUPDAQ_DNF_EXECUTABLE )
    cupdaq_install_system_packages ( pkgconf-pkg-config )
  elseif ( CUPDAQ_DEBIAN_PKG_EXECUTABLE )
    cupdaq_install_system_packages ( pkg-config )
  else ()
    cupdaq_install_system_packages ( pkgconfig )
  endif ()
  find_package ( PkgConfig REQUIRED )
else ()
  find_package ( PkgConfig REQUIRED )
endif ()

# ---- libusb (via pkg-config) ----
pkg_check_modules ( LIBUSB QUIET IMPORTED_TARGET libusb-1.0 )
if ( NOT LIBUSB_FOUND AND CUPDAQ_AUTO_INSTALL_DEPENDENCIES AND CUPDAQ_IS_ROOT )
  if ( CUPDAQ_DNF_EXECUTABLE )
    cupdaq_install_system_packages ( libusb1-devel )
  elseif ( CUPDAQ_DEBIAN_PKG_EXECUTABLE )
    cupdaq_install_system_packages ( libusb-1.0-0-dev )
  else ()
    cupdaq_install_system_packages ( libusb-devel )
  endif ()
  pkg_check_modules ( LIBUSB REQUIRED IMPORTED_TARGET libusb-1.0 )
else ()
  pkg_check_modules ( LIBUSB REQUIRED IMPORTED_TARGET libusb-1.0 )
endif ()

# ---- yaml-cpp for configuration ----
find_package ( yaml-cpp QUIET )
if ( NOT yaml-cpp_FOUND AND CUPDAQ_AUTO_INSTALL_DEPENDENCIES AND CUPDAQ_IS_ROOT )
  if ( CUPDAQ_DNF_EXECUTABLE )
    cupdaq_install_system_packages ( yaml-cpp-devel )
  elseif ( CUPDAQ_DEBIAN_PKG_EXECUTABLE )
    cupdaq_install_system_packages ( libyaml-cpp-dev )
  else ()
    cupdaq_install_system_packages ( yaml-cpp-devel )
  endif ()
  find_package ( yaml-cpp REQUIRED )
else ()
  find_package ( yaml-cpp REQUIRED )
endif ()

# ---- zeromq (cppzmq) for message server ----
find_package ( cppzmq QUIET )
if ( NOT cppzmq_FOUND AND CUPDAQ_AUTO_INSTALL_DEPENDENCIES AND CUPDAQ_IS_ROOT )
  if ( CUPDAQ_DNF_EXECUTABLE )
    cupdaq_install_system_packages ( zeromq-devel cppzmq-devel )
  elseif ( CUPDAQ_DEBIAN_PKG_EXECUTABLE )
    cupdaq_install_system_packages ( libzmq3-dev )
  else ()
    cupdaq_install_system_packages ( zeromq-devel )
  endif ()
  find_package ( cppzmq REQUIRED )
else ()
  find_package ( cppzmq REQUIRED )
endif ()

# ---- Global Compiler Settings ----
# Apply global optimization flags to all sub-directories
add_compile_options ( -O2 )

# ---- CUPDataModel (external package: provides RawObjs and optionally HDF5Utils) ----
find_package ( CUPDataModel REQUIRED )
if ( TARGET CUPDataModel::HDF5Utils )
  set ( CUPDAQ_ENABLE_HDF5 ON CACHE BOOL "HDF5 support (derived from CUPDataModel)" FORCE )
  add_compile_definitions ( ENABLE_HDF5 )
else ()
  set ( CUPDAQ_ENABLE_HDF5 OFF CACHE BOOL "HDF5 support (derived from CUPDataModel)" FORCE )
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

# ---- Package Export ----
include ( CMakePackageConfigHelpers )

configure_package_config_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CUPDAQConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CUPDAQ
)

install ( EXPORT CUPDAQTargets
    NAMESPACE CUPDAQ::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CUPDAQ
)

install ( FILES
    ${CMAKE_CURRENT_BINARY_DIR}/CUPDAQConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CUPDAQ
)

# ---- Run ldconfig after installing to a system path ----
install ( CODE "
  if (UNIX AND NOT APPLE)
    if (CMAKE_INSTALL_PREFIX MATCHES \"^(/usr|/usr/local)(/.*)?$\")
      message(STATUS \"Running ldconfig...\")
      execute_process(COMMAND ldconfig)
    endif()
  endif()
")