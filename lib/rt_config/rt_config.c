
#include <zephyr/kernel.h>
#include <zephyr/sys/iterable_sections.h>
#include "stdio.h"
#include <zephyr/sys/printk.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/sys/crc.h>
#include <zephyr/shell/shell.h>
#include <stdlib.h>
#include <rt_config.h>

#define NVS_PARTITION storage_partition
#define NVS_PARTITION_DEVICE FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET FIXED_PARTITION_OFFSET(NVS_PARTITION)

LOG_MODULE_REGISTER(rt_config, LOG_LEVEL_INF);

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

bool rt_config_get_value_string(struct rt_config_item *ci, char *ValueString, uint32_t Len)
{

    bool Result = true;

    ValueHolder V;

    switch (ci->DataType)
    {
    default:

        LOG_ERR("Could not get value string for type %i for %s",
                ci->DataType,
                ci->ConfigItemName);

        Result = false;

        break;

    case RT_CONFIG_DATA_TYPE_STRING:

        snprintf(&ValueString[0], Len,
                 "%s", (char *)ci->Value);

        break;

    case RT_CONFIG_DATA_TYPE_UINT8:
        V.uint32_t_Value = *((uint8_t *)ci->Value);
        snprintf(&ValueString[0], CONFIG_MAX_VALUE_STRING_LENGTH, "%u", (unsigned int)V.uint32_t_Value);
        break;

    case RT_CONFIG_DATA_TYPE_UINT16:
        V.uint32_t_Value = *((uint16_t *)ci->Value);
        snprintf(&ValueString[0], CONFIG_MAX_VALUE_STRING_LENGTH, "%u", (unsigned int)V.uint32_t_Value);
        break;

    case RT_CONFIG_DATA_TYPE_UINT32:
        V.uint32_t_Value = *((uint32_t *)ci->Value);
        snprintf(&ValueString[0], CONFIG_MAX_VALUE_STRING_LENGTH, "%u", (unsigned int)V.uint32_t_Value);
        break;

    case RT_CONFIG_DATA_TYPE_INT8:
        V.int32_t_Value = *((int8_t *)ci->Value);
        snprintf(&ValueString[0], CONFIG_MAX_VALUE_STRING_LENGTH, "%d", (int)V.int32_t_Value);
        break;

    case RT_CONFIG_DATA_TYPE_INT16:
        V.int32_t_Value = *((int16_t *)ci->Value);
        snprintf(&ValueString[0], CONFIG_MAX_VALUE_STRING_LENGTH, "%d", (int)V.int32_t_Value);
        break;

    case RT_CONFIG_DATA_TYPE_INT32:
        V.int32_t_Value = *((int32_t *)ci->Value);
        snprintf(&ValueString[0], CONFIG_MAX_VALUE_STRING_LENGTH, "%d", (int)V.int32_t_Value);
        break;

    case RT_CONFIG_DATA_TYPE_FLOAT32:
        V.float_Value = *((float *)ci->Value);
        snprintf(&ValueString[0], CONFIG_MAX_VALUE_STRING_LENGTH, "%f", (double)V.float_Value);
        break;
    }

    return Result;
}

void rt_config_show_current_values()
{
    char ValueString[CONFIG_MAX_VALUE_STRING_LENGTH];

    STRUCT_SECTION_FOREACH(rt_config_item, ci)
    {
        if (rt_config_get_value_string(ci, ValueString, CONFIG_MAX_VALUE_STRING_LENGTH))
        {
            LOG_INF("%s set to '%s'", ci->ConfigItemName, ValueString);
        }
    }
}

