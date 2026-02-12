# ------------------------------------------------------------
# CUPDAQ top-level CMakeLists.txt
# ------------------------------------------------------------
cmake_minimum_required ( VERSION 3.16 )
project ( CUPDAQ LANGUAGES CXX )

set ( CMAKE_CXX_STANDARD 17 )
set ( CMAKE_CXX_STANDARD_REQUIRED ON )
set ( CMAKE_CXX_EXTENSIONS OFF )

# Installation directories
set ( CMAKE_INSTALL_BINDIR bin )
set ( CMAKE_INSTALL_LIBDIR lib )
set ( CMAKE_INSTALL_INCLUDEDIR include )

# ---- ROOT ----
find_package ( ROOT REQUIRED COMPONENTS Core RIO )
include ( ${ROOT_USE_FILE} )

# ---- libusb (via pkg-config) ----
find_package ( PkgConfig REQUIRED )
pkg_check_modules ( LIBUSB REQUIRED IMPORTED_TARGET libusb-1.0 )

# ---- yaml parser for config ----
find_package(yaml-cpp REQUIRED)

# ---- HDF5 (optional) ----
option ( CUPDAQ_ENABLE_HDF5 "Enable HDF5-based utilities" OFF )
if ( CUPDAQ_ENABLE_HDF5 )
  find_package ( HDF5 QUIET COMPONENTS C CXX HL )
  if ( HDF5_FOUND )
    message ( STATUS "HDF5 found (${HDF5_VERSION}): enabling HDF5Utils module" )
    set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2" )
    add_definitions ( -DENABLE_HDF5 )
    add_subdirectory ( HDF5Utils )
  else ()
    message ( STATUS "HDF5 not found: skipping HDF5Utils module" )
    set(CUPDAQ_ENABLE_HDF5 OFF CACHE BOOL "Enable HDF5-based utilities" FORCE)
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
