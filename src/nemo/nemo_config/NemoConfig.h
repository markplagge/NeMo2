//
// Created by Mark Plagge on 12/6/19.
//

#ifndef NEMOTNG_NEMOCONFIG_H
#define NEMOTNG_NEMOCONFIG_H

#include "../nemo_globals.h"
#include "../nemo_io/ModelFile.h"
#include "../nemo_neuro_system/neurosynaptic_cores/NemoCoreLPWrapper.h"
#include "../nemo_neuro_system/neurosynaptic_cores/NemoNeuroCoreBase.h"
#include <map>
#include <string>

//#include <rapidjson/rapidjson.h>
namespace nemo {
	namespace config {
		typedef enum SchedType {
			FCFS,
			RR,
			FS
		} SchedType;
		struct NemoConfig {

			int ns_cores_per_chip = 4096;
			int total_chips = 2;
			int total_sim_size;
			bool do_neuro_os = false;
			SchedType scheduler_type;
			::std::vector<core_types> core_map;
			//core_types GetCoreType(::std::string core_type){}
			::std::map<core_types, neuro_system::NemoNeuroCoreBase*> core_type_map;
			std::vector<std::string> model_files;
			std::vector<std::string> spike_input_files;
			std::vector<std::string> scheduler_inputs;

			static std::string main_config_file;
			static tw_petype main_pe[4];
			//const tw_petype* get_main_pe() const {};
			static bool DEBUG_FLAG;
			static tw_optdef nemo_tw_options[];
			static u_long test;

			NemoConfig();
			void init_from_tw_opts();

			std::string get_settings();
		};
		extern char * primary_config_file;
		tw_peid nemo_map_linear(tw_lpid gid);


	}// namespace config
}// namespace nemo
#endif// NEMOTNG_NEMOCONFIG_H
/*
 *
 */
