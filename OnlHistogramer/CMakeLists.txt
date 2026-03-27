set ( MODULE_NAME OnlHistogramer )
message ( STATUS "Configuring ${MODULE_NAME} ..." )

# Use CONFIGURE_DEPENDS to automatically track changes in source files
file ( GLOB MODULE_HEADERS CONFIGURE_DEPENDS ${MODULE_NAME}/*.hh )
file ( GLOB MODULE_SOURCES CONFIGURE_DEPENDS src/*.cc )

# Build shared library
add_library ( ${MODULE_NAME} SHARED
    ${MODULE_SOURCES}
)

# Modern target-based linking automatically handles build-order dependencies.
# Therefore, add_dependencies() is removed.

# Include paths
target_include_directories ( ${MODULE_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        ${ROOT_INCLUDE_DIRS}
)

# Link dependencies and inherit global flags
target_link_libraries ( ${MODULE_NAME}
    PUBLIC
        ${ROOT_LIBRARIES}
        OnlConsts
        DAQUtils
        DAQConfig
        OnlObjs
)

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