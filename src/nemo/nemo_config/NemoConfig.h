//
// Created by Mark Plagge on 12/6/19.
//

#ifndef NEMOTNG_NEMOCONFIG_H
#define NEMOTNG_NEMOCONFIG_H
#include "../nemo_globals.h"
#include "../nemo_neuro_system/neuron_models/NemoNeuronGeneric.h"
#include "NemoModel.h"
#include <visit_struct/visit_struct.hpp>
#include <visit_struct/visit_struct_intrusive.hpp>
#include <configuru.hpp>


//#include "../nemo_io/ModelFile.h"
#include "../nemo_neuro_system/neuron_models/NemoNeuronGeneric.h"

#include <map>
#include <ostream>
#include <string>
#include <neuro_os.h>

//#include <rapidjson/rapidjson.h>
namespace nemo {

	namespace config {
		typedef enum SchedType {
			FCFS,
			RR,
			FS
		} SchedType;
		typedef enum OutputSystem {
			POSIX = 0
		}output_system;
		struct ScheduledTask {
			double start_time;
			int task_id;
			int model_id;
			friend std::ostream& operator<<(std::ostream& os, const ScheduledTask& task);
		};

		struct NemoConfig {
			unsigned int ns_cores_per_chip = 4096;
			unsigned int neurons_per_core = 256;
			unsigned int total_chips = 2;
			// computed:
			unsigned int total_sim_size = 0; //!< Total simulation size - DOES NOT INCLUDE SCHEDULER CORE. Number of compute neurosynaptic cores
			unsigned int lps_per_pe = 0; //!< LPs per PE in sim. DOES NOT INCLUDE SCHEDULER CORE
			unsigned int total_lps = 0;

			u_int64_t total_neurons_in_sim = 0;

			//read in again
			bool do_neuro_os = false;

			bool save_all_spikes = false;
			bool save_membrane_pots = false;
			bool save_nos_stats = true;
			bool use_nengo_dl = false;
			OutputSystem output_system;
			SchedType scheduler_type;
			std::string output_spike_file;
			std::string output_membrane_pot_file;
			std::string output_nos_stat_file;
			std::vector<std::string> stat_files() const;
			std::vector<core_types> core_map;
			//core_types GetCoreType(::std::string core_type){}
			std::vector<int> core_type_ids;
			std::vector<ScheduledTask> scheduler_inputs;
			std::vector<NemoModel> models;

			double jitter_factor = 0.001;//!< This is the range of jitter. Since core-core comms are used lookahead can now be 1 though
			int est_events_per_pe = 165535;
			double lookahead = 1.9;
			int world_size = 0;


			static bool DEBUG_FLAG;
			static tw_petype main_pe[4];
			static tw_optdef nemo_tw_options[];
			static u_long test;
			static std::string main_config_file;

			NemoConfig();
			void init_from_tw_opts(char* config_file);

			std::string get_settings();
			std::string sched_mode_to_string() const;
			NemoModel get_model(int model_id);

		};
		extern char* primary_config_file;
		
		extern unsigned int LPS_PER_PE;
		extern unsigned int SYNAPSES_IN_CORE;
		extern unsigned int CORE_SIZE;
		extern unsigned int SIM_SIZE;

		void set_sim_size();
		void error_reporter(std::string str);



	}// namespace config
}// namespace nemo
VISITABLE_STRUCT(nemo::config::ScheduledTask, start_time, task_id, model_id);
#endif// NEMOTNG_NEMOCONFIG_H
/*
 *
 */
