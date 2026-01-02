#include "ring_buffer.h"

#include <string.h>

void RingBuffer_Init(RingBuffer *rb, uint8_t *storage, size_t capacity)
{
    rb->buffer = storage;
    rb->capacity = capacity;
    rb->head = 0U;
    rb->tail = 0U;
    rb->size = 0U;
}

size_t RingBuffer_Space(const RingBuffer *rb)
{
    if (rb->capacity < rb->size)
    {
        return 0U;
    }
    return rb->capacity - rb->size;
}

size_t RingBuffer_Size(const RingBuffer *rb)
{
    return rb->size;
}

size_t RingBuffer_Write(RingBuffer *rb, const uint8_t *data, size_t len)
{
    size_t written = 0U;

    while ((written < len) && (rb->size < rb->capacity))
    {
        size_t space_to_end = rb->capacity - rb->head;
        size_t chunk = len - written;

        if (chunk > space_to_end)
        {
            chunk = space_to_end;
        }

        size_t free_space = rb->capacity - rb->size;
        if (chunk > free_space)
        {
            chunk = free_space;
        }

        if (chunk == 0U)
        {
            break;
        }

        (void)memcpy(&rb->buffer[rb->head], &data[written], chunk);
        rb->head = (rb->head + chunk) % rb->capacity;
        rb->size += chunk;
        written += chunk;
    }

    return written;
}

size_t RingBuffer_ReadLinear(RingBuffer *rb, uint8_t **ptr)
{
    if (rb->size == 0U)
    {
        *ptr = NULL;
        return 0U;
    }

    *ptr = &rb->buffer[rb->tail];
    size_t contiguous = rb->capacity - rb->tail;

    if (contiguous > rb->size)
    {
        contiguous = rb->size;
    }

    return contiguous;
}

void RingBuffer_Advance(RingBuffer *rb, size_t len)
{
    if (len > rb->size)
    {
        len = rb->size;
    }

    rb->tail = (rb->tail + len) % rb->capacity;
    rb->size -= len;
}
