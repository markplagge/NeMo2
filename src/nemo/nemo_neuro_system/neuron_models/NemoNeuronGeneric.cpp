//
// Created by Mark Plagge on 11/1/19.
//
#include "NemoNeuronGeneric.h"
namespace nemo {
    namespace neuro_system {

        template<typename PRC>
        NemoNeuronGeneric<PRC>::NemoNeuronGeneric(PRC membranePot, const std::vector<PRC> &weights, PRC leakV,
                                                  PRC threshold, PRC resetVal):membrane_pot(membranePot),
                                                                               weights(weights), leak_v(leakV),
                                                                               threshold(threshold),
                                                                               reset_val(resetVal) {}

    }
}