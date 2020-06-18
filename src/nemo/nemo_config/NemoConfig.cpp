//
// Created by Mark Plagge on 12/6/19.
//

#include "NemoConfig.h"
#define CONFIGURU_IMPLEMENTATION 1
#include <configuru.hpp>
#include <iostream>
#include <ross.h>
#include <sstream>
#include <strstream>
#include <utility>
namespace nemo {
	namespace config {
		void error_reporter(std::string str)
		{
			std::cerr << str << std::endl;// or throw or ignore
		}
		char* primary_config_file = (char*)calloc(sizeof(char), 1024);

		std::string NemoConfig::get_settings()
		{
			std::stringstream output;
			output << "Cores per chip: " << ns_cores_per_chip << "\n";
			output << "Total Chips" << total_chips << "\n";
			output << "Sim size (total NS CORES)" << total_sim_size << "\n";
			output << "Scheduler in use? " << do_neuro_os << "\n";
			if (do_neuro_os) {
				output << "Scheduler Type ";
				switch (scheduler_type) {
				case FCFS:
					output << "FCFS";
					break;
				case RR:
					output << "Round Robin";
					break;
				case FS:
					output << "Fair Share";
					break;
				}
				output << "\n";
			}
			output << "Main config file location: " << main_config_file << "\n";

			return output.str();
		}


		tw_optdef NemoConfig::nemo_tw_options[] = {
				TWOPT_GROUP("NeMo 2 - TNG Runtime Options"),
				TWOPT_FLAG("debug", NemoConfig::DEBUG_FLAG, "Debug mode?"),
				TWOPT_ULONG("mean", NemoConfig::test, "test_value"),
				TWOPT_CHAR("cfg",primary_config_file, "Main configuration file"),
				TWOPT_END()};

		tw_peid nemo_map_linear(tw_lpid gid)
		{

			return 1;
		}
		/**
		 * Main configuration function - reads the nemo config file specified by the CLI flag,
		 * parses it, and sets up the NeMo simulation.
		 * Uses configuru
		 */
		void NemoConfig::init_from_tw_opts()
		{

			auto cli_cfg_file = std::string(primary_config_file);
			if (cli_cfg_file.length() > 0) {
				NemoConfig::main_config_file = cli_cfg_file;
			}
			else {
				NemoConfig::main_config_file = std::string("../config/example_config.json");
			}
			std::cout << "NeMo config file loading from  " << NemoConfig::main_config_file << "\n";

			using namespace configuru;
			Config cfg = configuru::parse_file(main_config_file, JSON);
			auto cgbl = cfg["nemo_global"];
			std::cout << cgbl << "\n";
			this->ns_cores_per_chip = (u_long)cgbl["ns_cores_per_chip"];
			this->total_chips = (u_int)cgbl["total_chips"];
			this->do_neuro_os = (bool)cgbl["do_neuro_os"];
			if (this->do_neuro_os) {
				auto sched_type = (std::string)cgbl["sched_type"];
				if ("FCFS" == sched_type) {
					this->scheduler_type = SchedType::FCFS;
				}
				else if ("RR" == sched_type) {
					this->scheduler_type = SchedType::RR;
				}
				else {
					this->scheduler_type = SchedType::FS;
				}
			}
			auto model_set = cfg["models"];
			//			auto mdx = new std::vector<config::NemoModel>;
			configuru::deserialize(&this->models, model_set, error_reporter);
			configuru::deserialize(&this->scheduler_inputs, cfg["scheduler_inputs"], error_reporter);

			std::cout << "models\n";
		}
		NemoConfig::NemoConfig() = default;
		bool NemoConfig::DEBUG_FLAG;
		std::string NemoConfig::main_config_file = "../config/example_config.json";
		u_long NemoConfig::test = 0;

		NemoModel create_model_from_cfg(const configuru::Config& c)
		{
			auto nm = new NemoModel();

			nm->id = (int)c["id"];
			nm->needed_cores = (int)c["needed_cores"];
			nm->model_file_path = (std::string)c["model_file_path"];
			nm->spike_file_path = (std::string)c["spike_file_path"];
			nm->requested_time = (int)c["requested_time"];
			nm->benchmark_model_name = (std::string)c["benchmark_model_name"];
		}

	}// namespace config
}// namespace nemo
