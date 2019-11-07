#ifndef __VIRTIO_CONFIG_H
#define __VIRTIO_CONFIG_H

#ifdef RPMSG_BUFFER_SIZE
#error "RPMSG_BUFFER_SIZE is already defined"
#else
#define RPMSG_BUFFER_SIZE (512)
#endif

#endif // __VIRTIO_CONFIG_H
