/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

// #ifdef MM_PAGING
/*
 * System Library
 * Memory Module Library libmem.c 
 */

/*
* Assignment - Operating System
* CSE - HCMUT
* Semester 242
* Group Member: 
*  Nguyễn Phúc Nhân   2312438     Alloc, Free
*  Cao Thành Lộc      2311942     Read - Write
	 Nguyễn Ngọc Ngữ    2312401     Syscall 
	 Phan Đức Nhã       2312410     PutAllTogether, Report 
   Đỗ Quang Long      2311896     Scheduler 
*/

#include "string.h"
#include "mm.h"
#include "syscall.h"
#include "libmem.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

static pthread_mutex_t mmvm_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t log_msg = PTHREAD_MUTEX_INITIALIZER;

/*enlist_vm_freerg_list - add new rg to freerg_list
 *@mm: memory region
 *@rg_elmt: new region
 *
 */
int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct *rg_elmt)
{
  struct vm_rg_struct *rg_node = mm->mmap->vm_freerg_list;

  if (rg_elmt->rg_start >= rg_elmt->rg_end)
    return -1;

  if (rg_node != NULL)
    rg_elmt->rg_next = rg_node;

  /* Enlist the new region */
  mm->mmap->vm_freerg_list = rg_elmt;

  return 0;
}

/*get_symrg_byid - get mem region by region ID
 *@mm: memory region
 *@rgid: region ID act as symbol index of variable
 *
 */
struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
{
  if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return NULL;

  return &mm->symrgtbl[rgid];
}

/*
 *__alloc - allocate a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *@alloc_addr: address of allocated memory region
! Last modified: 14/04/2025 by Nguyen Phuc Nhan
*/
int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr)
{
  struct vm_rg_struct rgnode;

  // * Find free region that can be used to allocate memory
  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
      // * Put the new region into the free region list
      caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
      caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;

      *alloc_addr = rgnode.rg_start;

      pthread_mutex_unlock(&mmvm_lock);
      return 0;
  }
  // * If no free region is found, we need to extend the VM area
  else 
  {
      
      // * Get the current VM area
      struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

      // * Handle the case when cur_vma is NULL
      if (cur_vma == NULL)
      {
          pthread_mutex_unlock(&mmvm_lock);
          return -1;   // Can not find the VM area
      }
      
      // * Extend the VM area
      int inc_ret = inc_vma_limit(caller, vmaid, size);
      if (inc_ret < 0)   // extend failed
      {
          pthread_mutex_unlock(&mmvm_lock);
          return -1; 
      }

      // * Allocate the memory region
      *alloc_addr = cur_vma->sbrk - size;
      caller->mm->symrgtbl[rgid].rg_start = *alloc_addr;
      caller->mm->symrgtbl[rgid].rg_end = cur_vma->sbrk;

      pthread_mutex_unlock(&mmvm_lock);
      return 0;
  }

  return 0;

}

/*
 *__free - remove a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
! Last modified: 14/04/2025 by Nguyen Phuc Nhan
 */
int __free(struct pcb_t *caller, int vmaid, int rgid)
{
  //struct vm_rg_struct rgnode;

  // Dummy initialization for avoding compiler dummay warning
  // in incompleted TODO code rgnode will overwrite through implementing
  // the manipulation of rgid later

  // * Check if the caller and rgid are valid
  if(rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ || caller == NULL)
    return -1;

  // * Get the allocated region from the symbol table
  struct vm_rg_struct allocated_region = caller->mm->symrgtbl[rgid];
    
  // * Check if the allocated region is valid
  if(allocated_region.rg_start >= allocated_region.rg_end)
      return -1; // * Invalid region

  // * Create a new free region node to store the freed region
  struct vm_rg_struct *free_region = malloc(sizeof(struct vm_rg_struct));
  if (free_region == NULL)
      return -1;
  free_region->rg_start = allocated_region.rg_start;
  free_region->rg_end   = allocated_region.rg_end;
  free_region->rg_next  = NULL;

  // * Get the VM area by its ID
  struct vm_area_struct *vma = get_vma_by_num(caller->mm, vmaid);
  if (vma == NULL)
      return -1;

  // * enlist the free region node to the free region list
  enlist_vm_rg_node(&vma->vm_freerg_list, free_region);

  // * reset the entry in the symbol table to mark the region as freed
  caller->mm->symrgtbl[rgid].rg_start = 0;
  caller->mm->symrgtbl[rgid].rg_end = 0;
  caller->mm->symrgtbl[rgid].rg_next = NULL;

  return 0;
}

