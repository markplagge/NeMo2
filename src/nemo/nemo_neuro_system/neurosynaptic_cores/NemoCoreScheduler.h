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
	extern neuro_os::NengoInterface *nengo_scheduler;
	namespace neuro_system {

			using namespace neuro_os;
			using  SimProcess = neuro_os::sim_proc::SimProcess ;
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
				void set_running_process_at_core(unsigned long core_id,std::shared_ptr<sim_proc::SimProcess> process);

				void remove_assigned_process(unsigned long core_id);

				std::shared_ptr<sim_proc::SimProcess>clear_process_at_core(unsigned long core_id);

				std::shared_ptr<sim_proc::SimProcess>get_process_at_core(unsigned long core_id);

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

		class NemoCoreScheduler : public NemoNeuroCoreBase {
		public:
			std::ofstream debug_log;
			double last_active_time = 0;

			neuro_os::sim_proc::SimProcessQueue process_queue;
			std::vector<std::shared_ptr<nemo::config::ScheduledTask>> task_list;
			unsigned long current_scheduler_time;
			unsigned long time_slice = 1000;
			TaskProcessMap core_process_map; // Keeps track of what cores are running what processes.

			void set_models(const std::map<int, nemo::config::NemoModel>& models);
			void forward_scheduler_event(tw_bf *bf, nemo_message *m, tw_lp *lp);
			void reverse_scheduler_event(tw_bf *bf, nemo_message *m, tw_lp *lp);

			void set_task_list(const std::vector<nemo::config::ScheduledTask>& task_list);
			void init_process_models();
			void check_waiting_procs();
			/** nengo_scheduler - instance of the nengo interface to the neuro_os  system*/
			//neuro_os::NengoInterface nengo_scheduler;

			bool use_nengo_for_scheduling = true;
			/**
			 * Schedule Mode:
			 * 0 = FCFS, 1 = RR
			 */
			int schedule_mode = 1;

			/**
			 * primary function entry point for one scheduler tick. Called every neurosynaptic tick.
			 * Runs neuron based scheduler algorithms, updates process states, assigns processes to cores,
			 * and sends cores new state information.
			 */
			void scheduler_iteration();
			/**
			 * Starts the next process from the waiting process queue. This should be called
			 * after can_start_waiting_process() has determined if we can start the next process.

			 */
			void start_process(unsigned int process_id);

			/**
			 * Gets a list of cores that are idle.
			 * If the process can not fit in the cores then it will return a zero length array.
			 * @param number_of_cores_needed
			 */
			std::vector<unsigned int> get_idle_cores(unsigned int number_of_cores_needed);
			/**
			 * Function that sends the new neuron states out to the simulated cores.
			 * @param bf
			 * @param m
			 * @param lp
			 */
			void send_process_states(int dest_core, int model_id);

			/**
			 * Sends input spikes from the current model
			 */
			void send_input_spikes(int model_id, double time_t );
			/**
			 * Function that sends stop messages to the simulated cores and updates the task_process_map
			 * @param bf
			 * @param m
			 * @param lp
			 */
			void stop_process(unsigned int process_id);

			/** Function that actually sends the stop message. Called from stop_process.
			 *
			 * @param bf
			 * @param m
			 * @param lp
			 */
			void send_stop_process_message(tw_bf *bf, nemo_message *m, tw_lp *lp);

			/**
			 * function that checks if the waiting process can be started based on running cores, scheduler logic, etc.
			 * @return
			 */
			bool can_start_waiting_process();

			/**
			 * Is there a process waiting in the queue?
			 * @return
			 */
			bool is_process_waiting();
			/**
			 * Is there a process running at all?
			 * @param process_id
			 * @return
			 */
			bool is_process_running(unsigned int process_id);


			/**
			 * Sets a particular process's state
			 * @param state
			 * @param proc_id
			 */
			void set_process_state(neuro_os::sim_proc::PROC_STATE state, int proc_id);

			/**
			 * Get the next waiting process' state
			 * @return
			 */
			neuro_os::sim_proc::PROC_STATE get_top_waiting_process_state();
			/**
			 * Gets an arbitrary process' state
			 * @param proc_id
			 * @return
			 */
			neuro_os::sim_proc::PROC_STATE get_process_state(int proc_id);
			/**
			 * Get all currently assigned (running or possibly stopped but still in the "running" manager) process states.
			 * @return
			 */
			std::vector<std::shared_ptr<SimProcess>> get_running_process_states();





			int remove_assigned_done_processes();

			virtual std::vector<std::shared_ptr<SimProcess>>  get_removable_processes();

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
