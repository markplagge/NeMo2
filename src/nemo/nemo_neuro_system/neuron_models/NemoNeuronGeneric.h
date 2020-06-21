//
// Created by Mark Plagge on 11/1/19.
//

#ifndef NEMOTNG_NEMONEURONGENERIC_H
#define NEMOTNG_NEMONEURONGENERIC_H

#include "../../nemo_config/NemoConfig.h"
#include <utility>
#include <vector>

namespace nemo {
	//extern config::NemoConfig* global_config;
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
			bool self_manage_spike_events = false;
			tw_lp *cur_lp{};


			

		public:
			unsigned  int dest_core;
			unsigned  int dest_axon;

			bool is_self_manage_spike_events() const;
			void set_self_manage_spike_events(bool self_manage_spike_events);
			tw_lp* get_cur_lp() const;
			/** @todo: Use this to get LP info for this neuron - Will be pointers to the same LP?*/
			void set_cur_lp(tw_lp* new_lp);

			NemoNeuronGeneric(double membrane_pot, std::vector<double> weights, double leak_v, double threshold, double reset_val, tw_lp *lp) : membrane_pot(membrane_pot), weights(std::move(weights)), leak_v(leak_v), threshold(threshold), reset_val(reset_val),cur_lp(lp) {}
			NemoNeuronGeneric() = default;

			virtual void
			integrate(unsigned int source_id);

			virtual void leak();
			virtual void leak_n(int n_leaks);

			virtual bool fire();
			virtual void reset();


		};

	}// namespace neuro_system
}// namespace nemo
#endif//NEMOTNG_NEMONEURONGENERIC_H
