set ( MODULE_NAME DAQUtils )
message ( STATUS "Configuring ${MODULE_NAME} ..." )

file ( GLOB MODULE_HEADERS ${MODULE_NAME}/*.hh )
file ( GLOB MODULE_SOURCES src/*.cc )

# ---- Specify exactly the headers for dictionary ----
set ( DICT_HEADERS
    ${MODULE_NAME}/ELog.hh
    ${MODULE_NAME}/ELogger.hh
)

#--- Collect include dirs for dictionary
set(DICT_I_FLAGS
    -I${CMAKE_SOURCE_DIR}/OnlConsts 
)

# Generate ROOT dictionary (pass -I for OnlConsts)
ROOT_GENERATE_DICTIONARY ( G__${MODULE_NAME}
    ${DICT_HEADERS}
    LINKDEF ${MODULE_NAME}/LinkDef.h
    OPTIONS ${DICT_I_FLAGS}
)

add_library ( ${MODULE_NAME} SHARED
    ${MODULE_SOURCES}
    G__${MODULE_NAME}.cxx
)

# Ensure build order
add_dependencies ( ${MODULE_NAME} OnlConsts )

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
