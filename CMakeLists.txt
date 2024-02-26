cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(CUPDAQ)

#
# Locate the ROOT package and define a number of useful targets and variables.
#
find_package(ROOT REQUIRED)
include(${ROOT_USE_FILE})

# Find libusb-1.0 package
find_package(PkgConfig)
pkg_check_modules(libusb-1.0 REQUIRED libusb-1.0)
include_directories(${libusb-1.0_INCLUDE_DIRS})

# -----------------------------------------------------------------------------

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include)
set(PROJECT_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)

set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX}/bin)

include_directories(${PROJECT_INCLUDE_DIR})

add_subdirectory(OnlConsts EXCLUDE_FROM_ALL)
add_subdirectory(Notice)
add_subdirectory(OnlObjs)
add_subdirectory(RawObjs)
add_subdirectory(DAQUtils)
add_subdirectory(DAQConfig)
add_subdirectory(DAQSystem)
add_subdirectory(DAQTrigger)
add_subdirectory(OnlHistogramer)
add_subdirectory(DAQ)

# install
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${CMAKE_INSTALL_PREFIX})


