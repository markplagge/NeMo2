//
// Created by Mark Plagge on 7/26/20.
//

#ifndef NEMOTNG_TASKPROCESSMAP_H
#define NEMOTNG_TASKPROCESSMAP_H
#include "../../../libs/neuro_os/src/NemoSchedulerInterface.h"

namespace nemo {
	extern neuro_os::NengoInterface* nengo_scheduler;
	namespace neuro_system {

		using namespace neuro_os;
		using SimProcess = neuro_os::sim_proc::SimProcess;
		class TaskProcessMap {
		public:
			TaskProcessMap(unsigned long num_cores) : num_cores(num_cores) {
				for (unsigned long i = 0; i < num_cores; i++) {
					task_process_map[i] = std::make_shared<sim_proc::SimProcess>();
				}
			}
			TaskProcessMap(){};
			/**
			 * Sets 'process' to run mode on core_id, which sets up core->process tracking;
			 * @param core_id
			 * @param process
			 */
			void set_running_process_at_core(unsigned long core_id, std::shared_ptr<sim_proc::SimProcess> process);

			void remove_assigned_process(unsigned long core_id);

			std::shared_ptr<sim_proc::SimProcess> clear_process_at_core(unsigned long core_id);

			std::shared_ptr<sim_proc::SimProcess> get_process_at_core(unsigned long core_id);

			/**
			 * gets all *running* processes in the system. This is different from *stopped*
			 * @return
			 */
			std::vector<std::shared_ptr<sim_proc::SimProcess>> get_all_running_processes();

			/**
			 * gets the process assigned to a specific core, or returns a new process with -1 as a val
			 * @param core_id
			 * @return
			 */
			std::shared_ptr<sim_proc::SimProcess> get_assigned_process_at_core(unsigned long core_id);
			/**
			 * gets the processes assigned to all cores currently.
			 * @param core_id
			 * @return
			 */
			std::vector<std::shared_ptr<sim_proc::SimProcess>> get_all_assigned_processes();
			/**
			 * What cores are not assigned with any task
			 * @return
			 */
			std::vector<unsigned long> get_idle_cores();
			/**
			 * What cores currently have an assigned (running or stopped) process?
			 * @return
			 */
			std::vector<unsigned long> get_working_cores();

			std::map<unsigned long, std::shared_ptr<sim_proc::SimProcess>> task_process_map;

		protected:
			/**
		 * Returns the cores that are either working or idle. If running is true, return working cores.

		 * @param running
		 * @return
		 */
			std::vector<unsigned long> get_core_states(bool running);
			unsigned long num_cores = 0;
		};
	}

}

#endif//NEMOTNG_TASKPROCESSMAP_H
