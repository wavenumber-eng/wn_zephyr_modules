#include "rt_file.h"
#include "rt_config.h"
#include <stdio.h>
#include <stdlib.h>

#define STORAGE_PARTITION			storage_partition
#define STORAGE_PARTITION_ID		FIXED_PARTITION_ID(STORAGE_PARTITION)


uint8_t damaged_lines [CONFIG_RT_MAX_ITEMS] = {0};
config_status_t config_status_table [CONFIG_RT_MAX_ITEMS] = {0};

LOG_MODULE_REGISTER(rt_file, LOG_LEVEL_INF);

#define CONFIG_FILE_PATH (CONFIG_RT_CONFIG_MOUNT_POINT CONFIG_RT_CONFIG_FILE_NAME)
#define TEMP_FILE_NAME		"temp.txt"
#define TEMP_FILE_PATH		(CONFIG_RT_CONFIG_MOUNT_POINT TEMP_FILE_NAME)

// FILE SYSTEM SHOULD BE MOUNTED BEFORE THIS FUNCTION IS CALLED
rt_return_code_e rt_file__init()
{
	struct fs_file_t conf_file;
	int rc = 0;
	rt_return_code_e return_code;

	// Verify if a configuration file exists, if not create it
	LOG_INF("Checking for config file %s", CONFIG_FILE_PATH);

	fs_file_t_init(&conf_file);

	rc = fs_open(&conf_file, CONFIG_FILE_PATH, FS_O_RDWR);

	switch(rc)
	{
		case 0:
			LOG_INF("Config file %s exists\n\n", CONFIG_FILE_PATH);
			fs_close(&conf_file);
			return_code = RT_RETURN_OK;
			break;

		case (-ENOENT):
			LOG_INF("Config file %s does not exist, creating it", CONFIG_FILE_PATH);
			rc = fs_open(&conf_file, CONFIG_FILE_PATH, FS_O_CREATE | FS_O_RDWR);
			if (rc == 0)
			{
				LOG_INF("Config file %s created successfully\n\n", CONFIG_FILE_PATH);
				fs_close(&conf_file);
				return_code = RT_RETURN_NEW_FILE_CREATED;
			}
			else
			{
				LOG_ERR("Failed to create config file %s, error: %d\n\n", CONFIG_FILE_PATH, rc);
				fs_close(&conf_file);
				return_code = RT_RETURN_FILE_ERROR;
			}
			break;

		case (-EINVAL):
			LOG_ERR("Invalid file name for config file %s, error: %d\n\n", CONFIG_FILE_PATH, rc);
			fs_close(&conf_file);
			return_code = RT_RETURN_FILE_ERROR;
			break;

		default:
			LOG_ERR("Error opening config file %s, error: %d\n\n", CONFIG_FILE_PATH, rc);
			return_code = RT_RETURN_FILE_ERROR;
			break;
	}

	uint32_t item = 0;

	STRUCT_SECTION_FOREACH(rt_config_item, ci)
	{
		config_status_table[item].ci = ci;
		config_status_table[item].in_file = 0;
		item++;
	}

	return return_code;
}


