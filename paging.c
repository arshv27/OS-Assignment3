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
	*pte = *pte & !PTE_P ;
	// (*pte & PTE_P) = (*pte & PTE_P) & !(*pte & PTE_P);
	*pte = *pte | PTE_SWP;
	// (*pte & PTE_SWP) = (*pte & PTE_SWP) | 0Xfff;
	*pte = (baddr << 12) | (*pte & 0xfff);
	lcr3(V2P(myproc()->pgdir));
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
	char* paddr = kalloc();
	if(*paddr == 0x0){
		uint blockaddr = balloc_page(1);
	}else{
		pte_t *pte; 
		pte = walkpgdir(pgdir, (void*)addr, 1);
		if(*pte & PTE_SWP){
			read_page_from_disk(dev, *paddr, *pte);
		}
		*pte = V2P(*paddr) | PTE_P;
	}
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
