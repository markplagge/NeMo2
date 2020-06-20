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

		class NemoNeuronGeneric {
			double membrane_pot = 0;
			std::vector<double> weights;
			double leak_v = -1;
			double threshold = 1;
			double reset_val = 0;

		public:
			NemoNeuronGeneric(double membrane_pot, const std::vector<double>& weights, double leak_v, double threshold, double reset_val) : membrane_pot(membrane_pot), weights(weights), leak_v(leak_v), threshold(threshold), reset_val(reset_val) {}
			NemoNeuronGeneric() {}

			virtual void
			integrate(double source_id) {
				membrane_pot = membrane_pot + weights[source_id];
			}

			virtual void leak(int n_leaks) {
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

			virtual bool recv_hb(int n_leaks) {
				leak(n_leaks);
				return fire();
			}

			virtual void recv_spk(double source_id) {
				integrate(source_id);
			}
		};

	}// namespace neuro_system
}// namespace nemo
#endif//NEMOTNG_NEMONEURONGENERIC_H
