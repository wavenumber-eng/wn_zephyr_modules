#ifndef RT_FILE_H
#define RT_FILE_H

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/fs.h>
#include <zephyr/shell/shell.h>


#ifdef __cplusplus
extern "C" {
#endif


void rt_file__init();
int32_t rt_file__export();
int32_t rt_file__import();

#ifdef __cplusplus
}
#endif


#endif /* RT_FILE_H */