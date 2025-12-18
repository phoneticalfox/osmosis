#include "idt.h"
#include <string.h>

struct idt_entry idt[256];
struct idt_ptr idtp;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
   idt[num].base_low = (base & 0xFFFF);
   idt[num].base_high = (base >> 16) & 0xFFFF;
   idt[num].sel = sel;
   idt[num].always0 = 0;
   idt[num].flags = flags;
}

void idt_init(void) {
   idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
   idtp.base = (uint32_t)&idt;
   
   // Clear the table
   for(int i = 0; i < 256; i++) {
       idt_set_gate(i, 0, 0, 0);
   }

   // Load IDT
   __asm__ __volatile__("lidt %0" : : "m"(idtp));
}

5. Build System