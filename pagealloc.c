#include "mmu.h" // optional
#include <stdlib.h> 
#include <string.h>

#define MAX_PAGES (1 << 20) 
#define NIL 0xFFFFFFFF

struct page {
    unsigned int next;
};

struct free_pages {
    unsigned int head;
    struct page pages[MAX_PAGES];
};

struct free_pages page_allocator = {
  .head = NIL,
};


// returns the physical address of free page
unsigned int page_alloc() 
{
  unsigned int page_to_allocate = page_allocator.head;
  if(page_to_allocate != NIL){
    page_allocator.head = page_allocator.pages[page_to_allocate].next;
    return page_to_allocate*PGSIZE;
  }
  return NIL;
}

// free the provided physical address
void page_free(unsigned int pa) 
{
  unsigned int ppn = pa/PGSIZE;
  unsigned int temp = page_allocator.head;
  page_allocator.head = ppn;
  page_allocator.pages[ppn].next = temp;
}

void mmap(pde_t* pgdir, unsigned int va, unsigned int pa, unsigned int flag) 
{
  unsigned int pgdir_index = PDX(va);
  unsigned int pgtab_index = PTX(va);

  if(!(pgdir[pgdir_index] & PTE_P)){
    pgdir[pgdir_index] = (unsigned int)(page_alloc() | flag);
    memset((void *)(pgdir[pgdir_index] & ~0xFFF), 0, PGSIZE);
  }

  pte_t *pt_entry = (pte_t *)(pgdir[pgdir_index] & ~0xFFF);

  if (!(pt_entry[pgtab_index] & PTE_P)) {
    pt_entry[pgtab_index] = pa | flag;
  } 
}
