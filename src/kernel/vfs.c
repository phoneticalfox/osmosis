#include "osmosis/vfs.h"

#include <stddef.h>
#include <stdint.h>

#include "osmosis/kprintf.h"

#define MAX_VFS_NODES 32
#define NAME_MAX 96

static struct vfs_node nodes[MAX_VFS_NODES];
static uint32_t node_count = 0;

struct initramfs_entry {
    char name[64];
    uint32_t size;
};

void vfs_init(const uint8_t *initramfs, uint32_t size) {
    node_count = 0;
    const uint8_t *cursor = initramfs;
    const uint8_t *end = initramfs + size;
    while (cursor + sizeof(struct initramfs_entry) <= end) {
        const struct initramfs_entry *hdr = (const struct initramfs_entry *)cursor;
        cursor += sizeof(struct initramfs_entry);
        if (hdr->size == 0 || hdr->name[0] == 0) {
            break;
        }
        if (node_count >= MAX_VFS_NODES) {
            kprintf("vfs: initramfs truncated (capacity %u)\n", MAX_VFS_NODES);
            break;
        }
        if (cursor + hdr->size > end) {
            kprintf("vfs: entry %s truncated\n", hdr->name);
            break;
        }
        nodes[node_count].path = (const char *)hdr->name;
        nodes[node_count].data = cursor;
        nodes[node_count].size = hdr->size;
        node_count++;
        uint32_t aligned = (hdr->size + 3u) & ~3u;
        cursor += aligned;
    }
    kprintf("vfs: initramfs mounted with %u files\n", node_count);
}

const struct vfs_node *vfs_lookup(const char *path) {
    if (!path) {
        return NULL;
    }
    for (uint32_t i = 0; i < node_count; i++) {
        const struct vfs_node *n = &nodes[i];
        const char *p = path;
        const char *q = n->path;
        int match = 1;
        while (*p || *q) {
            if (*p != *q) {
                match = 0;
                break;
            }
            p++;
            q++;
        }
        if (match) {
            return n;
        }
    }
    return NULL;
}

int vfs_read(const struct vfs_node *node, uint32_t offset, void *buf, uint32_t len) {
    if (!node || !buf) {
        return -1;
    }
    if (offset >= node->size) {
        return 0;
    }
    if (offset + len > node->size) {
        len = node->size - offset;
    }
    uint8_t *dst = (uint8_t *)buf;
    const uint8_t *src = node->data + offset;
    for (uint32_t i = 0; i < len; i++) {
        dst[i] = src[i];
    }
    return (int)len;
}

void vfs_list(void) {
    for (uint32_t i = 0; i < node_count; i++) {
        const struct vfs_node *n = &nodes[i];
        kprintf("%s (%u bytes)\n", n->path, n->size);
    }
}
