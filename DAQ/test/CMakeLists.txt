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

    # The DAQ target links everything PUBLICLY (including Notice, DAQUtils, OnlObjs, etc.),
    # so linking to DAQ automatically resolves transative dependencies and include paths.
    target_link_libraries ( ${test_name}
        PRIVATE
            ${ROOT_LIBRARIES}
            PkgConfig::LIBUSB
            DAQ
    )

    # Transitive dependencies for HDF5 are already handled by DAQ if it was enabled,
    # making additional conditionals here unnecessary.
    install ( TARGETS ${test_name}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endforeach ()