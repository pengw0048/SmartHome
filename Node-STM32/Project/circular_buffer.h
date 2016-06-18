#ifndef CIRCULAR_BUFFER_H__
#define CIRCULAR_BUFFER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct CircularBuffer
{
    int size, capacity, rd_ptr, wr_ptr;
    uint8_t data[];
};

struct CircularBuffer* CircularBuffer_New(int size);
void CircularBuffer_Clear(struct CircularBuffer* b);
void CircularBuffer_Free(struct CircularBuffer* b);
bool CircularBuffer_Push(struct CircularBuffer* b, uint8_t element);
bool CircularBuffer_Pop(struct CircularBuffer* b, uint8_t *element);
int CircularBuffer_Size(struct CircularBuffer* b);

#ifdef __cplusplus
};
#endif

#endif
