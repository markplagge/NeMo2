//
// Created by Mark Plagge on 7/25/20.
//

#include "NemoCoreSchedulerLight.h"
#include <nemo_build_options.h>
namespace nemo {
	namespace neuro_system {

		void NemoCoreSchedulerLight::send_nos_control_message(nemo_message_type message_type, double offset,
															  unsigned int dest_gid, int task_id, int model_id) {

			while (g_tw_synchronization_protocol == CONSERVATIVE && offset < g_tw_lookahead) {
				offset += .1;
			}
			auto dest_time = JITTER(my_lp->rng) + offset;
			//int dest_core = get_gid_from_core_local(i, 1);
			auto evt = (tw_event*)tw_event_new(dest_gid, dest_time, my_lp);
			auto msg = (nemo_message*)tw_event_data(evt);
			msg->message_type = message_type;
			msg->debug_time = tw_now(my_lp);
			msg->model_id = model_id;
			msg->task_id = task_id;
			msg->random_call_count = my_lp->rng->count;
			if (dest_gid > global_config->ns_cores_per_chip) {
				tw_error(TW_LOC, "SEND FROM SCHEDULER TO NON-EXIST CHIP");
			}
			tw_event_send(evt);
#if NEMO_DEBUG()
			this->set_dbg_dest_gid(dest_gid);

			save_control_event(message_type);
			this->set_dbg_dest_gid(-1);
#endif
		}
		void NemoCoreSchedulerLight::scheduler_tick(tw_bf* bf, nemo_message* m) {
			static int announce = 0;
			this->my_bf = bf;
			if (m->message_type != NOS_TICK) {
				tw_error(TW_LOC, "GOT NON NOS TICK IN LIGHT SCHEDULER");
			}
			int current_time = tw_now(my_lp);
			auto starts = nos_scheduler->get_start_events(current_time);
			auto stops = nos_scheduler->get_stop_events(current_time);
			auto running = nos_scheduler->get_running_procs(current_time);
			auto stopped = nos_scheduler->get_waiting_procs(current_time);
			send_start_messages(starts);
			send_stop_messages(stops);
			//individual check files to ensure correct run
			auto now = tw_now(my_lp);
			auto nos_end = BuildOptions::nos_max_time;
			if(now<= nos_end  ) {
				for (const auto& run_evt : running) {
					send_input_spikes_to_cores(run_evt.model_id, run_evt.task_id);
				}
			}else if (announce == 0){
				std::cout <<"Scheduler at time " << current_scheduler_time << " stopped.\n";
				announce = 1;
			}
			send_scheduler_tick();
			current_scheduler_time++;
		}
		void NemoCoreSchedulerLight::send_start_messages(const std::vector<ProcEvent>& starts) {
			for (const auto& evt : starts) {

				for (unsigned int i = 0; i < num_cores_in_sim - 1; i++) {
					auto dest_gid = get_gid_from_core_local(i, 1);
					send_nos_control_message(NOS_START, 0.9, dest_gid, evt.task_id, evt.model_id);
				}
			}
		}
		void NemoCoreSchedulerLight::send_stop_messages(const std::vector<ProcEvent>& stops) {
		}

