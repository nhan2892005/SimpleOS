import re
import os
import argparse
import matplotlib.pyplot as plt

PAGE_SIZE = 256  # bytes per page/frame

class ProcessState:
    def __init__(self, initial_free):
        # VM areas: region_id -> (start, size)
        self.vmas = {}
        # Free regions list: list of (start, size)
        self.free = initial_free.copy()

    def allocate(self, region, addr, size):
        # Remove allocated segment from free list
        new_free = []
        for (fs, sz) in self.free:
            fe = fs + sz
            # no overlap
            if addr >= fe or addr + size <= fs:
                new_free.append((fs, sz))
            else:
                # left part
                if addr > fs:
                    new_free.append((fs, addr - fs))
                # right part
                if addr + size < fe:
                    new_free.append((addr + size, fe - (addr + size)))
        # self.free = new_free
        # Add VMA
        self.vmas[region] = (addr, size)

        n_fe = addr + size
        n_fe = ((n_fe // PAGE_SIZE) + 1) * PAGE_SIZE if (n_fe % PAGE_SIZE != 0) else n_fe
        new_free.append((addr + size, n_fe - (addr + size)))

        self.free = list(dict.fromkeys(new_free))

    def deallocate(self, region):
        if region not in self.vmas:
            return
        addr, size = self.vmas.pop(region)
        # add back to free and coalesce
        self.free.append((addr, size))
        # coalesce adjacent free
        self.free = sorted(self.free, key=lambda x: x[0])
        merged = []
        for (fs, sz) in self.free:
            if not merged:
                merged.append([fs, sz])
            else:
                prev = merged[-1]
                if fs <= prev[0] + prev[1]:
                    # overlapping or adjacent
                    prev[1] = max(prev[1], fs + sz - prev[0])
                else:
                    merged.append([fs, sz])
        self.free = [(f, s) for f, s in merged]

class MemoryVisualizer:
    def __init__(self):
        self.processes = {}  # pid -> ProcessState
        self.events = []
        self.max_virtual = 0

    def _parse_address(self, addr_str):
        try:
            return int(addr_str, 16)
        except ValueError:
            return int(addr_str)

    def parse_log(self, filepath):
        alloc_re = re.compile(r"PID=(\d+) - Region=(\d+) - Address=([0-9A-Fa-fx]+) - Size=(\d+) byte")
        dealloc_re = re.compile(r"PID=(\d+) - Region=(\d+)")
        write_re   = re.compile(r"write region=(\d+) offset=(\d+) value=(\d+)")
        read_re    = re.compile(r"read region=(\d+) offset=(\d+) value=(\d+)")

        with open(filepath) as f:
            lines = f.readlines()

        # First pass: detect max virtual extent
        for line in lines:
            m = alloc_re.search(line)
            if m:
                _, _, addr, size = m.groups()
                a = self._parse_address(addr)
                sz = int(size)
                self.max_virtual = max(self.max_virtual, a + sz)
        # ensure at least one page free beyond
        self.max_virtual = ((self.max_virtual // PAGE_SIZE) + 1) * PAGE_SIZE if (self.max_virtual % PAGE_SIZE != 0) else self.max_virtual

        # Second pass: build events
        event_id = 0
        for line in lines:
            line = line.strip()
            m = alloc_re.search(line)
            if m:
                pid, region, addr, size = m.groups()
                self.events.append((event_id, 'alloc', dict(pid=int(pid), region=int(region), address=self._parse_address(addr), size=int(size))))
                event_id += 1
                continue
            m = dealloc_re.search(line)
            if m and 'Size' not in line:
                pid, region = m.groups()
                self.events.append((event_id, 'dealloc', dict(pid=int(pid), region=int(region))))
                event_id += 1
                continue
            # skip write/read
        return self.events

    def update_state(self, event):
        etype, data = event[1], event[2]
        pid = data['pid']
        # init process state if new
        n_max = -1
        if etype == 'alloc':
            n_max = data['address'] + data['size']
            n_max = ((n_max // PAGE_SIZE) + 1) * PAGE_SIZE if (n_max % PAGE_SIZE != 0) else n_max
        if pid not in self.processes:
            # initial free from 0 to max_virtual
            self.processes[pid] = ProcessState([(0, self.max_virtual if n_max == -1 else n_max)])
        pstate = self.processes[pid]
        if etype == 'alloc':
            pstate.allocate(data['region'], data['address'], data['size'])
        elif etype == 'dealloc':
            pstate.deallocate(data['region'])

    def plot_state(self, event_id, event, outdir="figures"):
        os.makedirs(outdir, exist_ok=True)
        fig, ax = plt.subplots(figsize=(10, 2))

        pid = event[2]['pid']
        pstate = self.processes[pid]

        segments = []
        # allocated segments
        for region, (addr, size) in pstate.vmas.items():
            segments.append((addr, addr+size, f"P{pid}:R{region}", True))
        # free segments
        for (addr, size) in pstate.free:
            segments.append((addr, addr+size, 'free', False))

        # sort and plot
        segments.sort(key=lambda x: x[0])
        x_min = segments[0][0]
        x_max = segments[-1][1]
        for start, end, label, alloc in segments:
            width = end - start
            if alloc:
                ax.barh(0, width, left=start, height=0.8)
            else:
                ax.barh(0, width, left=start, height=0.8, facecolor='none', edgecolor='black')
            ax.text(start + width/2, 0, label, va='center', ha='center', fontsize=8)

        ax.set_xlim(x_min, x_max)
        ax.set_xticks(range(x_min, x_max, PAGE_SIZE))  # Thêm dòng này
        ax.set_ylim(-1, 1)
        ax.set_yticks([])
        ax.set_xlabel('Virtual Address')
        ax.set_title(f'PID {pid} VMA + free list after event {event_id} ({event[1]})')
        fig.tight_layout()
        fig.savefig(os.path.join(outdir, f'event_{event_id}.png'))
        plt.close(fig)

    def run(self, logfile, outdir):
        events = self.parse_log(logfile)
        for ev in events:
            self.update_state(ev)
            if ev[1] in ('alloc', 'dealloc'):
                self.plot_state(ev[0], ev, outdir)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Visualize VMA and free list in one row')
    parser.add_argument('logfile', help='Path to log file')
    parser.add_argument('--outdir', default='figures', help='Output directory for figures')
    args = parser.parse_args()

    viz = MemoryVisualizer()
    viz.run(args.logfile, args.outdir)