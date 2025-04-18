// #ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Virtual memory module mm/mm-vm.c
 */

#include "string.h"
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/*get_vma_by_num - get vm area by numID
 *@mm: memory region
 *@vmaid: ID vm area to alloc memory region
 *
 */
struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
{
  struct vm_area_struct *pvma = mm->mmap;

  if (mm->mmap == NULL)
    return NULL;

  int vmait = pvma->vm_id;

  while (vmait < vmaid)
  {
    if (pvma == NULL)
      return NULL;

    pvma = pvma->vm_next;
    vmait = pvma->vm_id;
  }

  return pvma;
}

int __mm_swap_page(struct pcb_t *caller, int vicfpn , int swpfpn)
{
    //__swap_cp_page(caller->mram, vicfpn, caller->active_mswp, swpfpn);
    BYTE buf1, buf2;

    // Đọc RAM vào buf1
    MEMPHY_read(caller->mram, vicfpn * PAGING_PAGESZ, &buf1);

    // Đọc SWAP vào buf2
    MEMPHY_read(caller->active_mswp, swpfpn * PAGING_PAGESZ, &buf2);

    // Ghi buf2 -> RAM
    MEMPHY_write(caller->mram, vicfpn * PAGING_PAGESZ, buf2);

    // Ghi buf1 -> SWAP
    MEMPHY_write(caller->active_mswp, swpfpn * PAGING_PAGESZ, buf1);
    return 0;
}

/*get_vm_area_node - get vm area for a number of pages
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, int size, int alignedsz)
{
  struct vm_rg_struct * newrg;
  /* TODO retrive current vma to obtain newrg, current comment out due to compiler redundant warning*/
  //struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  if (cur_vma == NULL)
        return NULL;
  newrg = malloc(sizeof(struct vm_rg_struct));
  if (newrg == NULL)
    return NULL;

  newrg->rg_start = cur_vma->vm_end;
  newrg->rg_end = cur_vma->vm_end + alignedsz;
  newrg->rg_next = NULL;

  return newrg;
}

/*validate_overlap_vm_area
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, int vmastart, int vmaend)
{
  //struct vm_area_struct *vma = caller->mm->mmap;
  struct vm_area_struct *vma = caller->mm->mmap;
  if (vma == NULL)
    return -1;

  /* TODO validate the planned memory area is not overlapped */
  struct vm_rg_struct *cur = vma->vm_freerg_list;
  while(cur != NULL) {
      if (!(vmaend <= cur->rg_start || vmastart >= cur->rg_end))
          return -1; // Giao nhau
      cur = cur->rg_next;
  }
  return 0;
}

/*inc_vma_limit - increase vm area limits to reserve space for new variable
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@inc_sz: increment size
 *
 */
int inc_vma_limit(struct pcb_t *caller, int vmaid, int inc_sz)
{
  // check invalid vmaid
  if (vmaid < 0 || vmaid >= PAGING_MAX_SYMTBL_SZ)
    return -1;
  // check null caller
  if (caller == NULL)
    return -1;
  struct vm_rg_struct * newrg = malloc(sizeof(struct vm_rg_struct));
  int inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);
  int incnumpage =  inc_amt / PAGING_PAGESZ;
  struct vm_rg_struct *area = get_vm_area_node_at_brk(caller, vmaid, inc_sz, inc_amt);
  if (area == NULL)
    return -1;
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  if (cur_vma == NULL)
    return -1;

  int old_end = cur_vma->vm_end;

  /*Validate overlap of obtained region */
  if (validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end) < 0)
    return -1; /*Overlap and failed allocation */

  cur_vma->sbrk = area->rg_start + inc_sz;

  cur_vma->vm_end = area->rg_end;
  int inc_limit_ret = cur_vma->vm_end - old_end;

  struct vm_rg_struct * free_rg_after_used = malloc(sizeof(struct vm_rg_struct));
  free_rg_after_used->rg_start = cur_vma->sbrk;
  free_rg_after_used->rg_end = cur_vma->vm_end;
  free_rg_after_used->rg_next = NULL;

  // link the newrg to the current vma
  if (cur_vma->vm_freerg_list == NULL) {
    cur_vma->vm_freerg_list = free_rg_after_used;
  } else {
    free_rg_after_used->rg_next = cur_vma->vm_freerg_list;
    cur_vma->vm_freerg_list = free_rg_after_used;
  //     struct vm_rg_struct *p = cur_vma->vm_freerg_list;
  //     while (p->rg_next != NULL)
  //         p = p->rg_next;
  //     p->rg_next = free_rg_after_used;
  }

  if (vm_map_ram(caller, area->rg_start, area->rg_end, 
                    old_end, incnumpage , newrg) < 0)
    return -1; /* Map the memory to MEMRAM */

  return inc_limit_ret;
}

// #endif
