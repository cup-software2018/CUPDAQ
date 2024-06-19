set(PROJECT_NAME "OnlObjs")

project(${PROJECT_NAME})
message(STATUS "cmaking ${PROJECT_NAME} ...")

# ----------------------------------------------------------------------------
# Locate sources and headers for this project
# ----------------------------------------------------------------------------
file(GLOB ${PROJECT_NAME}_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB ${PROJECT_NAME}_HEADERS ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/*.hh)

file(MAKE_DIRECTORY ${PROJECT_INCLUDE_DIR}/${PROJECT_NAME})

foreach(HEADER_i ${${PROJECT_NAME}_HEADERS})
  configure_file(${HEADER_i} ${PROJECT_INCLUDE_DIR}/${PROJECT_NAME} COPYONLY)
endforeach(HEADER_i)

configure_file(${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/LinkDef.h
               ${PROJECT_INCLUDE_DIR}/${PROJECT_NAME} COPYONLY)

set(${PROJECT_NAME}_HEADERS
    ${PROJECT_NAME}/ADCHeader.hh ${PROJECT_NAME}/AbsADCRaw.hh
    ${PROJECT_NAME}/FADCRawEvent.hh ${PROJECT_NAME}/SADCRawEvent.hh
    ${PROJECT_NAME}/BuiltEvent.hh ${PROJECT_NAME}/FADCRawChannel.hh ${PROJECT_NAME}/TimeCalConsts.hh)
set(${PROJECT_NAME}_DICT_FILE ${PROJECT_NAME}/LinkDef.h)

root_generate_dictionary(G__${PROJECT_NAME} ${${PROJECT_NAME}_HEADERS} LINKDEF
                         ${${PROJECT_NAME}_DICT_FILE})

add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES}
                                   G__${PROJECT_NAME}.cxx)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.so
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm
              ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.rootmap
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
