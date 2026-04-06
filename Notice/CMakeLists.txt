set ( MODULE_NAME Notice )
message ( STATUS "Configuring ${MODULE_NAME} ..." )

# Use CONFIGURE_DEPENDS to track new source files automatically
file ( GLOB MODULE_HEADERS CONFIGURE_DEPENDS ${MODULE_NAME}/*.hh )
file ( GLOB MODULE_SOURCES CONFIGURE_DEPENDS src/*.cc )

# --- Collect include dirs for dictionary
set ( DICT_I_FLAGS
    -I${CMAKE_CURRENT_SOURCE_DIR} # Critical: Allows rootcling to find headers using Module/Header.hh
    -I${PROJECT_SOURCE_DIR}/OnlConsts
    -I${PROJECT_SOURCE_DIR}/DAQUtils
    -I${PROJECT_SOURCE_DIR}/DAQConfig
)

# Safely append LIBUSB include directories if they exist
if ( LIBUSB_INCLUDE_DIRS )
    list ( APPEND DICT_I_FLAGS -I${LIBUSB_INCLUDE_DIRS} )
endif ()

# Generate ROOT dictionary
ROOT_GENERATE_DICTIONARY ( G__${MODULE_NAME}
    ${MODULE_HEADERS}
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

# Modern target-based linking automatically handles build-order dependencies.
# Therefore, add_dependencies() is no longer needed.
target_link_libraries ( ${MODULE_NAME}
    PUBLIC
        ${ROOT_LIBRARIES}
        PkgConfig::LIBUSB
        DAQUtils
        DAQConfig
)

target_compile_definitions ( ${MODULE_NAME} PUBLIC ${ROOT_DEFINITIONS} )

# ---- Install ----
install ( TARGETS ${MODULE_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} )
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

#add_subdirectory ( test )