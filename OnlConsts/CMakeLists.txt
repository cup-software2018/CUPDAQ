# ------------------------------------------------------------
# OnlConsts - Header-only library
# ------------------------------------------------------------
set ( MODULE_NAME OnlConsts )
message ( STATUS "Configuring ${MODULE_NAME} ..." )

# Create an interface target for a header-only library
add_library ( ${MODULE_NAME} INTERFACE )

# Define include directories for the interface
target_include_directories ( ${MODULE_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Install the directory containing the headers
install ( DIRECTORY ${MODULE_NAME}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${MODULE_NAME}
        FILES_MATCHING PATTERN "*.hh"
)