set(PROJECT_NAME "DAQ")

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

configure_file(${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/LinkDef.h ${PROJECT_INCLUDE_DIR}/${PROJECT_NAME} COPYONLY)

file(GLOB ${PROJECT_NAME}_HEADERS ${PROJECT_INCLUDE_DIR}/${PROJECT_NAME}/*.hh)
set(${PROJECT_NAME}_DICT_FILE ${PROJECT_INCLUDE_DIR}/${PROJECT_NAME}/LinkDef.h)

root_generate_dictionary(G__${PROJECT_NAME}
  ${${PROJECT_NAME}_HEADERS}
  LINKDEF ${${PROJECT_NAME}_DICT_FILE}
)

add_library(${PROJECT_NAME} SHARED
  ${${PROJECT_NAME}_SOURCES}
  G__${PROJECT_NAME}.cxx
)

install(PROGRAMS
  ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.so
  DESTINATION
  ${CMAKE_INSTALL_LIBDIR}
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}_rdict.pcm
  ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.rootmap
  DESTINATION
  ${CMAKE_INSTALL_LIBDIR}
)

# ----------------------------------------------------------------------------
# build executables
# ----------------------------------------------------------------------------
if(USE_HDF5)
  set(LINK_PACKAGES DAQConfig DAQSystem DAQTrigger DAQUtils OnlHistogramer OnlObjs RawObjs Notice HDF5Utils DAQ)
  set(LINK_EXTERNALS ${ROOT_LIBRARIES} ${libusb-1.0_LIBRARIES} ${HDF5_LIBRARY})
else()
  set(LINK_PACKAGES DAQConfig DAQSystem DAQTrigger DAQUtils OnlHistogramer OnlObjs RawObjs Notice DAQ)
  set(LINK_EXTERNALS ${ROOT_LIBRARIES} ${libusb-1.0_LIBRARIES})
endif()

file(GLOB ${PROJECT_NAME}_EXECUTABLES ${PROJECT_SOURCE_DIR}/test/*.cc)

foreach(testsourcefile ${${PROJECT_NAME}_EXECUTABLES})
  get_filename_component(testname ${testsourcefile} NAME_WE)
  add_executable(${testname} ${testsourcefile})
  target_link_libraries(${testname} ${LINK_PACKAGES} ${LINK_EXTERNALS})
  install(TARGETS ${testname} DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach(testsourcefile ${${PROJECT_NAME}_EXECUTABLES})


install(DIRECTORY ${PROJECT_SOURCE_DIR}/config DESTINATION ${CMAKE_INSTALL_PREFIX})
install(PROGRAMS
  ${PROJECT_SOURCE_DIR}/scripts/executedaq.sh
  ${PROJECT_SOURCE_DIR}/scripts/executenulldaq.sh
  DESTINATION
  ${CMAKE_INSTALL_BINDIR}
)
