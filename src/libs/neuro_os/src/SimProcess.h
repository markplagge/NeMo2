//
// Created by Mark Plagge on 3/11/20.
//

#ifndef SUPERNEMO_SIMPROCESS_H
#define SUPERNEMO_SIMPROCESS_H

#include <ostream>
#include "./process_states.h"
#include <iostream>
#include "../lib/json.hpp"


namespace neuro_os { namespace sim_proc {
        using nlohmann::json;
    template<class T>
    struct SimProcess {

        int PID;
        int needed_cores;
        int needed_run_time;
        double scheduled_start_time;
        int total_wait_time;
        int total_run_time;
        PROC_STATE current_state;
        //T neuron_state_system;


        SimProcess(int pid, int neededCores, int neededRunTime, double scheduledStartTime);

        SimProcess() {
            PID = 0;
            needed_cores = 0;
            needed_run_time = 0;
            scheduled_start_time = 0;
            total_wait_time = 0;
            total_run_time = 0;
            current_state = WAITING;

        }

        PROC_STATE get_current_state() const;

        void set_current_state(PROC_STATE current_state);

        int get_pid() const;

        int get_needed_cores() const;

        int get_needed_run_time() const;

        double get_scheduled_start_time() const;

        int get_total_wait_time() const;

        int get_total_run_time() const;

        void system_tick();

        friend std::ostream &operator<<(std::ostream &os, const SimProcess<T> &process);

        bool operator==(const SimProcess &rhs) const;

        bool operator!=(const SimProcess &rhs) const;


    };






} }

#endif //SUPERNEMO_SIMPROCESS_H
