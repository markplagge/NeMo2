//
// Created by Mark Plagge on 6/18/20.
//

#ifndef NEMOTNG_NEMOCORESCHEDULER_H
#define NEMOTNG_NEMOCORESCHEDULER_H
#include "../../../libs/neuro_os/src/NemoSchedulerInterface.h"
#include "../../nemo_io/ModelFile.h"
#include "../../nemo_io/SpikeFile.h"
#include "NemoNeuroCoreBase.h"
#include <configuru.hpp>
#include <neuro_os.h>
#include <visit_struct/visit_struct.hpp>
#include <visit_struct/visit_struct_intrusive.hpp>
#include "TaskProcessMap.h"
namespace nemo {
	extern neuro_os::NengoInterface *nengo_scheduler;
	namespace neuro_system {

			using namespace neuro_os;
			using  SimProcess = neuro_os::sim_proc::SimProcess ;


			/**
			 * NemoCoreScheduler is the primary scheduling "neurosynaptic core" of nemo.
			 *
			 */
		class NemoCoreScheduler : public NemoNeuroCoreBase {
		public:
			std::ofstream debug_log;
			double last_active_time = 0;

			std::unique_ptr<NemoSelfContainedScheduler> self_contained_scheduler;
			std::map<int,std::vector<bool>> job_list_status;

			neuro_os::sim_proc::SimProcessQueue process_queue;
			std::vector<std::shared_ptr<nemo::config::ScheduledTask>> task_list;
			std::vector<neuro_os::ProcEvent> event_list;
			unsigned long current_scheduler_time = 0;
			unsigned long time_slice = 1000;
			TaskProcessMap core_process_map; // Keeps track of what cores are running what processes.
			std::vector<int> running_models;
			std::vector<int> waiting_models;
			std::vector<int> init_models;
			void precompute_nengo_queue_update();
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
			 * Starts the next process from the waiting process queue. This should be called
			 * after can_start_waiting_process() has determined if we can start the next process.

			 */
			void start_process(unsigned int process_id);

			/**
			 * Function that sends stop messages to the simulated cores and updates the task_process_map
			 * @param bf
			 * @param m
			 * @param lp
			 */

			void stop_process(unsigned int process_id);

			void send_start_stop_messages(nemo_message_type type, unsigned int process_id );

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

			int precached_get_running_at_time();
			int precached_get_waiting_at_time();
			int precached_get_proc_list();

			// This class gets directly init'ed by ROSS through these functions:
			static void sched_core_init(NemoCoreScheduler * s, tw_lp* lp);
			//void sched_core_init(void* s, tw_lp* lp);
			static void sched_pre_run(NemoCoreScheduler * s, tw_lp* lp);
			static void sched_forward_event(NemoCoreScheduler * s, tw_bf* bf, nemo_message* m, tw_lp* lp);
			static void sched_reverse_event(NemoCoreScheduler * s, tw_bf* bf, nemo_message* m, tw_lp* lp);
			static void sched_core_commit(NemoCoreScheduler * s, tw_bf* bf, nemo_message* m, tw_lp* lp);
			static void sched_core_finish(NemoCoreScheduler * s, tw_lp* lp);
			static void init_non_nengo_sched(NemoCoreScheduler* s, tw_lp* lp);
			static void init_scheduler_system(NemoCoreScheduler* s, tw_lp* lp);
		};




	}








}

#endif//NEMOTNG_NEMOCORESCHEDULER_H