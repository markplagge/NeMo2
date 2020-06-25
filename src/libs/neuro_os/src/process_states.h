//
// Created by Mark Plagge on 3/11/20.
//

#ifndef SUPERNEMO_PROCESS_STATES_H
#define SUPERNEMO_PROCESS_STATES_H
namespace neuro_os {
    namespace sim_proc {
        typedef enum {
            WAITING = 	1 << 0,
            RUNNING = 	1 << 1,
            COMPLETE =	1 << 2,
			NO_OP = 0
        } PROC_STATE;

        typedef enum {
            CAN_ADD,
            IS_FULL
        } SCHEDULER_STATE;
    }
}
#endif //SUPERNEMO_PROCESS_STATES_H
