#include "osmosis/vfs.h"

#include <stddef.h>
#include <stdint.h>

#include "osmosis/kprintf.h"

#define MAX_VFS_NODES 32
#define VFS_NAME_CAPACITY 64

struct initramfs_entry {
    char name[VFS_NAME_CAPACITY];
    uint32_t size;
};

static struct vfs_node nodes[MAX_VFS_NODES];
static char node_names[MAX_VFS_NODES][VFS_NAME_CAPACITY];
static uint32_t node_count = 0;

static int copy_entry_name(char *destination, const char *source) {
    for (uint32_t i = 0; i < VFS_NAME_CAPACITY; i++) {
        destination[i] = source[i];
        if (source[i] == '\0') {
            return i != 0;
        }
    }
    return 0;
}

void vfs_init(const uint8_t *initramfs, uint32_t size) {
    node_count = 0;
    if (!initramfs || size < sizeof(struct initramfs_entry)) {
        kprintf("vfs: initramfs is missing or too small\n");
        return;
    }

    uint32_t offset = 0;
    while (size - offset >= sizeof(struct initramfs_entry)) {
        const struct initramfs_entry *header =
            (const struct initramfs_entry *)(initramfs + offset);
        offset += sizeof(*header);

        if (header->size == 0 && header->name[0] == '\0') {
            break;
        }
        if (node_count >= MAX_VFS_NODES) {
            kprintf("vfs: initramfs exceeds the %u-file capacity\n", MAX_VFS_NODES);
            break;
        }
        if (!copy_entry_name(node_names[node_count], header->name)) {
            kprintf("vfs: invalid or unterminated entry name\n");
            break;
        }
        if (header->size > size - offset) {
            kprintf("vfs: entry %s is truncated\n", node_names[node_count]);
            break;
        }

        uint32_t aligned_size = (header->size + 3u) & ~3u;
        if (aligned_size < header->size || aligned_size > size - offset) {
            kprintf("vfs: entry %s has invalid padding\n", node_names[node_count]);
            break;
        }

        nodes[node_count].path = node_names[node_count];
        nodes[node_count].data = initramfs + offset;
        nodes[node_count].size = header->size;
        node_count++;
        offset += aligned_size;
    }

    kprintf("VFS: mounted read-only initramfs with %u file%s.\n",
            node_count, node_count == 1 ? "" : "s");
}

const struct vfs_node *vfs_lookup(const char *path) {
    if (!path) {
        return NULL;
    }
    for (uint32_t i = 0; i < node_count; i++) {
        const char *left = path;
        const char *right = nodes[i].path;
        while (*left && *left == *right) {
            left++;
            right++;
        }
        if (*left == '\0' && *right == '\0') {
            return &nodes[i];
        }
    }
    return NULL;
}

int vfs_read(const struct vfs_node *node, uint32_t offset, void *buffer, uint32_t length) {
    if (!node || !buffer) {
        return -1;
    }
    if (offset >= node->size) {
        return 0;
    }
    if (length > node->size - offset) {
        length = node->size - offset;
    }

    uint8_t *destination = (uint8_t *)buffer;
    const uint8_t *source = node->data + offset;
    for (uint32_t i = 0; i < length; i++) {
        destination[i] = source[i];
    }
    return (int)length;
}

void vfs_list(void) {
    if (node_count == 0) {
        kprintf("(initramfs is empty)\n");
        return;
    }
    for (uint32_t i = 0; i < node_count; i++) {
        kprintf("%s (%u bytes)\n", nodes[i].path, nodes[i].size);
    }
}
