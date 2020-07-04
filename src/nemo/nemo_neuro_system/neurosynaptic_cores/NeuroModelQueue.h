//
// Created by Mark Plagge on 7/4/20.
//

#ifndef NEMOTNG_NEUROMODELQUEUE_H
#define NEMOTNG_NEUROMODELQUEUE_H


#include <stdio.h>
#include <queue>
#include <map>
#include "../../nemo_globals.h"
namespace nemo{
	namespace neuro_system {
		template<typename T>
		struct NeuroCoreStateHolder {
			NeuroCoreStateHolder(const std::shared_ptr<std::vector<std::shared_ptr<T>>>& neuron_states,
								 core_types core_type, const std::vector<unsigned int>& dest_cores,
								 const std::vector<unsigned int>& dest_axons) : neuron_states(neuron_states), core_type(core_type), dest_cores(dest_cores), dest_axons(dest_axons) {}
			NeuroCoreStateHolder(const std::shared_ptr<std::vector<std::shared_ptr<T>>>& neuron_states) : neuron_states(neuron_states) {}
			std::shared_ptr<std::vector<std::shared_ptr<T>>> neuron_states;
			core_types core_type;
			std::vector<unsigned int> dest_cores;
			std::vector<unsigned int> dest_axons;


		};

		template<typename T>
		struct NeuroModelQueue {

			int size;
			std::map<int, bool> model_id_init_map;
			std::queue<std::shared_ptr<NeuroCoreStateHolder<T>>> model_states;


			std::shared_ptr<NeuroCoreStateHolder<T>> pop(){
				auto state = this->model_states.front();
				this->model_states.pop();
				return state;
			}
			void push(std::shared_ptr<NeuroCoreStateHolder<T>> state, int model_id){
				this->model_id_init_map[model_id] = true;
				model_states.push(state);
			}
			bool is_model_init(int model_id){
				return model_id_init_map[model_id];
			}

		};

	}
}

#endif//NEMOTNG_NEUROMODELQUEUE_H
