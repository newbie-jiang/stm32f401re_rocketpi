#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uint8_t *buffer;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t size;
} RingBuffer;

void RingBuffer_Init(RingBuffer *rb, uint8_t *storage, size_t capacity);
size_t RingBuffer_Space(const RingBuffer *rb);
size_t RingBuffer_Size(const RingBuffer *rb);
size_t RingBuffer_Write(RingBuffer *rb, const uint8_t *data, size_t len);
size_t RingBuffer_ReadLinear(RingBuffer *rb, uint8_t **ptr);
void RingBuffer_Advance(RingBuffer *rb, size_t len);

#endif /* RING_BUFFER_H */
