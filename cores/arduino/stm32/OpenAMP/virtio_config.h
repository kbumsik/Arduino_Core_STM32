#ifndef __VIRTIO_CONFIG_H
#define __VIRTIO_CONFIG_H

#ifdef RPMSG_BUFFER_SIZE
#error "RPMSG_BUFFER_SIZE is already defined"
#else
#define RPMSG_BUFFER_SIZE (512)
#endif

// Size of the vqueue message in the buffer
#define RPMSG_VRING_HEADER_SIZE 16

#endif // __VIRTIO_CONFIG_H
