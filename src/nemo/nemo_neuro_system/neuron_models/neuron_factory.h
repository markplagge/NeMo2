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


		NemoNeuronGeneric *get_new_neuron(core_types neuron_type) {
			NemoNeuronGeneric *neuron;
			switch (neuron_type) {
			case LIF:
				neuron = new NemoNeuronGeneric();
				break;
			case TN:
				neuron = new NemoNeuronTrueNorth();
				break;
			}

			return neuron;
		}
	}
}


#endif//NEMOTNG_NEURON_FACTORY_H
