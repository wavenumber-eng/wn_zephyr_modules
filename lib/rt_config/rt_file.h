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


typedef struct
{
    struct rt_config_item *ci;
    uint8_t in_file;
} config_status_t;


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
int8_t rt_file__repair();

uint8_t rt_file__get_item_in_file(struct rt_config_item *ci);
void rt_file__set_item_in_file(struct rt_config_item *ci, uint8_t in_file);
uint8_t is_numeric_string(char *str, uint32_t length);


#ifdef __cplusplus
}
#endif


#endif /* RT_FILE_H */