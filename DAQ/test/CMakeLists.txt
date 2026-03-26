set ( TEST_SOURCES
# List of test executables to build from .cc files in this directory
set (TEST_EXECUTABLES
    tcb
    tcb.cc
    daq.cc
    merger.cc
    tcbdaq.cc
    stddaq.cc
#    nulltcb.cc
#    nulldaq.cc
#    nullmerger.cc
)

foreach ( src ${TEST_SOURCES} )
    get_filename_component ( test_name ${src} NAME_WE )
# Common source for option parsing, located in the src/ directory
set(DAQ_OPT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../src/daqopt.cc)

    add_executable ( ${test_name} ${src} )
# Loop over the sources and create an executable for each
foreach ( exec_name ${TEST_EXECUTABLES} )
    add_executable ( ${exec_name} ${exec_name}.cc ${DAQ_OPT_SRC} )

    target_include_directories ( ${test_name}
        PRIVATE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/DAQ>
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/DAQTrigger>
    )
    # Link against the main DAQ library.
    # All other include paths and link libraries are inherited transitively.
    target_link_libraries ( ${exec_name} PRIVATE DAQ )

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
    # Install the executable
    install ( TARGETS ${exec_name}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endforeach ()