		void NemoCoreSchedulerLight::send_input_spikes_to_cores(int model_id, int task_id) {
			auto spikes_at_tick = nos_scheduler->get_spikes_for_model_next_time(model_id);
			auto rng_start = my_lp->rng->count;

			unsigned int num_spikes_sent = 0;

			std::map<int, std::vector<FileSpike>> spike_holder;
			int sph_init = 0;
			for (const auto& spike : spikes_at_tick) {

				spike_holder[spike.dest_core].push_back(spike);
			}
			for (const auto& spike_dat : spike_holder) {
				auto dest_core = spike_dat.first;
				auto spike_list = spike_dat.second;
				auto dest_gid = get_gid_from_core_local(dest_core, 1);
				if (dest_gid == 3092){
					std::cout << "CAME FROM 89\n";
				}
				double offset = .01;
				if (g_tw_synchronization_protocol == CONSERVATIVE) {
					offset += g_tw_lookahead;
				}
				auto evt = (tw_event*)tw_event_new(dest_gid, JITTER(my_lp->rng) + offset, my_lp);
				auto msg = (nemo_message*)tw_event_data(evt);
				msg->model_id = model_id;
				msg->task_id = task_id;
				msg->message_type = NOS_SPIKE;
				msg->dest_axon = 0;
				msg->source_core = 0;
				msg->debug_time = current_scheduler_time;
				int cpos = 0;
				for (const auto& spike : spike_list) {
					auto dest_axon = spike.dest_axon;
					msg->spike_dest_axons[cpos] = dest_axon;
					cpos += 1;
					if (cpos == G_GROUPED_SPIKE_NUM) {
						tw_error(TW_LOC, "grouped spikes too small...");
					}
				}
				msg->num_grp_spikes = cpos;
				auto rng_msg = my_lp->rng->count - rng_start;
				msg->random_call_count = rng_msg;
				if (dest_gid > global_config->ns_cores_per_chip) {
					tw_error(TW_LOC, "SEND FROM SCHEDULER TO NON-EXIST CHIP");
				}
				tw_event_send(evt);
				num_spikes_sent++;
				if (num_spikes_sent >= g_tw_events_per_pe - 100) {
					break;
				}
			}

#if NEMO_DEBUG()
			this->save_input_spike_range(model_id, task_id, num_spikes_sent);
			std::cout << "Scheduler sent " << num_spikes_sent <<" spikes at time " << tw_now(my_lp) << "\n";
#endif
		}
		void NemoCoreSchedulerLight::rev_scheduler_tick(tw_bf* bf, nemo_message* m) {
		}
		void NemoCoreSchedulerLight::rev_send_start_messages() {
		}
		void NemoCoreSchedulerLight::rev_send_stop_messages() {
		}
		void NemoCoreSchedulerLight::rev_send_input_spikes_to_cores() {
		}

		void NemoCoreSchedulerLight::init_models_and_scheduler() {
			int model_counter = 0;

			auto l_task_list = global_config->scheduler_inputs;
			auto l_model_list = global_config->models;
			std::map<int, config::NemoModel> running_models;
			int max_models = -1;
			if (!global_config->do_neuro_os) {
				max_models = 1;
			}
			for (const auto& model : l_model_list) {
				if (max_models > 0 and max_models <= model_counter) {
					break;//this should run only once if we are not runing OS
				}
				auto model_id = model.id;
				running_models.emplace(model_id, model);
				if (model.model_file_path.length() != 0) {
					int rank;
					MPI_Comm_rank(MPI_COMM_WORLD, &rank);
					if (g_tw_mynode == 0 && rank == 0)
						tw_printf(TW_LOC, "Loading Model #%i: %s \n", model_counter, model.model_file_path.c_str());
					auto model_file = ModelFile(model.model_file_path);
					if (g_tw_mynode == 0 && rank == 0)
						tw_printf(TW_LOC, "Loading Spike #%i: %s \n", model_counter, model.spike_file_path.c_str());
					auto spike_file = SpikeFile(model.spike_file_path);
					model_files.emplace(model_id, model_file);
					spike_files.emplace(model_id, spike_file);
				}
				else {
					tw_printf(TW_LOC, "Loading benchmark model %s \n", model.benchmark_model_name.c_str());
					ModelFile mf;
					SpikeFile sf;
					model_files.emplace(model_id, mf);
					spike_files.emplace(model_id, sf);
				}
				model_counter += 1;
			}
			/* INIT the python scheduler algorithm interface */
			unsigned int sched_mode = neuro_os::SC_BASE_INIT;
			if (global_config->do_neuro_os) {
				if (global_config->scheduler_type == config::RR) {
					sched_mode |= neuro_os::SC_MD_RR;
				}
				else {
					sched_mode |= neuro_os::SC_MD_FCFS;
				}
				if (global_config->use_cached_scheduler_data) {
					sched_mode |= neuro_os::SC_MD_CACHED;
				}
				if (global_config->use_non_nengo_sched) {
					sched_mode |= neuro_os::SC_MD_CONVENT;
				}
				else {
					sched_mode |= neuro_os::SC_MD_NENGO;
				}
				if (global_config->use_nengo_dl) {
					sched_mode |= SC_MD_USE_NENGO_DL;
				}
			}
			else {
				tw_error(TW_LOC, "NEURO OS SHOULD BE ENABLED");
			}
			auto num_cores = global_config->ns_cores_per_chip * global_config->total_chips;
			auto arbiter = new neuro_os::NemoNosScheduler(static_cast<scheduler_mode>(sched_mode),
														  global_config->rr_time_slice, num_cores,
														  config::NemoConfig::main_config_file, false);
			auto init_models = arbiter->get_waiting_procs();
			for (const auto& model : init_models) {
				for (unsigned int i = 0; i < num_cores_in_sim; i++) {
					if(i == 3092){
						std::cout << " 209 happened \n";
					}else {
						auto init_event = tw_event_new(i + 1, JITTER(my_lp->rng) + .9, my_lp);
						auto message = (nemo_message*)tw_event_data(init_event);
						message->model_id = model.model_id;
						message->message_type = NOS_LOAD_MODEL;

						tw_event_send(init_event);
					}

				}
			}
			nos_scheduler = arbiter;
			num_cores_in_sim = num_cores;
		}

