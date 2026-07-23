#include "osmosis/userland.h"

#include "osmosis/arch/i386/paging.h"
#include "osmosis/kprintf.h"
#include "osmosis/pmm.h"
#include "osmosis/vfs.h"

#include <stddef.h>
#include <stdint.h>

#define USER_ELF_BASE 0x08000000u
#define USER_STACK_TOP 0x08100000u
#define USER_STACK_SIZE (4u * PAGE_SIZE)
#define USER_ELF_LIMIT (USER_STACK_TOP - USER_STACK_SIZE)
#define USER_PID 1u

struct user_program {
    uintptr_t entry;
    uintptr_t lowest;
    uintptr_t highest;
    uintptr_t stack_top;
    uintptr_t stack_base;
};

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

static struct {
    int active;
    int exit_code;
    uintptr_t region_low;
    uintptr_t region_high;
} user_state = {0};

extern void userland_enter(uintptr_t entry, uintptr_t user_stack);
extern void userland_resume_kernel(void) __attribute__((noreturn));

static uintptr_t align_up(uintptr_t value, uintptr_t align) {
    return (value + align - 1u) & ~(align - 1u);
}

static uintptr_t align_down(uintptr_t value, uintptr_t align) {
    return value & ~(align - 1u);
}

static int map_page(uintptr_t virt, uint32_t flags) {
    uintptr_t frame = pmm_alloc_frame();
    if (!frame) {
        kprintf("userland: frame allocation failed for 0x%x\n", (uint32_t)virt);
        return 0;
    }
    if (!paging_map(virt, frame, flags)) {
        kprintf("userland: mapping failed for 0x%x -> 0x%x\n", (uint32_t)virt, (uint32_t)frame);
        pmm_free_frame(frame);
        return 0;
    }
    /* Zero the page to avoid leaking data. */
    uint8_t *dest = (uint8_t *)(virt & ~(PAGE_SIZE - 1u));
    for (uint32_t i = 0; i < PAGE_SIZE; i++) {
        dest[i] = 0;
    }
    return 1;
}

static int map_segment(const struct elf32_phdr *ph, const uint8_t *image, uint32_t image_size, struct user_program *prog) {
    if (ph->p_type != PT_LOAD) {
        return 1;
    }
    if (ph->p_memsz == 0) {
        return 1;
    }
    if (ph->p_vaddr < USER_ELF_BASE) {
        kprintf("userland: segment below user base: 0x%x\n", ph->p_vaddr);
        return 0;
    }
    if (ph->p_memsz < ph->p_filesz || ph->p_offset > image_size ||
        ph->p_filesz > image_size - ph->p_offset) {
        kprintf("userland: segment overruns image (off=0x%x size=0x%x image=0x%x)\n",
                ph->p_offset, ph->p_filesz, image_size);
        return 0;
    }
    if (ph->p_memsz > UINTPTR_MAX - ph->p_vaddr) {
        kprintf("userland: segment address overflows\n");
        return 0;
    }

    uintptr_t seg_start = align_down(ph->p_vaddr, PAGE_SIZE);
    uintptr_t seg_end = align_up(ph->p_vaddr + ph->p_memsz, PAGE_SIZE);
    if (seg_end < ph->p_vaddr || seg_end > USER_ELF_LIMIT) {
        kprintf("userland: segment collides with the user stack\n");
        return 0;
    }
    uint32_t flags = PAGE_USER | ((ph->p_flags & PF_W) ? PAGE_WRITE : 0);

    for (uintptr_t addr = seg_start; addr < seg_end; addr += PAGE_SIZE) {
        if (!map_page(addr, flags)) {
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

static int map_user_stack(struct user_program *prog) {
    uintptr_t base = USER_STACK_TOP - USER_STACK_SIZE;
    for (uintptr_t addr = base; addr < USER_STACK_TOP; addr += PAGE_SIZE) {
        if (!map_page(addr, PAGE_USER | PAGE_WRITE)) {
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

static int load_elf_image(const uint8_t *image, uint32_t size, struct user_program *prog) {
    if (!image || !prog || size < sizeof(struct elf32_ehdr)) {
        kprintf("userland: ELF image is truncated\n");
        return 0;
    }

    const struct elf32_ehdr *ehdr = (const struct elf32_ehdr *)image;
    const uint8_t expected_magic[4] = {0x7F, 'E', 'L', 'F'};
    for (int i = 0; i < 4; i++) {
        if (ehdr->e_ident[i] != expected_magic[i]) {
            kprintf("userland: invalid ELF magic\n");
            return 0;
        }
    }

    if (ehdr->e_ident[4] != 1 || ehdr->e_ident[5] != 1 ||
        ehdr->e_machine != 3 || ehdr->e_phoff == 0 || ehdr->e_phnum == 0 ||
        ehdr->e_phentsize != sizeof(struct elf32_phdr)) {
        kprintf("userland: unsupported ELF header\n");
        return 0;
    }
    if (ehdr->e_phoff > size ||
        ehdr->e_phnum > (size - ehdr->e_phoff) / sizeof(struct elf32_phdr)) {
        kprintf("userland: ELF program headers are truncated\n");
        return 0;
    }

    prog->entry = ehdr->e_entry;
    prog->lowest = (uintptr_t)-1;
    prog->highest = 0;

    const struct elf32_phdr *phdrs = (const struct elf32_phdr *)(image + ehdr->e_phoff);
    for (uint16_t i = 0; i < ehdr->e_phnum; i++) {
        if (!map_segment(&phdrs[i], image, size, prog)) {
            return 0;
        }
    }

    if (prog->lowest == (uintptr_t)-1 || prog->entry < prog->lowest ||
        prog->entry >= prog->highest) {
        kprintf("userland: ELF entry point is outside its loadable segments\n");
        return 0;
    }

    if (!map_user_stack(prog)) {
        kprintf("userland: stack mapping failed\n");
        return 0;
    }

    return 1;
}

int userland_user_range_ok(uintptr_t ptr, uint32_t len) {
    if (!user_state.active) {
        return 0;
    }
    if (len == 0) {
        return 0;
    }
    uintptr_t end = ptr + len - 1;
    if (end < ptr) { /* overflow */
        return 0;
    }
    if (ptr < user_state.region_low || end >= user_state.region_high) {
        return 0;
    }
    return paging_range_has_flags(ptr, len, PAGE_USER);
}

int userland_exit_from_syscall(uint32_t code) {
    if (!user_state.active) {
        kprintf("sys_exit: no active user program (code=%u)\n", code);
        return -22;
    }
    user_state.active = 0;
    user_state.exit_code = (int)code;
    userland_resume_kernel();
}

uint32_t userland_current_pid(void) {
    return USER_PID;
}

int userland_run_demo(void) {
    struct user_program prog;
    const struct vfs_node *program = vfs_lookup("bin/hello_user");
    if (!program) {
        kprintf("userland: bin/hello_user is missing from initramfs\n");
        return -1;
    }

    if (!load_elf_image(program->data, program->size, &prog)) {
        kprintf("userland: failed to load demo ELF\n");
        return -1;
    }

    user_state.region_low = prog.lowest;
    user_state.region_high = prog.highest;
    user_state.exit_code = -1;
    user_state.active = 1;

    kprintf("userland: launching demo @0x%x (stack=0x%x-0x%x)\n",
            (uint32_t)prog.entry, (uint32_t)prog.stack_base, (uint32_t)prog.stack_top);

    userland_enter(prog.entry, prog.stack_top);
    kprintf("userland: demo exited with code %d\n", user_state.exit_code);
    return user_state.exit_code;
}
