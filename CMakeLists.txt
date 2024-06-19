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

#
# Locate the HDF5 package
#
set(LIB_TYPE SHARED)
string(TOLOWER ${LIB_TYPE} SEARCH_TYPE)
find_package(HDF5 QUIET NAMES hdf5 COMPONENTS C ${SEARCH_TYPE})

if(HDF5_FOUND)
  include_directories(${HDF5_INCLUDE_DIR})
  set(HDF5_LIBRARY ${HDF5_C_${LIB_TYPE}_LIBRARY})
  option(USE_HDF5 "Use fake HDF5 implementation" ON)  
  add_definitions(-DENABLE_HDF5)
endif()

# -----------------------------------------------------------------------------

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include)
set(PROJECT_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)

set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX}/bin)
set(CMAKE_INSTALL_INCDIR)

include_directories(${PROJECT_INCLUDE_DIR})

add_subdirectory(OnlConsts EXCLUDE_FROM_ALL)
add_subdirectory(Notice)
add_subdirectory(OnlObjs)
add_subdirectory(RawObjs)
if(USE_HDF5)
  add_subdirectory(HDF5Utils)
endif()
add_subdirectory(DAQUtils)
add_subdirectory(DAQConfig)
add_subdirectory(DAQSystem)
add_subdirectory(DAQTrigger)
add_subdirectory(OnlHistogramer)
add_subdirectory(DAQ)

# install
install(DIRECTORY ${CMAKE_BINARY_DIR}/include DESTINATION ${CMAKE_INSTALL_PREFIX})