int32_t rt_config_load_with_value(const struct rt_config_item *NextConfigurationItem, void *value)
{
    ValueHolder MyValueHolder;
    ValueHolder Max;
    ValueHolder Min;

    // uint32_t Temp;
    int32_t Result = CONFIG_LOAD_OK;

    switch (NextConfigurationItem->DataType)
    {
    default:

        LOG_ERR("Unknown Configuration Data type of %i for %s",
                NextConfigurationItem->DataType,
                NextConfigurationItem->ConfigItemName);

        Result = CONFIGURATION_TYPE_UNKNOWN;

        break;

    case RT_CONFIG_DATA_TYPE_STRING:

        ((char *)(NextConfigurationItem->Value))[0] = 0;

        strncpy((char *)NextConfigurationItem->Value,
                (const char *)value,
                NextConfigurationItem->MaxDataTypeLength);

        break;

    case RT_CONFIG_DATA_TYPE_UINT8:
    case RT_CONFIG_DATA_TYPE_UINT16:
    case RT_CONFIG_DATA_TYPE_UINT32:

        // We will first load in the value as if it was a 32bit unsigned data type and then wack it down to size if needed
        MyValueHolder.uint32_t_Value = atoi((const char *)value);
        Max.uint32_t_Value = atoi(NextConfigurationItem->Maximum);
        Min.uint32_t_Value = atoi(NextConfigurationItem->Minimum);

        // see if value exceeds Maximum or minimum
        if (MyValueHolder.uint32_t_Value >= Max.uint32_t_Value)
        {
            MyValueHolder.uint32_t_Value = Max.uint32_t_Value;
        }

        if (MyValueHolder.uint32_t_Value <= Min.uint32_t_Value)
        {
            MyValueHolder.uint32_t_Value = Min.uint32_t_Value;
        }

        // Copy in the value.  It is possible that the value will be outside of the bounds so we will cast to use C default rules
        // We will also look at the current data type to get the casting correct

        if (NextConfigurationItem->DataType == RT_CONFIG_DATA_TYPE_UINT8)
        {
            *((uint8_t *)NextConfigurationItem->Value) = (uint8_t)MyValueHolder.uint32_t_Value;
        }
        else if (NextConfigurationItem->DataType == RT_CONFIG_DATA_TYPE_UINT16)
        {
            *((uint16_t *)NextConfigurationItem->Value) = (uint16_t)MyValueHolder.uint32_t_Value;
        }
        else if (NextConfigurationItem->DataType == RT_CONFIG_DATA_TYPE_UINT32)
        {
            *((uint32_t *)NextConfigurationItem->Value) = (uint32_t)MyValueHolder.uint32_t_Value;
        }

        break;

    case RT_CONFIG_DATA_TYPE_INT8:
    case RT_CONFIG_DATA_TYPE_INT16:
    case RT_CONFIG_DATA_TYPE_INT32:
        // We will first load in the value as if it was a 32bit signed data type and then wack it down to size if needed
        MyValueHolder.int32_t_Value = atoi((const char *)value);
        Max.int32_t_Value = atoi(NextConfigurationItem->Maximum);
        Min.int32_t_Value = atoi(NextConfigurationItem->Minimum);

        // see if value exceeds Maximum or minimum
        if (MyValueHolder.int32_t_Value > Max.int32_t_Value)
        {
            MyValueHolder.int32_t_Value = Max.int32_t_Value;
        }

        if (MyValueHolder.int32_t_Value < Min.int32_t_Value)
        {
            MyValueHolder.int32_t_Value = Min.int32_t_Value;
        }

        // Copy in the value.  It is possible that the value will be outside of the bounds so we will cast to use C default rules
        // We will also look at the current data type to get the casting correct

        if (NextConfigurationItem->DataType == RT_CONFIG_DATA_TYPE_INT8)
        {
            *((int8_t *)NextConfigurationItem->Value) = (int8_t)MyValueHolder.int32_t_Value;
        }
        else if (NextConfigurationItem->DataType == RT_CONFIG_DATA_TYPE_INT16)
        {
            *((int16_t *)NextConfigurationItem->Value) = (int16_t)MyValueHolder.int32_t_Value;
        }
        else if (NextConfigurationItem->DataType == RT_CONFIG_DATA_TYPE_INT32)
        {
            *((int32_t *)NextConfigurationItem->Value) = (int32_t)MyValueHolder.int32_t_Value;
        }

        break;

    case RT_CONFIG_DATA_TYPE_FLOAT32:
        // We will first load in the value as if it was a 32bit signed data type and then wack it down to size if needed
        MyValueHolder.float_Value = atof((const char *)value);
        Max.float_Value = atof(NextConfigurationItem->Maximum);
        Min.float_Value = atof(NextConfigurationItem->Minimum);

        // see if value exceeds Maximum or minimum
        if (MyValueHolder.float_Value > Max.float_Value)
        {
            MyValueHolder.float_Value = Max.float_Value;
        }

        if (MyValueHolder.float_Value < Min.float_Value)
        {
            MyValueHolder.float_Value = Min.float_Value;
        }

        // Copy in the value.  It is possible that the value will be outside of the bounds so we will cast to use C default rules
        // We will also look at the current data type to get the casting correct
        *((float *)NextConfigurationItem->Value) = (float)MyValueHolder.float_Value;

        break;
    }

    return Result;
}

