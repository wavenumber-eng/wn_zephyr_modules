#include "rt_nvs.h"
#include "rt_config.h"


#define NVS_PARTITION_DEVICE FIXED_PARTITION_DEVICE(NVS_PARTITION)
#define NVS_PARTITION_OFFSET FIXED_PARTITION_OFFSET(NVS_PARTITION)


static struct nvs_fs fs;
static char KeyValue[CONFIG_RT_KEY_VALUE_MAX_LENGTH + 2]; // 1st 2 bytes will be CRC

LOG_MODULE_REGISTER(rt_nvs, LOG_LEVEL_INF);

char *rt_nvs__slot_in_use(uint32_t Slot)
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


int32_t rt_nvs__get_empty_slot(uint16_t *Slot)
{
    for (uint16_t i = 0; i < CONFIG_RT_MAX_ITEMS; i++)
    {
        if (rt_nvs__slot_in_use(i) == NULL)
        {
            *Slot = i;
            return (int32_t)i;
        }
    }

    return -ENOMEM;
}

char KVP_temp[CONFIG_RT_KEY_VALUE_MAX_LENGTH];

char *rt_nvs__get_key_value(char *KeyIn, uint16_t *Slot)
{
    char *Key = 0;
    char *Value = 0;
    char *KVP = 0;

    for (int i = 0; i < CONFIG_RT_MAX_ITEMS; i++)
    {

        if ((KVP = rt_nvs__slot_in_use(i)))
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

int32_t rt_nvs__store_key_value_at_slot(char *KeyIn, char *ValueIn, uint16_t Slot)
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



int32_t rt_nvs__store_key_value(char *KeyIn, char *ValueIn)
{
    uint16_t Slot = 0;

    int32_t Err = 0;

    if (rt_nvs__get_key_value(KeyIn, &Slot) == NULL)
    {
        Err = rt_nvs__get_empty_slot(&Slot);
    }

    if (Err < 0)
    {
        LOG_ERR("No more slots to store %s", KeyIn);
        return -ENOMEM;
    }

    return rt_nvs__store_key_value_at_slot(KeyIn, ValueIn, Slot);
}


void rt_nvs__load_values()
{
    char *Value;
    uint16_t Slot;

    STRUCT_SECTION_FOREACH(rt_config_item, config_item)
    {
        if ((Value = rt_nvs__get_key_value(config_item->ConfigItemName, &Slot)))
        {
            rt_config_load_with_value(config_item, Value);
        }
        else
        {
            LOG_INF("%s does not exist in storage.  Storing default.", config_item->ConfigItemName);

            rt_nvs__store_key_value(config_item->ConfigItemName, config_item->Default);
        }
    }
}



int32_t rt_nvs__init()
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


    uint32_t SlotsInUse = 0;

    char *KVP;
    for (int i = 0; i < CONFIG_RT_MAX_ITEMS; i++)
    {

        if ((KVP = rt_nvs__slot_in_use(i)))
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

    return 0;
}



void rt_nvs__wipe()
{
    for (int i = 0; i < CONFIG_RT_MAX_ITEMS; i++)
    {
        nvs_delete(&fs, CONFIG_RT_ITEM_BASE + i);
    }
}

void rt_nvs__export()
{
    char ValueString[CONFIG_MAX_VALUE_STRING_LENGTH];

    STRUCT_SECTION_FOREACH(rt_config_item, ci)
    {
        rt_config_get_value_string(ci, ValueString, CONFIG_MAX_VALUE_STRING_LENGTH);
        rt_nvs__store_key_value(ci->ConfigItemName, ValueString);
    }
}