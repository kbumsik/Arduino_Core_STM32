#ifndef __VIRTIO_CONFIG_H
#define __VIRTIO_CONFIG_H

// Size of buffer of each vring buffers
#ifdef RPMSG_BUFFER_SIZE
#error "RPMSG_BUFFER_SIZE should be aligned with the kernel definition."
#error "Reference: https://elixir.bootlin.com/linux/v5.5.2/source/drivers/rpmsg/virtio_rpmsg_bus.c#L137"
#else
#define RPMSG_BUFFER_SIZE (512)
#endif

// Size of the vqueue message in the buffer
#define RPMSG_VRING_HEADER_SIZE (16)
#define RPMSG_VRING_PAYLOAD_SIZE (RPMSG_BUFFER_SIZE - RPMSG_VRING_HEADER_SIZE)

/**
 * number of rpmsg buffer
 * This means that a single call of rproc_virtio_notified(rvdev.vdev, VRING1_ID)
 * will trigger VirtIOSerial::rxCallback() VRING_NUM_BUFFS times at maximum.
 * A buffer handles VirtIOSerial::rxCallback() requires to be
 * (RPMSG_VRING_PAYLOAD_SIZE * VRING_NUM_BUFFS) at minimum to prevent overflow.
 */
#define VRING_NUM_BUFFS         16
#define RPMSG_VRING_TOTAL_PAYLOAD_SIZE (RPMSG_VRING_PAYLOAD_SIZE * VRING_NUM_BUFFS)

#if defined (VIRTIO_LOG)
/**
 * OpenAMP trace (log) buffer configuration.
 * Users are free to redefine the size if needed.
 */
#ifndef VIRTIO_LOG_BUFFER_SIZE
#define VIRTIO_LOG_BUFFER_SIZE (2048)
#endif

#endif

#endif // __VIRTIO_CONFIG_H
