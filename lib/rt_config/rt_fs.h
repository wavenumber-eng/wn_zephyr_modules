#ifndef RT_FS_H
#define RT_FS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/kernel.h>
#include <zephyr/fs/fs.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/logging/log.h>
//#include "ff.h"


int8_t rt_fs__init();


#ifdef __cplusplus
}
#endif

#endif // RT_FS_H
