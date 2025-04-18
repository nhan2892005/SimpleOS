<div align="center">

  <div>
    <img src="https://img.shields.io/badge/Language-C-blue.svg?logo=c%2B%2B&style=for-the-badge" alt="C">
  </div>

  <h3 align="center">Assignment Operating System: SimpleOS</h3>
</div>

## 1. 📋 Mô tả dự án
**SimpleOS** mô phỏng cơ chế quản lý tài nguyên và hệ thống call trong một hệ điều hành đơn giản, bao gồm:
- **Scheduler**: đa cấp ưu tiên (Multi-Level Queue).  
- **Memory Management**: Vùng nhớ ảo (vma), phân trang (paging) và quản lý cấp phát/giải phóng bộ nhớ.  
- **System Call**: các cuộc gọi hệ thống cơ bản (list, kill, mem, ...).

## 2. Thành viên nhóm và phân công
| Họ và tên             | Mã SV    | Nhóm  | Nhiệm vụ chính                                      |
|-----------------------|----------|-------|-----------------------------------------------------|
| Nguyễn Phúc Nhân      | 2312438  | L02   | Implement `alloc`/`free` trong Memory Management    |
| Cao Thành Lộc         | 2311942  | L02   | Implement `read`/`write` trong Memory Management    |
| Nguyễn Ngọc Ngữ       | 2312401  | L02   | Implement phần System Call                          |
| Phan Đức Nhã          | 2312410  | L07   | Viết báo cáo và tóm tắt mã nguồn                    |
| Đỗ Quang Long         | 2311896  | L02   | Implement Scheduler                                 |

## 3. Cấu trúc thư mục
```
SimpleOS/
.
├── Makefile
├── README.md
├── docs
│   └── assignment_SystemCall_Hk251.pdf
├── ganttchart.py
├── include
│   ├── bitops.h
│   ├── common.h
│   ├── cpu.h
│   ├── libmem.h
│   ├── loader.h
│   ├── mem.h
│   ├── mm.h
│   ├── os-cfg.h
│   ├── os-mm.h
│   ├── queue.h
│   ├── sched.h
│   ├── syscall.h
│   └── timer.h
├── input
│   ├── os_0_mlq_paging
│   ├── os_1_mlq_paging
│   ├── os_1_mlq_paging_small_1K
│   ├── os_1_mlq_paging_small_4K
│   ├── os_1_singleCPU_mlq
│   ├── os_1_singleCPU_mlq_paging
│   ├── os_sc
│   ├── os_syscall
│   ├── os_syscall_list
│   ├── proc
│   ├── sched
│   ├── sched_0
│   └── sched_1
├── os
├── output
│   ├── chart
│   ├── os_0_mlq_paging.output
│   ├── os_1_mlq_paging.output
│   ├── os_1_mlq_paging_small_1K.output
│   ├── os_1_mlq_paging_small_4K.output
│   ├── os_1_singleCPU_mlq.output
│   ├── os_1_singleCPU_mlq_paging.output
│   ├── os_sc.output
│   ├── os_syscall.output
│   ├── os_syscall_list.output
│   ├── sched.output
│   ├── sched_0.output
│   └── sched_1.output
├── run.sh
├── src
│   ├── cpu.c
│   ├── libmem.c
│   ├── libstd.c
│   ├── loader.c
│   ├── mem.c
│   ├── mm-memphy.c
│   ├── mm-vm.c
│   ├── mm.c
│   ├── os.c
│   ├── paging.c
│   ├── queue.c
│   ├── sched.c
│   ├── sys_killall.c
│   ├── sys_listsyscall.c
│   ├── sys_mem.c
│   ├── syscall.c
│   ├── syscall.tbl
│   ├── syscalltbl.lst
│   ├── syscalltbl.sh
│   └── timer.c
├── test
│   ├── testqueue.c
│   ├── testsched.c
│   └── testvmem.c
└── test.sh
```

## 4. Yêu cầu hệ thống
- **GCC** (hoặc tương đương) để biên dịch C.  
- **Python 3.x** để chạy `ganttchart.py`.  
- Các thư viện Python cơ bản (e.g. `matplotlib`).

## 5. Hướng dẫn biên dịch và chạy
1. Biên dịch dự án:
   ```bash
   make
   ```
2. Chạy chương trình với file cấu hình workload:
   ```bash
   ./os <workload_config_file>
   ```
   Ví dụ:
   ```bash
   ./os sched_0
   ```

## 6. Vẽ biểu đồ Gantt cho job scheduling
1. Chạy và lưu kết quả thô vào `m_output/`:
   ```bash
   ./os <workload_config_file> > m_output/<workload_config_file>.output
   ```
2. Vẽ biểu đồ từ thư mục `m_output`:
   ```bash
   python3 ganttchart.py m_output <workload_config_file>.output
   ```
3. Vẽ biểu đồ từ thư mục `output` (nếu đã chuyển kết quả qua `output`):
   ```bash
   python3 ganttchart.py output <workload_config_file>.output
   ```

**Ví dụ**:
```bash
./os sched_0 > m_output/sched_0.output
python3 ganttchart.py m_output sched_0.output
python3 ganttchart.py output sched_0.output
```

## 7. Chạy toàn bộ workloads tự động
Sau khi `make`, bạn có thể dùng script `run.sh` để chạy tự động cho tất cả file cấu hình trong `input/`:
```bash
make && ./run.sh
```

## 8. Hướng dẫn chạy test
- Chạy toàn bộ test cases:
  ```bash
  make test
  ```
- Chạy riêng từng phần:
  ```bash
  make test_queue && ./test_queue
  make test_sched && ./test_sched
  make test_mem && ./test_memory
  ```


---

