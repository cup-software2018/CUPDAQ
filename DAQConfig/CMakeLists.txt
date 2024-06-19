set(PROJECT_NAME "DAQConfig")

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
    ${PROJECT_NAME}/AbsConf.hh
    ${PROJECT_NAME}/AbsConfList.hh
    ${PROJECT_NAME}/FADCTConf.hh
    ${PROJECT_NAME}/FADCSConf.hh
    ${PROJECT_NAME}/GADCSConf.hh
    ${PROJECT_NAME}/SADCTConf.hh
    ${PROJECT_NAME}/SADCSConf.hh
    ${PROJECT_NAME}/IADCTConf.hh
    ${PROJECT_NAME}/MADCSConf.hh
    ${PROJECT_NAME}/AmoreADCConf.hh
    ${PROJECT_NAME}/TCBConf.hh
    ${PROJECT_NAME}/STRGConf.hh
    ${PROJECT_NAME}/DAQConf.hh
    ${PROJECT_NAME}/RunConfig.hh
    ${PROJECT_NAME}/TriggerLookupTable.hh)
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
