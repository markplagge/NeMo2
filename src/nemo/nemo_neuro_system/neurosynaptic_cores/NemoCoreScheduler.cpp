//
// Created by Mark Plagge on 6/18/20.
//

#include "NemoCoreScheduler.h"
#include <json.hpp>
namespace nemo {
	namespace neuro_system {



		void NemoCoreScheduler::forward_scheduler_event(tw_bf* bf, nemo::nemo_message* m, tw_lp* lp) {
			//Scheduler events are only once per global tick
			//first queue up next scheduler event
			if (global_config->do_neuro_os) {
				//First schedule the next tick of the neuro_os_scheduler
				auto sched_time = JITTER(lp->rng) + 1;
				struct tw_event* os_tick = tw_event_new(lp->gid, sched_time, my_lp);
				nemo_message* msg = (nemo_message*)tw_event_data(os_tick);
				msg->message_type = NOS_TICK;

				msg->random_call_count = lp->rng->count;
				tw_event_send(os_tick);
				//update state
				this->current_neuro_tick = floor(tw_now(lp));
				//Now run the scheduler iteration code which handles scheduling processes.

				scheduler_iteration();
			}else{
				//no scheduler. Just run the model at position 0
				auto model_file = model_files[0];

				auto num_needed_cores = model_file.get_num_needed_cores();
				for(int i = 0; i < num_needed_cores; i ++){
					send_process_states(i, 0);
				}
			}
			//check the current list of processes:
		}

		void NemoCoreScheduler::send_process_states(int dest_core, int model_id){
			auto model_file = model_files[model_id];
			struct tw_event* set_state = tw_event_new(get_gid_from_core_local(dest_core,0),JITTER(my_lp->rng),my_lp);
			auto msg = (nemo_message*) tw_event_data(set_state);
			msg->message_type = NOS_LOAD_MODEL;
			msg->debug_time = tw_now(my_lp);
			msg->model_id = model_id;
			auto core_dat = model_files[model_id].get_core_settings(model_id);
			char * core_dat_s = const_cast<char*>(core_dat.c_str());
			msg->update_message = core_dat_s;
			//memcpy(msg->update_message,core_dat_s,core_dat.size());
			tw_event_send(set_state);


		}
		void NemoCoreScheduler::reverse_scheduler_event(tw_bf* bf, nemo::nemo_message* m, tw_lp* lp) {
		}

		void NemoCoreScheduler::set_task_list(const std::vector<nemo::config::ScheduledTask>& new_task_list) {
			for (const auto& task : new_task_list) {
				auto task_ptr = std::make_shared<nemo::config::ScheduledTask>(std::move(task));
				task_list.push_back(task_ptr);
			}
		}
		void NemoCoreScheduler::init_process_models() {

			int model_counter = 0;
			auto l_task_list = global_config->scheduler_inputs;
			auto l_model_list = global_config->models;

			/** @todo: Could do this in parallel - if IO is not the bottleneck, but using modern json for the file parse is - Could also switch to RapidJSON */
			/** @remark If time permits investigate OMP init or maybe switching to RAPIDJSON */
			std::map<int, config::NemoModel> models;
			int max_models = -1;
			if (!global_config->do_neuro_os){
				max_models = 1;
			}
			for (const auto& model : l_model_list) {
				/**@todo: handle unlimited runs */
				if (max_models > 0 and max_models <= model_counter){
					break;
				}
				bool does_model_have_known_runtime = true;

				auto model_id = model.id;
				models.emplace(model_id, model);
				if (model.model_file_path.length() != 0) {
					if (g_tw_mynode == 0)
						tw_printf(TW_LOC, "Loading Model #%i: %s \n", model_counter, model.model_file_path.c_str());
					auto model_file = ModelFile(model.model_file_path);
					if (g_tw_mynode == 0)
						tw_printf(TW_LOC, "Loading Spike #%i: %s \n", model_counter, model.spike_file_path.c_str());
					auto spike_file = SpikeFile(model.spike_file_path);
					model_files.push_back(model_file);
					spike_files.push_back(spike_file);
				}
				else {
					tw_printf(TW_LOC, "Loading benchmark model %s \n", model.benchmark_model_name.c_str());
					ModelFile mf;
					SpikeFile sf;
					model_files.push_back(mf);
					spike_files.push_back(sf);
				}
				model_counter += 1;
			}

			this->set_models(models);
			this->set_task_list(l_task_list);
			int task_counter = 0;
			int max_start_time = 0;
			int run_time_ttl = 0;

			//queue up a message to test for end-times
			for (const auto& task : task_list) {
				task_counter += 1;
				auto start_time = task->start_time;
				auto task_id = task->task_id;
				auto proc = std::make_shared<neuro_os::sim_proc::SimProc>();
				auto model = this->models[task_id];
				proc->needed_cores = model.needed_cores;
				proc->PID = model.id;
				proc->needed_run_time = model.requested_time;
				proc->scheduled_start_time = start_time;
				process_queue.enqueue(proc);
				if (max_start_time < proc->scheduled_start_time) {
					max_start_time = proc->scheduled_start_time;
				}
				run_time_ttl += proc->needed_run_time;
			}
			std::cout << "Models loaded: " << model_counter << "\n"
					  << task_counter << " tasks. \n";

			auto sched_time = JITTER(my_lp->rng) + run_time_ttl + max_start_time;
			struct tw_event* os_tick = tw_event_new(my_lp->gid, sched_time, my_lp);
			nemo_message* msg = (nemo_message*)tw_event_data(os_tick);
			msg->message_type = NOS_STATUS;
			msg->debug_time = tw_now(my_lp);
			msg->random_call_count = my_lp->rng->count;
			tw_event_send(os_tick);

		}
		void NemoCoreScheduler::check_waiting_procs() {
		}

