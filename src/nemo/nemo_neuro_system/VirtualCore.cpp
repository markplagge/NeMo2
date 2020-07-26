//
// Created by Mark Plagge on 6/25/20.
//

#include "VirtualCore.h"
namespace nemo {
	namespace neuro_system {
		void VirtualCore::assign_core_for_job(NemoNeuroCoreBase * core, int model_id){

			if(!constructed){
				this->current_jobs = std::make_shared<JobMap>();
				constructed = true;
			}
			this->current_jobs->job_map[model_id] = core;
			//this->forward_list.front()->job_map[model_id] = core;

		}
		NemoNeuroCoreBase* VirtualCore::get_core_for_job(unsigned int model_id) const{

			//return this->forward_list.front()->job_map[model_id];
			return this->current_jobs->job_map[model_id];
		}
		void VirtualCore::handle_start_message(tw_bf* bf, nemo_message* m, tw_lp* lp) const {
			auto model_id = m->model_id;
			auto core = get_core_for_job(model_id);
			core->forward_event(bf, m, lp);
		}
		void VirtualCore::handle_end_message(tw_bf* bf, nemo_message* m, tw_lp* lp) const {
			auto model_id = m->model_id;
			auto core = get_core_for_job(model_id);
			core->forward_event(bf, m, lp);
		}
		void VirtualCore::save_state(tw_lp* lp) {
			this->forward_list.emplace_front(this->current_jobs);
		}
		void VirtualCore::reverse_state(tw_lp* lp) {
			auto e = this->forward_list.front();
			 this->forward_list.pop_front();
			 this->current_jobs = e;
		}
		void VirtualCore::commit(VirtualCore* p_core, tw_bf* p_bf, nemo_message* p_message, tw_lp* p_lp) {

			forward_list.clear();

		}
		void VirtualCore::vc_init_cores_for_models(tw_lp* lp) {
			int mx_model_id = 0;

			for (const auto& model : global_config->models) {
				auto model_id = model.id;
				// create a generic core:
				auto core = new NemoNeuroCoreBase();
				NemoNeuroCoreBase::s_core_init(core, lp);
				NemoNeuroCoreBase::s_core_init_from_vcore(core, lp, model_id);
				// create a corresponding virtual process
				assign_core_for_job(core, model_id);
				//vcore->job_map->job_map[model_id] = core;
				mx_model_id += 1;
			}
			max_model_id = mx_model_id;
		}
		JobMap::~JobMap() {
			for (const auto& item : job_map) {
				auto core = item.second;
				delete(core);
			}

		}
	}// namespace neuro_system
}// namespace nemo