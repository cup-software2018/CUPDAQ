# ------------------------------------------------------------
# Tests for HDF5Utils module
# ------------------------------------------------------------
set ( TEST_SOURCES
    test_fadc_event.cc
    test_sadc_event.cc
    test_fadc_hit.cc
    test_sadc_hit.cc
    test_subrun.cc
    edge_test.cc
)

foreach ( src ${TEST_SOURCES} )
    get_filename_component ( test_name ${src} NAME_WE )

    add_executable ( ${test_name} ${src} )

    # Linking HDF5Utils automatically provides its PUBLIC include directories
    # and transative dependencies (like ROOT and HDF5).
    # Therefore, explicit target_include_directories and repetitive linking are removed.
    target_link_libraries ( ${test_name}
        PRIVATE
            HDF5Utils
    )

    install ( TARGETS ${test_name}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endforeach ()