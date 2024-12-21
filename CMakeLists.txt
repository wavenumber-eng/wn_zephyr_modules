# This CMake file is picked by the Zephyr build system because it is defined
# as the module CMake entry point (see zephyr/module.yml).


# This is needed so that custom driver classes using system calls are taken into
# account


zephyr_include_directories(lib/rt_config)
zephyr_include_directories(lib/data_buf)

add_subdirectory(lib)