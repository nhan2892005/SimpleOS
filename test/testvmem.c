/* test-all.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "common.h"
#include "os-mm.h"
#include "mm.h"
#include "libmem.h"

// Định nghĩa mmvm_lock nếu chưa có (để dùng trong liballoc/__alloc)
pthread_mutex_t mmvm_lock = PTHREAD_MUTEX_INITIALIZER;

/* Hàm in kết quả test */
void print_result(const char *test_name, const char *expected, const char *actual, int pass) {
    if (pass)
        printf("Pass test about %s\n", test_name);
    else {
        printf("Fail test about %s\n", test_name);
        printf("  Expected: %s\n", expected);
        printf("  Actual:   %s\n", actual);
    }
}

/* Test 1: Virtual Memory Management - init_mm */
int test_init_mm() {
    struct pcb_t *proc = malloc(sizeof(struct pcb_t));
    if (!proc) { perror("malloc proc"); exit(1); }
    proc->pid = 1;
    proc->bp = PAGE_SIZE;  // Giả sử bp ban đầu là PAGE_SIZE

    proc->mm = malloc(sizeof(struct mm_struct));
    if (!proc->mm) { perror("malloc mm"); exit(1); }

    if (init_mm(proc->mm, proc) != 0) {
        free(proc->mm);
        free(proc);
        return 0;
    }
    struct vm_area_struct *vma0 = get_vma_by_num(proc->mm, 0);
    int pass = (vma0 && vma0->vm_start == 0 && vma0->vm_end == 0 && vma0->sbrk == 0);

    free(proc->mm->pgd);
    free(proc->mm);
    free(proc);
    return pass;
}

/* Test 2: Virtual Memory Management - inc_vma_limit */
int test_inc_vma_limit() {
    struct pcb_t *proc = malloc(sizeof(struct pcb_t));
    if (!proc) { perror("malloc proc"); exit(1); }
    proc->pid = 1;
    proc->bp = PAGE_SIZE;
    proc->mm = malloc(sizeof(struct mm_struct));
    proc->mram = malloc(sizeof(struct memphy_struct));
    if (!proc->mram) {
        perror("malloc caller->mram");
        exit(1);
    }
    if (init_memphy(proc->mram, 500, 1) != 0) {
        printf("init_memphy failed\n");
        exit(1);
    }
    if (!proc->mm) { perror("malloc mm"); exit(1); }
    if (init_mm(proc->mm, proc) != 0) exit(1);
    struct vm_area_struct *vma0 = get_vma_by_num(proc->mm, 0);
    if (!vma0) exit(1);

    // Test inc_vma_limit cho 300 bytes.
    int expected_inc1 = PAGING_PAGE_ALIGNSZ(300); // ví dụ: nếu PAGING_PAGESZ=256, thì 300 căn chỉnh thành 512.
    int ret1 = inc_vma_limit(proc, 0, 300);
    int pass1 = (ret1 == expected_inc1) && (vma0->sbrk == expected_inc1) && (vma0->vm_end == expected_inc1);
    char expected[128], actual[128];
    sprintf(expected, "inc1=%d, sbrk=%d", expected_inc1, expected_inc1);
    sprintf(actual, "inc1=%d, sbrk=%lu", ret1, vma0->sbrk);
    print_result("inc_vma_limit (300 bytes)", expected, actual, pass1);

    // Test inc_vma_limit cho 100 bytes.
    int expected_inc2 = PAGING_PAGE_ALIGNSZ(100); // nếu 100<=256 => 256.
    int ret2 = inc_vma_limit(proc, 0, 100);
    int pass2 = (ret2 == expected_inc2) && (vma0->sbrk == expected_inc1 + expected_inc2) &&
                (vma0->vm_end == expected_inc1 + expected_inc2);
    sprintf(expected, "inc2=%d, sbrk=%d", expected_inc2, expected_inc1+expected_inc2);
    sprintf(actual, "inc2=%d, sbrk=%lu", ret2, vma0->sbrk);
    print_result("inc_vma_limit (100 bytes)", expected, actual, pass2);

    free(proc->mm->pgd);
    free(proc->mm);
    free(proc);
    return (pass1 && pass2);
}

/* Test 3: Virtual Memory Management - liballoc and libfree */
int test_liballoc_libfree() {
    struct pcb_t *proc = malloc(sizeof(struct pcb_t));
    if (!proc) { perror("malloc proc"); exit(1); }
    proc->pid = 1;
    proc->bp = PAGE_SIZE;
    proc->mm = malloc(sizeof(struct mm_struct));
    if (!proc->mm) exit(1);
    if (init_mm(proc->mm, proc) != 0) exit(1);
    
    // Để có free region, gọi inc_vma_limit trước
    int inc = inc_vma_limit(proc, 0, 300);
    if (inc < 0) {
        free(proc->mm->pgd);
        free(proc->mm);
        free(proc);
        return 0;
    }
    
    // Test liballoc cho region id 2
    pthread_mutex_lock(&mmvm_lock);
    int alloc_addr = liballoc(proc, 200, 2);
    // liballoc trả về alloc_addr nếu thành công
    int pass_alloc = (alloc_addr >= 0);
    char expected[64], actual[64];
    sprintf(expected, "alloc_addr >= 0");
    sprintf(actual, "alloc_addr = %d", alloc_addr);
    print_result("liballoc", expected, actual, pass_alloc);

    // Test libfree cho region id 2
    int free_ret = libfree(proc, 2);
    int pass_free = (free_ret == 0);
    sprintf(expected, "free_ret == 0");
    sprintf(actual, "free_ret = %d", free_ret);
    print_result("libfree", expected, actual, pass_free);

    free(proc->mm->pgd);
    free(proc->mm);
    free(proc);
    return (pass_alloc && pass_free);
}