static struct nvs_fs fs;

static char KeyValue[CONFIG_RT_KEY_VALUE_MAX_LENGTH + 2]; // 1st 2 bytes will be CRC

char *rt_config_slot_in_use(uint32_t Slot)
{

    int32_t ReadLen = nvs_read(&fs, CONFIG_RT_ITEM_BASE + Slot, KeyValue, sizeof(KeyValue));

    if ((ReadLen > 4) &&
        (ReadLen <= sizeof(KeyValue)))
    {
        uint16_t CRC16 = crc16_ccitt(0x1234, &KeyValue[2], ReadLen - 2);

        uint16_t *CRC16_Check = (uint16_t *)(&KeyValue[0]);

        if ((*CRC16_Check == CRC16) && (KeyValue[ReadLen - 1] == 0))
        {

            for (int i = 2; i < ReadLen; i++)
            {
                if (KeyValue[i] == '=')
                {
                    return &KeyValue[2];
                }
            }
        }
    }

    return NULL;
}

char KVP_temp[CONFIG_RT_KEY_VALUE_MAX_LENGTH];

char *rt_config_get_key_value(char *KeyIn, uint16_t *Slot)
{
    char *Key = 0;
    char *Value = 0;
    char *KVP = 0;

    for (int i = 0; i < CONFIG_RT_MAX_ITEMS; i++)
    {

        if ((KVP = rt_config_slot_in_use(i)))
        {

            // strtok will crush base64 values.  Brute force it.

            strcpy(KVP_temp, KVP);

            for (int i = 0; i < strlen(KVP_temp); i++)
            {
                if (KVP_temp[i] == '=')
                {
                    KVP_temp[i] = 0;
                }

                Key = &KVP_temp[0];
                Value = &KVP_temp[i + 1];
            }

            if ((Key != NULL) && (Value != NULL) && (strcmp(KeyIn, Key) == 0))
            {
                *Slot = i;
                return Value;
            }
        }
    }

    return NULL;
}

int32_t rt_config_get_empty_slot(uint16_t *Slot)
{
    for (uint16_t i = 0; i < CONFIG_RT_MAX_ITEMS; i++)
    {
        if (rt_config_slot_in_use(i) == NULL)
        {
            *Slot = i;
            return (int32_t)i;
        }
    }

    return -ENOMEM;
}

int32_t rt_config_store_key_value_at_slot(char *KeyIn, char *ValueIn, uint16_t Slot)
{
    int Len = snprintf(&KeyValue[2], CONFIG_RT_KEY_VALUE_MAX_LENGTH, "%s=%s", KeyIn, ValueIn);

    uint16_t CRC16 = crc16_ccitt(0x1234, &KeyValue[2], Len + 1);

    uint16_t *CRC = (uint16_t *)&KeyValue[0];

    *CRC = CRC16;

    Len += 3; // CRC + null terminator

    int32_t err = nvs_write(&fs, (uint16_t)Slot + CONFIG_RT_ITEM_BASE, KeyValue, Len);

    if (err < 0)
    {
        LOG_ERR("Error %i while trying to write key %s to slot %d", err, KeyIn, Slot);
    }
    else if (err == 0)
    {
        LOG_DBG("%s is unchanged at slot %d", KeyIn, Slot);
    }
    else if (err != Len)
    {
        LOG_DBG("%d of %d written to slot %d for key %s", err, Len, Slot, ValueIn);
    }
    else
    {
        LOG_DBG("%s=%s written to slot %d", KeyIn, ValueIn, Slot);
    }

    return err;
}

