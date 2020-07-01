//
// Created by Mark Plagge on 6/20/20.
//

#ifndef NEMOTNG_NEMONEURONTRUENORTH_H
#define NEMOTNG_NEMONEURONTRUENORTH_H
#include "NemoNeuronGeneric.h"
#include "TNNeuronState.h"
namespace nemo {

	namespace neuro_system {
		template<typename T>
		unsigned int DT(T x);

		/**
		 * NemoNeuronTrueNorth: A class that wraps the original nemo C implementation of the TrueNorth neuron.
		 * This class pulls from NemoNeuronGeneric so that it can be used as a drop in for nemo models.
		 * Bit widths in this version are not dynamic (ditched the typedefs)
		 * */
		class NemoNeuronTrueNorth final : public NemoNeuronGeneric {
			std::unique_ptr<TNNeuronState> ns;

		public:
			NemoNeuronTrueNorth();
			NemoNeuronTrueNorth(double membrane_pot, const std::vector<double>& weights, double leak_v, double threshold, double reset_val, tw_lp* lp);
			void integrate(unsigned int source_id) final;
			bool fire() final;
			void leak() final;
			void leak_n(int n_leaks) final;
			void reset() final;
			void init_from_json_string(std::string js_string) override;

			NemoNeuronTrueNorth(tw_lp* cur_lp, int n_id, int c_id);

		private:

			void init_ns_structures();


			bool should_fire();

			bool fire_floor_ceiling_reset();

			void stochastic_integrate(int weight);

			void numeric_leak_calc(tw_stime now);

			void post_integrate(tw_stime now, bool did_fire);

			void ringing(unsigned int old_voltage);

			bool fire_timing_check(tw_stime now);

			void neg_threshold_reset();

			void reset_normal();

			void reset_linear();

			void reset_none();

			bool over_underflow_check();
			void tn_fire();

			const int standard_num_axon_types = 4; // TN neurons have 4 weight types;
		};
		/** @} */

		/** \defgroup TN_Function_hdrs True North Function headers
		 * TrueNorth Neuron leak, integrate, and fire function forward decs.
 		* @{ */

		/** @} */
	}
}

#endif//NEMOTNG_NEMONEURONTRUENORTH_H
