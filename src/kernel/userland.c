#include "osmosis/userland.h"

#include <stddef.h>
#include <stdint.h>

#include "osmosis/arch/i386/paging.h"
#include "osmosis/arch/i386/segments.h"
#include "osmosis/kprintf.h"
#include "osmosis/process.h"
#include "osmosis/pmm.h"
#include "osmosis/vfs.h"

#define USER_ELF_BASE 0x4000000u
#define USER_STACK_TOP 0x4100000u
#define USER_STACK_SIZE (16u * PAGE_SIZE)

struct elf32_ehdr {
    uint8_t e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
};

struct elf32_phdr {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
};

#define PT_LOAD 1
#define PF_W 0x2

extern const uint8_t _binary_build_user_hello_user_elf_start[];
extern const uint8_t _binary_build_user_hello_user_elf_end[];

static uintptr_t align_up(uintptr_t value, uintptr_t align) {
    return (value + align - 1u) & ~(align - 1u);
}

static uintptr_t align_down(uintptr_t value, uintptr_t align) {
    return value & ~(align - 1u);
}

static int map_page(uint32_t *dir, uintptr_t virt, uint32_t flags) {
    uintptr_t frame = pmm_alloc_frame_below(paging_identity_limit_value());
    if (!frame) {
        frame = pmm_alloc_frame();
        if (!frame) {
            kprintf("userland: frame allocation failed for 0x%x\n", (uint32_t)virt);
            return 0;
        }
    }
    if (!paging_map_in(dir, virt, frame, flags)) {
        kprintf("userland: mapping failed for 0x%x -> 0x%x\n", (uint32_t)virt, (uint32_t)frame);
        return 0;
    }
    /* Zero the page to avoid leaking data. */
    uint8_t *dest = (uint8_t *)(virt & ~(PAGE_SIZE - 1u));
    for (uint32_t i = 0; i < PAGE_SIZE; i++) {
        dest[i] = 0;
    }
    return 1;
}

static int map_segment(uint32_t *dir, const struct elf32_phdr *ph, const uint8_t *image, uint32_t image_size, struct process_image *prog) {
    if (ph->p_type != PT_LOAD) {
        return 1;
    }
    if (ph->p_vaddr < USER_ELF_BASE) {
        kprintf("userland: segment below user base: 0x%x\n", ph->p_vaddr);
        return 0;
    }
    if (ph->p_offset + ph->p_filesz > image_size) {
        kprintf("userland: segment overruns image (off=0x%x size=0x%x image=0x%x)\n",
                ph->p_offset, ph->p_filesz, image_size);
        return 0;
    }

    uintptr_t seg_start = align_down(ph->p_vaddr, PAGE_SIZE);
    uintptr_t seg_end = align_up(ph->p_vaddr + ph->p_memsz, PAGE_SIZE);
    uint32_t flags = PAGE_USER | ((ph->p_flags & PF_W) ? PAGE_WRITE : 0);

    for (uintptr_t addr = seg_start; addr < seg_end; addr += PAGE_SIZE) {
        if (!map_page(dir, addr, flags)) {
            return 0;
        }
    }

    uint8_t *dest = (uint8_t *)(uintptr_t)ph->p_vaddr;
    for (uint32_t i = 0; i < ph->p_filesz; i++) {
        dest[i] = image[ph->p_offset + i];
    }
    for (uint32_t i = ph->p_filesz; i < ph->p_memsz; i++) {
        dest[i] = 0;
    }

    if (seg_start < prog->lowest) {
        prog->lowest = seg_start;
    }
    if (seg_end > prog->highest) {
        prog->highest = seg_end;
    }
    return 1;
}

static int map_user_stack(uint32_t *dir, struct process_image *prog) {
    uintptr_t base = USER_STACK_TOP - USER_STACK_SIZE;
    for (uintptr_t addr = base; addr < USER_STACK_TOP; addr += PAGE_SIZE) {
        if (!map_page(dir, addr, PAGE_USER | PAGE_WRITE)) {
            return 0;
        }
    }
    prog->stack_top = USER_STACK_TOP;
    prog->stack_base = base;
    if (base < prog->lowest) {
        prog->lowest = base;
    }
    if (USER_STACK_TOP > prog->highest) {
        prog->highest = USER_STACK_TOP;
    }
    return 1;
}

int userland_load_elf_into(const uint8_t *image, uint32_t size, uint32_t *page_directory, struct process_image *prog) {
    const struct elf32_ehdr *ehdr = (const struct elf32_ehdr *)image;
    const uint8_t expected_magic[4] = {0x7F, 'E', 'L', 'F'};
    for (int i = 0; i < 4; i++) {
        if (ehdr->e_ident[i] != expected_magic[i]) {
            kprintf("userland: invalid ELF magic\n");
            return 0;
        }
    }

    if (ehdr->e_machine != 3 || ehdr->e_phoff == 0 || ehdr->e_phnum == 0) {
        kprintf("userland: unsupported ELF header\n");
        return 0;
    }

    prog->entry = ehdr->e_entry;
    prog->lowest = (uintptr_t)-1;
    prog->highest = 0;

    const struct elf32_phdr *phdrs = (const struct elf32_phdr *)(image + ehdr->e_phoff);
    for (uint16_t i = 0; i < ehdr->e_phnum; i++) {
        if (!map_segment(page_directory, &phdrs[i], image, size, prog)) {
            return 0;
        }
    }

    if (!map_user_stack(page_directory, prog)) {
        kprintf("userland: stack mapping failed\n");
        return 0;
    }

    return 1;
}

int userland_clone_region(uint32_t *src_dir, uint32_t *dst_dir, uintptr_t low, uintptr_t high) {
    for (uintptr_t addr = align_down(low, PAGE_SIZE); addr < high; addr += PAGE_SIZE) {
        uintptr_t src_phys = paging_resolve_in(src_dir, addr);
        if (!src_phys) {
            continue;
        }
        uintptr_t new_phys = pmm_alloc_frame_below(paging_identity_limit_value());
        if (!new_phys) {
            new_phys = pmm_alloc_frame();
            if (!new_phys) {
                return 0;
            }
        }
        if (!paging_map_in(dst_dir, addr, new_phys, PAGE_USER | PAGE_WRITE)) {
            return 0;
        }
        uint8_t *dst = (uint8_t *)new_phys;
        const uint8_t *src = (const uint8_t *)src_phys;
        for (uint32_t i = 0; i < PAGE_SIZE; i++) {
            dst[i] = src[i];
        }
    }
    return 1;
}

int userland_bootstrap_demo(void) {
    const uint8_t *image = _binary_build_user_hello_user_elf_start;
    uint32_t size = (uint32_t)(uintptr_t)(_binary_build_user_hello_user_elf_end - _binary_build_user_hello_user_elf_start);
    return process_spawn_from_image(image, size, "hello_user");
}