		void NemoCoreScheduler::set_models(const std::map<int, nemo::config::NemoModel>& models) {
			NemoCoreScheduler::models = models;
		}

		std::vector<std::shared_ptr<SimProcess>>   NemoCoreScheduler::get_removable_processes() {
			auto assigned_processes = this->core_process_map.get_all_assigned_processes();
			for (int i = 0; i < assigned_processes.size() ; ++i) {
				auto proc = assigned_processes[i];
				if(proc->current_state == neuro_os::sim_proc::COMPLETE){
					auto pid = proc->get_pid();

				}
			}
		}
		int NemoCoreScheduler::remove_assigned_done_processes() {

		}
		void NemoCoreScheduler::scheduler_iteration(){
			//1. set process queue time:
			this->process_queue.system_tick();
			current_scheduler_time ++;
			//2. get_working_cores:
			auto working_cores = core_process_map.get_working_cores();
			//3. get processes assigned to cores:
			auto assigned_procs = core_process_map.get_all_assigned_processes();
			//4. Check each assigned process if it should be removed from assigned processes
			//(Process is completed then remove it, or if using some other scheduler system mark it too)



		}


		void sched_core_init(NemoCoreScheduler* s, tw_lp* lp) {
			new (s) NemoCoreScheduler();
			//Make sure we are running on PE 0
			if (g_tw_mynode != 0) {
				tw_error(TW_LOC, "Neuromorphic scheduler core was init'ed from non 0 pe: PEID: %i", g_tw_mynode);
			}
			s->my_lp = lp;
			auto sched_time = JITTER(lp->rng) + 1;
			struct tw_event* os_tick = tw_event_new(lp->gid, sched_time, lp);
			auto* msg = (nemo_message*)tw_event_data(os_tick);
			msg->message_type = NOS_TICK;
			msg->debug_time = tw_now(lp);
			msg->random_call_count = lp->rng->count;
			tw_event_send(os_tick);
			s->init_process_models();
		}
		void sched_pre_run(NemoCoreScheduler* s, tw_lp* lp) {
		}
		/**
 			* on forward events call the parent class
 			* @param s
 			* @param bf
 			* @param m
 			* @param lp
 		*/
		void sched_forward_event(NemoCoreScheduler* s, tw_bf* bf, nemo::nemo_message* m, tw_lp* lp) {
			s->forward_scheduler_event(bf, m, lp);
		}
		void sched_reverse_event(NemoCoreScheduler* s, tw_bf* bf, nemo::nemo_message* m, tw_lp* lp) {
			s->reverse_scheduler_event(bf, m, lp);
		}
		void sched_core_commit(NemoCoreScheduler* s, tw_bf* bf, nemo::nemo_message* m, tw_lp* lp) {
			s->core_commit(bf, m, lp);
		}
		void sched_core_finish(NemoCoreScheduler* s, tw_lp* lp) {
			s->core_finish(lp);
		}



	}// namespace neuro_system
}// namespace nemo