/*
 *liballoc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
! Last modified: 14/04/2025 by Nguyen Phuc Nhan
 */
int liballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  // * Check if the process and region index are valid
  if (!proc || reg_index >= PAGING_MAX_SYMTBL_SZ)
    return -1;

  int addr;

  // * Call the __alloc function to allocate memory
  if (__alloc(proc, 0, reg_index, size, &addr) < 0)
      return -1; // * Failed to allocate memory

  // * Dump the physical memory after allocation
  pthread_mutex_lock(&log_msg);
  printf("===== PHYSICAL MEMORY AFTER ALLOCATION =====\n");
  printf("PID=%d - Region=%d - Address=%08x - Size=%d byte\n", proc->pid, reg_index, addr, size);
  print_pgtbl(proc, 0, -1); 
  printf("================================================================\n");
  pthread_mutex_unlock(&log_msg);

  // * Return the allocated address
  return addr;
}

/*
 *libfree - PAGING-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
! Last modified: 14/04/2025 by Nguyen Phuc Nhan
 */

int libfree(struct pcb_t *proc, uint32_t reg_index)
{
  // * Check if the process and region index are valid
  if (!proc || reg_index >= PAGING_MAX_SYMTBL_SZ)
    return -1;

  // * Call the __free function to free memory
  int val = __free(proc, 0, reg_index);

  // * Dump the physical memory after deallocation
  if (val == 0) {
    pthread_mutex_lock(&log_msg);
    printf("===== PHYSICAL MEMORY AFTER DEALLOCATION =====\n");
    printf("PID=%d - Region=%d\n", proc->pid, reg_index);
    print_pgtbl(proc, 0, -1);
    printf("================================================================\n");
    pthread_mutex_unlock(&log_msg);
  }

  // * Return the result of deallocation */
  return val;
}

/*
 *pg_getpage - get the page in ram
 *@mm: memory region
 *@pagenum: PGN
 *@framenum: return FPN
 *@caller: caller
! Last modified: 14/04/2025 by Cao Thanh Loc
 */
int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
{
  uint32_t pte = mm->pgd[pgn];

  if (!PAGING_PAGE_PRESENT(pte))
  { 
    int new_fpn;
        
    // * Get a free frame page number (FPN) from the memory physical
    if (MEMPHY_get_freefp(caller->mram, &new_fpn) != 0)
    {
        // * Find a victim page to swap out
        int victim_pgn;
        if (find_victim_page(mm, &victim_pgn) != 0)
        {
            return -1;  // Không tìm thấy trang nạn nhân
        }
        // * Swap out the victim page
        new_fpn = PAGING_FPN(mm->pgd[victim_pgn]);
    }
    
    // * Swap the page from MEMSWAP to MEMRAM
    pte_set_fpn(&mm->pgd[pgn], new_fpn);

    // * Update the page table entry
    enlist_pgn_node(&caller->mm->fifo_pgn, pgn);
  }

  *fpn = PAGING_FPN(mm->pgd[pgn]);

  return 0;
}

/*
 *pg_getval - read value at given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
! Last modified: 14/04/2025 by Cao Thanh Loc
 */
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  // * Get the page to MEMRAM, swap from MEMSWAP if needed
  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
    return -1; /* invalid page access */

  int phyaddr = fpn * PAGING_PAGESZ + off;
  int ret = MEMPHY_read(caller->mram, phyaddr, data);

  return ret;
}

/*
 *pg_setval - write value to given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
! Last modified: 14/04/2025 by Cao Thanh Loc
 */
int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  // * Get the page to MEMRAM, swap from MEMSWAP if needed
  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
    return -1; /* invalid page access */

  int phyaddr = fpn * PAGING_PAGESZ + off;

  int ret = MEMPHY_write(caller->mram, phyaddr, value);

  return ret;
}

/*
 *__read - read value in region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
  *@size: allocated size
! Last modified: 14/04/2025 by Cao Thanh Loc
 */
int __read(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE *data)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
    return -1;

  return pg_getval(caller->mm, currg->rg_start + offset, data, caller);
}

