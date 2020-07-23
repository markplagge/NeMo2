//
// Created by Mark Plagge on 6/30/20.
//
#include "TNNeuronState.h"
#include "../../include/nemo.h"
#include "NemoNeuronTrueNorth.h"
#include "../../mapping_functions.h"

namespace nemo {
	namespace neuro_system {
#define G_NEURONS_IN_CORE global_config->neurons_per_core//! < patch so neuron config from nemo1 will see global config value
#define CORES_IN_SIM    (global_config->ns_cores_per_chip * global_config->total_chips)
		void TN_set_neuron_dest(int signalDelay, uint64_t gid, TNNeuronState* n) {
			n->delay_val = signalDelay;
			n->output_gid = gid;
		}

		//--------------------------------------------------------------------------------
		/** OG NEMO INIT FUNCTIONS
		  * @relates tn_neuron_struct
		  * @TODO: Migrate these to the TN Struct, constructor
		 \defgroup TNNeuronInit TrueNorth Init
		 * *  TrueNorth Neuron initialization functions
		 *  TrueNorth Neuron Parameter setting functions. Used as helper functions for
		 *  * init
		 *  * @{ */
		//--------------------------------------------------------------------------------

		/**
		 * Creates a neuron /w encoded RV from TN init data
		 * @param core_id
		 * @param n_id
		 * @param synaptic_connectivity
		 * @param G_i
		 * @param sigma
		 * @param S
		 * @param b
		 * @param epsilon
		 * @param sigma_l
		 * @param lambda
		 * @param c
		 * @param alpha
		 * @param beta
		 * @param TM
		 * @param VR
		 * @param sigma_vr
		 * @param gamma
		 * @param kappa
		 * @param n
		 * @param signal_delay
		 * @param dest_global_id
		 * @param dest_axon_id
		 */
		void tn_create_neuron_encoded_rv(
				unsigned int core_id, unsigned int n_id, bool synaptic_connectivity[],
				short G_i[], short sigma[], short S[], bool b[],
				bool epsilon, short sigma_l, short lambda, bool c, uint32_t alpha,
				uint32_t beta, short TM, short VR, short sigma_vr, short gamma, bool kappa,
				TNNeuronState* n, int signal_delay, uint64_t dest_global_id,
				int dest_axon_id) {
			tn_create_neuron(core_id, n_id, synaptic_connectivity, G_i, sigma, S, b, epsilon,
							 sigma_l, lambda, c, alpha, beta, TM, VR, sigma_vr, gamma,
							 kappa, n, signal_delay, dest_global_id, dest_axon_id);
			n->sigma_vr = sigma_vr;
			n->encoded_reset_voltage = VR;
			n->reset_voltage = (n->sigma_vr * (pow(2, n->encoded_reset_voltage) - 1));
		}
		/**
		 * Non global connectivity neuron creation function /w encoded RV
		 * @param core_id
		 * @param n_id
		 * @param synaptic_connectivity
		 * @param G_i
		 * @param sigma
		 * @param S
		 * @param b
		 * @param epsilon
		 * @param sigma_l
		 * @param lambda
		 * @param c
		 * @param alpha
		 * @param beta
		 * @param TM
		 * @param VR
		 * @param sigma_vr
		 * @param gamma
		 * @param kappa
		 * @param n
		 * @param signal_delay
		 * @param dest_core_id
		 * @param dest_axon_id
		 */
		void tn_create_neuron_encoded_rv_non_global(
				int core_id, int n_id, bool synaptic_connectivity[],
				short G_i[], short sigma[], short S[], bool b[],
				bool epsilon, int sigma_l, int lambda, bool c, int alpha,
				int beta, int TM, int VR, int sigma_vr, int gamma, bool kappa,
				TNNeuronState* n, int signal_delay, int dest_core_id,
				int dest_axon_id) {
			uint64_t dest_global = get_gid_from_core_local(dest_core_id, dest_axon_id);
			tn_create_neuron_encoded_rv(core_id, n_id, synaptic_connectivity, G_i, sigma, S, b, epsilon, sigma_l, lambda, c, alpha, beta, TM, VR, sigma_vr, gamma, kappa, n, signal_delay, dest_global, dest_axon_id);
		}

