#ifndef _rt_config_h
#define _rt_config_h

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>


#define CONFIG_MAX_VALUE_STRING_LENGTH      256
#define CONFIG_RT_MAX_ITEMS                 64
#define CONFIG_RT_ITEM_BASE                 0x9000

#define CONFIG_RT_KEY_VALUE_MAX_LENGTH      256
#define NVS_PARTITION storage_partition

typedef union
{
    uint8_t uint8_t_Value;
    uint16_t uint16_t_Value;
    uint32_t uint32_t_Value;
    int8_t int8_t_Value;
    int16_t int16_t_Value;
    int32_t int32_t_Value;
    float float_Value;
} ValueHolder;

typedef enum
{
    RT_CONFIG_LEVEL__USER,
    RT_CONFIG_LEVEL__ADMIN,
} rt_config_level_e;

struct rt_config_item
{
	char * ConfigItemName;
    char * DescriptionString;
    void * Value;
    uint32_t DataType;
    uint32_t MaxDataTypeLength;
    char *Minimum;
    char *Maximum;
    char *Default;
    rt_config_level_e access_level;   
};


#define Z_CONFIG_ITEM_INITIALIZER(name,description_string,value,data_type,data_type_max_length,minimum,maximum,default,var_access_level) \
	{ \
	.ConfigItemName = Z_STRINGIFY(name), \
	.DescriptionString = description_string,\
    .Value = value,\
    .DataType = data_type,\
    .MaxDataTypeLength = data_type_max_length,\
    .Minimum = minimum,\
    .Maximum = maximum,\
    .Default = default,\
    .access_level = var_access_level,\
    }


#define RT_CONFIG_ITEM(name,description_string,value,data_type,data_type_max_length,minimum,maximum,default,var_access_level) \
	STRUCT_SECTION_ITERABLE(rt_config_item, name) = \
		Z_CONFIG_ITEM_INITIALIZER(name,description_string,value,data_type,data_type_max_length,minimum,maximum,default,var_access_level)




#define RT_CONFIG_DATA_TYPE_UINT8         (1)
#define RT_CONFIG_DATA_TYPE_UINT16        (2)
#define RT_CONFIG_DATA_TYPE_UINT32        (3)
#define RT_CONFIG_DATA_TYPE_INT8          (4)
#define RT_CONFIG_DATA_TYPE_INT16         (5)
#define RT_CONFIG_DATA_TYPE_INT32         (6)
#define RT_CONFIG_DATA_TYPE_FLOAT32       (7)
#define RT_CONFIG_DATA_TYPE_TIME           (8)
#define RT_CONFIG_DATA_TYPE_STRING         (9)   
#define RT_CONFIG_DATA_TYPE_CUSTOM         (10)   

#define CONFIG_SAVE_OK  0
#define CONFIG_LOAD_OK  0

#define CONFIG_SAVE_ERROR                -1
#define CONFIGURATION_ITEM_NOT_FOUND     -1
#define CONFIGURATION_ITEM_OUT_OF_RANGE  -2
#define CONFIGURATION_TYPE_UNKNOWN		 -3

extern int rt_config_init();

void rt_config_export();
struct rt_config_item * rt_config_get_config_item(char * Name);
bool rt_config_get_value_string(struct rt_config_item *ci, char *ValueString, uint32_t Len);
int32_t rt_config_load_with_value(const struct rt_config_item *NextConfigurationItem,void *value);


uint8_t rt_config__is_integer_config(struct rt_config_item *ci);

#endif