//
// Created by Mark Plagge on 3/29/20.
//

#ifndef NEMOTNG_NEMONEURONFACTORY_H
#define NEMOTNG_NEMONEURONFACTORY_H

#include "NemoNeuronGeneric.h"
#include "../../nemo_io/NemoNeuroInitState.h"

namespace nemo {
    namespace neuro_system {
        class NemoNeuronFactory {

            template<typename BASE_NEURO_DATATYPE>
            NemoNeuronGeneric<BASE_NEURO_DATATYPE> *create_generic_neuron_from_init_state(NemoNeuroInitState *s) {


            };

            void *create_neuron_from_params() {

            }

            void *create_neuron_from_str() {

            }
        };

    }
}
#endif //NEMOTNG_NEMONEURONFACTORY_H
