
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "data_buf.h"

LOG_MODULE_REGISTER(data_buf);

K_HEAP_DEFINE(data_buffer_heap, CONFIG_DATA_BUF_POOL_BYTES);

data_buf_t * data_buf__create(uint32_t length,data_buf__callback_t cb)
{
    data_buf_t * db = 0;
    
    db = k_heap_alloc(&data_buffer_heap, length + sizeof(data_buf_t),K_NO_WAIT);

    if(db)
    {
        db->data = (uint8_t *)db + sizeof(data_buf_t);
        db->max_length = length;
        db->current_length = 0;
        db->complete = cb;
        db->user = db;
    }
    else
    {
        LOG_ERR("Could not allocate %d bytes.",length);
    }

    return db;
}

void data_buf__destroy(data_buf_t * db)
{
    if(db->complete != NULL)
    {
        db->complete((void *)db->user);
    }

    k_heap_free(&data_buffer_heap,db);
}