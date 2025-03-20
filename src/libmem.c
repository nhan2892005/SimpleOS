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

/*__alloc - allocate a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *@alloc_addr: address of allocated memory region
 *
 */
int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr)
{
  struct vm_rg_struct rgnode;

  /* (Nếu cần) Commit vmaid vào rgnode.
   * Nếu bạn muốn lưu thông tin vmaid trong vùng, bạn cần thêm trường vmaid vào struct vm_rg_struct.
   * Ở đây, chúng ta giả sử không có trường đó nên bỏ qua.
   */

  /* Cố gắng tìm một vùng free đủ lớn trong VM area */
  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
      caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
      caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;

      *alloc_addr = rgnode.rg_start;

      pthread_mutex_unlock(&mmvm_lock);
      return 0;
  }
  else
  {
      /* Không tìm thấy free region đủ lớn.
       * Do đó, ta cần mở rộng VM area bằng cách tăng sbrk (giới hạn sử dụng vùng ảo).
       */
      struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
      if (cur_vma == NULL)
      {
          pthread_mutex_unlock(&mmvm_lock);
          return -1;  // Không tìm thấy VM area
      }
      
      //int old_sbrk = cur_vma->sbrk;  // Lưu lại điểm sbrk hiện tại
      /* Tính kích thước tăng cần thiết, căn chỉnh theo kích thước trang */
      // inc_amt sẽ bằng số byte cần tăng sau khi căn chỉnh (ví dụ: nếu size = 300 và PAGING_PAGESZ = 256,
      // thì inc_amt = 512)
      //int inc_amt = PAGING_PAGE_ALIGNSZ(size);
      
      /* Gọi hàm inc_vma_limit để mở rộng VM area.
       * Hàm này sẽ tự cập nhật các trường sbrk và vm_end, cũng như ánh xạ thêm các trang cần thiết.
       */
      int inc_ret = inc_vma_limit(caller, vmaid, size);
      if (inc_ret < 0)
      {
          pthread_mutex_unlock(&mmvm_lock);
          return -1; // Không thể mở rộng VM area
      }

      /* Sau khi mở rộng, vùng mới cấp phát được xem là [old_sbrk, cur_vma->sbrk)
       * (Lưu ý: cur_vma->sbrk đã được cập nhật trong inc_vma_limit)
       */
      *alloc_addr = cur_vma->sbrk - size;
      caller->mm->symrgtbl[rgid].rg_start = *alloc_addr;
      caller->mm->symrgtbl[rgid].rg_end = cur_vma->sbrk;

      pthread_mutex_unlock(&mmvm_lock);
      return 0;
  }

  /* TODO get_free_vmrg_area FAILED handle the region management (Fig.6)*/

  /* TODO retrive current vma if needed, current comment out due to compiler redundant warning*/
  /*Attempt to increate limit to get space */
  //struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);


  //int inc_sz = PAGING_PAGE_ALIGNSZ(size);
  //int inc_limit_ret;

  /* TODO retrive old_sbrk if needed, current comment out due to compiler redundant warning*/
  //int old_sbrk = cur_vma->sbrk;

  /* TODO INCREASE THE LIMIT as inovking systemcall 
   * sys_memap with SYSMEM_INC_OP 
   */
  //struct sc_regs regs;
  //regs.a1 = ...
  //regs.a2 = ...
  //regs.a3 = ...
  
  /* SYSCALL 17 sys_memmap */

  /* TODO: commit the limit increment */

  /* TODO: commit the allocation address 
  // *alloc_addr = ...
  */

  return 0;

}

/*__free - remove a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __free(struct pcb_t *caller, int vmaid, int rgid)
{
  //struct vm_rg_struct rgnode;

  // Dummy initialization for avoding compiler dummay warning
  // in incompleted TODO code rgnode will overwrite through implementing
  // the manipulation of rgid later

  if(rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return -1;

  /* TODO: Manage the collect freed region to freerg_list */
  // Lấy vùng bộ nhớ đã cấp phát từ symbol table
  struct vm_rg_struct allocated_region = caller->mm->symrgtbl[rgid];
    
  // Kiểm tra vùng đã cấp phát có hợp lệ (rg_end phải lớn hơn rg_start)
  if(allocated_region.rg_start >= allocated_region.rg_end)
      return -1; // Vùng đã bị hủy hoặc không hợp lệ

  // Tạo một node mới để đưa vào danh sách free region của VM area
  struct vm_rg_struct *free_region = malloc(sizeof(struct vm_rg_struct));
  if (free_region == NULL)
      return -1;
  free_region->rg_start = allocated_region.rg_start;
  free_region->rg_end   = allocated_region.rg_end;
  free_region->rg_next  = NULL;

  // Lấy VM area tương ứng theo vmaid
  struct vm_area_struct *vma = get_vma_by_num(caller->mm, vmaid);
  if (vma == NULL)
      return -1;

  // Thêm vùng free vào danh sách vm_freerg_list của VM area bằng cách sử dụng hàm enlist_vm_rg_node
  enlist_vm_rg_node(&vma->vm_freerg_list, free_region);

  // Reset lại entry trong symbol table để đánh dấu vùng đã được giải phóng
  caller->mm->symrgtbl[rgid].rg_start = 0;
  caller->mm->symrgtbl[rgid].rg_end = 0;
  caller->mm->symrgtbl[rgid].rg_next = NULL;

  return 0;

  /*enlist the obsoleted memory region */
  //enlist_vm_freerg_list();
}

