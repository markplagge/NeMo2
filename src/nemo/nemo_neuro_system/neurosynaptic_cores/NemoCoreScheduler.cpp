//
// Created by Mark Plagge on 6/18/20.
//

#include "NemoCoreScheduler.h"
#include <json.hpp>
namespace nemo {
	neuro_os::NengoInterface* nengo_scheduler;
	namespace neuro_system {

		void NemoCoreScheduler::forward_scheduler_event(tw_bf* bf, nemo::nemo_message* m, tw_lp* lp) {
			//Scheduler events are only once per global tick
			//first queue up next scheduler event
			this->my_bf = bf;
			static int did_init = 0;
			this->cur_message = m;

			if (m->message_type == NOS_TICK) {
				if (global_config->do_neuro_os) {
					//First schedule the next tick of the neuro_os_scheduler
					auto sched_time = JITTER(lp->rng) + 1;
					struct tw_event* os_tick = tw_event_new(lp->gid, sched_time, my_lp);
					auto* msg = (nemo_message*)tw_event_data(os_tick);
					msg->message_type = NOS_TICK;
					msg->random_call_count = lp->rng->count;//@TODO: Add rng counts
					tw_event_send(os_tick);

					//update current_neuro_tick
					this->current_neuro_tick = floor(tw_now(lp));
					//Now run the scheduler iteration code which handles scheduling processes.
					scheduler_iteration();
				}
				else if (!did_init) {// If there is no NOS mode, and we have not already started the first model then start it up.
					did_init = 1;
					//no scheduler. Just run the model at position 0
					auto model_file = model_files[0];

					auto num_needed_cores = model_file.get_num_needed_cores();
					for (int i = 0; i < num_needed_cores; i++) {
						send_process_states(i, 0);
					}
					for (int i = 0; i < g_tw_ts_end; ++i) {
						send_input_spikes(0, i);
					}
				}
			}
			else if (m->message_type == NOS_STATUS) {
				tw_printf(TW_LOC, "INFO: NOS STATUS MESSAGE @ %f\n ", tw_now(this->my_lp));
			}
			else {
				tw_error(TW_LOC, "Nemo Core Scheduler got a non-tick message, %i", m->message_type);
			}

			//check the current list of processes:
		}
		template<typename types>
		std::string debug_csv(types args) {
			return std::to_string(args) + "\n";
		}
		template<typename T, typename... types>
		std::string debug_csv(T first, types... args) {
			return std::to_string(first) + std::string(",") + debug_csv(args...);
		}

		void NemoCoreScheduler::send_process_states(int dest_core, int model_id) {
			static unsigned int num_states_sent = 0;
			struct tw_event* set_state = tw_event_new(get_gid_from_core_local(dest_core, 0), JITTER(my_lp->rng), my_lp);
			auto msg = (nemo_message*)tw_event_data(set_state);
			msg->message_type = NOS_LOAD_MODEL;
			msg->debug_time = tw_now(my_lp);
			msg->model_id = model_id;
			//			auto core_dat = model_files[model_id].get_core_settings(model_id);
			//			char * core_dat_s = const_cast<char*>(core_dat.c_str());
			//			strncpy(msg->update_message,core_dat_s,NEMO_MAX_CHAR_DATA_SIZE - 1);
			//			//msg->update_message = core_dat_s;
			//memcpy(msg->update_message,core_dat_s,core_dat.size());
			tw_event_send(set_state);
			num_states_sent++;
			if (!global_config->do_neuro_os && num_states_sent > global_config->ns_cores_per_chip * global_config->total_chips) {
				tw_error(TW_LOC, "SENT %ui events - check it \n", num_states_sent);
			}
			if (config::NemoConfig::DEBUG_FLAG) {

				auto msg_type = 'M';
				debug_log << debug_csv(msg_type, model_id, set_state->recv_ts, get_gid_from_core_local(dest_core, 0));
			}
		}
		void NemoCoreScheduler::reverse_scheduler_event(tw_bf* bf, nemo::nemo_message* m, tw_lp* lp) {
		}

