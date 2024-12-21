#include "stdint.h"
#include <stddef.h>

#ifndef _DATA_BUFFER_H
#define _DATA_BUFFER_H

typedef void (*data_buf__callback_t)(void *) ; 

typedef struct
{
    void * fifo_reserved; //zephyr needs this for the linked list

    uint32_t current_length; //initialize to zero on creation
    uint32_t max_length;     //initialized to the length argument on create
    uint8_t *data;           //pointer to the data
    void *user;              //user argument that will be passed to the callback when the buffer is destroyed
    data_buf__callback_t complete;  //optional callback when data_buf is destroyed

} data_buf_t;


//returns a valid pointer on success, zero/null on failure
data_buf_t * data_buf__create(uint32_t length,data_buf__callback_t cb);

void data_buf__destroy(data_buf_t * db);

#endif