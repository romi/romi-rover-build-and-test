#include "block.h"

block_buffer_t block_buffer;

int16_t block_buffer_readpos()
{
        return block_buffer.readpos;
}

int16_t block_buffer_writepos()
{
        return block_buffer.writepos;
}

void init_block_buffer()
{
        block_buffer.readpos = 0;
        block_buffer.writepos = 0;
}

static inline int16_t _space()
{
        if (block_buffer.writepos >= block_buffer.readpos)
                return (BLOCK_BUFFER_SIZE
                        - block_buffer.writepos
                        + block_buffer.readpos - 1);
        else
                return block_buffer.readpos - block_buffer.writepos - 1;

        // The following optimization takes advantage from the fact
        // that the buffer size is a power of two.
        // return (BLOCK_BUFFER_SIZE
        //         - block_buffer.writepos
        //         + block_buffer.readpos - 1) & BLOCK_BUFFER_SIZE_MASK;
}

int16_t block_buffer_space()
{
        return _space();
}

block_t *block_buffer_get_empty()
{
        if (_space() == 0)
                return 0;
        return &block_buffer.block[block_buffer.writepos];
}

void block_buffer_ready()
{
        int16_t p = block_buffer.writepos + 1;
        if (p == BLOCK_BUFFER_SIZE)
                p = 0;
        block_buffer.writepos = p;
}

int16_t block_buffer_available()
{
        return BLOCK_BUFFER_AVAILABLE();
}

block_t *_block_buffer_get_next()
{
        block_t *b = &block_buffer.block[block_buffer.readpos];
        block_buffer.readpos++;
        block_buffer.readpos &= BLOCK_BUFFER_SIZE_MASK;
        return b;
}

void block_buffer_clear()
{
        block_buffer.readpos = block_buffer.writepos;
}
