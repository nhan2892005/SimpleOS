#!/bin/bash

# test.sh - Mẫu script test cho dự án
# Cách sử dụng:
#   ./test.sh <option> [suboption]
# Ví dụ:
#   ./test.sh scheduler
#   ./test.sh memo_manage alloc

if [ "$1" = "scheduler" ]; then
    echo "Running scheduler test..."
    # Thêm các lệnh test cho Scheduler vào đây

elif [ "$1" = "memo_manage" ]; then
    if [ "$2" = "alloc" ]; then
        echo "Running MM_ALLOC test..."
        # Thêm các lệnh test cho MM_ALLOC vào đây
    elif [ "$2" = "free" ]; then
        echo "Running MM_FREE test..."
        # Thêm các lệnh test cho MM_FREE vào đây
    elif [ "$2" = "read" ]; then
        echo "Running MM_READ test..."
        # Thêm các lệnh test cho MM_READ vào đây
    elif [ "$2" = "write" ]; then
        echo "Running MM_WRITE test..."
        # Thêm các lệnh test cho MM_WRITE vào đây
    else
        echo "Invalid argument for memo_manage"
        exit 1
    fi

elif [ "$1" = "syscall" ]; then
    echo "Running syscall test..."
    # Thêm các lệnh test cho syscall vào đây

else
    echo "Invalid test option"
    exit 1
fi

# Nếu tất cả các test pass, exit với trạng thái 0
exit 0
