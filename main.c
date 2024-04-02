#include "console.h"
#include "mmu.h"


struct segdesc gdt[NSEGS] = {
    [SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0),
    [SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0),
};

struct gdtdesc gdtdesc = { .limit = sizeof(gdt) - 1, .base =(uint) &gdt[0] };


// Entry 0 of the page table maps to physical page 0, entry 1 to
// physical page 1, etc.
__attribute__((__aligned__(PGSIZE)))
pte_t entry_pgtable1[NPTENTRIES];

__attribute__((__aligned__(PGSIZE)))
pte_t entry_pgtable2[NPTENTRIES];

__attribute__((__aligned__(PGSIZE)))
pde_t entry_pgdir[NPDENTRIES] = {
    // Map VA's [0, 4MB) to PA's [0, 4MB)
    [0] = ((uint)entry_pgtable1) + PTE_P + PTE_W,
    // Map VA's [4MB, 8MB) to PA's [4MB, 8MB)
    [1] = ((uint)entry_pgtable2) + PTE_P + PTE_W,
};

static inline void halt(void)
{
    asm volatile("hlt" : : );
}

static inline void
write_gdt(struct segdesc *p, int size)
{
    volatile ushort pd[3];

    pd[0] = size-1;
    pd[1] = (uint)p;
    pd[2] = (uint)p >> 16;

    asm volatile("lgdt (%0)" : : "r" (pd));
}

static inline uint
read_cr0(void)
{
    uint val;
    asm volatile("movl %%cr0,%0" : "=r" (val));
    return val;
}

static inline void
write_cr0(uint val)
{
    asm volatile("movl %0,%%cr0" : : "r" (val));
}

static inline void
write_cr3(uint val)
{
    asm volatile("movl %0,%%cr3" : : "r" (val));
}


void load_gdt(void *gdtdesc) {
    asm("lgdt (%0)" : : "r" (gdtdesc));
}

void test();

int main(void)
{
    int i; 
    int sum = 0;
    
    
    // Initialize the page table here

    // mapping first 4MB of VA to PA
    for (int i = 0; i < NPTENTRIES; i++) { 
        entry_pgtable1[i] = (i * PGSIZE) | PTE_P | PTE_W;
    }

    // mapping first 4MB - 8MB of VA to PA
    for (int i = 0; i < NPTENTRIES; i++) {
        entry_pgtable2[i] = ((i + 1024) * PGSIZE)  | PTE_P | PTE_W;
    }

    write_cr3((uint)&entry_pgdir);

    int cr0 = read_cr0();
    cr0 |= CR0_PG;
    write_cr0(cr0);

    // Initialize the console
    uartinit(); 

    printk("Hello from C\n");

    // This test code touches 32 pages in the range 0 to 8MB
    for (i = 0; i < 64; i++) {
        int *p = (int *)(i * 4096 * 32);
        sum += *p; 
                
        printk("page\n"); 
    }
    test();
    halt(); 
    return sum; 
}


