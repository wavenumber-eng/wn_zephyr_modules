
if(CONFIG_RT_CONFIG)

    zephyr_linker_sources(SECTIONS "./rt_config_sections.ld")

    zephyr_library()

    zephyr_library_sources(rt_config.c
                            rt_fs.c)

    if(CONFIG_RT_CONFIG_USE_FILE)
        zephyr_library_sources(rt_file.c)

    elseif(CONFIG_RT_CONFIG_USE_NVS)
        zephyr_library_sources(rt_nvs.c)

    else()
        message(FATAL_ERROR "No valid storage method selected. Please select one of the following: CONFIG_RT_CONFIG_USE_FILE or CONFIG_RT_CONFIG_USE_NVS")
    
    endif()

endif()
