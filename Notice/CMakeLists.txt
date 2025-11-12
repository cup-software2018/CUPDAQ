set ( MODULE_NAME Notice )
message ( STATUS "Configuring ${MODULE_NAME} ..." )

# Sources (normal .cc)
file ( GLOB MODULE_SOURCES src/*.cc )

# ---- Specify exactly the headers for dictionary ----
set ( DICT_HEADERS
    ${MODULE_NAME}/usb3comroot.hh
    ${MODULE_NAME}/usb3tcbroot.hh
)

# Generate ROOT dictionary for only the two headers above
ROOT_GENERATE_DICTIONARY ( G__${MODULE_NAME}
    ${DICT_HEADERS}
    LINKDEF ${MODULE_NAME}/LinkDef.h
)

# Build shared library (add dictionary source)
add_library ( ${MODULE_NAME} SHARED
    ${MODULE_SOURCES}
    G__${MODULE_NAME}.cxx
)

# Ensure build order
add_dependencies ( ${MODULE_NAME} DAQUtils )

# Include paths
target_include_directories ( ${MODULE_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    ${ROOT_INCLUDE_DIRS}
)

# Link ROOT and apply its compile defs
target_link_libraries ( ${MODULE_NAME}
    PUBLIC
    ${ROOT_LIBRARIES}
    PkgConfig::LIBUSB
    DAQUtils
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