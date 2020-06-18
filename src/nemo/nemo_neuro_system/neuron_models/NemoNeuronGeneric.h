//
// Created by Mark Plagge on 11/1/19.
//

#ifndef NEMOTNG_NEMONEURONGENERIC_H
#define NEMOTNG_NEMONEURONGENERIC_H

#include "../../nemo_config/NemoConfig.h"
#include <vector>

namespace nemo {
	extern config::NemoConfig* global_config;
	namespace neuro_system {

		/**
 * Base Neuron class - contains the states for a LIF neuron
 * Base LIF neurons compute membrane potential in a standard loop function.
 * See NeMoNeuroCoreMat for matrix based integration
 *
 */
		template<typename PRC>
		class NemoNeuronGeneric {
			PRC membrane_pot = 0;
			std::vector<PRC> weights;
			PRC leak_v = -1;
			PRC threshold = 1;
			PRC reset_val = 0;

		public:
			NemoNeuronGeneric(PRC membranePot, const std::vector<PRC>& weights, PRC leakV, PRC threshold, PRC resetVal);

			virtual void
			integrate(PRC source_id)
			{
				membrane_pot = membrane_pot + weights[source_id];
			}

			virtual void leak(int n_leaks)
			{
				for (int i = 0; i < n_leaks; i++) {
					membrane_pot += leak_v;
				}
			}

			virtual bool fire()
			{
				if (membrane_pot >= threshold) {
					membrane_pot = reset_val;
					return true;
				}
				else {
					return false;
				}
			}

			virtual bool recv_hb(int n_leaks)
			{
				leak(n_leaks);
				return fire();
			}

			virtual void recv_spk(PRC source_id)
			{
				integrate(source_id);
			}
		};

	}// namespace neuro_system
}// namespace nemo
#endif//NEMOTNG_NEMONEURONGENERIC_H
