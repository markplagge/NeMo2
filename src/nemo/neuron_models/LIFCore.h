//
// Created by Mark Plagge on 12/6/19.
//

#ifndef NEMOTNG_LIFCORE_H
#define NEMOTNG_LIFCORE_H
#include <array>
#include "../NemoNeuroCoreBase.h"
#include "../nemo_globals.h"

class LIFCore: public  INeuroCoreBase {
    nemo_id_type core_id;
    nemo_id_type output_mode;

    void core_init(tw_lp *lp) override ;
    void pre_run(tw_lp *lp) override ;
    void forward_event(tw_bf *bf, nemo_message *m, tw_lp *lp) override ;
    void reverse_event(tw_bf *bf, nemo_message *m, tw_lp *lp) override ;
    void core_commit(tw_bf *bf, nemo_message *m, tw_lp *lp)override ;
    void core_finish(tw_lp *lp)override ;

    void create_lif_neuron(nemo_id_type neuron_id, std::array<int,LIF_NEURONS_PER_CORE> weights, std::array <int,LIF_NUM_OUTPUTS> destination_cores,
                           std::array<int, LIF_NEURONS_PER_CORE> destination_axons, int leak, int threshold);

    Matrix <int,LIF_NEURONS_PER_CORE,LIF_NEURONS_PER_CORE> weights;
    std::array<int,LIF_NEURONS_PER_CORE> membrane_pots;
    Matrix <int,LIF_NEURONS_PER_CORE,LIF_NUM_OUTPUTS> destination_cores;
    Matrix <int,LIF_NEURONS_PER_CORE,LIF_NUM_OUTPUTS> destination_axons;
    std::array<int,LIF_NEURONS_PER_CORE> leak_values;
    std::array<int,LIF_NEURONS_PER_CORE> thresholds;
    std::array<bool,LIF_NEURONS_PER_CORE> fire_status;
};


#endif //NEMOTNG_LIFCORE_H