/* Test 4: Physical Memory Management - init_memphy */
int test_init_memphy() {
    int mem_size = 1024;  // 1024 bytes
    struct memphy_struct *mp = malloc(sizeof(struct memphy_struct));
    if (!mp) { perror("malloc mp"); exit(1); }
    if (init_memphy(mp, mem_size, 1) != 0) {
        free(mp);
        return 0;
    }
    int expected_frames = mem_size / PAGING_PAGESZ;
    int count = 0;
    struct framephy_struct *cur = mp->free_fp_list;
    while(cur) { count++; cur = cur->fp_next; }
    int pass = (count == expected_frames);
    char expected[64], actual[64];
    sprintf(expected, "free frames = %d", expected_frames);
    sprintf(actual, "free frames = %d", count);
    print_result("init_memphy", expected, actual, pass);
    free(mp->storage);
    free(mp);
    return pass;
}

/* Test 5: Physical Memory Management - MEMPHY_read/write */
int test_memphy_rw() {
    int mem_size = 1024;
    struct memphy_struct *mp = malloc(sizeof(struct memphy_struct));
    if (!mp) { perror("malloc mp"); exit(1); }
    if (init_memphy(mp, mem_size, 1) != 0) { free(mp); return 0; }
    int addr = 100;
    BYTE write_val = 0x5A;
    if (MEMPHY_write(mp, addr, write_val) != 0) { free(mp->storage); free(mp); return 0; }
    BYTE read_val = 0;
    if (MEMPHY_read(mp, addr, &read_val) != 0) { free(mp->storage); free(mp); return 0; }
    int pass = (read_val == write_val);
    char expected[64], actual_str[64];
    sprintf(expected, "0x%02x", write_val);
    sprintf(actual_str, "0x%02x", read_val);
    print_result("MEMPHY_read/write", expected, actual_str, pass);
    free(mp->storage);
    free(mp);
    return pass;
}

/* Test 6: Physical Memory Management - MEMPHY_get_freefp and MEMPHY_put_freefp */
int test_memphy_get_put() {
    int mem_size = 1024;
    struct memphy_struct *mp = malloc(sizeof(struct memphy_struct));
    if (!mp) { perror("malloc mp"); exit(1); }
    if (init_memphy(mp, mem_size, 1) != 0) { free(mp); return 0; }
    int count_before = 0;
    struct framephy_struct *cur = mp->free_fp_list;
    while(cur) { count_before++; cur = cur->fp_next; }
    
    int free_frame;
    int ret = MEMPHY_get_freefp(mp, &free_frame);
    int pass_get = (ret == 0);
    
    int count_after_get = 0;
    cur = mp->free_fp_list;
    while(cur) { count_after_get++; cur = cur->fp_next; }
    
    ret = MEMPHY_put_freefp(mp, free_frame);
    int pass_put = (ret == 0);
    
    int count_after_put = 0;
    cur = mp->free_fp_list;
    while(cur) { count_after_put++; cur = cur->fp_next; }
    
    int pass = (pass_get && pass_put && (count_after_put == count_before));
    char expected[64], actual[64];
    sprintf(expected, "free frames = %d", count_before);
    sprintf(actual, "after get+put, free frames = %d", count_after_put);
    print_result("MEMPHY_get_freefp/put_freefp", expected, actual, pass);
    
    free(mp->storage);
    free(mp);
    return pass;
}

int main() {
    int pass_all = 1;
    
    if (test_init_mm())
        printf("Pass test about init_mm\n");
    else {
        printf("Fail test about init_mm\n");
        pass_all = 0;
    }
    
    // if (test_inc_vma_limit())
    //     printf("Pass test about inc_vma_limit\n");
    // else {
    //     printf("Fail test about inc_vma_limit\n");
    //     pass_all = 0;
    // }
    
    // if (test_liballoc_libfree())
    //     printf("Pass test about liballoc/libfree\n");
    // else {
    //     printf("Fail test about liballoc/libfree\n");
    //     pass_all = 0;
    // }
    
    if (test_init_memphy())
        printf("Pass test about init_memphy\n");
    else {
        printf("Fail test about init_memphy\n");
        pass_all = 0;
    }
    
    if (test_memphy_rw())
        printf("Pass test about MEMPHY_read/write\n");
    else {
        printf("Fail test about MEMPHY_read/write\n");
        pass_all = 0;
    }
    
    if (test_memphy_get_put())
        printf("Pass test about MEMPHY_get_freefp/put_freefp\n");
    else {
        printf("Fail test about MEMPHY_get_freefp/put_freefp\n");
        pass_all = 0;
    }
    
    if (pass_all)
        printf("\nAll Memory Management tests PASSED.\n");
    else
        printf("\nSome Memory Management tests FAILED.\n");
    
    return 0;
}
