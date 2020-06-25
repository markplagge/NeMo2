//
// Created by Mark Plagge on 6/24/20.
//
#include "NemoCoreScheduler.h"
#include <vector>
namespace nemo{
	namespace neuro_system{

		void TaskProcessMap::set_running_process_at_core(unsigned long core_id,std::shared_ptr<sim_proc::SimProcess> process){
			task_process_map[core_id] = process;
		}

		std::shared_ptr<sim_proc::SimProcess> TaskProcessMap::get_process_at_core(unsigned long core_id) {
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
				if (((task_process_map[i]->current_state == state) && running) or
						(task_process_map[i]->current_state != neuro_os::sim_proc::NO_OP && (!running))) {
					task_list.push_back(i);
				}
			}

			return task_list; //c++11 return by value - it's okay
		}
		std::shared_ptr<sim_proc::SimProcess>TaskProcessMap::clear_process_at_core(unsigned long core_id) {
			return task_process_map[core_id];
		}
		std::vector<std::shared_ptr<sim_proc::SimProcess>> TaskProcessMap::get_all_running_processes() {
			std::vector<std::shared_ptr<sim_proc::SimProcess>> smp;
			auto all_assigned = get_all_assigned_processes();
			for (const auto& assigned : all_assigned) {
				if (assigned->current_state == neuro_os::sim_proc::RUNNING){
					smp.push_back(assigned);
				}
			}
			return smp;
		}
		std::shared_ptr<sim_proc::SimProcess> TaskProcessMap::get_assigned_process_at_core(unsigned long core_id) {
			return task_process_map[core_id];
		}
		std::vector<std::shared_ptr<sim_proc::SimProcess>> TaskProcessMap::get_all_assigned_processes() {
			 std::vector<std::shared_ptr<sim_proc::SimProcess>> smp;
			for (int i =0; i < num_cores; i ++) {
				if(task_process_map[i]->current_state != neuro_os::sim_proc::NO_OP) {
					smp.push_back(task_process_map[i]);
				}
			}
			return smp;
		}

	}
}