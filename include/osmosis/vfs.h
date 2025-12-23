#ifndef OSMOSIS_VFS_H
#define OSMOSIS_VFS_H

#include <stdint.h>

struct vfs_node {
    const char *path;
    const uint8_t *data;
    uint32_t size;
};

void vfs_init(const uint8_t *initramfs, uint32_t size);
const struct vfs_node *vfs_lookup(const char *path);
void vfs_list(void);

int vfs_read(const struct vfs_node *node, uint32_t offset, void *buf, uint32_t len);

#endif
