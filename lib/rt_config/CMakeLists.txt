
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

    if(CONFIG_RT_CONFIG_VALIDATE_ITEM_COUNT)
        set(rt_config_kernel_elf_name zephyr.elf)
        if(KERNEL_ELF_NAME)
            set(rt_config_kernel_elf_name ${KERNEL_ELF_NAME})
        endif()

        set_property(GLOBAL APPEND PROPERTY extra_post_build_commands
            COMMAND
            ${PYTHON_EXECUTABLE}
            ${CMAKE_CURRENT_LIST_DIR}/check_rt_config_item_count.py
            --elf ${rt_config_kernel_elf_name}
            --nm ${CMAKE_NM}
            --max-items ${CONFIG_RT_MAX_ITEMS}
        )
    endif()

endif()
