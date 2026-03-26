set ( TEST_SOURCES
    test_tlt.cc
)

foreach ( src ${TEST_SOURCES} )
    get_filename_component ( test_name ${src} NAME_WE )

    add_executable ( ${test_name} ${src} )

    target_include_directories ( ${test_name}
        PRIVATE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/DAQ>
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/DAQTrigger>
    )

    target_link_libraries ( ${test_name}
        PRIVATE
            ${ROOT_LIBRARIES}
            yaml-cpp
            DAQConfig
    )

    install ( TARGETS ${test_name}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endforeach ()