int32_t rt_config_store_key_value(char *KeyIn, char *ValueIn)
{
    uint16_t Slot = 0;

    int32_t Err = 0;

    if (rt_config_get_key_value(KeyIn, &Slot) == NULL)
    {
        Err = rt_config_get_empty_slot(&Slot);
    }

    if (Err < 0)
    {
        LOG_ERR("No more slots to store %s", KeyIn);
        return -ENOMEM;
    }

    return rt_config_store_key_value_at_slot(KeyIn, ValueIn, Slot);
}

void rt_config_load_defaults()
{
    STRUCT_SECTION_FOREACH(rt_config_item, config_item)
    {
        rt_config_load_with_value(config_item, config_item->Default);
    }
}

void rt_config_load_values()
{
    char *Value;
    uint16_t Slot;

    STRUCT_SECTION_FOREACH(rt_config_item, config_item)
    {
        if ((Value = rt_config_get_key_value(config_item->ConfigItemName, &Slot)))
        {
            rt_config_load_with_value(config_item, Value);
        }
        else
        {
            LOG_INF("%s does not exist in storage.  Storing default.", config_item->ConfigItemName);

            rt_config_store_key_value(config_item->ConfigItemName, config_item->Default);
        }
    }
}

static int rt_config_init()
{
    int rc = 0;
    struct flash_pages_info info;

    fs.flash_device = NVS_PARTITION_DEVICE;
    fs.offset = NVS_PARTITION_OFFSET;

    rc = flash_get_page_info_by_offs(fs.flash_device, fs.offset, &info);

    fs.sector_size = info.size;
    fs.sector_count = 4U;

    // rc = nvs_init(&fs, DT_CHOSEN_ZEPHYR_FLASH_CONTROLLER_LABEL);
    rc = nvs_mount(&fs);

    rt_config_load_defaults();

    uint32_t SlotsInUse = 0;

    char *KVP;
    for (int i = 0; i < CONFIG_RT_MAX_ITEMS; i++)
    {

        if ((KVP = rt_config_slot_in_use(i)))
        {
            LOG_DBG("Slot %d:%s", i, KVP);

            SlotsInUse++;
        }
        else
        {
            LOG_DBG("Slot %d:empty", i);
        }
    }

    LOG_INF("%d / %d slots in use", SlotsInUse, CONFIG_RT_MAX_ITEMS);

    rt_config_load_values();

    rt_config_show_current_values();

    return 0;
}

struct rt_config_item *rt_config_get_config_item(char *Name)
{

    STRUCT_SECTION_FOREACH(rt_config_item, config_item)
    {
        if (strcmp(Name, config_item->ConfigItemName) == 0)
        {
            return config_item;
        }
    }

    return NULL;
}

SYS_INIT(rt_config_init, APPLICATION, 0);

static int rt_config_wipe_handler(const struct shell *shell,
                                  size_t argc,
                                  char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    for (int i = 0; i < CONFIG_RT_MAX_ITEMS; i++)
    {
        nvs_delete(&fs, CONFIG_RT_ITEM_BASE + i);
    }

    return 0;
}

SHELL_CMD_REGISTER(wipe, NULL, "wipes runtime configuration", rt_config_wipe_handler);

void OutputParameterError(const struct shell *shell, char *ErrorString)
{
    shell_fprintf(shell, SHELL_NORMAL, "%s", ErrorString);
}