		void NemoCoreSchedulerLight::sched_core_init(void* s, tw_lp* lp) {
			auto core = static_cast<NemoCoreSchedulerLight*>(s);
			new (core) NemoCoreSchedulerLight();
#if NEMO_DEBUG()
			std::cout << "SYSTEM DEBUG ENABLED \n";
			core->start_debug_io("scheduler_core_debug.csv", "", "SCHEDULER_CORE");
#endif

			if (g_tw_mynode != 0) {
				tw_error(TW_LOC, "Neuromorphic scheduler core was init'ed from non 0 pe: PEID: %i", g_tw_mynode);
			}
			core->my_lp = lp;
			core->send_scheduler_tick();
			core->init_models_and_scheduler();
		}

		void NemoCoreSchedulerLight::sched_pre_run(void* s, tw_lp* lp) {
		}
		void NemoCoreSchedulerLight::sched_forward_event(void* s, tw_bf* bf, nemo_message* m, tw_lp* lp) {
			auto core = static_cast<NemoCoreSchedulerLight*>(s);
			core->my_lp = lp;
			core->scheduler_tick(bf, m);
		}
		void NemoCoreSchedulerLight::sched_reverse_event(void* s, tw_bf* bf, nemo_message* m, tw_lp* lp) {
			auto core = static_cast<NemoCoreSchedulerLight*>(s);
			core->my_lp = lp;
			core->rev_scheduler_tick(bf, m);
		}
		void NemoCoreSchedulerLight::sched_core_commit(void* s, tw_bf* bf, nemo_message* m, tw_lp* lp) {
			//			auto core = static_cast<NemoCoreSchedulerLight*>(s);
			//			core->my_lp = lp;
			//			core->scheduler_tick(reinterpret_cast<const tw_bf&>(bf), m);
		}
		void NemoCoreSchedulerLight::sched_core_finish(void* s, tw_lp* lp) {
#if NEMO_DEBUG()
			auto core = static_cast<NemoCoreSchedulerLight*>(s);
			core->end_debug_io();
#endif
		}