int32_t rt_file__export()
{
	struct fs_file_t conf_file;
	char ValueString[CONFIG_MAX_VALUE_STRING_LENGTH];

	fs_file_t_init(&conf_file);
	
	// if the file exists, delete the content
	if(fs_open(&conf_file, CONFIG_FILE_PATH, FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC) != 0)
	{
		LOG_INF("Could not open file for writing");
		return -1;
	}

	LOG_INF("Exporting configuration to %s", CONFIG_FILE_PATH);

	STRUCT_SECTION_FOREACH(rt_config_item, config_item)
    {
		if (config_item->Value == NULL)
		{
			continue;
		}

		// Only include in the rt_config file the user-accessible items
		if(config_item->access_level == RT_CONFIG_LEVEL__USER)
		{
			// rt-config header
			snprintf(ValueString, CONFIG_MAX_VALUE_STRING_LENGTH, "# ---------------------------------------\n");
			fs_write(&conf_file, ValueString, strlen(ValueString));

			snprintf(ValueString, CONFIG_MAX_VALUE_STRING_LENGTH, "# Setting: %s\n", config_item->ConfigItemName);
			fs_write(&conf_file, ValueString, strlen(ValueString));

			// rt-config description
			snprintf(ValueString, CONFIG_MAX_VALUE_STRING_LENGTH, "# Description: %s\n", config_item->DescriptionString);
			fs_write(&conf_file, ValueString, strlen(ValueString));

			// rt-config min value
			snprintf(ValueString, CONFIG_MAX_VALUE_STRING_LENGTH, "# Min Value: %s\n", config_item->Minimum);
			fs_write(&conf_file, ValueString, strlen(ValueString));

			// rt-config max value
			snprintf(ValueString, CONFIG_MAX_VALUE_STRING_LENGTH, "# Max Value: %s\n\n", config_item->Maximum);
			fs_write(&conf_file, ValueString, strlen(ValueString));

			rt_config_get_value_string(config_item, ValueString, CONFIG_MAX_VALUE_STRING_LENGTH);
	
            fs_write(&conf_file, config_item->ConfigItemName, strlen(config_item->ConfigItemName));
			fs_write(&conf_file, "=", 1);
			fs_write(&conf_file, ValueString, strlen(ValueString));
			fs_write(&conf_file, "\n\n", 2);

			LOG_INF("- %s=%s", config_item->ConfigItemName, ValueString);
		}

		// Admin accessible items
		else
		{
			uint8_t value_overwritten = 0;

			rt_config_get_value_string(config_item, ValueString, CONFIG_MAX_VALUE_STRING_LENGTH);

			if(config_item->DataType == RT_CONFIG_DATA_TYPE_FLOAT32)
			{
				float float_default = atof(config_item->Default);
				float float_value = atof(ValueString);
				value_overwritten = (float_value != float_default);
			}
			else
			{
				value_overwritten = (strcmp(ValueString, config_item->Default) != 0);
			}
			

			if(value_overwritten)
			{
				rt_config_get_value_string(config_item, ValueString, CONFIG_MAX_VALUE_STRING_LENGTH);
				fs_write(&conf_file, config_item->ConfigItemName, strlen(config_item->ConfigItemName));
				fs_write(&conf_file, "=", 1);
				fs_write(&conf_file, ValueString, strlen(ValueString));
				fs_write(&conf_file, "\n\n", 2);
			}
		}
    }

	fs_close(&conf_file);
	return 0;
}

// Function to read a line from the file
// Returns the number of characters read
uint32_t rt_file__read_line(struct fs_file_t *conf_file, char *line, uint32_t max_length)
{
	int32_t i = 0;
	char c;

	while(i < (max_length - 1))
	{
		// Read a character from the file
		c = fs_read(conf_file, &line[i], 1);

		if (c <= 0)
		{
			break; // End of file or error
		}

		if (line[i] == '\n')
		{
			i++;
			break; // End of line
		}

		i++;
	}

	line[i] = '\0'; // Null-terminate the string
	return i;
}

// Function to get the number of missing config items, defined in code but not present in the config file
uint32_t rt_file__get_missing_items()
{
	uint32_t missing_items = 0;

	STRUCT_SECTION_FOREACH(rt_config_item, ci)
	{
		if(ci->access_level == RT_CONFIG_LEVEL__USER)
		{
			if(rt_file__get_item_in_file(ci) == 0)
			{
				LOG_INF("Missing config item: %s", ci->ConfigItemName);
				missing_items++;
			}
		}
	}

	return missing_items;
}

void rt_file__set_item_in_file(struct rt_config_item *ci, uint8_t in_file)
{
	for(uint32_t i = 0; i < CONFIG_RT_MAX_ITEMS; i++)
	{
		if(config_status_table[i].ci == ci)
		{
			config_status_table[i].in_file = in_file;
			return;
		}
	}
}


uint8_t rt_file__get_item_in_file(struct rt_config_item *ci)
{
	for(uint32_t i = 0; i < CONFIG_RT_MAX_ITEMS; i++)
	{
		if(config_status_table[i].ci == ci)
		{
			return config_status_table[i].in_file;
		}
	}
	return 0;
}

// Verify that the lines in the configuration file are valid
int32_t rt_file__verify_file()
{
	struct fs_file_t conf_file;
	char line[CONFIG_MAX_VALUE_STRING_LENGTH];
	uint32_t line_length;
	char *key;
	char *value;
	uint32_t missing_items = 0;
	int rc;
	struct rt_config_item *ci = NULL;


	fs_file_t_init(&conf_file);

	rc = fs_open(&conf_file, CONFIG_FILE_PATH, FS_O_READ);
	if (rc != 0)
	{
		LOG_ERR("Failed to open config file %s, error: %d", CONFIG_FILE_PATH, rc);
		return rc;
	}


	while (1)
	{
		line_length = rt_file__read_line(&conf_file, line, sizeof(line));

		// End of file 
		if (line_length == 0)
		{
			break; 
		}

		// Skip comment or empty lines
		if(line[0] == '#' || line[0] == '\n' || line[0] == '\r')
		{
			continue;
		}


		key = strtok(line, "=");
		value = strtok(NULL, "\n");

		if(key && value)
		{
			ci = rt_config_get_config_item(key);

			// Unknown item
			if (ci == NULL)
			{
				continue;
			}

			// If numeric variable, doublecheck the value before assign
			if(rt_config__is_integer_config(ci) && !is_numeric_string(value, strlen(value)))
			{
				LOG_WRN("Invalid value for config item %s: %s", key, value);
				continue;
			}

			rt_config_load_with_value(ci, value);
			rt_file__set_item_in_file(ci, 1);
		}
	}

	fs_close(&conf_file);


	missing_items = rt_file__get_missing_items();
	if(missing_items)
	{
		LOG_WRN("There are %d configuration items defined in code but missing in the config file", missing_items);
		LOG_WRN("Re generating config file %s", CONFIG_FILE_PATH);
		rt_file__export();
	}
	else
	{
		LOG_INF("All configuration items are present in the config file");
	}

	return 0;

}



