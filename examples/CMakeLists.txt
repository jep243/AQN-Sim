build_lib_example(
    NAME quantum-network-example
    SOURCE_FILES quantum-network-example.cc
    LIBRARIES_TO_LINK ${libquantum-network}
)

build_lib_example(
	NAME sim
	SOURCE_FILES sim.cc
	LIBRARIES_TO_LINK ${libquantum-network}
)