		void NemoCoreSchedulerLight::send_scheduler_tick() {
			send_nos_control_message(NOS_TICK, 1, 0, 0, 0);
			//			auto scheduler_time = JITTER(my_lp->rng) + 1;
			//			struct tw_event* os_tick = tw_event_new(my_lp->gid, scheduler_time, my_lp);
			//			auto* msg = (nemo_message*)tw_event_data(os_tick);
			//			msg->message_type = NOS_TICK;
			//			msg->debug_time = tw_now(my_lp);
			//			msg->random_call_count = rng_count;
			//			tw_event_send(os_tick);
		}
		unsigned long NemoCoreSchedulerLight::get_dbg_dest_gid() const {
			return _dbg_dest_gid;
		}
		void NemoCoreSchedulerLight::set_dbg_dest_gid(unsigned long dbg_dest_gid) {
			_dbg_dest_gid = dbg_dest_gid;
		}

		/** Debug functions below */
		constexpr const char* proc_event_str(uint16_t event_type) {
			switch (event_type) {
			case SCH_INTERRUPT:
				return "INTERR";
			case SCH_PRE_WAIT:
				return "PRE_WAIT";
			case SCH_DONE:
				return "DONE";
			case SCH_RUNNING:
				return "RUNNING";
			case SCH_START_RUNNING:
				return "START_RUNNING";
			case SCH_START_WAIT:
				return "START_WAIT";
			case SCH_WAIT:
				return "WAIT";
			default:
				return "DEFT";
			}
		}
		nemo_message_type x;
#define X(a) #a,
		static const char* nemo_message_type_strings[] = {NEMO_MESSAGE_TYPES};
#undef X

		void NemoCoreSchedulerLight::i_end_debug_io() {
		}
		void NemoCoreSchedulerLight::i_start_debug_io() {
			this->output_file << "LIGHT SCHED DEBUG \n"
							  << "TIME,EVENT_TYPE,EVENT_DEST_GID,MODEL_ID,TASK_ID\n";
		}

		template<>
		void NemoCoreSchedulerLight::i_save_control_event<ProcEvent>(ProcEvent event) {
			auto msg = util::assemble_csv_elements(tw_now(my_lp), proc_event_str(event.event_type),
												   get_dbg_dest_gid(), event.model_id, event.task_id);
			this->output_file << msg << "\n";
			//			this->output_file << tw_now(my_lp) <<"," << proc_event_str(event.event_type) << ","
		}
		template<>
		void NemoCoreSchedulerLight::i_save_control_event<nemo_message_type>(nemo::nemo_message_type event) {
			auto msg = util::assemble_csv_elements(tw_now(my_lp), nemo_message_type_strings[event],
												   get_dbg_dest_gid(), 0, 0);
			this->output_file << msg << "\n";
		}
		//		template<typename EVT_TYPE>
		//		void NemoCoreSchedulerLight::i_save_control_event(EVT_TYPE event) {
		//			auto msg = util::assemble_csv_elements(event);
		//			msg = "UNK_TP:," + msg;
		//			this->output_file << msg << "\n";
		//		}
		template<>
		void NemoCoreSchedulerLight::i_save_control_event<nemo_message*>(nemo_message* event) {
			auto msg = util::assemble_csv_elements(tw_now(my_lp), nemo_message_type_strings[event->message_type],
												   get_dbg_dest_gid(), event->model_id, event->task_id);
			this->output_file << msg << "\n";
		}
		void NemoCoreSchedulerLight::i_save_spike_record(int model_id, int task_id) {
			auto msg = util::assemble_csv_elements(tw_now(my_lp), "INPUT_SPIKE_SEND", get_dbg_dest_gid(), model_id, task_id);
			this->output_file << msg << "\n";
		}
		void NemoCoreSchedulerLight::i_save_input_spike_range(int model_id, int task_id, int num_spikes_sent) {
			auto msg = util::assemble_csv_elements(tw_now(my_lp), "INPUT_SPIKE_GRP-TOTALM:", num_spikes_sent, model_id, task_id);
			this->output_file << msg << "\n";
		}

	}// namespace neuro_system
}// namespace nemo
