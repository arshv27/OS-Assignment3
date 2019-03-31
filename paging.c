#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"
#include "paging.h"
#include "fs.h"

// Return the address of the PTE in page table pgdir
// that corresponds to virtual address va.  If alloc!=0,
// create any required page table pages.
static pte_t *
walkpgdir(pde_t *pgdir, const void *va, int alloc)
{
  pde_t *pde;
  pte_t *pgtab;

  pde = &pgdir[PDX(va)];
  if(*pde & PTE_P){
    pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
  } else {
    if(!alloc)
      return 0;
  	if ((pgtab = (pte_t*)kalloc()) == 0)
  	{
  		swap_page(pgdir);
  		pgtab = (pte_t*)kalloc();
  	}
    // Make sure all those PTE_P bits are zero.
    memset(pgtab, 0, PGSIZE);
    // The permissions here are overly generous, but they can
    // be further restricted by the permissions in the page table
    // entries, if necessary.
    *pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
  }
  return &pgtab[PTX(va)];
}

/* Allocate eight consecutive disk blocks.
 * Save the content of the physical page in the pte
 * to the disk blocks and save the block-id into the
 * pte.
 */
void
swap_page_from_pte(pte_t *pte)
{
	begin_op();
	uint baddr = balloc_page(1);
	char *aa = (char*) P2V(PTE_ADDR(*pte));
	write_page_to_disk(1, aa, baddr);
	//cprintf("lol\n");
	*pte = (baddr << 12) | PTE_SWP | (*pte & 0xffe);
	lcr3(V2P(myproc()->pgdir));
	kfree(aa);
	end_op();
}

/* Select a victim and swap the contents to the disk.
 */
int
swap_page(pde_t *pgdir)
{
	// begin_op();
	// cprintf("%s\n", "swapping page!");
	pte_t *p = select_a_victim(pgdir);
	//cprintf("%s\n", "swapped page");
	swap_page_from_pte(p);
	// end_op();
	return 1;
}

/* Map a physical page to the virtual address addr.
 * If the page table entry points to a swapped block
 * restore the content of the page from the swapped
 * block and free the swapped block.
 */
void
map_address(pde_t *pgdir, uint addr)
{
	begin_op();
	char* paddr = kalloc();
	if(paddr == 0){
		swap_page(pgdir);
		paddr = kalloc();
	}
	pte_t *p = walkpgdir(pgdir, (char*)addr, 1);
	uint blk = getswappedblk(pgdir, addr);
	if (blk != -1) {
		*p = V2P(paddr) | (*p & 0xfff);
		*p &= (~PTE_SWP);
		*p |= PTE_P;
		read_page_from_disk(1, (char *) paddr, blk);
		bfree_page(1, blk);
	}
	else
		*p = V2P(paddr) | PTE_P | PTE_U | PTE_W;
	end_op();
}

/* page fault handler */
void
handle_pgfault()
{
	unsigned addr;
	struct proc *curproc = myproc();

	asm volatile ("movl %%cr2, %0 \n\t" : "=r" (addr));
	addr &= ~0xfff;
	map_address(curproc->pgdir, addr);
}
