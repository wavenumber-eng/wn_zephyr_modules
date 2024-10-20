# SPDX-License-Identifier: Apache-2.0



#zephyr_include_directories(.)
#zephyr_library_include_directories(${ZEPHYR_BASE}/include)

zephyr_linker_sources(SECTIONS "./rt_config_sections.ld")

zephyr_library()

zephyr_library_sources(rt_config.c)
