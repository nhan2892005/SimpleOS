#!/bin/bash

# test.sh - Mẫu script test cho dự án
# Cách sử dụng:
#   ./test.sh <option> [suboption]

if [ "$1" = "queue" ]; then
    echo "Running queue test..."
    make test_queue && ./test_queue

elif [ "$1" = "scheduler" ]; then
    echo "Running scheduler test..."
    make test_sched && ./test_sched

elif [ "$1" = "mem" ]; then
    echo "Running memory management test..."
    make test_mem && ./test_memory

elif [ "$1" = "syscall" ]; then
    echo "Running syscall test..."
    # Thêm các lệnh test cho syscall vào đây

else
    echo "Invalid test option"
    exit 1
fi

# Nếu tất cả các test pass, exit với trạng thái 0
exit 0
