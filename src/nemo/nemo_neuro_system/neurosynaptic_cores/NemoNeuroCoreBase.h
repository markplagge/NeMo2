//
// Created by Mark Plagge on 11/1/19.
//





#ifndef NEMOTNG_NEMONEUROCOREBASE_H
#define NEMOTNG_NEMONEUROCOREBASE_H


#include "../../nemo_globals.h"
#include "../neuron_models/NemoNeuronGeneric.h"
#include "../../nemo_io/NemoCoreOutput.h"
#include "../../nemo_globals.h"
#include <ross.h>

#define RNG_START(lp) auto rng_count = lp->rng->count
#define RNG_END(lp)  msg->random_call_count = (lp->rng->count - rng_count)
namespace nemo {
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
        protected:
            /**
             * forward_loop_handler(). Neurosynaptic generally have common functionality - there
             * is a neurosynaptic tick (handled by heartbeat messages), integration, and leak functions.
             * Common code for forward events is here - this function handles basic versions of:
             * - the heartbeat / neurosnaptic tick sync
             * -
             *
             *
             *
             */
            virtual void forward_heartbeat_handler();

            virtual void reverse_heartbeat_handler();

            virtual void send_heartbeat();

            void save_spike(nemo_message *m, long dest_core, long neuron_id);

            ::std::shared_ptr<NemoCoreOutput> spike_output;
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

            bool has_self_firing_neuron=false;
            /**
             * my_lp -> current lp state, holds the lp state given to us from the calling function.
             */
            tw_lp *my_lp;
            tw_bf *my_bf;
            /**
             * Current message holder - @todo: may not need this.
             */
            ::std::shared_ptr<nemo_message> cur_message;
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


        public:
            NemoNeuronGeneric<double> neuron_template;

            void core_init(tw_lp *lp);

            void forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp);

            void reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp);

            void core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp);

            void pre_run(tw_lp *lp);

            void core_finish(tw_lp *lp);

            void cleanup_output();

            /** NemoNeuroCoreBase contains neurons and neuron states in a structure */
            std::vector<NemoNeuronGeneric<double> *> neuron_array;


/**
 * output_mode - sets the spike output mode of this core.
 * Mode 0 is no output,
 * Mode 1 is output spikes only
 * Mode 2 is all spikes output
 */
            int output_mode = 2;
        };

    }
}
#endif //NEMOTNG_NEMONEUROCOREBASE_H

