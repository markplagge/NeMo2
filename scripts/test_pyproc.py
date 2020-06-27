import pytest
import numpy as np
import PythonProcess




n_procs = 30
p_start = 0
p_end = 1000
time_needed_s = 5
time_needed_e = p_end // n_procs

p_core_list = np.linspace(10, n_procs * 10, 30, dtype=int)
p_start_times = np.linspace(p_start, p_end, n_procs, dtype=int)
p_needed_times = np.linspace(time_needed_s, time_needed_e, n_procs)
pids = np.arange(0, 30)

names = [f"PROC{id}" for id in pids]


def test_proc_pre_wait_ticks():
    pre_wait_t1 = 0
    pre_wait_t2 = 30
    time_needed_t1 = 5
    time_needed_t2 = 7
    process_1 = PythonProcess.Process(scheduled_start_time=pre_wait_t1,time_needed=time_needed_t1)
    process_2 = PythonProcess.Process(scheduled_start_time=pre_wait_t2,time_needed=time_needed_t2)

    p1_should_start_time = 0
    p2_should_start_time = 30
    assert(process_2.state == PythonProcess.PROC_STATE.PRE_WAIT)
    for i in range(50):
        m = process_1.tick()
        m2 = process_2.tick()
        assert (process_1.clock == process_2.clock)


        if i == 0:
            assert(m == PythonProcess.PROC_MESSAGE.SHOULD_START)
            process_1.start_proc()
        elif  i < time_needed_t1:
            assert(m == PythonProcess.PROC_MESSAGE.NO_CHANGE)
        elif  i == time_needed_t1:
            assert(m == PythonProcess.PROC_MESSAGE.SHOULD_END)
            process_1.stop_proc()
            assert(process_1.state == PythonProcess.PROC_STATE.COMPLETE)
        else: # i > time_needed_t1
            assert(m == PythonProcess.PROC_MESSAGE.NO_CHANGE)


        if i < pre_wait_t2:
            assert(m2 == PythonProcess.PROC_MESSAGE.NO_CHANGE)
        elif i == pre_wait_t2:
            assert(m2 == PythonProcess.PROC_MESSAGE.SHOULD_WAIT)
        elif i == pre_wait_t2 + 1:
            assert(m2 == PythonProcess.PROC_MESSAGE.SHOULD_START)
            process_2.start_proc()
        elif (pre_wait_t2 + time_needed_t2) + 1  == i :
            assert(m2 == PythonProcess.PROC_MESSAGE.SHOULD_END)
            process_2.stop_proc()
        elif i > pre_wait_t2 + time_needed_t2:
            assert(m2 == PythonProcess.PROC_MESSAGE.NO_CHANGE)




