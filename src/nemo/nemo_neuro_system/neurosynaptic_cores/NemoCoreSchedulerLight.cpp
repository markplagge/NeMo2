//
// Created by Mark Plagge on 7/25/20.
//

#include "NemoCoreSchedulerLight.h"
namespace nemo {
	namespace neuro_system {
		void NemoCoreSchedulerLight::send_nos_control_message(nemo_message_type TYPE, int offset, unsigned int dest_gid, int task_id, int model_id) {
			auto dest_time =  JITTER(my_lp->rng) + offset;
			//int dest_core = get_gid_from_core_local(i, 1);
			auto evt = (tw_event *) tw_event_new(dest_gid, dest_time, my_lp );
			auto msg = (nemo_message *) tw_event_data(evt);
			msg->message_type = TYPE;
			msg->debug_time = tw_now(my_lp);
			msg->model_id = model_id;
			msg->task_id = task_id;
			msg->random_call_count = my_lp->rng->count;
			tw_event_send(evt);
		}
		void NemoCoreSchedulerLight::scheduler_tick(tw_bf bf, nemo_message* m) {
			this->my_bf = bf;
			if (m->message_type != NOS_TICK){
				tw_error(TW_LOC, "GOT NON NOS TICK IN LIGHT SCHEDULER");
			}
			int current_time = tw_now(my_lp);
			auto starts = nos_scheduler->get_start_events(current_time);
			auto stops = nos_scheduler->get_stop_events(current_time);
			auto running = nos_scheduler->get_running_procs(current_time);
			auto stopped = nos_scheduler->get_waiting_procs(current_time);
			send_start_messages(starts);
			send_stop_messages(stops);
			for (const auto& run_evt : running){
				send_input_spikes_to_cores(run_evt.model_id,run_evt.task_id);
			}
			send_scheduler_tick();
			current_scheduler_time ++;
		}
		void NemoCoreSchedulerLight::send_start_messages(std::vector<ProcEvent> starts) {
			for (const auto& evt : starts) {

				for (unsigned int i = 0; i < num_cores_in_sim; i ++){
					auto dest_gid = get_gid_from_core_local(i, 1);
					send_nos_control_message(NOS_START, 0, dest_gid, evt.task_id,evt.model_id);
				}
			}
		}
		void NemoCoreSchedulerLight::send_stop_messages(std::vector<ProcEvent> stops) {
		}
		void NemoCoreSchedulerLight::send_input_spikes_to_cores(int model_id,int task_id) {
			auto spikes_at_tick = nos_scheduler->get_spikes_for_model_next_time(model_id);
			auto rng_start = my_lp->rng->count;
			for (const auto& spike : spikes_at_tick) {
				auto dest_core = spike.dest_core;
				auto dest_axon = spike.dest_axon;
				auto sched_time = spike.time;
				auto dest_gid = get_gid_from_core_local(dest_core, dest_axon);
				auto evt = (tw_event *) tw_event_new(dest_gid, JITTER(my_lp->rng), my_lp );
				auto msg = (nemo_message *) tw_event_data(evt);
				msg->dest_axon = dest_axon;
				msg->model_id = model_id;
				msg->message_type = NEURON_SPIKE;
				msg->source_core = 0;
				msg->debug_time = sched_time;
				auto rng_msg = my_lp->rng->count - rng_start;
				msg->random_call_count = rng_msg;
				tw_event_send(evt);

			}
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
														  global_config->main_config_file, false);
			nos_scheduler = std::move(arbiter);
			num_cores_in_sim = num_cores;
		}

		void NemoCoreSchedulerLight::sched_core_init(void* s, tw_lp* lp) {
			auto core = static_cast<NemoCoreSchedulerLight*>(s);
			new (core) NemoCoreSchedulerLight();
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
		}
		void NemoCoreSchedulerLight::sched_reverse_event(void* s, tw_bf* bf, nemo_message* m, tw_lp* lp) {
			auto core = static_cast<NemoCoreSchedulerLight*>(s);
			core->my_lp = lp;
			core->rev_scheduler_tick(bf, m);
		}
		void NemoCoreSchedulerLight::sched_core_commit(void* s, tw_bf* bf, nemo_message* m, tw_lp* lp) {
			auto core = static_cast<NemoCoreSchedulerLight*>(s);
			core->my_lp = lp;
			core->scheduler_tick(reinterpret_cast<const tw_bf&>(bf), m);
		}
		void NemoCoreSchedulerLight::sched_core_finish(void* s, tw_lp* lp) {
		}

		void NemoCoreSchedulerLight::send_scheduler_tick() {
			send_nos_control_message(NOS_TICK,1,0,0,0);
//			auto scheduler_time = JITTER(my_lp->rng) + 1;
//			struct tw_event* os_tick = tw_event_new(my_lp->gid, scheduler_time, my_lp);
//			auto* msg = (nemo_message*)tw_event_data(os_tick);
//			msg->message_type = NOS_TICK;
//			msg->debug_time = tw_now(my_lp);
//			msg->random_call_count = rng_count;
//			tw_event_send(os_tick);
		}

	}// namespace neuro_system
}// namespace nemo
