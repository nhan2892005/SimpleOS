**SimpleOS**

## 1. Mô tả dự án
**SimpleOS** mô phỏng cơ chế quản lý tài nguyên và hệ thống call trong một hệ điều hành đơn giản, bao gồm:
- **Scheduler**: đa cấp ưu tiên (Multi-Level Queue).  
- **Memory Management**: Vùng nhớ ảo (vma), phân trang (paging) và quản lý cấp phát/giải phóng bộ nhớ.  
- **System Call**: các cuộc gọi hệ thống cơ bản (list, kill, mem, ...).  
- **Gantt Chart**: sinh biểu đồ Gantt cho job scheduling.

## 2. Thành viên nhóm và phân công
| Họ và tên             | Mã SV    | Nhóm  | Nhiệm vụ chính                                      |
|-----------------------|----------|-------|-----------------------------------------------------|
| Nguyễn Phúc Nhân      | 2312438  | L02   | Implement `alloc`/`free` trong Memory Management    |
| Cao Thành Lộc         | 2311942  | L02   | Implement `read`/`write` trong Memory Management    |
| Nguyễn Ngọc Ngữ       | 2312401  | L02   | Implement phần System Call                          |
| Phan Đức Nhã          | 2312410  | L07   | Viết báo cáo và tóm tắt mã nguồn                    |
| Đỗ Quang Long         | 2311896  | L02   | Implement Scheduler                                  |

## 3. Cấu trúc thư mục
```
SimpleOS/
├── Makefile               # Tập tin cấu hình build
├── README.md              # Tài liệu hướng dẫn (hiện tại)
├── docs/                  # Tài liệu bài tập và yêu cầu
├── include/               # Header files
├── src/                   # Mã nguồn chính
├── test/                  # Test cases cho queue, scheduler, memory
├── input/                 # Các file cấu hình workload
├── output/                # Kết quả sau khi chạy (biểu đồ Gantt)
├── m_output/              # Kết quả chạy (đầu ra thô)
├── obj/                   # Thư mục chứa file .o
├── run.sh                 # Script chạy toàn bộ workloads
└── ganttchart.py          # Script vẽ biểu đồ Gantt
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

