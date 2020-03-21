//
// Created by Mark Plagge on 3/19/20.
//

#ifndef NEMOTNG_HEARTBEATTESTCORE_H
#define NEMOTNG_HEARTBEATTESTCORE_H


#include "NemoNeuroCoreBase.h"
namespace nemo{
    namespace neuro_system {
        using namespace Eigen;
        class HeartbeatTestCore : INeuroCoreBase {
            void core_init(tw_lp *lp)  override;

            void forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp) override;

            void reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp) override;

            void core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp) override;

            void pre_run(tw_lp *lp) override;

            void core_finish(tw_lp *lp) override;

            Vector<int,LIF_NEURONS_PER_CORE> neuron_connectivity;
            Matrix<int,LIF_NEURONS_PER_CORE,LIF_NEURONS_PER_CORE> neuron_weights;
            Vector<int,LIF_NEURONS_PER_CORE> thresholds;
            Vector<int,LIF_NEURONS_PER_CORE> current_membrane_pots;

        };
    }
}

#endif //NEMOTNG_HEARTBEATTESTCORE_H
