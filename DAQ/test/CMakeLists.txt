set ( TEST_SOURCES
    tcb.cc
    daq.cc
    merger.cc
    tcbdaq.cc
    stddaq.cc
    nulltcb.cc
    nulldaq.cc
    nullmerger.cc
)

foreach ( src ${TEST_SOURCES} )
    get_filename_component ( test_name ${src} NAME_WE )

    add_executable ( ${test_name} ${src} )

    target_include_directories ( ${test_name}
        PRIVATE
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/DAQ>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/DAQTrigger>
    )

    target_link_libraries ( ${test_name}
        PRIVATE
            ${ROOT_LIBRARIES}
            PkgConfig::LIBUSB
            DAQ
            DAQUtils
            DAQConfig
            DAQSystem
            DAQTrigger
            OnlHistogramer
            OnlObjs
            RawObjs
            Notice
    )

    if ( CUPDAQ_ENABLE_HDF5 )
        target_link_libraries ( ${test_name}
            PRIVATE
            ${HDF5_LIBRARIES}
            HDF5Utils
        )
    endif ()

    install ( TARGETS ${test_name}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endforeach ()
