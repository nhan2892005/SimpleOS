echo "Running sched"
./os sched > m_output/sched.output
python3 ganttchart.py m_output sched.output
python3 ganttchart.py output sched.output
echo "Done"

echo "Running sched_0"
./os sched_0 > m_output/sched_0.output
python3 ganttchart.py m_output sched_0.output
python3 ganttchart.py output sched_0.output
echo "Done"

echo "Running sched_1"
./os sched_1 > m_output/sched_1.output
python3 ganttchart.py m_output sched_1.output
python3 ganttchart.py output sched_1.output
echo "Done"