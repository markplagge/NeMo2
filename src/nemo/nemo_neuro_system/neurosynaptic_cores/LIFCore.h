//
// Created by Mark Plagge on 12/6/19.
//

#ifndef NEMOTNG_LIFCORE_H
#define NEMOTNG_LIFCORE_H

#include "../../nemo_globals.h"
#include "NemoNeuroCoreBase.h"

namespace nemo {
    namespace neuro_system {
        using namespace Eigen;
        class LIFCore : public INeuroCoreBase {
            nemo_id_type core_id;
            nemo_id_type output_mode;


            void core_init(tw_lp *lp) override;

            void pre_run(tw_lp *lp) override;

            void forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp) override;

            void reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp) override;

            void core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp) override;

            void core_finish(tw_lp *lp) override;

            void create_lif_neuron(nemo_id_type neuron_id, ::std::vector<int> weights,
                                            ::std::vector<int> destination_cores,
                                            ::std::vector<int> destination_axons, int leak,
                                            int threshold);

            Matrix<int,Dynamic,Dynamic> weights;
            Vector<int,Dynamic> membrane_pots;
            Matrix<int,Dynamic, Dynamic> destination_cores;
            Matrix<int,Dynamic, Dynamic> destination_axons;
            Vector<int,Dynamic> leak_values;
            Vector<int,Dynamic> thresholds;
            Vector<bool,Dynamic> fire_status;
            //, LIF_NEURONS_PER_CORE, LIF_NEURONS_PER_CORE
        };

    }
}

#endif //NEMOTNG_LIFCORE_H
