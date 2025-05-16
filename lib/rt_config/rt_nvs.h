#ifndef RT_NVS_H
#define RT_NVS_H

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/sys/crc.h>
#include <stdlib.h>
#include "stdio.h"
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>

int32_t rt_nvs__init();
void rt_nvs__export();
void rt_nvs__wipe();
void rt_nvs__load_values();

#endif