# ------------------------------------------------------------
# DAQUtils - Header-only utility library
# ------------------------------------------------------------
set ( MODULE_NAME DAQUtils )
message ( STATUS "Configuring ${MODULE_NAME} ..." )

# Create an interface target for the header-only library
add_library ( ${MODULE_NAME} INTERFACE )

# Define include directories for consumers
target_include_directories ( ${MODULE_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Install the directory containing the utility headers
install ( DIRECTORY ${MODULE_NAME}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${MODULE_NAME}
        FILES_MATCHING PATTERN "*.hh"
)