//
// Created by Mark Plagge on 6/25/20.
//

#include "VirtualCore.h"
namespace nemo {
	namespace neuro_system {
		void VirtualCore::assign_core_for_job(NemoNeuroCoreBase * core, int model_id){
			this->forward_list.front()->job_map[model_id] = core;
		}
		NemoNeuroCoreBase* VirtualCore::get_core_for_job(unsigned int model_id){

			return this->forward_list.front()->job_map[model_id];
		}
		void VirtualCore::handle_start_message(tw_bf* bf, nemo_message* m, tw_lp* lp) {
			auto model_id = m->model_id;
			auto core = get_core_for_job(model_id);
			core->forward_event(bf, m, lp);
		}
		void VirtualCore::handle_end_message(tw_bf* bf, nemo_message* m, tw_lp* lp) {
			auto model_id = m->model_id;
			auto core = get_core_for_job(model_id);
			core->forward_event(bf, m, lp);
		}
		void VirtualCore::save_state(tw_lp* lp) {
			this->forward_list.emplace_front(this->forward_list.front());
		}
		void VirtualCore::reverse_state(tw_lp* lp) {
			auto e = this->forward_list.front();
			 this->forward_list.pop_front();
			 delete[](e);
		}
		void VirtualCore::commit(VirtualCore* p_core, tw_bf* p_bf, nemo_message* p_message, tw_lp* p_lp) {
			forward_list.clear();
		}
	}// namespace neuro_system
}// namespace nemo