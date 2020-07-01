//
// Created by Mark Plagge on 6/30/20.
//

#include "BenchmarkModelDef.h"

namespace nemo{
	namespace neuro_system {

#define X(a) #a,
		//static char * benchmark_strings[] = {NEMO_BENCHMARK_TYPES};
		constexpr const char * const benchmark_strings[] = {NEMO_BENCHMARK_TYPES};
#undef X

		void GenericIdentityInit::_init_benchmark_neuron(int nid, int n_neurons) {
			std::cout << "From derived";
			for(int i = 0; i < n_neurons; i ++){
				weights.push_back( i == nid ? 1:0 );

			}
			dest_core = rand() % n_neurons;
			dest_axon = rand() % n_neurons;

		}
	}

}