int8_t rt_file__repair()
{
    struct fs_file_t temp_file;
	struct fs_file_t conf_file;
	char line[CONFIG_MAX_VALUE_STRING_LENGTH];


	uint32_t i_damaged_line = 0;
	uint32_t i_line = 1;
	uint32_t read_bytes;
	int err;

	// Create temp file
	fs_file_t_init(&temp_file);
    err = fs_open(&temp_file, TEMP_FILE_PATH, FS_O_CREATE | FS_O_WRITE);
    if (err < 0)
    {
        fs_close(&temp_file);
        return -1;
    }

	fs_file_t_init(&conf_file);
	err = fs_open(&conf_file, CONFIG_FILE_PATH, FS_O_READ);
	if (err < 0)
	{
		fs_close(&conf_file);
		return -1;
	}


	// Copy all the not damaged lines
	while(read_bytes > 0)
	{
		read_bytes = rt_file__read_line(&conf_file, line, sizeof(line));

		if(read_bytes == 0)
		{
			break; // End of file
		}

		if(damaged_lines[i_damaged_line] == i_line)
		{
			LOG_INF("Skipping damaged line %d: %s", i_line, line);
			i_damaged_line++;
		}
		else
		{
			fs_write(&temp_file, line, read_bytes);
		}

		i_line++;
	}


	// Append missing configurations


	fs_close(&conf_file);
	fs_close(&temp_file);

	return 0;
}



int32_t rt_file__import_values()
{
	return 0;
}

int32_t rt_file__import()
{
//	struct fs_file_t conf_file;
//	char line[CONFIG_MAX_VALUE_STRING_LENGTH];
//	int32_t rc = 0;
//	uint32_t line_length;
//	struct rt_config_item *ci = NULL;


	rt_file__verify_file();

//	fs_file_t_init(&conf_file);
//
//	rc = fs_open(&conf_file, CONFIG_FILE_PATH, FS_O_READ);
//	if (rc != 0)
//	{
//		LOG_ERR("Failed to open config file %s, error: %d", CONFIG_FILE_PATH, rc);
//		return rc;
//	}
//
//	LOG_INF("Importing configuration from %s", CONFIG_FILE_PATH);
//
//	while (1)
//	{
//		line_length = rt_file__read_line(&conf_file, line, sizeof(line));
//
//		// End of file 
//		if (line_length == 0)
//		{
//			break; 
//		}
//
//		if(line[0] == '#')
//		{
//			// Skip comment lines
//			continue;
//		}
//
//		char *key = strtok(line, "=");
//		char *value = strtok(NULL, "\n");
//
//    	ci = rt_config_get_config_item(key);
//
//		if (key && value)
//		{
//			rt_config_load_with_value(ci, value);
//			LOG_INF("Loaded config: %s=%s", key, value);
//		}
//
//
//	}
//
//	fs_close(&conf_file);
	return 0;
}


void rt_file__wipe()
{
	struct fs_file_t conf_file;
	int rc = 0;

	fs_file_t_init(&conf_file);

	rc = fs_open(&conf_file, CONFIG_FILE_PATH, FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
	if (rc != 0)
	{
		LOG_ERR("Failed to open config file %s, error: %d", CONFIG_FILE_PATH, rc);
		return;
	}

	fs_close(&conf_file);
	LOG_INF("Wiped config file %s", CONFIG_FILE_PATH);
}
	


uint8_t is_numeric_string(char *str, uint32_t length)
{
	uint32_t i = 0;

	if (str == NULL || *str == '\0')
	{
		return 0;
	}

	for (i = 0; i < length; i++)
	{
		if((str[i] < '0') || (str[i] > '9'))
		{
			return 0;
		}
	}

	return 1;
}