//
// Created by Mark Plagge on 11/1/19.
//

#ifndef NEMOTNG_NEMONEUROCOREBASE_H
#define NEMOTNG_NEMONEUROCOREBASE_H

#include "../../nemo_config/NemoConfig.h"
#include "../../nemo_globals.h"
#include "../../nemo_io/NemoCoreOutput.h"
#include "../neuron_models/NemoNeuronGeneric.h"
#include <visit_struct/visit_struct.hpp>
#include <visit_struct/visit_struct_intrusive.hpp>
#include <configuru.hpp>
#include <neuro_os.h>
#include <ross.h>

#define RNG_START(lp) auto rng_count = lp->rng->count
#define RNG_END(lp) msg->random_call_count = (lp->rng->count - rng_count)
namespace nemo {
	extern config::NemoConfig* global_config;
	namespace neuro_system {

		/**
 * @defgroup nemo_cores NeMo2 Core Definitions
 * NeMo2 neurosynaptic cores, as well as the ROSS LP wrapper.
 * NeMo2 uses a "fat lp" technique. Neurons are defined in a core.
 * To wrap the LP state from ROSS, we use the CoreLP class, which
 * holds a INeuroCoreBase object.
 *
 * These classes are the base core definitions for NeMo.
 * @{
 */
		/**
 * INeuroCoreBase
 * Base interface for neuromorphic cores in NeMo. Defines the expected functions
 * that will be used by neuromorphic cores.
 *
 * Also implements basic heartbeat code.
 */

		class NemoNeuroCoreBase {

			//INeuroCoreBase();
		public:
			NemoNeuroCoreBase();
			/**
   * forward_loop_handler(). Neurosynaptic generally have common functionality - there
   * is a neurosynaptic tick (handled by heartbeat messages), integration, and leak functions.
   * Common code for forward events is here - this function handles basic versions of:
   * - the heartbeat / neurosnaptic tick sync
   */

			virtual void
			forward_heartbeat_handler();

			virtual void
			reverse_heartbeat_handler();

			virtual void
			send_heartbeat();

			void
			save_spike(nemo_message* m, long dest_core, long neuron_id, double current_time);

			NemoCoreOutput* output_system;

			bool save_spikes;
			bool save_mpots;
			/**
* The last time that this core had activity. This refers to any  message.
*/

			long last_active_time = 0;
			long current_neuro_tick = 0;
			long previous_neuro_tick = -1;
			/**
* The last time this core computed the leak.
*/
			long last_leak_time = 0;
			long leak_needed_count = 0;

			/**
   * A heartbeat check value.
   */
			bool heartbeat_sent = false;
			/**
   * * the local core id. If linear mapping is enabled, then this will be equal to the GID/PE id
*/

			int core_local_id = 0;

			bool has_self_firing_neuron = false;

			/**
   * Current message holder - @todo: may not need this.
   */
			nemo_message* cur_message;
			/**
   * random number generator counter - used to keep the RNG counter state intact through various calls.
   */
			unsigned long cur_rng_count = 0;

			/**
* evt_stat holds the event status for the current event. This is used to compute
* reverse computation. BF_Event_Stats is used instead of the tw_bf as it allows
* more explicit naming. The concept is the same, however.
*/
			BF_Event_Status evt_stat;

			/**
* my_lp -> current lp state, holds the lp state given to us from the calling function.
*/
			tw_lp* my_lp;
			tw_bf* my_bf;
			NemoNeuronGeneric neuron_template;
			std::map<int, nemo::config::NemoModel> models;
			nemo::config::NemoModel current_model;
			std::vector<NemoNeuroCoreBase> state_stack;

			void core_init(tw_lp* lp);

			void forward_event(tw_bf* bf, nemo_message* m, tw_lp* lp);

			void reverse_event(tw_bf* bf, nemo_message* m, tw_lp* lp);

			void core_commit(tw_bf* bf, nemo_message* m, tw_lp* lp);

			void pre_run(tw_lp* lp);

			void core_finish(tw_lp* lp);

			void cleanup_output();

			void run_leaks();
			void run_fires();
			void run_resets();

			void init_current_model();
			void interrupt_run();
			void resume_run();

			bool is_dest_interchip(int neuron_id);
			/** NemoNeuroCoreBase contains neurons and neuron states in a structure */
			std::vector<NemoNeuronGeneric*> neuron_array;
			std::vector<unsigned int> neuron_dest_cores;
			std::vector<unsigned int> neuron_dest_axons;

			/**
 * output_mode - sets the spike output mode of this core.
 * Mode 0 is no output,
 * Mode 1 is output spikes only
 * Mode 2 is all spikes output
 */
			int output_mode = 2;
		};

	}// namespace neuro_system
}// namespace nemo
#endif//NEMOTNG_NEMONEUROCOREBASE_H
