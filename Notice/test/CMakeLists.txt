# ------------------------------------------------------------
# Tests for Notice module
# ------------------------------------------------------------
set ( TEST_SOURCES
    test_usb3com.cc
    test_minitcb.cc
)

foreach ( src ${TEST_SOURCES} )
    get_filename_component ( test_name ${src} NAME_WE )

    add_executable ( ${test_name} ${src} )

    # Linking the Notice target automatically provides its PUBLIC include directories.
    # The legacy 'NewNotice' path and redundant 'DAQUtils' inclusions are removed.
    target_link_libraries ( ${test_name}
        PRIVATE
            Notice
    )

    install ( TARGETS ${test_name}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endforeach ()