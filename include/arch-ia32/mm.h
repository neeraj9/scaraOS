#ifndef __ARCH_IA32_MM__
#define __ARCH_IA32_MM__

#include <arch/kimage.h>

/* Characteristics of the IA32 architecture */
#define NR_PDE 		1024
#define NR_PTE 		1024
#define PAGE_SHIFT	12
#define PDE_SHIFT	(10 + PAGE_SHIFT)

/* Load the kernel in at 3GB */
#define PAGE_OFFSET	0xc0000000

/* mm_get_page() returns code */
#define MGP_INVALID	0xFFFFFFFF

/* Segment selectors (see GDT in arch-ia32/setup.c) */
#define __KERNEL_CS	0x08
#define __KERNEL_DS	0x10
#define __USER_CS	0x18
#define __USER_DS	0x20

/* C Specific */
#ifndef __ASM__

typedef uint32_t *pgd_t;
typedef uint32_t *pgt_t;

#define load_pdbr(pgdir) \
	asm volatile("movl %0,%%cr3": :"r" (pgdir));

#define __flush_tlb()							\
	do {								\
		unsigned int tmpreg;					\
									\
		__asm__ __volatile__(					\
			"movl %%cr3, %0;  # flush TLB \n"		\
			"movl %0, %%cr3;              \n"		\
			: "=r" (tmpreg)					\
			:: "memory");					\
	} while (0)

#define __pa(x)		((unsigned long)(x)-PAGE_OFFSET)
#define __va(x)		((void *)((unsigned long)(x)+PAGE_OFFSET))

/* Initial page directory and table at 3MB phys,
* this gives us 2MB-4KB for kernel code etc.. */
extern uint32_t _init_pgd, _init_pgt;

/* Macros to mask top 10, middle 10, and bottom 12 bits of
* an address respectively */
#define __dir(x)		(((unsigned long)(x))&0xFFC00000)
#define __tbl(x)		(((unsigned long)(x))&0x003FF000)
#define __ofs(x)		(((unsigned long)(x))&0x00000FFF)

/* Get the actual indexes */
#define dir(x)			(__dir(x)>>22)
#define tbl(x)			(__tbl(x)>>12)
#define ofs(x)			(__ofs(x))

/* Generate a virtual address */
#define __mkvirt(d,t,o)		(__dir(d<<22) | __tbl(t<<12) | __ofs(o))

/* Macros to seperate value and flags from pdes and ptes */
#define __val(x)		(x&0xFFFFF000)
#define __flags(x)		(x&0x00000FFF)

/* Page directory entry flags */
#define PDE_PRESENT	(1<<0)
#define PDE_RW		(1<<1)
#define PDE_USER	(1<<2)
#define PDE_WRITE_THRU	(1<<3)
#define PDE_NO_CACHE	(1<<4)
#define PDE_ACCESSED	(1<<5)
#define PDE_DIRTY	(1<<6)
#define PDE_4MB		(1<<7)
#define PDE_GLOBAL	(1<<8)
#define PDE_RES1	(1<<9)
#define PDE_RES2	(1<<10)
#define PDE_RES3	(1<<11)

/* Page table entry flags */
#define PTE_PRESENT	(1<<0)
#define PTE_RW		(1<<1)
#define PTE_USER	(1<<2)
#define PTE_WRITE_THRU	(1<<3)
#define PTE_NO_CACHE	(1<<4)
#define PTE_ACCESSED	(1<<5)
#define PTE_DIRTY	(1<<6)
#define PTE_PTAI	(1<<7)
#define PTE_GLOBAL	(1<<8)
#define PTE_RES1	(1<<9)
#define PTE_RES2	(1<<10)
#define PTE_RES3	(1<<11)

/* #PF handler */
void pagefault(uint32_t, uint32_t);

void mm_init(void);

#endif /* __ASM__ */
#endif /* __ARCH_IA32_MM__ */
