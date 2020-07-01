//
// Created by Mark Plagge on 6/30/20.
//

#ifndef NEMOTNG_TNNEURONSTATE_H
#define NEMOTNG_TNNEURONSTATE_H
#include "../../nemo_globals.h"
#include <ross.h>
namespace nemo {
	namespace neuro_system {

/** TODO: Eventually replace this with generic macro and non-branching ABS code. */
#define IABS(a) (((a) < 0) ? (-a) : (a))//!< Typeless integer absolute value function
/** TODO: See if there is a non-branching version of the signum function, maybe in MAth libs and use that. */
#define SGN(x) ((x > 0) - (x < 0))//!< Signum function
		//#define DT(x) !(x)                    //!<Kronecker Delta function.

#define BINCOMP(s, p) IABS((s)) >= (p)//!< binary comparison for conditional stochastic evaluation

		/**
		 * @defgroup TN_System
		 * True North Neurons
		 * All TN functionality, states, etc.
		 * @{
		 */
		/**
		 * TNNeuronState - Original neuron state from $\text{NeMo}_1$
		 */
		struct TNNeuronState {

			// 64
			tw_stime
					last_active_time = 0; /**< last time the neuron fired - used for calculating
                            leak and reverse functions. Should be a whole number
                            (or very close) since big-ticks happen on whole
                            numbers. */
			tw_stime last_leak_time = 0;  /**< Timestamp for leak functions. Should be a mostly
                            whole number, since this happens once per big tick.
                            */

			tw_lpid output_gid = 0;//!< The output GID (axon global ID) of this neuron.

			uint64_t  fire_count = 0;//!< count of this neuron's output
			stat_type rcvd_msg_count = 0;//!<  The number of synaptic messages received.
			stat_type sops_count = 0;    //!<  A count for SOPS calculation

			stat_type rng_count = 0;//!< Random number generator count

			// 32
			unsigned int membrane_potential = 0;//!< current "voltage" of neuron, \f$V_j(t)\f$.
			//!Since this is PDES, \a t is implicit
			unsigned int pos_threshold = 0;//!< neuron's threshold value 𝛼
			unsigned int neg_threshold = 0;//!< neuron's negative threshold, 𝛽

			// 16
			unsigned int dendrite_local;//!< Local ID of the remote dendrite -- not LPID, but
			//!a local axon value (0-i)
			unsigned int drawn_random_number = 0;//!<When activated, neurons draw a new random
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

			unsigned int my_core_id;//!< Neuron's coreID

			unsigned int my_local_id;//!< my local ID - core wise. In a 512 size core, neuron 0
			//!would have a local id of 262,657.

			// small
			short largest_random_value;
			short lambda;//!< leak weight - \f$𝜆\f$ Leak tuning parameter - the leak
			//!rate applied to the current leak function.
			short reset_mode;//!<Gamma or resetMode. 𝛾 Reset mode selection. Valid options are 0,1,2 .

			short reset_voltage;        //!< Reset voltage for reset params, \f$R\f$.
			short sigma_vr;             //!< reset voltage - reset voltage sign
			short encoded_reset_voltage;//!< encoded reset voltage - VR.
			short omega;                //!<temporary leak direction variable

			//char *neuronTypeDesc;  //!< a debug tool, contains a text desc of the neuron.
			short sigma_l;         //!< leak sign bit - eqiv. to σ
			unsigned int delay_val;//!<@todo: Need to fully implement this - this value
			//!is between 1 and 15, a "delay" of n timesteps of a
			//!neuron. -- outgoing delay //from BOOTCAMP!

			//@TODO - convert this to a bitfield for bools. Check ROSS BF implementation

			bool fired_last = false;
			bool heartbeat_out =false ;
			bool is_self_firing;
			bool is_output_neuron;//Is this an output neruon?
			bool epsilon;         //!<epsilon function - leak reversal flag. from the paper this
			//!changes the function of the leak from always directly being
			//!integrated (false), or having the leak directly integrated
			//!when membrane potential is above zero, and the sign is
			//!reversed when the membrane potential is below zero.
			bool c;//!< leak weight selection. If true, this is a stochastic leak
			//!function and the \a leakRateProb value is a probability, otherwise
			//!it is a leak rate.
			bool kappa;//!<Kappa or negative reset mode. From the paper's ,\f$𝜅_j\f$,
			//!negative threshold setting to reset or saturate
			bool can_generate_spontanious_spikes;

			bool is_active_neuron; /**!< If true, this neuron is an inactive one in the
						  simulation */


			tw_stime last_fire = 0;
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
			/* Member Functions Added below data */
			TNNeuronState();
		};

		/** Stand Alone constructor / Init a new neuron. assumes that the reset voltage is NOT
		 *  encoded (i.e., a reset value of -5 is allowed. Sets reset voltage sign from input reset voltage).
		 */
		void tn_create_neuron(unsigned int core_id, unsigned int n_id,
							  bool synaptic_connectivity[],
							  short G_i[4], short sigma[4], short S[4],
							  bool b[4], bool epsilon, short sigma_l, short lambda,
							  bool c, uint32_t alpha, uint32_t beta, short TM, short VR,
							  short sigma_vr, short gamma, bool kappa,
							  TNNeuronState* n, int signal_delay,
							  uint64_t dest_global_id, int dest_axon_id);

		void TN_set_neuron_dest(int signal_delay, uint64_t gid, TNNeuronState* n);
		void tn_create_neuron_encoded_rv(
				unsigned int core_id, unsigned int n_id, bool synaptic_connectivity[],
				short G_i[], short sigma[], short S[], bool b[],
				bool epsilon, short sigma_l, short lambda, bool c, uint32_t alpha,
				uint32_t beta, short TM, short VR, short sigma_vr, short gamma, bool kappa,
				TNNeuronState* n, int signal_delay, uint64_t dest_global_id,
				int dest_axon_id);
		void tn_create_neuron_encoded_rv_non_global(
				int core_id, int n_id, bool synaptic_connectivity[],
				short G_i[], short sigma[], short S[], bool b[],
				bool epsilon, int sigma_l, int lambda, bool c, int alpha,
				int beta, int TM, int VR, int sigma_vr, int gamma, bool kappa,
				TNNeuronState* n, int signal_delay, int dest_core_id,
				int dest_axon_id);

		/**
		 * @brief      Creates a simple neuron for a identity matrix benchmark.
		 * Weights are set up such that axon $n$ has weight 1, where $n$ is the
		 * neuron local id. Other axons have weight 0. Leak is set to zero as well.
		 * The output axon is a randomly selected axon.
		 * @callgraph
		 * @param tn_neuron_state     { parameter_description }
		 * @param lp    The pointer to the LP this neuron lives in
		 * @param n_id	This neuron's core-wise ID
		 * @param core_id This neuron's parent core (Used for computing destinations)
		 **/
		void tn_create_simple_neuron(TNNeuronState* tn_neuron_state, tw_lp* lp, int n_id, int core_id);

		void tn_create_simple_neuron(TNNeuronState* tn_neuron_state, tw_lp* lp, int n_id, int core_id, int neurons_per_core);
	}// namespace neuro_system
}// namespace nemo
#endif//NEMOTNG_TNNEURONSTATE_H
