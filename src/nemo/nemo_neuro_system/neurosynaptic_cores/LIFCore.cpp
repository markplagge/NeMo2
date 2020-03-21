//
// Created by Mark Plagge on 12/6/19.
//
#include "LIFCore.h"
namespace nemo {
    namespace neuro_system {



        void LIFCore::core_init(tw_lp *lp) {

        }

        void LIFCore::pre_run(tw_lp *lp) {

        }

        void LIFCore::forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp) {

        }

        void LIFCore::reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp) {

        }

        void LIFCore::core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp) {

        }

        void LIFCore::core_finish(tw_lp *lp) {

        }

        void LIFCore::create_lif_neuron(nemo_id_type neuron_id, ::std::vector<int> weights,
                                        ::std::vector<int> destination_cores,
                                        ::std::vector<int> destination_axons, int leak,
                                        int threshold) {

        }
    }
}