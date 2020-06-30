//
// Created by Mark Plagge on 6/30/20.
//
#include "../../include/nemo.h"
#include "NemoNeuronTrueNorth.h"
namespace nemo {
	namespace neuro_system {
#define NEURONS_IN_CORE global_config->neurons_per_core//! < patch so neuron config from nemo1 will see global config value

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
			for (int i = 0; i < NEURONS_IN_CORE; i++) {
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
			// n->thresholdMaskBits = TM;
			// n->thresholdPRNMask = getBitMask(n->thresholdMaskBits);
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
	}// namespace neuro_system
}// namespace nemo