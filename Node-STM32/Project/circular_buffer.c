#include "common.h"
#include "circular_buffer.h"
#include <stdlib.h>

struct CircularBuffer* CircularBuffer_New(int size)
{
    struct CircularBuffer* b;
    b = (struct CircularBuffer*)malloc(sizeof(struct CircularBuffer) + size);
    if(b){
        b->capacity = size;
        b->rd_ptr = b->wr_ptr = b->size = 0;
    }
    return b;
}

void CircularBuffer_Free(struct CircularBuffer* b)
{
    free(b);
}

void CircularBuffer_Clear(struct CircularBuffer* b)
{
    __disable_irq();
    b->rd_ptr = b->wr_ptr = b->size = 0;
    __enable_irq();
}

bool CircularBuffer_Push(struct CircularBuffer* b, uint8_t element)
{
    __disable_irq();
    if(b->size == b->capacity){ //buffer is full
        __enable_irq();
        return false;
    }
    b->data[b->wr_ptr++] = element;
    if(b->wr_ptr == b->capacity)
        b->wr_ptr = 0;
    b->size++;
    __enable_irq();
    return true;
}

bool CircularBuffer_Pop(struct CircularBuffer* b, uint8_t *element)
{
    __disable_irq();
    if(b->size == 0){ //buffer is empty
        __enable_irq();
        return false;
    }
    *element = b->data[b->rd_ptr++];
    if(b->rd_ptr == b->capacity)
        b->rd_ptr = 0;
    b->size--;
    __enable_irq();
    return true;
}

int CircularBuffer_Size(struct CircularBuffer* b)
{
    return b->size;
}

