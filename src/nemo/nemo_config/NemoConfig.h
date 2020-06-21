//
// Created by Mark Plagge on 12/6/19.
//

#ifndef NEMOTNG_NEMOCONFIG_H
#define NEMOTNG_NEMOCONFIG_H
#include "../nemo_globals.h"
#include "../nemo_neuro_system/neuron_models/NemoNeuronGeneric.h"
#include <visit_struct/visit_struct.hpp>
#include <visit_struct/visit_struct_intrusive.hpp>
#include <configuru.hpp>


//#include "../nemo_io/ModelFile.h"
//#include "../nemo_neuro_system/neurosynaptic_cores/NemoCoreLPWrapper.h"
#include "../nemo_neuro_system/neuron_models/NemoNeuronGeneric.h"
#include <map>
#include <ostream>
#include <string>

//#include <rapidjson/rapidjson.h>
namespace nemo {
	namespace config {
		typedef enum SchedType {
			FCFS,
			RR,
			FS
		} SchedType;

		struct ScheduledTask {
			double start_time;
			int task_id;
			int model_id;
			friend std::ostream& operator<<(std::ostream& os, const ScheduledTask& task);
		};
		/**
		 * NeMoModel -
		 * Struct containing parameters for a particular model, file paths for the model configs / input spikes,
		 * a string that determines if it is a benchmark network and what network it is.
		 *
		 * NemoCoreScheduler takes a vector of these, and uses them to manage the simulation.
		 */
		struct NemoModel {

			int id;
			int needed_cores;
			std::string model_file_path;
			std::string spike_file_path;
			double requested_time;
			std::string benchmark_model_name;

			const std::string to_string() const;
			friend std::ostream& operator<<(std::ostream& os, const NemoModel& model);
		};

		struct test_s {
			int a;
			int b;
		};
		struct NemoConfig {
			unsigned int ns_cores_per_chip = 4096;
			unsigned int neurons_per_core = 256;
			unsigned int total_chips = 2;
			unsigned int total_sim_size = 0;
			unsigned int lps_per_pe = 0;
			u_int64_t total_neurons_in_sim = 0;
			bool do_neuro_os = false;

			bool save_all_spikes = false;
			bool save_membrane_pots = false;
			bool save_nos_stats = true;
			SchedType scheduler_type;
			std::vector<core_types> core_map;
			//core_types GetCoreType(::std::string core_type){}
			std::vector<int> core_type_ids;
			std::vector<ScheduledTask> scheduler_inputs;
			std::vector<NemoModel> models;

			double jitter_factor = 0.001;//!< This is the range of jitter. Since core-core comms are used lookahead can now be 1 though
			int est_events_per_pe = 1024;
			double lookahead = 0.9;

			int world_size = 0;

			static bool DEBUG_FLAG;
			static tw_petype main_pe[4];
			static tw_optdef nemo_tw_options[];
			static u_long test;
			static std::string main_config_file;

			NemoConfig();
			void init_from_tw_opts();

			std::string get_settings();
			std::string sched_mode_to_string();

		};
		extern char* primary_config_file;
		
		extern unsigned int LPS_PER_PE;
		extern unsigned int SYNAPSES_IN_CORE;
		extern unsigned int CORE_SIZE;
		extern unsigned int SIM_SIZE;

		void set_sim_size();

	}// namespace config
}// namespace nemo
VISITABLE_STRUCT(nemo::config::NemoModel, id, needed_cores, model_file_path, spike_file_path, requested_time, benchmark_model_name);
VISITABLE_STRUCT(nemo::config::ScheduledTask, start_time, task_id, model_id);
#endif// NEMOTNG_NEMOCONFIG_H
/*
 *
 */
