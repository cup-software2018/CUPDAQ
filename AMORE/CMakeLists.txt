# ------------------------------------------------------------
#  AMORE — header-only library
# ------------------------------------------------------------
set(MODULE_NAME AMORE)
message(STATUS "Configuring ${MODULE_NAME} ...")

# Create interface library (header-only)
add_library(${MODULE_NAME} INTERFACE)
target_include_directories(${MODULE_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Install headers
install(DIRECTORY ${MODULE_NAME}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${MODULE_NAME}
        FILES_MATCHING PATTERN "*.hh"
)
