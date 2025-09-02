check_include_file_cxx(stdint.h HAVE_STDINT_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif()

set(examples_as_tests_sources)
if(${ENABLE_EXAMPLES})
    set(examples_as_tests_sources
        #test/quantum-network-examples-test-suite.cc
        )
endif()

build_lib(
    LIBNAME quantum-network
    SOURCE_FILES model/time-node.cc
				 model/quantum-tag.cc
				 model/optical-tag.cc
				 model/optical-data-header.cc
				 model/optical-control-header.cc
				 model/optical-header.cc
				 model/optical-channel.cc
                 model/optical-device.cc
				 model/quantum-application.cc
				 helper/optical-helper.cc
				 helper/quantum-helper.cc
    HEADER_FILES model/quantum-tag.h
				 model/optical-tag.h
				 model/time-node.h
				 model/optical-data-header.h
				 model/optical-control-header.h
				 model/optical-header.h
				 model/optical-channel.h
                 model/optical-device.h
				 model/quantum-application.h
				 helper/optical-helper.h
				 helper/quantum-helper.h
    LIBRARIES_TO_LINK ${libcore}
					  ${libnetwork}
					  ${libinternet}
    TEST_SOURCES test/quantum-network-test-suite.cc
                 ${examples_as_tests_sources}
)
