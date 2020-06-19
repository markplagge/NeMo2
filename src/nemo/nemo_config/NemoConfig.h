//
// Created by Mark Plagge on 12/6/19.
//

#ifndef NEMOTNG_NEMOCONFIG_H
#define NEMOTNG_NEMOCONFIG_H
#include "../nemo_globals.h"
#include <configuru.hpp>
#include <visit_struct/visit_struct.hpp>
#include <visit_struct/visit_struct_intrusive.hpp>

//#include "../nemo_io/ModelFile.h"
//#include "../nemo_neuro_system/neurosynaptic_cores/NemoCoreLPWrapper.h"

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

		struct ScheduledTask {
			double start_time;
			int task_id;
		};
		struct NemoModel {
			int id;
			int needed_cores;
			std::string model_file_path;
			std::string spike_file_path;
			double requested_time;
			std::string benchmark_model_name;
		};
		struct test_s {
			int a;
			int b;
		};
		struct NemoConfig {
			int ns_cores_per_chip = 4096;
			int total_chips = 2;
			int total_sim_size{};
			bool do_neuro_os = false;
			SchedType scheduler_type;
			std::vector<core_types> core_map;
			//core_types GetCoreType(::std::string core_type){}
			std::vector<int> core_type_ids;
			std::vector<ScheduledTask> scheduler_inputs;
			std::vector<NemoModel> models;

			static bool DEBUG_FLAG;
			static tw_petype main_pe[4];
			static tw_optdef nemo_tw_options[];
			static u_long test;
			static std::string main_config_file;

			NemoConfig();
			void init_from_tw_opts();
			std::string get_settings();
		};
		extern char* primary_config_file;
		tw_peid nemo_map_linear(tw_lpid gid);

	}// namespace config
}// namespace nemo
VISITABLE_STRUCT(nemo::config::NemoModel, id, needed_cores, model_file_path, spike_file_path, requested_time, benchmark_model_name);
VISITABLE_STRUCT(nemo::config::ScheduledTask, task_id, start_time);
#endif// NEMOTNG_NEMOCONFIG_H
/*
 *
 */
