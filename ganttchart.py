import re
import sys
import os
import matplotlib.pyplot as plt

def parse_log(filename):
    """
    Đọc file log và xây dựng lịch chạy cho mỗi CPU.
    Trả về một dict: {cpu_id: {time_slot: process_id hoặc None}}.
    Mỗi khi gặp dòng "Time slot", ta cập nhật current_time và lưu trạng thái của CPU.
    Các sự kiện:
      - "Dispatched process": cập nhật CPU đó đang chạy process nào.
      - "Processed ... has finished" hoặc "stopped": CPU chuyển về trạng thái idle (None).
    """
    # Giả sử hệ thống có 2 CPU (CPU 0 và CPU 1); có thể mở rộng nếu cần
    cpu_state = {0: None, 1: None, 2: None, 3: None}
    cpu_schedule = {0: {}, 1: {}, 2: {}, 3: {}}
    current_time = None

    # Các pattern regex
    time_slot_pattern = re.compile(r"Time slot\s+(\d+)")
    dispatched_pattern = re.compile(r"CPU\s+(\d+):\s+Dispatched process\s+(\d+)")
    processed_pattern = re.compile(r"CPU\s+(\d+):\s+Processed\s+(\d+)\s+has finished")
    stopped_pattern = re.compile(r"CPU\s+(\d+)\s+stopped")

    with open(filename, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            # Nếu là dòng "Time slot"
            ts_match = time_slot_pattern.search(line)
            if ts_match:
                current_time = int(ts_match.group(1))
                # Lưu trạng thái hiện tại của mỗi CPU tại time slot này
                for cpu in cpu_state:
                    cpu_schedule[cpu][current_time] = cpu_state[cpu]
                continue
            # Nếu gặp dòng "Dispatched process"
            d_match = dispatched_pattern.search(line)
            if d_match:
                cpu = int(d_match.group(1))
                proc = int(d_match.group(2))
                cpu_state[cpu] = proc
                if current_time is not None:
                    cpu_schedule[cpu][current_time] = proc
                continue
            # Nếu gặp dòng "Processed ... has finished"
            p_match = processed_pattern.search(line)
            if p_match:
                cpu = int(p_match.group(1))
                cpu_state[cpu] = None
                if current_time is not None:
                    cpu_schedule[cpu][current_time] = None
                continue
            # Nếu gặp dòng "stopped"
            s_match = stopped_pattern.search(line)
            if s_match:
                cpu = int(s_match.group(1))
                cpu_state[cpu] = None
                if current_time is not None:
                    cpu_schedule[cpu][current_time] = None
                continue
    return cpu_schedule

def merge_schedule(schedule):
    """
    Nhận vào lịch chạy của 1 CPU (dict: time_slot -> process id hoặc None)
    và gom các time slot liên tiếp có cùng trạng thái thành các khoảng liên tục.
    Mỗi khoảng có dạng (start, duration, process).
    """
    intervals = []
    sorted_times = sorted(schedule.keys())
    if not sorted_times:
        return intervals
    current_proc = schedule[sorted_times[0]]
    start_time = sorted_times[0]
    for t in sorted_times[1:]:
        if schedule[t] == current_proc:
            continue
        else:
            duration = t - start_time
            intervals.append((start_time, duration, current_proc))
            start_time = t
            current_proc = schedule[t]
    # Thêm khoảng cuối cùng
    t = sorted_times[-1] + 1
    duration = t - start_time
    intervals.append((start_time, duration, current_proc))
    return intervals

def main():
    if len(sys.argv) < 3:
        #print("Usage: python ganttchart.py folder path_to_log_file")
        sys.exit(1)
    
    folder = sys.argv[1]
    filename = sys.argv[2]
    log_filename = folder + '/' + filename
    
    # Phân tích file log
    cpu_schedule = parse_log(log_filename)
    
    # Xác định time slot lớn nhất (để vẽ trục Ox)
    max_time = max(max(s.keys()) for s in cpu_schedule.values()) + 1
    
    # Gom lịch cho mỗi CPU thành các khoảng liên tục
    cpu_intervals = {}
    for cpu, schedule in cpu_schedule.items():
        cpu_intervals[cpu] = merge_schedule(schedule)
    
    #print("CPU Schedules (intervals):")
    for cpu, intervals in cpu_intervals.items():
        #print(f"CPU {cpu}: {intervals}")
        pass
    
    # Định nghĩa màu sắc cho các process; trạng thái idle (None) dùng màu lightgray
    process_colors = {
        1: 'tab:blue',
        2: 'tab:orange',
        3: 'tab:green',
        4: 'tab:red',
        5: 'tab:purple',
        6: 'tab:brown',
        7: 'tab:pink',
        8: 'tab:olive',
        9: 'tab:cyan',
        10: 'tab:gray',
        None: 'lightgray'
    }
    
    fig, ax = plt.subplots(figsize=(12, 6))
    row_height = 8
    row_gap = 10
    yticks = []
    y_labels = []
    
    # Vẽ mỗi CPU trên một lane (ví dụ: CPU 0 ở y=0, CPU 1 ở y=10, ...)
    for cpu, intervals in cpu_intervals.items():
        y = cpu * row_gap
        yticks.append(y + row_height / 2)
        y_labels.append(f"CPU {cpu}")
        for (start, duration, proc) in intervals:
            ax.broken_barh([(start, duration)], (y, row_height),
                           facecolors=process_colors.get(proc, 'tab:gray'),
                           edgecolor='black', linewidth=1)
            # Thêm nhãn bên trong thanh nếu có process (proc khác None)
            if proc is not None:
                ax.text(start + duration/2, y + row_height/2, f"P{proc}",
                        ha='center', va='center', color='white', fontsize=10)
    
    # Thiết lập trục Ox:
    # - Major ticks đặt ở giữa mỗi time slot (x + 0.5)
    # - Minor ticks đặt tại biên của các time slot (0, 1, 2, …)
    ax.set_xticks([x + 0.5 for x in range(max_time)], minor=False)
    ax.set_xticklabels(range(max_time))
    ax.set_xticks(range(max_time + 1), minor=True)
    ax.grid(which='minor', axis='x', linestyle='--', color='gray', alpha=0.5)
    
    ax.set_xlabel("Time Slot")
    ax.set_yticks(yticks)
    ax.set_yticklabels(y_labels)
    ax.set_title("Gantt Chart from Log File")
    ax.set_xlim(0, max_time)
    
    os.makedirs(folder + '/chart', exist_ok=True)
    plt.savefig(folder + '/chart/' + filename + '_ganttchart.png')

if __name__ == '__main__':
    main()
