//
// Created by Mark Plagge on 6/25/20.
//

#ifndef NEMOTNG_VIRTUALCORE_H
#define NEMOTNG_VIRTUALCORE_H
#include "./neurosynaptic_cores/NemoNeuroCoreBase.h"
#include <map>
#include <neuro_os.h>
#include <vector>
/**
 * Virtual Core - The Scheduler's interface to a neurosynaptic core.
 * Manages stop and start functions
 * Cores are LPs - VirtualCores can too!
 */
namespace nemo {
	namespace neuro_system {

		struct VirtualCore {

			std::map<int, NemoNeuroCoreBase*> job_map;
			int current_model_id = 0;
			int max_model_id = 0;

			static VirtualCore* cast_from(void* s) {
				return static_cast<VirtualCore*>(s);
			}
			static VirtualCore* new_from(void* s) {
				auto c = VirtualCore::cast_from(s);
				new (c) VirtualCore();
				return c;
			}

			static void s_virtual_core_init(void* s, tw_lp* lp) {
				auto vcore = VirtualCore::new_from(s);
				int mx_model_id = 0;
				for (const auto& model : global_config->models) {
					auto model_id = model.id;
					// create a generic core:
					auto core = new NemoNeuroCoreBase();
					NemoNeuroCoreBase::s_core_init_from_vcore(core, lp, model_id);
					// create a corresponding virtual process
					vcore->job_map[model_id] = core;
					mx_model_id += 1;
				}

				vcore->max_model_id = mx_model_id;
			}

			static void s_virtual_pre_run(void* s, tw_lp* lp) {
				auto vcore = VirtualCore::cast_from(s);
				for (int i = 0; i < vcore->max_model_id; i++) {
					auto core = vcore->job_map[i];
					core->pre_run(lp);
				}
			}
			/**
			  * Intercept the forward event from the scheduler, and send it to the right model/core in the map.
			  * @param s
			  * @param bf
			  * @param m
			  * @param lp
			  */
			static void s_virtual_forward_event(void* s, tw_bf* bf, void* m, tw_lp* lp) {
				auto vcore = cast_from(s);
				auto v = (nemo_message*)m;
				if (v->message_type == NOS_LOAD_MODEL) {
					vcore->current_model_id = v->model_id;
					auto core = vcore->job_map[v->model_id];
					core->s_forward_event(core, bf, m, lp);
				}
				else if (v->message_type == NOS_START) {
					vcore->handle_start_message(bf, v, lp);
				}
				else if (v->message_type == NOS_STOP) {
					vcore->handle_end_message(bf, v, lp);
				}
				else {
					auto core = vcore->job_map[vcore->current_model_id];
					core->s_forward_event(core, bf, m, lp);
				}
			}
			static void s_virtual_reverse_event(void* s, tw_bf* bf, void* m, tw_lp* lp) {
			}
			static void s_virtual_core_commit(void* s, tw_bf* bf, void* m, tw_lp* lp) {
				auto vcore = VirtualCore::cast_from(s);
				for (int i = 0; i < vcore->max_model_id; i++) {
					void* core = vcore->job_map[i];
				}
			}
			static void s_virtual_core_finish(void* s, tw_lp* lp) {
				auto vcore = cast_from(s);
				for (int i = 0; i < vcore->max_model_id; i++) {
					auto core = vcore->job_map[i];
					core->s_core_finish(core, lp);
				}
			}
			void handle_start_message(tw_bf* bf, nemo_message* m, tw_lp* lp);
			void handle_end_message(tw_bf* bf, nemo_message* m, tw_lp* lp);
			//given a core def (from modelfile), and
			/*
	 * original:
	 * core_x,neuron_n  -> core_y, neuron_y
	 * virtualCore:
	 * core_z = core_y
	 */

			unsigned int core_translation(unsigned int process_dest_core, unsigned int process_id);
		};

	}// namespace neuro_system
}// namespace nemo

#endif//NEMOTNG_VIRTUALCORE_H
