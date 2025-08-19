
#include <zephyr/kernel.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/sys/printk.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/shell/shell.h>
#include <rt_config.h>

#if defined(CONFIG_RT_CONFIG_USE_FILE)
#include "rt_file.h"
#endif

#if defined(CONFIG_RT_CONFIG_USE_NVS)
#include "rt_nvs.h"
#endif


LOG_MODULE_REGISTER(rt_config, LOG_LEVEL_INF);


// Function to get the value string of a configuration item
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

// Function to print the current values of all configuration items
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


// Function to load a configuration item with a value, value is a string
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


// Function load the default values for all configuration items
void rt_config_load_defaults()
{
    STRUCT_SECTION_FOREACH(rt_config_item, config_item)
    {
        rt_config_load_with_value(config_item, config_item->Default);
    }
}



int rt_config_init()
{
    int32_t rc = 0;
#if defined(CONFIG_RT_CONFIG_USE_NVS)
    rt_config_load_defaults();
    rt_nvs__init();
    rt_nvs__load_values();

#elif defined(CONFIG_RT_CONFIG_USE_FILE)

    /*
    RT_CONFIG file behavior:
    - If RT_CONFIG_USE_FILE is defined, so we will use the file system to store the configuration.
    - If the configuration file does exist, import the values from it.
    - If it does not exist, create a new file with default values.
    */

    rt_config_load_defaults();
    rc = rt_file__init();

    if( rc == RT_RETURN_OK)
    {
        rt_file__import();
    }
    else if (rc == RT_RETURN_NEW_FILE_CREATED)
    {
        rt_file__export();
    }
    else
    {
        LOG_ERR("Error opening config file, please check your configuration");
    }

#endif

    //rt_config_show_current_values();
    
    return 0;
}

#if defined(CONFIG_RT_CONFIG_USE_NVS)
SYS_INIT(rt_config_init, APPLICATION, 0);
#endif




// Function to get a configuration item by name
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


/*
* WHIPE COMMAND
*/

static int rt_config_wipe_handler(const struct shell *shell,
                                  size_t argc,
                                  char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

#if defined(CONFIG_RT_CONFIG_USE_NVS)
    rt_nvs__wipe();

#elif defined(CONFIG_RT_CONFIG_USE_FILE)
    rt_file__wipe();
#endif
    return 0;
}

SHELL_CMD_REGISTER(wipe, NULL, "wipes runtime configuration", rt_config_wipe_handler);



/*
* GET COMMAND
*/

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

    shell_fprintf(shell, SHELL_NORMAL,  "Access Level : %s\r\n", (ci->access_level == RT_CONFIG_LEVEL__USER) ? "User" : "Admin");

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

        shell_fprintf(shell, SHELL_NORMAL,  "    Access Level : %s\r\n", (ci->access_level == RT_CONFIG_LEVEL__USER) ? "User" : "Admin");

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



/*
* SET COMMAND
*/
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


/*
* EXPORT COMMAND
*/
void rt_config_export()
{

#if defined(CONFIG_RT_CONFIG_USE_NVS)
    rt_nvs__export();
#elif defined(CONFIG_RT_CONFIG_USE_FILE)
    rt_file__export();
#endif

}

int save_handler(const struct shell *shell, int32_t argc, char **argv)
{
    rt_config_export();
    get_all(shell);

    return 0;
}

SHELL_CMD_REGISTER(save, NULL, "saves the current runtime config", save_handler);




uint8_t rt_config__is_integer_config(struct rt_config_item *ci)
{
    if (ci == NULL || ci->Value == NULL)
    {
        return 0;
    }

    switch (ci->DataType)
    {
    case RT_CONFIG_DATA_TYPE_UINT8:
    case RT_CONFIG_DATA_TYPE_UINT16:
    case RT_CONFIG_DATA_TYPE_UINT32:
    case RT_CONFIG_DATA_TYPE_INT8:
    case RT_CONFIG_DATA_TYPE_INT16:
    case RT_CONFIG_DATA_TYPE_INT32:
        return 1;

    default:
        return 0;
    }
}