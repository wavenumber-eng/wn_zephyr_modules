
if(CONFIG_RT_CONFIG)

    zephyr_linker_sources(SECTIONS "./rt_config_sections.ld")

    zephyr_library()

    zephyr_library_sources(rt_config.c)

    if(CONFIG_RT_CONFIG_USE_FILE)
        zephyr_library_sources(rt_file.c)
    endif()

endif()
