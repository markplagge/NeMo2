//
// Created by Mark Plagge on 6/30/20.
//

#ifndef NEMOTNG_BENCHMARKMODELDEF_H
#define NEMOTNG_BENCHMARKMODELDEF_H
#include <iostream>
#include <vector>
namespace nemo{
	namespace neuro_system{
		extern const char * const benchmark_strings[];

		/**
		 * Benchmark models:
		 * Identity model: From NeMo$_1$, diagonal weights are 1, others are zero
		 * Saturation: Randomized network, weighted s.t. a handful of neurons get most of the traffic.
		 * Tonic: IZK Tonic Spiking
		 */
#define NEMO_BENCHMARK_TYPES \
	X(IDENTITY )                 \
	X(SATURATION )               \
	X(TONIC 	)


#define X(a) a,
		enum BENCHMARK_TYPE { NEMO_BENCHMARK_TYPES };
#undef X

		template <typename Derived>
		struct BenchmarkModelInit{
			std::vector<int> weights;
			int dest_core;
			int dest_axon;

			void init_benchmark_neuron_interface( int n_id, int n_neurons){
				static_cast<Derived *>(this) -> _init_benchmark_neuron(n_id, n_neurons);
			}

			void _init_benchmark_neuron(int n_id, int n_neurons){
				for(int i =0; i < n_neurons; i ++){
					weights.push_back(-1); // base implementation 'error' code;
				}

			}
		};

		struct GenericIdentityInit : BenchmarkModelInit<GenericIdentityInit>{


			void _init_benchmark_neuron(int nid, int n_neurons);
		};

		struct TNIdentityInit : BenchmarkModelInit<GenericIdentityInit>{
			void _init_benchmark_neuron(int nid, int n_neurons);
		};

	}

}

#endif//NEMOTNG_BENCHMARKMODELDEF_H