void get_single(const struct shell *shell, char *Param)
{
    char ValueString[CONFIG_RT_KEY_VALUE_MAX_LENGTH];

    struct rt_config_item *ci = NULL;

    ci = rt_config_get_config_item(Param);


    rt_config_get_value_string(ci, ValueString, CONFIG_MAX_VALUE_STRING_LENGTH);
        
    shell_fprintf(shell, SHELL_NORMAL,"\r\n");

    shell_fprintf(shell, SHELL_NORMAL,  "Name : ");
    shell_fprintf(shell, SHELL_VT100_COLOR_GREEN,  "%s\r\n", ci->ConfigItemName);

    shell_fprintf(shell, SHELL_NORMAL,  "    Description : %s\r\n", ci->DescriptionString);
       
    shell_fprintf(shell, SHELL_NORMAL,  "    Value : ");
    shell_fprintf(shell, SHELL_VT100_COLOR_GREEN,  "%s\r\n", ValueString);
        
    shell_fprintf(shell, SHELL_NORMAL,  "    Min : %s\r\n", ci->Minimum);
    shell_fprintf(shell, SHELL_NORMAL,  "    Max : %s\r\n", ci->Maximum);
    shell_fprintf(shell, SHELL_NORMAL,  "    Default : %s\r\n", ci->Default);

}

void get_all(const struct shell *shell)
{
    char ValueString[CONFIG_RT_KEY_VALUE_MAX_LENGTH];


    shell_fprintf(shell, SHELL_NORMAL,"\r\n");

    shell_fprintf(shell, SHELL_NORMAL,"Run-time configuration parameters\r\n");
    shell_fprintf(shell, SHELL_NORMAL,"-------------------------------------------\r\n");
    shell_fprintf(shell, SHELL_NORMAL,"\r\n");   

    STRUCT_SECTION_FOREACH(rt_config_item, ci)
    {

        rt_config_get_value_string(ci, ValueString, CONFIG_MAX_VALUE_STRING_LENGTH);
        
        shell_fprintf(shell, SHELL_NORMAL,"\r\n");

        shell_fprintf(shell, SHELL_NORMAL,  "Name : ");
        shell_fprintf(shell, SHELL_VT100_COLOR_GREEN,  "%s\r\n", ci->ConfigItemName);

        shell_fprintf(shell, SHELL_NORMAL,  "    Description : %s\r\n", ci->DescriptionString);
        
        shell_fprintf(shell, SHELL_NORMAL,  "    Value : ");
        shell_fprintf(shell, SHELL_VT100_COLOR_GREEN,  "%s\r\n", ValueString);
        
        shell_fprintf(shell, SHELL_NORMAL,  "    Min : %s\r\n", ci->Minimum);
        shell_fprintf(shell, SHELL_NORMAL,  "    Max : %s\r\n", ci->Maximum);
        shell_fprintf(shell, SHELL_NORMAL,  "    Default : %s\r\n", ci->Default);

    }

    shell_fprintf(shell, SHELL_NORMAL,"\r\n");

}

int get_handler(const struct shell *shell,
                size_t argc,
                char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

#define BQ shell

    if (argc == 1)
    {
        get_all(shell);
    }
    else
    {
        get_single(shell, argv[1]);
    }
    return 0;
}

SHELL_CMD_REGISTER(get, NULL, "Gets a configuration item", get_handler);

int set_handler(const struct shell *shell, int32_t argc, char **argv)
{
    if (argc == 3)
    {

        struct rt_config_item *ci = NULL;

        ci = rt_config_get_config_item(argv[1]);

        if (ci == NULL)
        {
            OutputParameterError(shell, "Parameter Not Found");
        }
        else
        {
            rt_config_load_with_value(ci, argv[2]);
            get_single(shell, argv[1]);
        }
    }
    else
    {
        OutputParameterError(shell, "Bad Arguments");
    }

    return 0;
}

SHELL_CMD_REGISTER(set, NULL, "sets a config parameter", set_handler);

void rt_config_export()
{
    char ValueString[CONFIG_MAX_VALUE_STRING_LENGTH];

    STRUCT_SECTION_FOREACH(rt_config_item, ci)
    {
        rt_config_get_value_string(ci, ValueString, CONFIG_MAX_VALUE_STRING_LENGTH);
        rt_config_store_key_value(ci->ConfigItemName, ValueString);
    }
}

int save_handler(const struct shell *shell, int32_t argc, char **argv)
{
    rt_config_export();
    get_all(shell);

    return 0;
}

SHELL_CMD_REGISTER(save, NULL, "saves the current runtime config", save_handler);
