#include "osmosis/userland.h"

#include "osmosis/arch/i386/paging.h"
#include "osmosis/arch/i386/segments.h"
#include "osmosis/kprintf.h"
#include "osmosis/pmm.h"

#include <stddef.h>
#include <stdint.h>

#define USER_ELF_BASE 0x4000000u
#define USER_STACK_TOP 0x4100000u
#define USER_STACK_SIZE (16u * PAGE_SIZE)
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
    uint32_t return_eip;
    uint32_t return_esp;
    int active;
    int exit_code;
    uintptr_t region_low;
    uintptr_t region_high;
} user_state = {0};

extern const uint8_t _binary_build_user_hello_user_elf_start[];
extern const uint8_t _binary_build_user_hello_user_elf_end[];

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
        if (!map_segment(&phdrs[i], image, size, prog)) {
            return 0;
        }
    }

    if (!map_user_stack(prog)) {
        kprintf("userland: stack mapping failed\n");
        return 0;
    }

    return 1;
}

static __attribute__((noreturn)) void enter_user_mode(uintptr_t entry, uintptr_t user_stack) {
    uint32_t data_sel = USER_DATA_SELECTOR | 0x03;
    uint32_t code_sel = USER_CODE_SELECTOR | 0x03;
    __asm__ __volatile__(
        "cli\n"
        "mov %0, %%eax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "pushl %0\n"
        "pushl %1\n"
        "pushfl\n"
        "popl %%eax\n"
        "orl $0x200, %%eax\n"
        "pushl %%eax\n"
        "pushl %2\n"
        "pushl %3\n"
        "iret\n"
        :
        : "r"(data_sel), "r"(user_stack), "r"(code_sel), "r"(entry)
        : "eax");
    __builtin_unreachable();
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
    if (ptr < user_state.region_low || end > user_state.region_high) {
        return 0;
    }
    return paging_range_has_flags(ptr, len, PAGE_USER);
}

void userland_exit_from_syscall(struct isr_frame *frame, uint32_t code) {
    if (!user_state.active) {
        kprintf("sys_exit: no active user program (code=%u)\n", code);
        frame->eax = (uint32_t)(-22);
        return;
    }
    if (!user_state.return_eip || !user_state.return_esp) {
        kprintf("sys_exit: missing return trampoline (code=%u)\n", code);
        frame->eax = (uint32_t)(-22);
        return;
    }

    user_state.active = 0;
    user_state.exit_code = (int)code;

    frame->eip = user_state.return_eip;
    frame->cs = KERNEL_CODE_SELECTOR;
    frame->eflags |= 0x200; /* IF */
    frame->useresp = user_state.return_esp;
    frame->ss = KERNEL_DATA_SELECTOR;
}

uint32_t userland_current_pid(void) {
    return USER_PID;
}

int userland_run_demo(void) {
    struct user_program prog;
    int ok;

    const uint8_t *image = _binary_build_user_hello_user_elf_start;
    uint32_t size = (uint32_t)(uintptr_t)(_binary_build_user_hello_user_elf_end - _binary_build_user_hello_user_elf_start);

    ok = load_elf_image(image, size, &prog);
    if (!ok) {
        kprintf("userland: failed to load demo ELF\n");
        return -1;
    }

    user_state.region_low = prog.lowest;
    user_state.region_high = prog.highest;
    user_state.exit_code = -1;
    user_state.active = 1;

    kprintf("userland: launching demo @0x%x (stack=0x%x-0x%x)\n",
            (uint32_t)prog.entry, (uint32_t)prog.stack_base, (uint32_t)prog.stack_top);

    __asm__ __volatile__("mov %%esp, %0" : "=r"(user_state.return_esp));
    user_state.return_eip = (uint32_t)(uintptr_t)&&user_return;

    enter_user_mode(prog.entry, prog.stack_top);

user_return:
    kprintf("userland: demo exited with code %d\n", user_state.exit_code);
    return user_state.exit_code;
}
