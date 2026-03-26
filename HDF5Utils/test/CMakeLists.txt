set ( TEST_SOURCES
    write_amore.cc
    write_fadc.cc
    write_fadc_random_ch.cc
    write_sadc.cc
)

foreach ( src ${TEST_SOURCES} )
    get_filename_component ( test_name ${src} NAME_WE )

    add_executable ( ${test_name} ${src} )

    target_include_directories ( ${test_name}
        PRIVATE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/HDF5Utils>
    )

    target_link_libraries ( ${test_name}
        PRIVATE
            ${ROOT_LIBRARIES}
            ${HDF5_LIBRARIES}
            HDF5Utils
    )

    install ( TARGETS ${test_name}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endforeach ()