/*liballoc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int liballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  /* Kiểm tra tiến trình hợp lệ */
  if (!proc || reg_index >= PAGING_MAX_SYMTBL_SZ)
    return -1;

  /* TODO Implement allocation on vm area 0 */
  int addr;

  /* Gọi hàm __alloc để cấp phát vùng nhớ trong VM area 0 */
  if (__alloc(proc, 0, reg_index, size, &addr) < 0)
      return -1; // Cấp phát thất bại

  pthread_mutex_lock(&log_msg);
  printf("===== PHYSICAL MEMORY AFTER ALLOCATION =====\n");
  printf("PID=%d - Region=%d - Address=%08x - Size=%d byte\n", proc->pid, reg_index, addr, size);
  print_pgtbl(proc, 0, -1); // In bảng trang sau khi cấp phát
  printf("================================================================\n");
  pthread_mutex_unlock(&log_msg);
  /* By default using vmaid = 0 */
  return addr;
}

/*libfree - PAGING-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */

int libfree(struct pcb_t *proc, uint32_t reg_index)
{
  /* TODO Implement free region */
  int val = __free(proc, 0, reg_index);
  if (val == 0) {
    pthread_mutex_lock(&log_msg);
    printf("===== PHYSICAL MEMORY AFTER DEALLOCATION =====\n");
    printf("PID=%d - Region=%d\n", proc->pid, reg_index);
    print_pgtbl(proc, 0, -1);
    printf("================================================================\n");
    pthread_mutex_unlock(&log_msg);
  }
  /* By default using vmaid = 0 */
  return val;
}

/*pg_getpage - get the page in ram
 *@mm: memory region
 *@pagenum: PGN
 *@framenum: return FPN
 *@caller: caller
 *
 */
int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
{
  uint32_t pte = mm->pgd[pgn];

  if (!PAGING_PAGE_PRESENT(pte))
  { /* Page is not online, make it actively living */
    /*int vicpgn, swpfpn*/; 
    // Page fault: trang chưa được nạp vào RAM.
    int new_fpn;
        
    // Cố gắng lấy một khung trang trống từ MEMRAM
    if (MEMPHY_get_freefp(caller->mram, &new_fpn) != 0)
    {
        // Nếu không còn khung trang trống, chọn một trang nạn nhân từ danh sách FIFO
        int victim_pgn;
        if (find_victim_page(mm, &victim_pgn) != 0)
        {
            return -1;  // Không tìm thấy trang nạn nhân
        }
        // Giả sử victim page được dùng để giải phóng khung trang
        new_fpn = PAGING_FPN(mm->pgd[victim_pgn]);
        // (Ở đây ta có thể giả lập swap-out của victim và swap-in của trang cần truy cập)
    }
    
    // Cập nhật bảng trang: đánh dấu trang có mặt và gán frame mới
    pte_set_fpn(&mm->pgd[pgn], new_fpn);

    // Theo dõi cho mục đích thay thế trang: thêm trang vừa được nạp vào danh sách FIFO
    enlist_pgn_node(&caller->mm->fifo_pgn, pgn);
    //int vicfpn;
    //uint32_t vicpte;

    //int tgtfpn = PAGING_PTE_SWP(pte);//the target frame storing our variable

    /* TODO: Play with your paging theory here */
    /* Find victim page */
    //find_victim_page(caller->mm, &vicpgn);

    /* Get free frame in MEMSWP */
    //MEMPHY_get_freefp(caller->active_mswp, &swpfpn);

    /* TODO: Implement swap frame from MEMRAM to MEMSWP and vice versa*/

    /* TODO copy victim frame to swap 
     * SWP(vicfpn <--> swpfpn)
     * SYSCALL 17 sys_memmap 
     * with operation SYSMEM_SWP_OP
     */
    //struct sc_regs regs;
    //regs.a1 =...
    //regs.a2 =...
    //regs.a3 =..

    /* SYSCALL 17 sys_memmap */

    /* TODO copy target frame form swap to mem 
     * SWP(tgtfpn <--> vicfpn)
     * SYSCALL 17 sys_memmap
     * with operation SYSMEM_SWP_OP
     */
    /* TODO copy target frame form swap to mem 
    //regs.a1 =...
    //regs.a2 =...
    //regs.a3 =..
    */

    /* SYSCALL 17 sys_memmap */

    /* Update page table */
    //pte_set_swap() 
    //mm->pgd;

    /* Update its online status of the target page */
    //pte_set_fpn() &
    //mm->pgd[pgn];
    //pte_set_fpn();

    //enlist_pgn_node(&caller->mm->fifo_pgn,pgn);
  }

  *fpn = PAGING_FPN(mm->pgd[pgn]);

  return 0;
}

