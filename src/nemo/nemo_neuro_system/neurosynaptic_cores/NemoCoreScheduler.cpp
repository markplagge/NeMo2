//
// Created by Mark Plagge on 6/18/20.
//

#include "NemoCoreScheduler.h"
#include "../../../libs/neuro_os/src/NemoSchedulerInterface.h"
#include "../../mapping_functions.h"
#include <json.hpp>
namespace nemo {
	neuro_os::NengoInterface* nengo_scheduler;
	namespace neuro_system {
		namespace ns_txt {
#define X(a) #a
			static const char* nemo_message_type_strings[] = {NEMO_MESSAGE_TYPES};
#undef X
		}// namespace ns_txt
		void init_scheduler_system(NemoCoreScheduler* s, tw_lp* lp);
		void NemoCoreScheduler::forward_scheduler_event(tw_bf* bf, nemo::nemo_message* m, tw_lp* lp) {
			//Scheduler events are only once per global tick
			//first queue up next scheduler event
			this->my_bf = bf;
			static int did_init = 0;
			this->cur_message = m;

			if (m->message_type == NOS_TICK) {
				//				std::cout << "Got NOS TICK at " << tw_now(my_lp) << "\n";
				if (global_config->do_neuro_os) {
					//First schedule the next tick of the neuro_os_scheduler
					auto sched_time = JITTER(lp->rng) + 1;
					//					std::cout << "Scheduled NOS TICK for " << sched_time << "\n";

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

				tw_printf(TW_LOC, "NCS: ERR: %i %d %i", m->source_core, m->dest_axon, m->debug_time);
				tw_error(TW_LOC, "Nemo Core Scheduler got a non-tick message, %s", ns_txt::nemo_message_type_strings[m->message_type]);
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
			auto dest_gid = get_gid_from_core_local(dest_core, 0);
			if (num_states_sent < 2){
				std::cout <<"SENT STATES TO A CORE \n";
			}
			struct tw_event* set_state = tw_event_new(dest_gid, JITTER(my_lp->rng), my_lp);
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
					int rank;
					MPI_Comm_rank(MPI_COMM_WORLD, &rank);
					if (g_tw_mynode == 0 && rank == 0)
						tw_printf(TW_LOC, "Loading Model #%i: %s \n", model_counter, model.model_file_path.c_str());
					auto model_file = ModelFile(model.model_file_path);
					if (g_tw_mynode == 0 && rank == 0)
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

//			this->set_models(models);
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
				if (max_start_time < (int)proc->scheduled_start_time) {
					max_start_time = (int)proc->scheduled_start_time;
				}
				run_time_ttl += (int)proc->needed_run_time;
			}
			std::cout << "Models loaded: " << model_counter << "\n"
					  << task_counter << " tasks. \n";

			auto sched_time = JITTER(my_lp->rng) + run_time_ttl + max_start_time;
			if (sched_time < 0){
				sched_time = 1.01;
			}
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

		void nengo_scheduler_cached_proc(NemoCoreScheduler* s) {
			static int is_init = 0;


			std::vector<int> tasks_to_start;
			std::vector<int> tasks_to_stop;
			std::vector<int> running_tasks;
			std::vector<int> waiting_tasks;

			auto evts = config::generate_pc_schedule(global_config->precomputed_scheduler_file);
			for (ProcEvent evt : evts) {
				if(evt.task_id > 0){
					evt.task_id --;
				}
				if (evt.event_time == s->current_scheduler_time) {
					switch (evt.event_type) {
					case SCH_INTERRUPT:
					case SCH_START_WAIT:
						tasks_to_stop.push_back(evt.task_id);
						break;
					case SCH_PRE_WAIT:
					case SCH_WAIT:
						waiting_tasks.push_back(evt.task_id);
						break;

					case SCH_RUNNING:
						running_tasks.push_back(evt.task_id);
						break;
					case SCH_START_RUNNING:
						tasks_to_start.push_back(evt.task_id);
						running_tasks.push_back(evt.task_id);
						break;
					case SCH_DONE:
						break;
					default:
						tw_error(TW_LOC, "Got event type %d! WHAT? ", evt.event_type);
					}
					s->event_list.push_back(evt);
				}

				s->waiting_models = waiting_tasks;
				s->running_models = running_tasks;
			}

			//int next_tick =  (int)tw_now(s->my_lp) + 1;
			if (is_init == 0) {
				is_init = 1;
				auto models = s->model_files;
				auto spikes = s->spike_files;

//				for (auto task : s->task_list){
//					task->task_id
			}

			/* once the set of tasks (waiting, running, starting and stopping tasks) are set up,
			 * send any input spike to the running task list. Next, send the start signals to the starting
			 * tasks (next iteration will do input spikes). Finally send stop signals to the now waiting tasks. */

		int current_time = (int)tw_now(s->my_lp);
		int next_tick = current_time + 1;
		for(auto running_task : s->running_models){
			//for(int i = current_time ; i <= next_tick; i ++){
			s->send_input_spikes(running_task,current_time);
			//}
		}
		for(auto waiting_task : s->waiting_models){
			auto spike_file = s->spike_files[waiting_task];
			spike_file.parent_process_wait();
		}
		for(auto starting_task :tasks_to_start) {
			s->running_models.push_back(starting_task);
			s->send_start_stop_messages(NOS_START, starting_task);
		}
		for(auto stopping_task : tasks_to_stop){
			s->send_start_stop_messages(NOS_STOP, stopping_task);
			s->waiting_models.push_back(stopping_task);
		}



		}

		void non_nengo_scheduler_iteration(NemoCoreScheduler* s,bool init=false) {
			/* 15: interrupted, 12: pre_waiting, 16: proc_complete, 10: running, 14: start_running,
		 * 13: start_waiting, 11: waiting */
			auto events_at_time = s->self_contained_scheduler->proc_events[s->current_scheduler_time];
			//Got events - load them up
			std::vector<int> tasks_to_start;
			std::vector<int> tasks_to_stop;
			for (auto evt : events_at_time) {
				switch (evt.event_type) {
				case SCH_INTERRUPT:
				case SCH_PRE_WAIT:
				case SCH_START_WAIT:
				case SCH_WAIT:
					tasks_to_stop.push_back(evt.task_id);
					break;
				case SCH_RUNNING:
				case SCH_START_RUNNING:
					tasks_to_start.push_back(evt.task_id);
					break;
				default:
					tw_error(TW_LOC, "Got event type %d! WHAT? ", evt.event_type);
				}
			}
			for (auto task_id : tasks_to_stop) {
				s->stop_process(task_id);
				s->running_models[task_id] = false;
			}
			for (auto task_id : tasks_to_start) {
				s->start_process(task_id);
				s->send_input_spikes(task_id, tw_now(s->my_lp));
				s->running_models[task_id] = true;
			}

			s->waiting_models = tasks_to_stop;
			s->running_models = tasks_to_start;

		}
		void NemoCoreScheduler::scheduler_iteration() {
			//1. set process queue time:
			this->process_queue.system_tick();
			current_scheduler_time++;
			if (global_config->use_cached_scheduler_data){
				nengo_scheduler_cached_proc(this);
			}else {
				if (this->use_nengo_for_scheduling) {
					if (global_config->use_non_nengo_sched) {
						non_nengo_scheduler_iteration(this);
					}
					else {
						neuro_os::increment_pc(*nengo_scheduler);
						if (!global_config->precompute_nengo && !global_config->use_non_nengo_sched) {
							neuro_os::run_precompute_sim(*nengo_scheduler, 3);
						}

						precompute_nengo_queue_update();
						//find new processes to start
						for (auto model_id : this->running_models) {
							std::cout << "Starting process " << model_id << "\n";
							this->start_process(model_id);
							this->send_input_spikes(model_id, tw_now(my_lp));
						}
						for (auto model_id : waiting_models) {
							std::cout << "Interrupt process " << model_id << " \n";
							this->stop_process(model_id);
						}

						//reassign runq and waitq
						/*
					this->running_models = runq;
					this->waiting_models = waitq;
	*/
					}
				}
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

			if(model_id >= spike_files.size()){

				tw_error(TW_LOC, "E- Core sched trying to send out model id %i \n", model_id);

			}
			auto spike_r = spike_files[model_id].get_spikes_at_time(time);
			std::cout << "NEMO SCHEDULER SENDING INPUT SPIKES FOR MODEL " << model_id << " TIME: " << time_t << "\n";
			int i = 0;
			for (const auto& spk : spike_r) {
				i ++;
				auto dest = get_gid_from_core_local(spk.dest_core, spk.dest_axon);
//				if (dest >= g_tw_nlp) {
//					tw_error(TW_LOC, "CS EVENT TOO BIG: %d \n", dest);
//				}
				auto dest_t = JITTER(this->my_lp->rng) + spk.time;
				dest = rand() % (global_config->ns_cores_per_chip);
				struct tw_event* os_input_spike_evt = tw_event_new(dest, dest_t, this->my_lp);
				auto* msg = (nemo_message*)tw_event_data(os_input_spike_evt);
				msg->dest_axon = spk.dest_axon;
				msg->message_type = NEURON_SPIKE;
				//add random call count
				msg->intended_neuro_tick = spk.time;
				msg->source_core = -1;
				//				if(dest >= 1024){
				//					std::cout << "EVENT SEND > 1024 at line 278 CS\n";
				//				}
				tw_event_send(os_input_spike_evt);
//				if (config::NemoConfig::DEBUG_FLAG) {
//
//					debug_log << "msg_type,model_id,dest_time,dest_core\n";
//					debug_log << debug_csv('S', model_id, dest_t, get_gid_from_core_local(spk.dest_core, spk.dest_axon));
//				}
				if (i > 1024){
					break;
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

				if (dest_gid == this->my_lp->gid) {
					tw_printf(TW_LOC, "NOS ERROR SSTI: dest_id_f: %d, dest_gid_compute %d\n", i, dest_gid);
					tw_error(TW_LOC, "NOS Sending spike messages to itself.");
				}
				auto start_message = tw_event_new(dest_gid, start_time, this->my_lp);
				auto msg = (nemo_message*)tw_event_data(start_message);

				msg->message_type = type;
				msg->model_id = process_id;
				msg->source_core = -1;
				msg->intended_neuro_tick = (unsigned long)start_time;
				//				if(dest_gid >= 1024){
				//					std::cout << "EVENT SEND > 1024 at line 313 CS\n";
				//					std::cout  << "dest_core: " << i << "GID Calc: " << dest_gid << "\n";
				//				}

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
		void NemoCoreScheduler::precompute_nengo_queue_update() {
			std::vector<int> runq;
			std::vector<int> waitq;
			if (global_config->use_non_nengo_sched) {
				non_nengo_scheduler_iteration(this,true);

			}
			else {
				runq = neuro_os::precompute_run_q(*nengo_scheduler);
				waitq = neuro_os::precompute_wait_q(*nengo_scheduler);
				this->waiting_models = waitq;
				this->running_models = runq;
			}

		}

		void NemoCoreScheduler::sched_core_init(NemoCoreScheduler* s, tw_lp* lp) {
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
			init_scheduler_system(s, lp);
		}
		void NemoCoreScheduler::init_non_nengo_sched(NemoCoreScheduler* s, tw_lp* lp) {
			auto num_cores_in_sim = (global_config->ns_cores_per_chip * global_config->total_chips);

			s->self_contained_scheduler = std::make_unique<NemoSelfContainedScheduler>(s->schedule_mode, num_cores_in_sim, global_config->rr_time_slice,
																					   true, global_config->main_config_file, g_tw_ts_end);
			std::cout << "Self-Contained NN Scheduler enabled ";
		}

		void NemoCoreScheduler::init_scheduler_system(NemoCoreScheduler* s, tw_lp* lp) {
			if (global_config->use_cached_scheduler_data) {//cached/precompute scheduler
				//nengo_scheduler = new NemoSchedulerInterface(global_config->use_nengo_dl, num_cores_in_sim, s->schedule_mode, 1000, config::NemoConfig::main_config_file, true, false);
				s->my_lp = lp;
				nengo_scheduler_cached_proc(s);
			}else if (s->use_nengo_for_scheduling) {
				auto num_cores_in_sim = (global_config->ns_cores_per_chip * global_config->total_chips);
				//What kind of scheduler (Nengo-Sim, Nengo-Precompute, Non-Nengo(GT)

				if (global_config->use_non_nengo_sched) {//non-nengo(GT )
					init_non_nengo_sched(s, lp);
				}else {
					nengo_scheduler = new NengoInterface(global_config->use_nengo_dl, num_cores_in_sim,
														 s->schedule_mode, 1000, config::NemoConfig::main_config_file, false, true);
					if (global_config->precompute_nengo) {// precomute nengo before simulation start?
						auto precompute_time = g_tw_ts_end + 10;
						std::cout << "Precompute scheduler enabled to " << precompute_time << "\n";
						run_precompute_sim(*nengo_scheduler, int(precompute_time));
						std::cout << "Precompute done, gathering wait \n SC EPOCH: " << s->current_scheduler_time << " NG EPOCH: " << nengo_scheduler->nengo_os_iface.attr("precompute_time").cast<int>() << "\n";
					}
					else {
						run_sim_n_ticks(*nengo_scheduler, 2);
					}
				}
				s->precompute_nengo_queue_update();
			}
		}

		void NemoCoreScheduler::sched_pre_run(NemoCoreScheduler* s, tw_lp* lp) {

		}
		/**
 			* on forward events call the parent class
 			* @param s
 			* @param bf
 			* @param m
 			* @param lp
 		*/
		void NemoCoreScheduler::sched_forward_event(NemoCoreScheduler* s, tw_bf* bf, nemo::nemo_message* m, tw_lp* lp) {
			s->forward_scheduler_event(bf, m, lp);
		}
		void NemoCoreScheduler::sched_reverse_event(NemoCoreScheduler* s, tw_bf* bf, nemo::nemo_message* m, tw_lp* lp) {
			s->reverse_scheduler_event(bf, m, lp);
		}
		void NemoCoreScheduler::sched_core_commit(NemoCoreScheduler* s, tw_bf* bf, nemo::nemo_message* m, tw_lp* lp) {
			s->core_commit(bf, m, lp);
		}
		void NemoCoreScheduler::sched_core_finish(NemoCoreScheduler* s, tw_lp* lp) {
			s->debug_log.close();
			s->core_finish(lp);
		}

	}// namespace neuro_system
}// namespace nemo
