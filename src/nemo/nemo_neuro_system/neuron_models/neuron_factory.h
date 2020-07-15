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


		NemoNeuronGeneric *get_new_neuron(core_types neuron_type, tw_lp *lp, int n_id, int core_id) {
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
	}
}


#endif//NEMOTNG_NEURON_FACTORY_H
