//
// Created by Mark Plagge on 6/20/20.
//

#ifndef NEMOTNG_NEMONEURONTRUENORTH_H
#define NEMOTNG_NEMONEURONTRUENORTH_H
#include "NemoNeuronGeneric.h"
namespace nemo {

	namespace neuro_system {
		/**
		 * NemoNeuronTrueNorth: A class that wraps the original nemo C implementation of the TrueNorth neuron.
		 * This class pulls from NemoNeuronGeneric so that it can be used as a drop in for nemo models.
		 * Bit widths in this version are not dynamic (ditched the typedefs)
		 */

		struct TNNeuronState{
			// 64
			tw_stime
					last_active_time;    /**< last time the neuron fired - used for calculating
                            leak and reverse functions. Should be a whole number
                            (or very close) since big-ticks happen on whole
                            numbers. */
			tw_stime last_leak_time; /**< Timestamp for leak functions. Should be a mostly
                            whole number, since this happens once per big tick.
                            */

			tw_lpid output_gid;  //!< The output GID (axon global ID) of this neuron.


			// stat_type fireCount; //!< count of this neuron's output
			stat_type rcvd_msg_count;  //!<  The number of synaptic messages received.
			stat_type sops_count;     //!<  A count for SOPS calculation

			stat_type rng_count; //!< Random number generator count

			// 32
			unsigned int membrane_potential;  //!< current "voltage" of neuron, \f$V_j(t)\f$.
			//!Since this is PDES, \a t is implicit
			unsigned int pos_threshold;     //!< neuron's threshold value 𝛼
			unsigned int neg_threshold;     //!< neuron's negative threshold, 𝛽

			// 16
			unsigned int dendrite_local;  //!< Local ID of the remote dendrite -- not LPID, but
			//!a local axon value (0-i)
			unsigned int drawn_random_number;  //!<When activated, neurons draw a new random
			//!number. Reset after every big-tick as
			//!needed.
			unsigned int threshold_prn_mask; /**!< The neuron's random threshold mask - used for
                      *randomized thresholds ( \f$M_j\f$ ).
                      *	In the TN hardware this is defined as a ones maks of
                      *configurable width, starting at the
                      * least significant bit. The mask scales the range of the
                      *random drawn number (PRN) of the model,
                      * here defined as @link drawnRandomNumber @endlink. used
                      *as a scale for the random values. */

			unsigned int my_core_id;  //!< Neuron's coreID

			unsigned int my_local_id;  //!< my local ID - core wise. In a 512 size core, neuron 0
			//!would have a local id of 262,657.

			// small
			short largest_random_value;
			short lambda;  //!< leak weight - \f$𝜆\f$ Leak tuning parameter - the leak
			//!rate applied to the current leak function.
			short reset_mode;     //!<Gamma or resetMode. 𝛾 Reset mode selection. Valid options are 0,1,2 .

			short reset_voltage;  //!< Reset voltage for reset params, \f$R\f$.
			short sigma_vr;           //!< reset voltage - reset voltage sign
			short encoded_reset_voltage;  //!< encoded reset voltage - VR.
			short omega;                //!<temporary leak direction variable

			//char *neuronTypeDesc;  //!< a debug tool, contains a text desc of the neuron.
			short sigma_l;          //!< leak sign bit - eqiv. to σ
			unsigned int delay_val;  //!<@todo: Need to fully implement this - this value
			//!is between 1 and 15, a "delay" of n timesteps of a
			//!neuron. -- outgoing delay //from BOOTCAMP!

			//@TODO - convert this to a bitfield for bools. Check ROSS BF implementation

			bool fired_last;
			bool heartbeat_out;
			bool is_self_firing;
			bool is_output_neuron; //Is this an output neruon?
			bool epsilon;  //!<epsilon function - leak reversal flag. from the paper this
			//!changes the function of the leak from always directly being
			//!integrated (false), or having the leak directly integrated
			//!when membrane potential is above zero, and the sign is
			//!reversed when the membrane potential is below zero.
			bool c;        //!< leak weight selection. If true, this is a stochastic leak
			//!function and the \a leakRateProb value is a probability, otherwise
			//!it is a leak rate.
			bool kappa;  //!<Kappa or negative reset mode. From the paper's ,\f$𝜅_j\f$,
			//!negative threshold setting to reset or saturate
			bool can_generate_spontanious_spikes;

			bool is_active_neuron; /**!< If true, this neuron is an inactive one in the
						  simulation */


			int fire_count;
			tw_stime last_fire;
			long output_neuron_dest;
			long output_core_dest;


			std::vector<int> axon_types;
			std::vector<int> synaptic_weight;
			std::vector<bool> synaptic_connectivity;

			/** stochastic weight mode selection. $b_j^{G_i}$ */
			std::vector<bool> weight_selection;

//			int axonTypes[AXONS_IN_CORE];
//			int synapticWeight[NUM_NEURON_WEIGHTS];
//			bool synapticConnectivity[AXONS_IN_CORE];  //!< is there a connection between axon i and
//			//!neuron j?

//			bool weightSelection[NUM_NEURON_WEIGHTS];
		};

		class NemoNeuronTrueNorth final : public NemoNeuronGeneric {
			TNNeuronState * ns;


		public:


			void integrate(unsigned int source_id) final;
			bool fire() final;
			void leak() final;
			void leak_n(int n_leaks) final;
			void reset() final;
			void init_from_json_string(std::string js_string) override;


		private:




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
		};

		/** \defgroup TN_Function_hdrs True North Function headers
		 * TrueNorth Neuron leak, integrate, and fire function forward decs.
 		* @{ */






		/** @} */
	}
}

#endif//NEMOTNG_NEMONEURONTRUENORTH_H
