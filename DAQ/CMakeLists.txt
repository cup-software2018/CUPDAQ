set ( MODULE_NAME DAQ )
message ( STATUS "Configuring ${MODULE_NAME} ..." )

# Use CONFIGURE_DEPENDS to automatically track changes in source files
file ( GLOB MODULE_SOURCES CONFIGURE_DEPENDS src/*.cc )

# --- Collect include dirs for dictionary
set ( DICT_I_FLAGS
    -I${PROJECT_SOURCE_DIR}/OnlConsts
    -I${PROJECT_SOURCE_DIR}/Notice
    -I${PROJECT_SOURCE_DIR}/DAQUtils
    -I${PROJECT_SOURCE_DIR}/DAQSystem
    -I${PROJECT_SOURCE_DIR}/DAQConfig
    -I${PROJECT_SOURCE_DIR}/DAQTrigger
    -I${PROJECT_SOURCE_DIR}/OnlObjs
    -I${PROJECT_SOURCE_DIR}/RawObjs
    -I${PROJECT_SOURCE_DIR}/OnlHistogramer
)

# Safely append LIBUSB include directories if they exist
if ( LIBUSB_INCLUDE_DIRS )
    list ( APPEND DICT_I_FLAGS -I${LIBUSB_INCLUDE_DIRS} )
endif ()

if ( CUPDAQ_ENABLE_HDF5 )
    list ( APPEND DICT_I_FLAGS -I${HDF5_INCLUDE_DIRS} )
    list ( APPEND DICT_I_FLAGS -I${PROJECT_SOURCE_DIR}/HDF5Utils )
endif ()

# ---- Specify exactly the headers for dictionary ----
set ( DICT_HEADERS
    ${MODULE_NAME}/CupDAQManager.hh
    ${MODULE_NAME}/CupGeneralTCB.hh
)

# Generate ROOT dictionary for only the two headers above
ROOT_GENERATE_DICTIONARY ( G__${MODULE_NAME}
    ${DICT_HEADERS}
    LINKDEF ${MODULE_NAME}/LinkDef.h
    OPTIONS ${DICT_I_FLAGS} -Wno-inconsistent-missing-override
)

# Build shared library
add_library ( ${MODULE_NAME} SHARED
    ${MODULE_SOURCES}
    G__${MODULE_NAME}.cxx
)

# Include paths
target_include_directories ( ${MODULE_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        ${ROOT_INCLUDE_DIRS}
)

if ( CUPDAQ_ENABLE_HDF5 )
    target_include_directories ( ${MODULE_NAME} PUBLIC ${HDF5_INCLUDE_DIRS} )
endif ()

# Link dependencies and inherit global flags
target_link_libraries ( ${MODULE_NAME}
    PUBLIC
        yaml-cpp
        cppzmq
        ${ROOT_LIBRARIES}
        OnlConsts
        Notice
        DAQUtils
        DAQSystem
        DAQConfig
        DAQTrigger
        OnlObjs
        RawObjs
        OnlHistogramer
)

if ( CUPDAQ_ENABLE_HDF5 )
    target_link_libraries ( ${MODULE_NAME} PUBLIC ${HDF5_LIBRARIES} HDF5Utils )
endif ()

target_compile_definitions ( ${MODULE_NAME}
    PUBLIC
        ${ROOT_DEFINITIONS}
)

# ---- Install ----
install ( TARGETS ${MODULE_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install ( DIRECTORY ${MODULE_NAME}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${MODULE_NAME}
    FILES_MATCHING PATTERN "*.hh"
)
install ( FILES
    ${CMAKE_CURRENT_BINARY_DIR}/lib${MODULE_NAME}_rdict.pcm
    ${CMAKE_CURRENT_BINARY_DIR}/lib${MODULE_NAME}.rootmap
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    OPTIONAL
)

file ( GLOB DAQ_SCRIPTS "scripts/*" )

install ( PROGRAMS ${DAQ_SCRIPTS}
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT Runtime )

add_subdirectory ( test )