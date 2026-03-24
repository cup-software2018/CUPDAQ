set ( MODULE_NAME OnlHistogramer )
message ( STATUS "Configuring ${MODULE_NAME} ..." )

file ( GLOB MODULE_HEADERS ${MODULE_NAME}/*.hh )
file ( GLOB MODULE_SOURCES src/*.cc )

add_library ( ${MODULE_NAME} SHARED
    ${MODULE_SOURCES}
)

# Ensure build order
add_dependencies ( ${MODULE_NAME} OnlConsts DAQUtils DAQConfig OnlObjs)

target_include_directories ( ${MODULE_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        ${ROOT_INCLUDE_DIRS}
)

target_link_libraries ( ${MODULE_NAME}
    PUBLIC
        ${ROOT_LIBRARIES}
        OnlConsts
        DAQUtils
        DAQConfig
        OnlObjs
)

target_compile_definitions ( ${MODULE_NAME}
    PUBLIC ${ROOT_DEFINITIONS}
)

# Installation
install ( TARGETS ${MODULE_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install ( DIRECTORY ${MODULE_NAME}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${MODULE_NAME}
    FILES_MATCHING PATTERN "*.hh"
)