		void NemoCoreScheduler::set_task_list(const std::vector<nemo::config::ScheduledTask>& new_task_list) {
			for (const auto& task : new_task_list) {
				auto task_ptr = std::make_shared<nemo::config::ScheduledTask>(task);
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
			if (!global_config->do_neuro_os) {
				max_models = 1;
			}
			for (const auto& model : l_model_list) {
				/**@todo: handle unlimited runs */
				if (max_models > 0 and max_models <= model_counter) {
					break;
				}
				//bool does_model_have_known_runtime = true;

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
				auto model = nemo::neuro_system::NemoCoreScheduler::models[task_id];
				proc->needed_cores = model.needed_cores;
				proc->PID = model.id;
				proc->needed_run_time = model.requested_time;
				proc->scheduled_start_time = start_time;
				process_queue.enqueue(proc);
				if (max_start_time < (int) proc->scheduled_start_time) {
					max_start_time = (int) proc->scheduled_start_time;
				}
				run_time_ttl += (int)proc->needed_run_time;
			}
			std::cout << "Models loaded: " << model_counter << "\n"
					  << task_counter << " tasks. \n";

			auto sched_time = JITTER(my_lp->rng) + run_time_ttl + max_start_time;
			struct tw_event* os_tick = tw_event_new(my_lp->gid, sched_time, my_lp);
			auto* msg = (nemo_message*)tw_event_data(os_tick);
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

		std::vector<std::shared_ptr<SimProcess>> NemoCoreScheduler::get_removable_processes() {
			auto assigned_processes = this->core_process_map.get_all_assigned_processes();
			for (auto proc : assigned_processes) {
				if (proc->current_state == neuro_os::sim_proc::COMPLETE) {
					auto pid = proc->get_pid();
				}
			}
			return assigned_processes;
		}
		int NemoCoreScheduler::remove_assigned_done_processes() {
			return 0;
		}

		void NemoCoreScheduler::scheduler_iteration() {
			//1. set process queue time:
			this->process_queue.system_tick();
			current_scheduler_time++;
			if (this->use_nengo_for_scheduling) {
				auto runq = neuro_os::precompute_run_q(*nengo_scheduler);
				auto waitq = neuro_os::precompute_wait_q(*nengo_scheduler);
				//find new processes to start
				for (auto model_id : runq) {
					this->start_process(model_id);
				}
				for (auto model_id : waitq) {
					this->stop_process(model_id);
				}

				//reassign runq and waitq
				this->running_models = runq;
				this->waiting_models = waitq;
			}
			//2. get_working_cores:
			auto working_cores = core_process_map.get_working_cores();
			//3. get processes assigned to cores:
			auto assigned_procs = core_process_map.get_all_assigned_processes();
			//4. Check each assigned process if it should be removed from assigned processes
			//(Process is completed then remove it, or if using some other scheduler system mark it too)
		}
		void NemoCoreScheduler::send_input_spikes(int model_id, double time_t) {
			auto time = floor(time_t);
			auto spike_r = spike_files[model_id].get_spikes_at_time(time);
			for (const auto& spk : spike_r) {
				auto dest = get_gid_from_core_local(spk.dest_core, spk.dest_axon);
				auto dest_t = JITTER(this->my_lp->rng) + spk.time;
				struct tw_event* os_input_spike_evt = tw_event_new(dest, dest_t, this->my_lp);
				auto* msg = (nemo_message*)tw_event_data(os_input_spike_evt);
				msg->dest_axon = spk.dest_axon;
				msg->message_type = NEURON_SPIKE;
				//add random call count
				msg->intended_neuro_tick = spk.time;
				msg->source_core = -1;
				tw_event_send(os_input_spike_evt);
				if (config::NemoConfig::DEBUG_FLAG) {

					debug_log << "msg_type,model_id,dest_time,dest_core\n";
					debug_log << debug_csv('S', model_id, dest_t, get_gid_from_core_local(spk.dest_core, spk.dest_axon));
				}
			}
		}
		void NemoCoreScheduler::send_start_stop_messages(nemo_message_type type, unsigned int process_id) {
			double start_time;
			if (type == NOS_START) {
				start_time = 2 + JITTER(my_lp->rng);
			}
			else {
				start_time = 1 + JITTER(my_lp->rng);
			}
			auto model_file = model_files[process_id];
			auto num_needed_cores = model_file.get_num_needed_cores();
			for (int i = 0; i < num_needed_cores; i++) {
				auto dest_gid = get_gid_from_core_local(i, 1);

				auto start_message = tw_event_new(dest_gid, start_time, this->my_lp);
				auto msg = (nemo_message*)tw_event_data(start_message);

				msg->message_type = type;
				msg->model_id = process_id;
				msg->source_core = -1;
				msg->intended_neuro_tick = (unsigned long)start_time;
				tw_event_send(start_message);
			}
		}
		void NemoCoreScheduler::stop_process(unsigned int process_id) {
			// send stop messages
			send_start_stop_messages(NOS_STOP, process_id);
		}
		void NemoCoreScheduler::start_process(unsigned int process_id) {
			//send start messages:
			send_start_stop_messages(NOS_START, process_id);
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
			if (config::NemoConfig::DEBUG_FLAG) {
				s->debug_log = std::ofstream("core_debug.txt");
				s->debug_log << "msg_type,model_id,dest_time,dest_core\n";
			}
			s->init_process_models();
			//Initialize and set up the nengo scheduler
			if (s->use_nengo_for_scheduling) {
				auto num_cores_in_sim = (global_config->ns_cores_per_chip * global_config->total_chips);
				nengo_scheduler = new neuro_os::NengoInterface(global_config->use_nengo_dl, num_cores_in_sim,
															   s->schedule_mode, 4096, config::NemoConfig::main_config_file);
				auto precompute_time = g_tw_ts_end + 10;
				std::cout << "Precompute scheduler enabled to " << precompute_time << "\n";
				neuro_os::run_precompute_sim(*nengo_scheduler, int(precompute_time));
				std::cout << "Precompute done, gathering wait \n SC EPOCH: " << s->current_scheduler_time << " NG EPOCH: " << nengo_scheduler->nengo_os_iface.attr("precompute_time").cast<int>() << "\n";
				auto runq = neuro_os::precompute_run_q(*nengo_scheduler);
				auto waitq = neuro_os::precompute_wait_q(*nengo_scheduler);
				s->waiting_models = waitq;
				s->running_models = runq;
			}
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
			std::cout << "SCHEDULER CORE AT " << tw_now(lp) << "Message " << m->message_type << "\n";
		}
		void sched_reverse_event(NemoCoreScheduler* s, tw_bf* bf, nemo::nemo_message* m, tw_lp* lp) {
			s->reverse_scheduler_event(bf, m, lp);
		}
		void sched_core_commit(NemoCoreScheduler* s, tw_bf* bf, nemo::nemo_message* m, tw_lp* lp) {
			s->core_commit(bf, m, lp);
		}
		void sched_core_finish(NemoCoreScheduler* s, tw_lp* lp) {
			s->debug_log.close();
			s->core_finish(lp);
		}

	}// namespace neuro_system
}// namespace nemo
