#include "rt_file.h"
#include "rt_config.h"

#define STORAGE_PARTITION			storage_partition
#define STORAGE_PARTITION_ID		FIXED_PARTITION_ID(STORAGE_PARTITION)


LOG_MODULE_REGISTER(rt_file, LOG_LEVEL_INF);



#define CONFIG_FILE_PATH (CONFIG_RT_CONFIG_MOUNT_POINT"/"CONFIG_RT_CONFIG_FILE_NAME)


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


		rt_config_get_value_string(config_item, ValueString, CONFIG_MAX_VALUE_STRING_LENGTH);
		fs_write(&conf_file, config_item->ConfigItemName, strlen(config_item->ConfigItemName));
		fs_write(&conf_file, "=", 1);
		fs_write(&conf_file, ValueString, strlen(ValueString));
		fs_write(&conf_file, "\n", 1);

		LOG_INF("- %s=%s", config_item->ConfigItemName, ValueString);

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
			break; // End of line
		}

		i++;
	}

	line[i] = '\0'; // Null-terminate the string

	return i;
}


int32_t rt_file__import()
{
	struct fs_file_t conf_file;
	char line[CONFIG_MAX_VALUE_STRING_LENGTH];
	int32_t rc = 0;
	uint32_t line_length;


	fs_file_t_init(&conf_file);

	rc = fs_open(&conf_file, CONFIG_FILE_PATH, FS_O_READ);
	if (rc != 0)
	{
		LOG_ERR("Failed to open config file %s, error: %d", CONFIG_FILE_PATH, rc);
		return rc;
	}

	LOG_INF("Importing configuration from %s", CONFIG_FILE_PATH);

	while (1)
	{
		line_length = rt_file__read_line(&conf_file, line, sizeof(line));

		// End of file 
		if (line_length == 0)
		{
			break; 
		}

		char *key = strtok(line, "=");
		char *value = strtok(NULL, "\n");

		if (key && value)
		{
			rt_config_load_with_value(rt_config_get_config_item(key), value);
			LOG_INF("Loaded config: %s=%s", key, value);
		}


	}

	fs_close(&conf_file);
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
	
