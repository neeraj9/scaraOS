#ifndef __ARCH_IA32_KIMAGE__
#define __ARCH_IA32_KIMAGE__

#ifndef __ASM__

struct pagefault_fixup {
	vaddr_t fault_addr;
	vaddr_t fixup_addr;
};

extern uint8_t __begin;

extern uint8_t __text;
extern uint8_t __text_end;

extern uint8_t __rodata;
extern struct pagefault_fixup __rodata_pagefault;
extern struct pagefault_fixup __rodata_pagefault_end;
extern uint8_t __rodata_end;

extern uint8_t __data;
extern uint8_t __data_end;

extern uint8_t __bss;
extern uint8_t __bss_end;

extern uint8_t __kernel_end;

extern uint8_t __init_start;

extern initcall_t __initcall_start;
extern initcall_t __initcall_end;

extern uint8_t __init_end;

extern uint8_t __end;

extern uint8_t __init_stack_top;
extern uint32_t __init_pgd;
extern uint32_t __init_pgd_pa;
extern uint32_t __init_pgt;
extern uint32_t __init_pgt_pa;
extern uint8_t __bootmem_begin;

#endif /* __ASM__ */

#endif /* __ARCH_IA32_KIMAGE__ */
