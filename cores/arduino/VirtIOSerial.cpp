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

#if defined (VIRTIOCON)

#include "openamp.h"
#include "openamp_log.h"
#include "wiring.h"
#include "ringbuffer.h"

VirtIOSerial SerialVirtIO;
void serialEventVirtIO() __attribute__((weak));

static VIRT_UART_HandleTypeDef huart;
static bool initialized = false;
static ringbuffer_t ring;

void rxCallback(VIRT_UART_HandleTypeDef *huart);

void VirtIOSerial::begin(void)
{
  ringbuffer_init(&ring);
  if (initialized) {
    return;
  }
  OPENAMP_Init(NULL);
  if (VIRT_UART_Init(&huart) != VIRT_UART_OK) {
    // log_err("VIRT_UART_Init UART0 failed.\r\n");
    Error_Handler();
  }
  /*Need to register callback for message reception by channels*/
  if (VIRT_UART_RegisterCallback(&huart, VIRT_UART_RXCPLT_CB_ID, rxCallback) != VIRT_UART_OK) {
    Error_Handler();
  }
  initialized = true;
}

void VirtIOSerial::begin(uint32_t /* baud_count */)
{
  // uart config is ignored in USB-CDC
  begin();
}

void VirtIOSerial::begin(uint32_t /* baud_count */, uint8_t /* config */)
{
  // uart config is ignored in USB-CDC
  begin();
}

void VirtIOSerial::end()
{
  OPENAMP_DeInit();
  ringbuffer_init(&ring);
  initialized = false;
}

int VirtIOSerial::available(void)
{
  return ringbuffer_read_available(&ring);
}

int VirtIOSerial::availableForWrite()
{
  // Just return max length of VIRT_UART_Transmit() can transmit.
  // See VIRT_UART_Transmit().
  return RPMSG_BUFFER_SIZE - 16;
}

int VirtIOSerial::peek(void)
{
  if (ringbuffer_read_available(&ring) > 0) {
    uint8_t tmp;
    ringbuffer_peek(&ring, &tmp, 1);
    return tmp;
  } else {
    return -1;
  }
}

int VirtIOSerial::read(void)
{
  if (available() > 0) {
    char ch;
    readBytes(&ch, 1);
    return ch;
  } else {
    return -1;
  }
}

size_t VirtIOSerial::readBytes(char *buffer, size_t length)
{
  const size_t size = length;
  _startMillis = millis();
  while (length > 0 && (millis() - _startMillis < _timeout)) {
    uint16_t prev_write_available = ringbuffer_write_available(&ring);
    length -= ringbuffer_read(&ring, reinterpret_cast<uint8_t *>(buffer), length);
    if (prev_write_available < RPMSG_BUFFER_SIZE
        && ringbuffer_write_available(&ring) >= RPMSG_BUFFER_SIZE) {
      MAILBOX_Notify_Rx_Buf_Free();
    }
  }
  return size - length;
}

size_t VirtIOSerial::write(uint8_t ch)
{
  // Just write single-byte buffer.
  return write(&ch, 1);
}

// Warning: Currently VirtIOSerial implementation is synchronous, blocking
// until all bytes are sent.
size_t VirtIOSerial::write(const uint8_t *buffer, size_t size)
{
  if (VIRT_UART_Transmit(&huart, const_cast<uint8_t *>(buffer), size) == VIRT_UART_ERROR) {
    return 0;
  }
  return size;
}

void VirtIOSerial::flush(void)
{
  // write() is blocked until all bytes are sent. So flush() doesn't need to do
  // anything. See rpmsg_send().
  return;
}

/* USER CODE BEGIN 4 */
void rxCallback(VIRT_UART_HandleTypeDef *huart)
{
  log_info("Msg received on VIRTUAL UART0 channel:  %s \n\r", (char *) huart->pRxBuffPtr);

  /* copy received msg in a variable to sent it back to master processor in main infinite loop*/
  size_t size = min(huart->RxXferSize, ringbuffer_write_available(&ring));
  while (size > 0) {
    size -= ringbuffer_write(&ring, huart->pRxBuffPtr, size);
  }
  if (ringbuffer_write_available(&ring) >= RPMSG_BUFFER_SIZE) {
    MAILBOX_Notify_Rx_Buf_Free();
  }
}

#endif /* VIRTIOCON */
