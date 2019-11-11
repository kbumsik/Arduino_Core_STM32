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
#ifdef VIRTIOCON

#include "virtio_config.h"
#include "virtio_buffer.h"
#include <string.h>
#include "wiring.h"

#define BUFFER_END (VIRTIO_BUFFER_SIZE - 1)

static uint16_t read_tmp(virtio_buffer_t *ring, uint8_t *dst, uint16_t size);
static void read_commit(virtio_buffer_t *ring);
static void read_rollback(virtio_buffer_t *ring);

void virtio_buffer_init(virtio_buffer_t *ring)
{
  ring->write = 0;
  ring->read = 0;
  ring->read_tmp = 0;
}

uint16_t virtio_buffer_read_available(virtio_buffer_t *ring)
{
  // This will make the function safe when write openrations are done in interrupts
  volatile uint16_t write = ring->write;

  if (write < ring->read) {
    return (BUFFER_END - ring->read) + (write + 1);
  }
  return write - ring->read;
}

static uint16_t read_tmp(virtio_buffer_t *ring, uint8_t *dst, uint16_t size)
{
  // This will make the function safe when write openrations are done in interrupts
  volatile uint16_t write = ring->write;
  uint16_t end = (write >= ring->read_tmp) ? write : BUFFER_END + 1;

  size = min(end - ring->read_tmp, size);
  memcpy(dst, ring->buffer + ring->read_tmp, size);
  ring->read_tmp += size;
  if (ring->read_tmp > BUFFER_END) {
    ring->read_tmp = 0;
  }
  return size;
}

static void read_commit(virtio_buffer_t *ring)
{
  ring->read = ring->read_tmp;
}

static void read_rollback(virtio_buffer_t *ring)
{
  ring->read_tmp = ring->read;
}

uint16_t virtio_buffer_read(virtio_buffer_t *ring, uint8_t *dst, uint16_t size)
{
  uint16_t recv_size = read_tmp(ring, dst, size);
  read_commit(ring);
  return recv_size;
}

/**
 * WARNING: The size of read cannot be larger than virtio_buffer_read_available().
 */
uint16_t virtio_buffer_peek(virtio_buffer_t *ring, uint8_t *dst, uint16_t size)
{
  size = min(size, virtio_buffer_read_available(ring));
  uint16_t recv_size = 0;
  while (recv_size < size) {
    recv_size += read_tmp(ring, dst + recv_size, size - recv_size);
  }
  read_rollback(ring);
  return recv_size;
}

uint16_t virtio_buffer_write_available(virtio_buffer_t *ring)
{
  // This will make the function safe when read openrations are done in interrupts
  volatile uint16_t read = ring->read;

  if (ring->write < read) {
    return (read - 1) - ring->write;
  }
  return read + (BUFFER_END - ring->write);
}

uint16_t virtio_buffer_write(virtio_buffer_t *ring, uint8_t *src, uint16_t size)
{
  // This will make the function safe when read openrations are done in a interrupt
  volatile uint16_t read = ring->read;
  uint16_t end = (ring->write < read) ? read - 1
                 : (read == 0) ? BUFFER_END : BUFFER_END + 1;

  size = min(end - ring->write, size);
  memcpy(ring->buffer + ring->write, src, size);
  ring->write += size;
  if (ring->write > BUFFER_END) {
    ring->write = 0;
  }
  return size;
}

#endif /* VIRTIOCON */
