#!/bin/bash

# Biến đếm số lần thành công và thất bại
success_count=0
error_count=0

run_command() {
    local name=$1
    echo "Running $name"

    # Chạy lệnh ./os với timeout 5 giây
    timeout 5s ./os "$name" > "m_output/$name.output" 2> "m_output/$name.error"
    if [ $? -eq 124 ]; then
        echo "Reach Runtime Limit (5 seconds)" >> "m_output/$name.error"
        echo -e "\e[31mError: $name -> Reach Runtime Limit (5 seconds)\e[0m"  # Màu đỏ
        ((error_count++))
    else
        # Kiểm tra lỗi của Python script
        python3 ganttchart.py m_output "$name.output" 2> "m_output/$name.gantt_error"
        python3 mem_visual.py /mnt/n/Code_space/SimpleOS/m_output/"$name.output" --outdir figures/"$name"
        if [ $? -ne 0 ]; then
            echo -e "\e[31mPython Error: $name\e[0m"  # Màu đỏ
            cat "m_output/$name.gantt_error"
            ((error_count++))
        else
            ((success_count++))
        fi
    fi

    echo "Done"
}

# Danh sách các lệnh cần chạy
commands=(
    "sched"
    "sched_0"
    "sched_1"
    "os_1_singleCPU_mlq"
    "os_1_singleCPU_mlq_paging"
    "os_0_mlq_paging"
    "os_1_mlq_paging"
    "os_1_mlq_paging_small_1K"
    "os_1_mlq_paging_small_4K"
    "os_sc"
    "os_syscall"
    "os_syscall_list"
)

# Tạo thư mục chứa output nếu chưa có
mkdir -p m_output

# Chạy tất cả các lệnh
for cmd in "${commands[@]}"; do
    run_command "$cmd"
done

# In kết quả tổng kết
echo "====================================="
echo "Total Success: $success_count"
echo -e "\e[31mTotal Errors: $error_count\e[0m" 
echo "====================================="

if [ $error_count -eq 0 ]; then
    echo -e "\e[32mRun Success\e[0m" 
    exit 0
else
    echo -e "\e[31mRun test failed\e[0m" 
    exit 1
fi