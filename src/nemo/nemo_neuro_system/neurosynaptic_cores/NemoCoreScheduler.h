//
// Created by Mark Plagge on 6/18/20.
//

#ifndef NEMOTNG_NEMOCORESCHEDULER_H
#define NEMOTNG_NEMOCORESCHEDULER_H
#include "../../nemo_io/ModelFile.h"
#include "../../nemo_io/SpikeFile.h"
#include "NemoNeuroCoreBase.h"
#include <configuru.hpp>
#include <neuro_os.h>
#include <visit_struct/visit_struct.hpp>
#include <visit_struct/visit_struct_intrusive.hpp>
namespace nemo {
	namespace neuro_system {

			using namespace neuro_os;
			class TaskProcessMap {
			public:
				TaskProcessMap(unsigned long num_cores) : num_cores(num_cores) {
					for (unsigned long i = 0; i < num_cores; i++) {
						task_process_map[i] = sim_proc::SimProcess();
					}
					task_process_map[0] = sim_proc::SimProcess();
				}
				TaskProcessMap(){};
				void set_running_process_at_core(unsigned long core_id, sim_proc::SimProcess process);
				sim_proc::SimProcess get_running_process_at_core(unsigned long core_id);
				std::vector<unsigned long> get_idle_cores();
				std::vector<unsigned long> get_working_cores();


				std::map<unsigned long, sim_proc::SimProcess> task_process_map;

			protected:
				/**
			 * Returns the cores that are either working or idle. If running is true, return working cores.

			 * @param running
			 * @return
			 */
				std::vector<unsigned long> get_core_states(bool running);
				unsigned long num_cores = 0;
			};

		class NemoCoreScheduler : public NemoNeuroCoreBase {
		public:
			double last_active_time = 0;
			neuro_os::sim_proc::SimProcessQueue process_queue;
			std::vector<nemo::config::ScheduledTask> task_list;

			void set_models(const std::map<int, nemo::config::NemoModel>& models);
			void forward_scheduler_event(tw_bf *bf, nemo_message *m, tw_lp *lp);
			void reverse_scheduler_event(tw_bf *bf, nemo_message *m, tw_lp *lp);

			void set_task_list(const std::vector<nemo::config::ScheduledTask>& task_list);
			void init_process_models();
			void check_waiting_procs();

			/**
			 * Starts the next process from the waiting process queue. This should be called
			 * after can_start_waiting_process() has determined if we can start the next process.
			 * @param bf
			 * @param m
			 * @param lp
			 */
			void start_process(tw_bf *bf, nemo_message *m, tw_lp *lp);
			/**
			 * Function that sends the new neuron states out to the simulated cores.
			 * @param bf
			 * @param m
			 * @param lp
			 */
			void send_process_states(tw_bf *bf, nemo_message *m, tw_lp *lp);

			/**
			 * Function that sends stop messages to the simulated cores and updates the task_process_map
			 * @param bf
			 * @param m
			 * @param lp
			 */
			void stop_process(tw_bf *bf, nemo_message *m, tw_lp *lp);

			/** Function that actually sends the stop message. Called from stop_process.
			 *
			 * @param bf
			 * @param m
			 * @param lp
			 */
			void send_stop_process_message(tw_bf *bf, nemo_message *m, tw_lp *lp);

			bool can_start_waiting_process(tw_bf *bf, nemo_message *m, tw_lp *lp);

			bool is_process_waiting(tw_bf *bf, nemo_message *m, tw_lp *lp);
			bool is_process_running(tw_bf *bf, nemo_message *m, tw_lp *lp);
			void set_current_process_state(tw_bf *bf, nemo_message *m, tw_lp *lp,neuro_os::sim_proc::PROC_STATE state);
			void set_process_state(tw_bf *bf, nemo_message *m, tw_lp *lp,neuro_os::sim_proc::PROC_STATE state, int proc_id);


			neuro_os::sim_proc::PROC_STATE get_top_waiting_process_state();
			neuro_os::sim_proc::PROC_STATE get_process_state(int proc_id);
			std::vector<neuro_os::sim_proc::PROC_STATE> get_running_process_states();



		protected:
			unsigned long current_scheduler_time;
			TaskProcessMap core_process_map; // Keeps track of what cores are running what processes.

		};
		// This class gets directly init'ed by ROSS through these functions:
		void sched_core_init(NemoCoreScheduler * s, tw_lp* lp);
		//void sched_core_init(void* s, tw_lp* lp);
		void sched_pre_run(NemoCoreScheduler * s, tw_lp* lp);
		void sched_forward_event(NemoCoreScheduler * s, tw_bf* bf, nemo_message* m, tw_lp* lp);
		void sched_reverse_event(NemoCoreScheduler * s, tw_bf* bf, nemo_message* m, tw_lp* lp);
		void sched_core_commit(NemoCoreScheduler * s, tw_bf* bf, nemo_message* m, tw_lp* lp);
		void sched_core_finish(NemoCoreScheduler * s, tw_lp* lp);





	}








}

#endif//NEMOTNG_NEMOCORESCHEDULER_H
