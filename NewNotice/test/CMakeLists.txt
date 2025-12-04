set ( TEST_SOURCES
    test_usb3com.cc
    test_minitcb.cc
)

foreach ( src ${TEST_SOURCES} )
    get_filename_component ( test_name ${src} NAME_WE )

    add_executable ( ${test_name} ${src} )

    target_include_directories ( ${test_name}
        PRIVATE
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/DAQUtils>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/NewNotice>
    )

    target_link_libraries ( ${test_name}
        PRIVATE
            ${ROOT_LIBRARIES}
            NewNotice
    )

    install ( TARGETS ${test_name}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endforeach ()
