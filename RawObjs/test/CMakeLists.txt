# ------------------------------------------------------------
# Tests for RawObjs module
# ------------------------------------------------------------
set ( TEST_SOURCES
    write_fadc_root.cc
)

foreach ( src ${TEST_SOURCES} )
    get_filename_component ( test_name ${src} NAME_WE )

    add_executable ( ${test_name} ${src} )

    target_link_libraries ( ${test_name}
        PRIVATE
            ${ROOT_LIBRARIES}
            RawObjs
    )

    install ( TARGETS ${test_name}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endforeach ()