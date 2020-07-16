//
// Created by Mark Plagge on 6/26/20.
//

#ifndef NEMOTNG_NEURON_FACTORY_H
#define NEMOTNG_NEURON_FACTORY_H
#include "NemoNeuronGeneric.h"
#include "NemoNeuronTrueNorth.h"
#include "../../nemo_globals.h"
namespace nemo {
	namespace neuro_system {
		template<typename T>
		T item() {
			return T();
		}

		template<>
		float item<float>() {
			return 1.0f;
		}


		NemoNeuronGeneric * get_neuron_ptr (core_types neuron_type, tw_lp *lp, int n_id, int core_id) {
			NemoNeuronGeneric *neuron;
			switch (neuron_type) {
			case NO_CORE_TYPE:
			case LIF:
				neuron = new NemoNeuronGeneric(lp);
				break;
			case TN:

				neuron = new NemoNeuronTrueNorth(lp,n_id, core_id);
				break;
			}

			return neuron;
		}
		template<typename NEURON_TYPE>
		NEURON_TYPE get_new_neuron(core_types neuron_type, tw_lp *lp, int n_id, int core_id){
			return get_neuron_ptr(neuron_type, lp, n_id, core_id);
		}

		template<>
		std::shared_ptr<NemoNeuronGeneric>get_new_neuron<std::shared_ptr<NemoNeuronGeneric>>(core_types neuron_type, tw_lp *lp, int n_id, int core_id){
			return std::shared_ptr<NemoNeuronGeneric>( get_neuron_ptr(neuron_type,lp,n_id,core_id) );
		}
		template<>
		std::unique_ptr<NemoNeuronGeneric> get_new_neuron<std::unique_ptr<NemoNeuronGeneric>>(core_types neuron_type, tw_lp *lp, int n_id, int core_id) {
			return std::unique_ptr<NemoNeuronGeneric>(get_neuron_ptr(neuron_type,lp,n_id,core_id) );
			NemoNeuronGeneric *neuron;

			switch (neuron_type) {
			case NO_CORE_TYPE:
			case LIF:
				neuron = new NemoNeuronGeneric(lp);
				break;
			case TN:

				neuron = new NemoNeuronTrueNorth(lp,n_id, core_id);
				break;
			}


			return std::unique_ptr<NemoNeuronGeneric>(neuron);
		}
	}
}


#endif//NEMOTNG_NEURON_FACTORY_H