/*pg_getval - read value at given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
    return -1; /* invalid page access */

  /* TODO 
   *  MEMPHY_read(caller->mram, phyaddr, data);
   *  MEMPHY READ 
   *  SYSCALL 17 sys_memmap with SYSMEM_IO_READ
   */
  // int phyaddr
  //struct sc_regs regs;
  //regs.a1 = ...
  //regs.a2 = ...
  //regs.a3 = ...

  /* SYSCALL 17 sys_memmap */

  // Update data
  // data = (BYTE)
  int phyaddr = fpn * PAGING_PAGESZ + off;
  int ret = MEMPHY_read(caller->mram, phyaddr, data);
  // if (ret == 0) {
  //   pthread_mutex_lock(&log_msg);
  //   printf("===== PHYSICAL MEMORY AFTER READING =====\n");
  //   printf("read region=%d offset=%d value=%d\n", PAGING_PGN(addr), PAGING_OFFST(addr), *data);
  //   print_pgtbl(caller, 0, -1);
  //   printf("================================================================\n");
  //   pthread_mutex_unlock(&log_msg);
  // }
  return ret;
}

/*pg_setval - write value to given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
    return -1; /* invalid page access */

  /* TODO
   *  MEMPHY_write(caller->mram, phyaddr, value);
   *  MEMPHY WRITE
   *  SYSCALL 17 sys_memmap with SYSMEM_IO_WRITE
   */
  // int phyaddr
  //struct sc_regs regs;
  //regs.a1 = ...
  //regs.a2 = ...
  //regs.a3 = ...

  /* SYSCALL 17 sys_memmap */

  // Update data
  // data = (BYTE) 
  int phyaddr = fpn * PAGING_PAGESZ + off;

  int ret = MEMPHY_write(caller->mram, phyaddr, value);
  // if (ret == 0){
  //   pthread_mutex_lock(&log_msg);
  //   printf("===== PHYSICAL MEMORY AFTER WRITING =====\n");
  //   printf("write region=%d offset=%d value=%d\n", PAGING_PGN(addr), PAGING_OFFST(addr), value);
  //   printf("================================================================\n");
  //   pthread_mutex_unlock(&log_msg);
  // }

  return ret;
}

/*__read - read value in region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
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

  /* Update result of reading action:
   * Gán giá trị dữ liệu đọc được (data) vào biến destination.
   */
  if (val == 0) {
    *destination = (uint32_t)data;
}

  /* TODO update result of reading action*/
  //destination 
  pthread_mutex_lock(&log_msg);
  printf("===== PHYSICAL MEMORY AFTER READING =====\n");
  printf("destination=%d\n", *destination);
#ifdef IODUMP
  printf("read region=%d offset=%d value=%d\n", source, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
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
  //pthread_mutex_unlock(&log_msg);
#ifdef PAGETBL_DUMP
  //pthread_mutex_lock(&log_msg);
  print_pgtbl(proc, 0, -1); //print max TBL
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


/*find_victim_page - find victim page
 *@caller: caller
 *@pgn: return page number
 *
 */
int find_victim_page(struct mm_struct *mm, int *retpgn)
{
  struct pgn_t *pg = mm->fifo_pgn;

  /* TODO: Implement the theorical mechanism to find the victim page */
  if (pg == NULL)
        return -1;  // Không có trang nào trong danh sách
    
  *retpgn = pg->pgn;  // Chọn trang đầu tiên trong danh sách làm nạn nhân
    
    // Loại bỏ node đầu tiên khỏi danh sách FIFO
  mm->fifo_pgn = pg->pg_next;

  free(pg);

  return 0;
}

/*get_free_vmrg_area - get a free vm region
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@size: allocated size
 *
 */
int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
{
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  if (cur_vma == NULL)
        return -1;

  struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;
  struct vm_rg_struct **pp = &cur_vma->vm_freerg_list;
  // Khởi tạo newrg chưa được thiết lập
  newrg->rg_start = newrg->rg_end = -1;
  newrg->rg_next = NULL;

  if (rgit == NULL)
    return -1;

  /* Probe unintialized newrg */
  newrg->rg_start = newrg->rg_end = -1;

  /* TODO Traverse on list of free vm region to find a fit space */
  //while (...)
  // ..
// Duyệt danh sách free region để tìm một vùng đủ lớn
  while (*pp != NULL) {
    int region_size = (*pp)->rg_end - (*pp)->rg_start;
    if (region_size >= size) {
        // Cấp phát phần đầu của free region cho newrg
        newrg->rg_start = (*pp)->rg_start;
        newrg->rg_end = (*pp)->rg_start + size;
        
        // Cập nhật free region: nếu vừa đủ, loại bỏ node; nếu lớn hơn, cập nhật rg_start
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

  // Không tìm thấy free region đủ lớn
  return -1;
}

//#endif
