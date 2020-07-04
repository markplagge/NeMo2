
from enum import IntFlag

class PROC_STATE(IntFlag):
    WAITING = 1
    RUNNING = 2
    COMPLETE = 4
    PRE_WAIT = 8
    NO_OP = 0
class SCHEDULER_STATE(IntFlag):
    ADD = 1
    FULL = 2

class PROC_MESSAGE(IntFlag):
    SHOULD_START = 1
    SHOULD_END = 2
    SHOULD_WAIT = 4


    NO_CHANGE = 0

class Process:
    def __init__(self, n_cores=4096, time_needed=1000, process_id = 0,state = PROC_STATE.WAITING, scheduled_start_time=0, name="CIFAR" ):
        self.n_cores = n_cores
        self.needed_time = time_needed
        self.process_id = process_id
        if state == PROC_STATE.WAITING and scheduled_start_time > 0:
                current_state = PROC_STATE.PRE_WAIT
        elif state == PROC_STATE.NO_OP:
            if scheduled_start_time > 0:
                current_state = PROC_STATE.PRE_WAIT
            else:
                current_state = PROC_STATE.WAITING
        else:
            current_state = state

        self.waiting_time = 0
        self.running_time = 0
        self.scheduled_start_time = scheduled_start_time
        self.start_time = 0
        self.stop_time = 0
        self.clock = -1
        self.name = name

        self.running = False
        self.messages = []
        self._state = current_state

    @property
    def state(self):
        return self._state

    @state.setter
    def state(self, value):
        self.add_event(value)
        self._state = value

    def start_proc(self):
        assert(self.state == PROC_STATE.WAITING)
        self.start_time = self.clock
        self.running = True
        self.state = PROC_STATE.RUNNING
        return self.state

    def stop_proc(self):
        assert(self.state == PROC_STATE.RUNNING)
        #todo: Potential bug - should check if run_time is > than clock
        if self.needed_time >= self.running_time:
            self.state = PROC_STATE.COMPLETE
            self.stop_time = self.clock
        else:
            self.state = PROC_STATE.WAITING
        return self.state

    def msg_tmplate(self):
        return f"P {self.process_id} clock: {self.clock} "

    def add_message(self, proc_message = PROC_MESSAGE.SHOULD_WAIT):
        m = f"{self.msg_tmplate()} says {proc_message}"
        self.messages.append(m)

    def add_event(self, proc_message = PROC_STATE.PRE_WAIT):
        m = f"{self.msg_tmplate()} is now {proc_message}"
        self.messages.append(m)


    def tick(self):
        self.clock += 1
        ctx = 0
        if self.state == PROC_STATE.PRE_WAIT:
            if self.scheduled_start_time <= self.clock:
               ctx = PROC_MESSAGE.SHOULD_WAIT
               self.state = PROC_STATE.WAITING
            else:
                ctx = PROC_MESSAGE.NO_CHANGE

        elif self.state == PROC_STATE.WAITING:
            self.waiting_time += 1
            ctx =  PROC_MESSAGE.SHOULD_START

        elif self.state == PROC_STATE.RUNNING:
            new_run_time = self.running_time + 1
            assert(new_run_time <= self.needed_time)
            if new_run_time < self.needed_time:
                ctx = PROC_MESSAGE.NO_CHANGE
            elif new_run_time == self.needed_time:
                ctx = PROC_MESSAGE.SHOULD_END
                self.messages.append(
                    f"Process {self.name} completed with rt:{self.running_time}, wt:{self.waiting_time}")
            self.running_time = new_run_time

        elif self.state == PROC_STATE.COMPLETE:
            ctx = PROC_MESSAGE.NO_CHANGE
        self.add_message(ctx)
        return ctx.value

    def hdr(self):
        return "name,scheduled_start_time,wait_time,start_time,running_time,current_state\n"
    def __str__(self):
        return f"{self.name},{self.scheduled_start_time},{self.waiting_time},{self.start_time},{self.running_time},{self.state}"




