//
// Created by Mark Plagge on 11/1/19.
//

#ifndef NEMOTNG_NEMONEUROCOREBASE_H
#define NEMOTNG_NEMONEUROCOREBASE_H
#include "../../mapping_functions.h"
#include "../../nemo_config/NemoConfig.h"
#include "../../nemo_globals.h"
#include "../../nemo_io/ModelFile.h"
#include "../../nemo_io/NemoCoreOutput.h"
#include "../../nemo_io/SpikeFile.h"
#include "../neuron_models/NemoNeuronGeneric.h"
#include <vector>
#include <configuru.hpp>
#include <neuro_os.h>
#include <ross.h>
#include <visit_struct/visit_struct.hpp>
#include <visit_struct/visit_struct_intrusive.hpp>

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


			static void s_core_init(void* s, tw_lp* lp) {

				auto core = static_cast<NemoNeuroCoreBase*>(s);
				new (core) NemoNeuroCoreBase();

				//determine the type of core we want through mapping
				for (const auto& model : global_config->models) {
					core->models[model.id] = model;
				}

				auto global_id = lp->gid;
				core->core_local_id = get_core_id_from_gid(global_id);
				core->my_lp = lp;
				/* @note using standard POSIX output - one file per PE */

			}
			int init_id_DBG = 0;
			static void s_core_init_from_vcore(NemoNeuroCoreBase* s, tw_lp* lp, int model_id){
				s->init_id_DBG = model_id;
				for (const auto& model : global_config->models){
					if (model.id == model_id){
						s->models[model_id] = model;
						s->model_id = model_id;
					}
				}
				auto global_id = lp->gid;
				s->core_local_id = get_core_id_from_gid(global_id);
				s->my_lp = lp;
			}

			static void s_pre_run(void* s, tw_lp* lp) {
				auto core = static_cast<NemoNeuroCoreBase*>(s);
				core->pre_run(lp);
			}

			static void s_forward_event(void* s, tw_bf* bf, void* m, tw_lp* lp) {

				auto core = static_cast<NemoNeuroCoreBase*>(s);
				auto ms = static_cast<nemo_message*>(m);
				core->forward_event(bf, ms, lp);
//				if( is_output_spike_sent(core->evt_stat)){
//					std::cout << "SENT SPIKES";
//				}
			}

			static void s_reverse_event(void* s, tw_bf* bf, void* m, tw_lp* lp) {
				auto core = static_cast<NemoNeuroCoreBase*>(s);
				auto ms = static_cast<nemo_message*>(m);
				core->reverse_event(bf, ms, lp);
			}


			static void s_core_commit(void* s, tw_bf* bf, void* m, tw_lp* lp) {
				auto core = static_cast<NemoNeuroCoreBase*>(s);
				auto ms = static_cast<nemo_message*>(m);
				core->core_commit(bf, ms, lp);
			}

			static void s_core_finish(void* s, tw_lp* lp) {
				auto core = static_cast<NemoNeuroCoreBase*>(s);
				core->core_finish(lp);
			}



			virtual void forward_heartbeat_handler();

			virtual void reverse_heartbeat_handler();

			virtual void send_heartbeat();

			void save_spike(nemo_message* m, long dest_core, long neuron_id, double current_time) const;


			void forward_event(tw_bf* bf, nemo_message* m, tw_lp* lp);

			void reverse_event(tw_bf* bf, nemo_message* m, tw_lp* lp);

			void core_commit(tw_bf* bf, nemo_message* m, tw_lp* lp);

			void pre_run(tw_lp* lp);

			void core_finish(tw_lp* lp);

			void cleanup_output();
			void run_leaks();
			void run_fires();
			void run_resets();
			void create_blank_neurons();
			void init_current_model(std::string model_def);
			void interrupt_running_model();
			void resume_running_model();
			void reverse_start();
			void reverse_stop();
			void f_save_spikes(nemo_message* m);
			void f_save_mpots(tw_lp* lp);
			bool is_dest_interchip(int neuron_id);
			//virtual ~NemoNeuroCoreBase(){};

			static NemoCoreOutput *output_system;
			static NemoDebugJSONHandler *debug_system;
			static bool is_init;


			std::vector<std::vector<std::unique_ptr<NemoNeuronGeneric>>> neuron_stack;
			bool neurons_init = false;
			bool save_spikes;
			bool save_mpots;
			/**
* The last time that this core had activity. This refers to any  message.
*/

			unsigned long current_neuro_tick = 0;
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

			unsigned int core_local_id = 0;

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
			static std::map<int, nemo::config::NemoModel> models;
			std::map<int, std::string> test_map;
			nemo::config::NemoModel current_model;
			std::vector<std::unique_ptr<NemoNeuronGeneric>> state_stack;

			/** NemoNeuroCoreBase contains neurons and neuron states in a structure */
			std::vector<std::unique_ptr<NemoNeuronGeneric>> neuron_array;
			virtual ~NemoNeuroCoreBase();
			std::vector<unsigned int> neuron_spike_record;
			std::vector<unsigned int> neuron_dest_cores;
			std::vector<unsigned int> neuron_dest_axons;

			static std::vector<ModelFile> model_files;
			std::vector<SpikeFile> spike_files;
			void init_model_files();
			void init_load_model_into_core(int new_model_id);
			core_types my_core_type = NO_CORE_TYPE;

			/**
 * output_mode - sets the spike output mode of this core.
 * Mode 0 is no output,
 * Mode 1 is output spikes only
 * Mode 2 is all spikes output
 */
			int output_mode = 2;

			int model_id = 0;
			bool is_model_running = false;
		};

	}// namespace neuro_system
}// namespace nemo
#endif//NEMOTNG_NEMONEUROCOREBASE_H
