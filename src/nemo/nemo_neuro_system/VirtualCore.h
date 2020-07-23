//
// Created by Mark Plagge on 6/25/20.
//

#ifndef NEMOTNG_VIRTUALCORE_H
#define NEMOTNG_VIRTUALCORE_H
#include "./neurosynaptic_cores/NemoNeuroCoreBase.h"
#include <map>
#include <neuro_os.h>
#include <vector>
#include "../nemo_io/VirtualCoreReports.h"
/**
 * Virtual Core - The Scheduler's interface to a neurosynaptic core.
 * Manages stop and start functions
 * Cores are LPs - VirtualCores can too!
 */
namespace nemo {
	namespace neuro_system {
		struct JobMap{
			std::map<int, NemoNeuroCoreBase*> job_map;
			double active_time = 0.0;
			~JobMap();
		};

		struct CoreRCStates{

		};

		struct VirtualCore {
			void assign_core_for_job(NemoNeuroCoreBase * core, int model_id);
			std::shared_ptr<VirtualCoreReports> reporter;
			std::forward_list<std::shared_ptr<JobMap>> forward_list;
			std::shared_ptr<JobMap> current_jobs;
			bool constructed = false;
			//std::map<int, NemoNeuroCoreBase*> job_map;
			int current_model_id = 0;
			int max_model_id = 0;
			void vc_init_cores_for_models(tw_lp* lp);

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
				vcore->vc_init_cores_for_models(lp);
				vcore->reporter = std::make_shared<VirtualCoreReports>();
				vcore->reporter->vcore_id = lp->gid;

			}

			static void s_virtual_pre_run(void* s, tw_lp* lp) {

				auto vcore = VirtualCore::cast_from(s);
				for (int i = 0; i < vcore->max_model_id; i++) {
					auto core = vcore->get_core_for_job(i);
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
					if(g_tw_mynode == 0){
						tw_printf(TW_LOC, "VC CORE# %d init model ID %d ",lp->id, v->model_id);
					}
					vcore->current_model_id = v->model_id;
					auto core = vcore->get_core_for_job(v->model_id);
					core->s_forward_event(core, bf, m, lp);
				}
				else if (v->message_type == NOS_START) {
					//vcore->handle_start_message(bf, v, lp);
				}
				else if (v->message_type == NOS_STOP) {
					//vcore->handle_end_message(bf, v, lp);
				}
				else if (v->message_type == NEURON_SPIKE || v->message_type == HEARTBEAT) {
					if(g_tw_synchronization_protocol != NO_SYNCH && g_tw_synchronization_protocol != SEQUENTIAL && g_tw_synchronization_protocol != CONSERVATIVE)
						vcore->save_state(lp);
					auto core =vcore->get_core_for_job(vcore->current_model_id);
					core->s_forward_event(core, bf, m, lp);
				}else{
					tw_printf(TW_LOC,"VC CORE GOT WEIRD MESSAGE %i", v->message_type);
				}
			}
			static void s_virtual_reverse_event(void* s, tw_bf* bf, void* m, tw_lp* lp) {
				auto vcore = cast_from(s);
				auto v = (nemo_message*)m;
				if (v->message_type == NOS_START){
					//vcore->get_core_for_job(v->model_id)->reverse_start();
				}else if (v->message_type == NOS_STOP){
					//vcore->get_core_for_job(v->model_id)->reverse_stop;
				}else{
					vcore->reverse_state(lp);
				}
			}
			void commit(VirtualCore* p_core, tw_bf* p_bf, nemo_message* p_message, tw_lp* p_lp);
			static void s_virtual_core_commit(void* s, tw_bf* bf, void* m, tw_lp* lp) {
				auto vcore = VirtualCore::cast_from(s);
				auto v = (nemo_message*)m;
				if (v->message_type == NOS_START){
					vcore->handle_start_message(bf, v, lp);

				}else if (v->message_type == NOS_STOP){
					vcore->handle_end_message(bf, v, lp);
				}
				vcore->commit(vcore, bf, v, lp);
				if(v->message_type == NOS_START || v->message_type == NOS_STOP){
					add_nos_message(vcore->reporter,tw_now(lp),vcore->current_model_id,v->message_type);
				}
				//for (int i = 0; i < vcore->max_model_id; i++) {
					//void* core = vcore->job_map[i];
				//}

			}
			static void s_virtual_core_finish(void* s, tw_lp* lp) {
				auto vcore = cast_from(s);
				for (int i = 0; i < vcore->max_model_id; i++) {
					auto core = vcore->get_core_for_job(i);
					core->s_core_finish(core, lp);
				}
				save_messages(vcore->reporter);
			}
			NemoNeuroCoreBase* get_core_for_job(unsigned int model_id) const;
			void handle_start_message(tw_bf* bf, nemo_message* m, tw_lp* lp) const;
			void handle_end_message(tw_bf* bf, nemo_message* m, tw_lp* lp) const;
			void save_state(tw_lp *lp);
			void reverse_state(tw_lp *lp);
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