/*libread - PAGING-based read a region memory */
int libread(
    struct pcb_t *proc, // Process executing the instruction
    uint32_t source,    // Index of source register
    uint32_t offset,    // Source address = [source] + [offset]
    uint32_t* destination)
{
  BYTE data;
  int val = __read(proc, 0, source, offset, &data);

  /* 
   * Update result of reading action:
   * Assign the value read from memory to the destination register
   */
  if (val == 0) {
    *destination = (uint32_t)data;
}

  // * Dump the physical memory after reading
  pthread_mutex_lock(&log_msg);
  printf("===== PHYSICAL MEMORY AFTER READING =====\n");
  printf("destination=%d\n", *destination);
#ifdef IODUMP
  printf("read region=%d offset=%d value=%d\n", source, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1);
#endif
  MEMPHY_dump(proc->mram);
#endif
  printf("================================================================\n");
  pthread_mutex_unlock(&log_msg);

  return val;
}

/*__write - write a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __write(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE value)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
    return -1;

  pg_setval(caller->mm, currg->rg_start + offset, value, caller);

  return 0;
}

/*libwrite - PAGING-based write a region memory */
int libwrite(
    struct pcb_t *proc,   // Process executing the instruction
    BYTE data,            // Data to be wrttien into memory
    uint32_t destination, // Index of destination register
    uint32_t offset)
{
  int val = __write(proc, 0, destination, offset, data);
  pthread_mutex_lock(&log_msg);
  printf("===== PHYSICAL MEMORY AFTER WRITING =====\n");
#ifdef IODUMP
  printf("write region=%d offset=%d value=%d\n", destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1);
#endif
  MEMPHY_dump(proc->mram);
#endif
  printf("================================================================\n");
  pthread_mutex_unlock(&log_msg);

  return val;
}

/*free_pcb_memphy - collect all memphy of pcb
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 */
int free_pcb_memph(struct pcb_t *caller)
{
  int pagenum, fpn;
  uint32_t pte;


  for(pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++)
  {
    pte= caller->mm->pgd[pagenum];

    if (!PAGING_PAGE_PRESENT(pte))
    {
      fpn = PAGING_PTE_FPN(pte);
      MEMPHY_put_freefp(caller->mram, fpn);
    } else {
      fpn = PAGING_PTE_SWP(pte);
      MEMPHY_put_freefp(caller->active_mswp, fpn);    
    }
  }

  return 0;
}

/*
 *find_victim_page - find victim page
 *@caller: caller
 *@pgn: return page number
! Last modified: 14/04/2025 by Nguyen Phuc Nhan
*/
int find_victim_page(struct mm_struct *mm, int *retpgn)
{
  struct pgn_t *pg = mm->fifo_pgn;

  // * Check if the FIFO list is empty
  if (pg == NULL)
        return -1;
    
  *retpgn = pg->pgn;  // * Set the return page number
    
  // * Remove the first page from the FIFO list
  mm->fifo_pgn = pg->pg_next;

  // * Free the memory allocated for the removed page
  free(pg);

  return 0;
}

/*
 *get_free_vmrg_area - get a free vm region
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@size: allocated size
! Last modified: 14/04/2025 by Nguyen Phuc Nhan
*/
int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
{
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  if (cur_vma == NULL)
        return -1;

  struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;
  struct vm_rg_struct **pp = &cur_vma->vm_freerg_list;

  // * Initialize newrg
  newrg->rg_start = newrg->rg_end = -1;
  newrg->rg_next = NULL;

  if (rgit == NULL)
    return -1;

  newrg->rg_start = newrg->rg_end = -1;

  // * Traverse the free region list to find a suitable region
  while (*pp != NULL) {
    int region_size = (*pp)->rg_end - (*pp)->rg_start;
    if (region_size >= size) {
        // * Found a free region that is large enough
        newrg->rg_start = (*pp)->rg_start;
        newrg->rg_end = (*pp)->rg_start + size;
        
        // * Update the free region list
        if (region_size == size) {
            struct vm_rg_struct *temp = *pp;
            *pp = (*pp)->rg_next;
            free(temp);
        } else {
            (*pp)->rg_start += size;
        }
        return 0;
    }
    pp = &((*pp)->rg_next);
  }

  // * No suitable free region found
  return -1;
}

//#endif
