cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

project(testproj)

#
# Locate the ROOT package and define a number of useful targets and variables.
#
find_package(ROOT REQUIRED)
include(${ROOT_USE_FILE})

# -----------------------------------------------------------------------------

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
include_directories("/usr/include/libusb-1.0")
set(USB_LIBRARY "usb-1.0")

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include)
set(PROJECT_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)

set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX}/bin)

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


