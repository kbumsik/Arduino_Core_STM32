/**
 * MIT License:
 * Copyright (c) 2019 Bumsik kim <k.bumsik@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __VIRTIO_RINGBUFFER_H
#define __VIRTIO_RINGBUFFER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VIRTIO_RINGBUFFER_SIZE (RPMSG_BUFFER_SIZE * 2)

typedef struct {
  uint8_t buffer[VIRTIO_RINGBUFFER_SIZE];
  volatile uint16_t write;
  volatile uint16_t read;
  volatile uint16_t read_tmp;
} ringbuffer_t;

void ringbuffer_init(ringbuffer_t *ring);

uint16_t ringbuffer_read_available(ringbuffer_t *ring);
uint16_t ringbuffer_read(ringbuffer_t *ring, uint8_t *dst, uint16_t size);
uint16_t ringbuffer_peek(ringbuffer_t *ring, uint8_t *dst, uint16_t size);

uint16_t ringbuffer_write_available(ringbuffer_t *ring);
uint16_t ringbuffer_write(ringbuffer_t *ring, uint8_t *src, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif // __VIRTIO_RINGBUFFER_H
