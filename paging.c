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

/* Allocate eight consecutive disk blocks.
 * Save the content of the physical page in the pte
 * to the disk blocks and save the block-id into the
 * pte.
 */
void
swap_page_from_pte(pte_t *pte)
{
	int baddr = balloc_page(1);
	char dd[4096];
	uint phys_addr = PTE_ADDR(*pte);
	char *aa = (char*) P2V(phys_addr);
	for(int i = 0; i < 4096; i++){
		dd[i] = *aa;
		aa++;
	}
	write_page_to_disk(1, dd, baddr);
	(*pte & PTE_P) = (*pte & PTE_P) & !(*pte & PTE_P);
	(*pte & PTE_SWP) = (*pte & PTE_SWP) | 0Xfff;
	*pte = (baddr << 12) | (*pte & 0xfff);
	kfree(P2V(phys_addr));
}

/* Select a victim and swap the contents to the disk.
 */
int
swap_page(pde_t *pgdir)
{
	panic("swap_page is not implemented");
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
	panic("map_address is not implemented");
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
