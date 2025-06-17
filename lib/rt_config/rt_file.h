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


typedef enum
{
    RT_RETURN_OK = 0,
    RT_RETURN_NEW_FILE_CREATED = 1,
    RT_RETURN_FILE_ERROR = 2,
    RT_RETURN_NUMBER_OF_RETURN_CODES
} rt_return_code_e;


rt_return_code_e rt_file__init();
int32_t rt_file__export();
int32_t rt_file__import();
void rt_file__wipe();

#ifdef __cplusplus
}
#endif


#endif /* RT_FILE_H */