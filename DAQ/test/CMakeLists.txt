# ------------------------------------------------------------
# Tests for DAQ module
# ------------------------------------------------------------
set ( TEST_SOURCES
    tcb.cc
    daq.cc
    merger.cc
    tcbdaq.cc
    stddaq.cc
    # nulltcb.cc
    # nulldaq.cc
    # nullmerger.cc
)

foreach ( src ${TEST_SOURCES} )
    get_filename_component ( test_name ${src} NAME_WE )

    add_executable ( ${test_name} ${src} )

    target_link_libraries ( ${test_name}
        PRIVATE
            ${ROOT_LIBRARIES}
            PkgConfig::LIBUSB
            DAQ
    )

    install ( TARGETS ${test_name}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endforeach ()