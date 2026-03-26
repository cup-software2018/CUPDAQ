# Sources for the DAQ library
file(GLOB DAQ_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
)

# Headers for dictionary generation
file(GLOB DAQ_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/../DAQ/*.hh"
)

# Generate ROOT dictionary
# Assumes LinkDef.h is in the parent DAQ/ directory
include(${ROOT_USE_FILE})
ROOT_GENERATE_DICTIONARY(G__DAQ
    ${DAQ_HEADERS}
    MODULE DAQ
    LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/../DAQ/LinkDef.h
)

# Define the DAQ shared library
add_library(DAQ SHARED ${DAQ_SOURCES} ${G__DAQ})

# Set library version properties
set_target_properties(DAQ PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Define include directories
target_include_directories(DAQ
    PUBLIC
        # For DAQ/*.hh headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        # For headers from other modules like DAQConfig/
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        # For installation
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${ROOT_INCLUDE_DIRS}
)

# Link dependencies
target_link_libraries(DAQ
    PUBLIC
        # Internal libraries whose headers are included by DAQ's public headers
        OnlConsts DAQUtils DAQConfig Notice DAQSystem OnlObjs DAQTrigger OnlHistogramer RawObjs
    PRIVATE
        # External libraries used only in DAQ's implementation (.cc files)
        ROOT::Core
        ROOT::RIO
        ROOT::Net
        ROOT::Tree
        LibUSB::LibUSB
        yaml-cpp::yaml-cpp
        cppzmq::cppzmq
        nlohmann_json::nlohmann_json
)

# Conditionally link HDF5
if(CUPDAQ_ENABLE_HDF5)
    target_link_libraries(DAQ
        PUBLIC HDF5Utils
        PRIVATE HDF5::C HDF5::HL
    )
endif()

# Installation rules
install(TARGETS DAQ
    EXPORT DAQTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