		/**
		 * Implementation details of the stand-alone constructor.
		 *
		 * @param core_id
		 * @param n_id
		 * @param synaptic_connectivity
		 * @param G_i
		 * @param sigma
		 * @param S
		 * @param b
		 * @param epsilon
		 * @param sigma_l
		 * @param lambda
		 * @param c
		 * @param alpha
		 * @param beta
		 * @param TM
		 * @param VR
		 * @param sigma_vr
		 * @param gamma
		 * @param kappa
		 * @param n
		 * @param signal_delay
		 * @param dest_global_id
		 * @param dest_axon_id
		 */
		void tn_create_neuron(unsigned int core_id, unsigned int n_id,
							  bool synaptic_connectivity[],
							  short G_i[4], short sigma[4], short S[4],
							  bool b[4], bool epsilon, short sigma_l, short lambda,
							  bool c, uint32_t alpha, uint32_t beta, short TM, short VR,
							  short sigma_vr, short gamma, bool kappa,
							  TNNeuronState* n, int signal_delay,
							  uint64_t dest_global_id, int dest_axon_id) {
			for (int i = 0; i < 4; i++) {
				n->synaptic_weight.push_back(sigma[i] * S[i]);
				n->weight_selection.push_back(b[i]);
				//n->synaptic_weight[i] = sigma[i]*S[i];
				//n->weight_selection[i] = b[i];
			}
			for (int i = 0; i < G_NEURONS_IN_CORE; i++) {
				n->synaptic_connectivity.push_back(synaptic_connectivity[i]);
				n->axon_types.push_back(G_i[i]);
			}


			// set up other parameters
			n->my_core_id = core_id;
			n->my_local_id = n_id;
			n->epsilon = epsilon;
			n->sigma_l = sigma_l;
			n->lambda = lambda;
			n->c = c;
			n->pos_threshold = alpha;
			n->neg_threshold = beta;
			//n->threshold_mask_bits = TM;
			//n->threshold_prn_mask = getBitMask(n->threshold_mask_bits);
			n->threshold_prn_mask = TM;
			n->sigma_vr = SGN(VR);
			n->encoded_reset_voltage = VR;
			n->reset_voltage = VR;//* sigmaVR;

			n->reset_mode = gamma;
			n->kappa = kappa;

			//! @TODO: perhaps calculate if a neuron is self firing or not.
			n->fired_last = false;
			n->heartbeat_out = false;
			// n->isSelfFiring = false;
			// n->receivedSynapseMsgs = 0;

			TN_set_neuron_dest(signal_delay, dest_global_id, n);

			// synaptic neuron setup:
			n->largest_random_value = n->threshold_prn_mask;
			if (n->largest_random_value > 256) {
				tw_error(TW_LOC, "Error - neuron (%i,%i) has a PRN Max greater than 256\n ",
						 n->my_core_id, n->my_local_id);
			}
			// just using this rather than bit shadowing.

			n->dendrite_local = dest_axon_id;
			n->output_gid = dest_global_id;

			// Check to see if we are a self-firing neuron. If so, we need to send
			// heartbeats every big tick.
			n->is_self_firing =
					false;
		}
		void tn_create_simple_neuron(TNNeuronState* tn_neuron_state, tw_lp* lp, int n_id, int core_id) {
			// Rewrote this function to have a series of variables that are easier to
			// read.
			// Since init time is not so important, readability wins here.
			// AutoGenerated test neurons:
			auto NEURONS_IN_CORE = G_NEURONS_IN_CORE;
			static int created = 0;
			bool synaptic_connectivity[NEURONS_IN_CORE];
			short g_i[NEURONS_IN_CORE];
			short sigma[4];
			short s[4] = {[0] = 3};
			bool b[4];
			bool epsilon = 0;
			bool sigma_l = 0;
			short lambda = 0;
			bool c = false;
			short TM = 0;
			short VR = 0;
			short sigma_vr = 1;
			short gamma = 0;
			bool kappa = 0;
			int signal_delay = 0;// tw_rand_integer(lp->rng, 0,5);

			for (int i = 0; i < NEURONS_IN_CORE; i++) {
				// s->synapticConnectivity[i] = tw_rand_integer(lp->rng, 0, 1);
				tn_neuron_state->axon_types[i] = 1;
				g_i[i] = 0;
				synaptic_connectivity[i] = 0;
				// synapticConnectivity[i] = tw_rand_integer(lp->rng, 0, 1)
			}

			int my_local_id = n_id;

			synaptic_connectivity[my_local_id] = 1;

			//(creates an "ident. matrix" of neurons.
			for (int i = 0; i < 4; i++) {
				// int ri = tw_rand_integer(lp->rng, -1, 0);
				// unsigned int mk = tw_rand_integer(lp->rng, 0, 1);

				// sigma[i] = (!ri * 1) + (-1 & ri))
				// sigma[i] = (mk ^ (mk - 1)) * 1;
				sigma[i] = 1;
				b[i] = 0;
			}
			int alpha = 1;
			int  beta = -1;
			tn_create_neuron_encoded_rv(core_id, n_id,synaptic_connectivity, g_i, sigma, s, b, epsilon, sigma_l, lambda, c,
					alpha, beta, TM, VR, sigma_vr, gamma, kappa, tn_neuron_state, signal_delay, 0, 0);
			// we re-define the destination axons here, rather than use the constructor.

			float remote_core_probability = .905;
			long int dendrite_core = tn_neuron_state->my_core_id;
			// This neuron's core is X. There is a 90.5% chance that my destination will
			// be X - and a 10% chance it will be a different core.
			if (tw_rand_unif(lp->rng) < remote_core_probability) {
				//		long dendriteCore = s->myCoreID;
				//		dendriteCore = tw_rand_integer(lp->rng, 0, CORES_IN_SIM - 1);
				dendrite_core = tw_rand_integer(lp->rng, 0, CORES_IN_SIM - 1);
			}  /**@note This random setup will create neurons that have an even chance of
 				* getting an axon inside their own core
 				* vs an external core.
 				* */
			// s->dendriteCore = tw_rand_integer(lp->rng, 0, CORES_IN_SIM - 1);
			tn_neuron_state->dendrite_local = tn_neuron_state->my_local_id;  // tw_rand_integer(lp->rng, 0, AXONS_IN_CORE - 1);
			//     if (tnMapping == LLINEAR) {
			tn_neuron_state->output_gid = get_gid_from_core_local(dendrite_core, tn_neuron_state->my_local_id);
			created++;
		}
		/**
		 * Default TNNeuronState constructor
		 */
		TNNeuronState::TNNeuronState() {}

		/** @} */
	}// namespace neuro_system
}// namespace nemo