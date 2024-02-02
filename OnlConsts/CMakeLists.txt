project(OnlConsts)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#----------------------------------------------------------------------------
file(GLOB OnlConsts_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB OnlConsts_HEADERS ${PROJECT_SOURCE_DIR}/OnlConsts/*.hh)


file(MAKE_DIRECTORY ${PROJECT_INCLUDE_DIR}/OnlConsts)
foreach(HEADER_i ${OnlConsts_HEADERS})
    configure_file(${HEADER_i} ${PROJECT_INCLUDE_DIR}/OnlConsts COPYONLY)
endforeach(HEADER_i)
