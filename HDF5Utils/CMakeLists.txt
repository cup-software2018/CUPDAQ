# ------------------------------------------------------------
# HDF5Utils — shared library depending on HDF5
# ------------------------------------------------------------
set ( MODULE_NAME HDF5Utils )
message ( STATUS "Configuring ${MODULE_NAME} ..." )

file ( GLOB MODULE_HEADERS ${MODULE_NAME}/*.hh )
file ( GLOB MODULE_SOURCES src/*.cc )

# --- Collect include dirs for dictionary (from OnlConsts INTERFACE) ---
set ( DICT_I_FLAGS
    -I${HDF5_INCLUDE_DIR}
)

# Generate ROOT dictionary (pass -I for OnlConsts)
ROOT_GENERATE_DICTIONARY ( G__${MODULE_NAME}
    ${MODULE_HEADERS}
    LINKDEF ${MODULE_NAME}/LinkDef.h
    OPTIONS ${DICT_I_FLAGS} -Wno-inconsistent-missing-override
)

# Build shared library
add_library ( ${MODULE_NAME} SHARED
    ${MODULE_SOURCES}
    G__${MODULE_NAME}.cxx
)

# Include dirs: module itself and install interface
target_include_directories ( ${MODULE_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        ${ROOT_INCLUDE_DIRS}
        ${HDF5_INCLUDE_DIRS}
)

target_link_libraries ( ${MODULE_NAME}
    PUBLIC
    ${ROOT_LIBRARIES}
    ${HDF5_LIBRARIES}
)

target_compile_definitions ( ${MODULE_NAME} PUBLIC ${ROOT_DEFINITIONS} )

# If HDF5 provides compile definitions, propagate them
if ( DEFINED HDF5_DEFINITIONS )
    target_compile_definitions ( ${MODULE_NAME} PUBLIC ${HDF5_DEFINITIONS} )
endif ()

# If this module uses OnlConsts or others, link them here (example commented)
# target_link_libraries(${MODULE_NAME} PUBLIC OnlConsts)

# Installation
install ( TARGETS ${MODULE_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} )
install ( DIRECTORY ${MODULE_NAME}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${MODULE_NAME}
    FILES_MATCHING PATTERN "*.hh"
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/lib${MODULE_NAME}_rdict.pcm
    ${CMAKE_CURRENT_BINARY_DIR}/lib${MODULE_NAME}.rootmap
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    OPTIONAL
)

add_subdirectory ( test )