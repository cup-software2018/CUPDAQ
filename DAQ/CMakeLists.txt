set ( MODULE_NAME DAQ )
message ( STATUS "Configuring ${MODULE_NAME} ..." )

# Sources (normal .cc)
file ( GLOB MODULE_SOURCES src/*.cc )

# --- Collect include dirs for dictionary (from OnlConsts INTERFACE) ---
set ( DICT_I_FLAGS
    -I${LIBUSB_INCLUDE_DIRS}
    -I${CMAKE_SOURCE_DIR}/OnlConsts
    -I${CMAKE_SOURCE_DIR}/Notice
    -I${CMAKE_SOURCE_DIR}/NewNotice
    -I${CMAKE_SOURCE_DIR}/DAQUtils
    -I${CMAKE_SOURCE_DIR}/DAQSystem
    -I${CMAKE_SOURCE_DIR}/DAQConfig
    -I${CMAKE_SOURCE_DIR}/DAQTrigger
    -I${CMAKE_SOURCE_DIR}/OnlObjs
    -I${CMAKE_SOURCE_DIR}/RawObjs
    -I${CMAKE_SOURCE_DIR}/OnlHistogramer
)

if ( CUPDAQ_ENABLE_HDF5 )
    list ( APPEND DICT_I_FLAGS -I${HDF5_INCLUDE_DIRS} )
    list ( APPEND DICT_I_FLAGS -I${CMAKE_SOURCE_DIR}/HDF5Utils )
endif ()

# ---- Specify exactly the headers for dictionary ----
# (replace with your actual header filenames)
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

# Build shared library (add dictionary source)
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

# Link ROOT and apply its compile defs
target_link_libraries ( ${MODULE_NAME}
    PUBLIC
    ${ROOT_LIBRARIES}
    OnlConsts
    NewNotice
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

add_subdirectory ( test )