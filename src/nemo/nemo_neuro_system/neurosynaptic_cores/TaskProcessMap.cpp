//
// Created by Mark Plagge on 6/24/20.
//
#include "NemoCoreScheduler.h"
namespace nemo{
	namespace neuro_system{
		void nemo::neuro_system::TaskProcessMap::set_running_process_at_core(unsigned long core_id, sim_proc::SimProcess process) {
			task_process_map[core_id] = process;
		}
		sim_proc::SimProcess nemo::neuro_system::TaskProcessMap::get_running_process_at_core(unsigned long core_id) {
			return task_process_map[core_id];
		}
		std::vector<unsigned long> TaskProcessMap::get_idle_cores() {
			return std::vector<unsigned long>();
		}
		std::vector<unsigned long> TaskProcessMap::get_working_cores() {
			return std::vector<unsigned long>();
		}
		std::vector<unsigned long> TaskProcessMap::get_core_states(bool running) {
			sim_proc::PROC_STATE state = neuro_os::sim_proc::RUNNING;
			std::vector<unsigned long> task_list;
			for (unsigned long i = 0; i < num_cores; i ++){
				if ( (task_process_map[i].current_state == state) && running) {
					task_list.push_back(i);
				}
			}

			return task_list; //c++11 return by value - it's okay
		}

	}
}