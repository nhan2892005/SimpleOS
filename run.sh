echo "Running os_sc"
./os os_sc > m_output/os_sc.output
python3 ganttchart.py m_output os_sc.output
python3 ganttchart.py output os_sc.output
echo "Done"

echo "Running os_syscall"
./os os_syscall > m_output/os_syscall.output
python3 ganttchart.py m_output os_syscall.output
python3 ganttchart.py output os_syscall.output
echo "Done"

echo "Running os_syscall_list"
./os os_syscall_list > m_output/os_syscall_list.output
python3 ganttchart.py m_output os_syscall_list.output
python3 ganttchart.py output os_syscall_list.output
echo "Done"