//
// Created by Mark Plagge on 12/6/19.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#include "NemoConfig.h"

#include <iostream>
#include <ross.h>
#include <sstream>

#include <utility>

#include <codecvt>
#include <visit_struct/visit_struct.hpp>
#define CONFIGURU_IMPLEMENTATION 1
#include <configuru.hpp>
#include <iosfwd>

namespace nemo {
	namespace config {
		void error_reporter(std::string str) {
			std::cerr << str << std::endl;// or throw or ignore
		}


		std::string NemoConfig::get_settings() {
			std::stringstream output;
			output << "Main config file location: " << main_config_file << "\n";
			output << "Cores per chip: " << ns_cores_per_chip << "\n";
			output << "Total Chips: " << total_chips << "\n";
			output << "Sim size (total NS CORES): " << total_sim_size << "\n";
			output << "Scheduler in use? " << do_neuro_os << "\n";
			output << sched_mode_to_string() << "\n";

			return output.str();
		}

		std::string NemoConfig::sched_mode_to_string() const {
			std::stringstream output;
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
			}
			return output.str();
		}





		/**
		 * Main configuration function - reads the nemo config file specified by the CLI flag,
		 * parses it, and sets up the NeMo simulation.
		 * Uses configuru
		 */
		void NemoConfig::init_from_tw_opts(char* config_file) {
			world_size = tw_nnodes();
			auto cli_cfg_file = std::string(config_file);
			if (cli_cfg_file.length() > 0) {
				NemoConfig::main_config_file = cli_cfg_file;
			}
			else {
				NemoConfig::main_config_file = std::string("../config/example_config.json");
			}

			using namespace configuru;
			Config cfg = configuru::parse_file(main_config_file, FORGIVING);
			auto cgbl = cfg["nemo_global"];
			this->ns_cores_per_chip = (u_long)cgbl["ns_cores_per_chip"];
			this->total_chips = (u_int)cgbl["total_chips"];
			this->do_neuro_os = (bool)cgbl["do_neuro_os"];
			this->neurons_per_core = (u_int64_t)cgbl["neurons_per_core"];
			this->save_all_spikes = (bool)cgbl["save_all_spikes"];
			this->save_membrane_pots = (bool)cgbl["save_membrane_pots"];
			this->save_nos_stats = (bool)cgbl["save_nos_stats"];
			this->output_spike_file =(std::string)cgbl["output_spike_file"];
			this->output_nos_stat_file =(std::string)cgbl["output_nos_stat_file"];
			this->output_membrane_pot_file =(std::string)cgbl["output_membrane_pot_file"];
			this->core_type_ids = (std::vector<int>)cgbl["core_type_ids"];

			auto os = (int)cgbl["output_system"];
			switch (os) {
			case 0:
				this->output_system = POSIX;
				break;
			}


			DEBUG_FLAG = ((bool)cgbl["GLOBAL_DEBUG"]);

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

			total_sim_size = total_chips * ns_cores_per_chip;
			total_lps = total_sim_size + 1;
			lps_per_pe = total_lps / tw_nnodes();
			est_events_per_pe = total_sim_size / world_size;
			//Does not include the scheduler LP which runs on PE 0
			lps_per_pe = (ns_cores_per_chip * total_chips) / world_size;
		}
		NemoConfig::NemoConfig() = default;
		bool NemoConfig::DEBUG_FLAG;
		std::string NemoConfig::main_config_file = "../config/example_config.json";
		u_long NemoConfig::test = 0;
		std::vector<std::string> NemoConfig::stat_files() const {
			std::vector<std::string> ofs;

			ofs.push_back(output_spike_file);
			ofs.push_back(output_nos_stat_file);
			ofs.push_back(output_membrane_pot_file);
			 return ofs;
		}

		std::unique_ptr<NemoModel> create_model_from_cfg(const configuru::Config& c)
		{
			auto nm = std::make_unique<NemoModel>();

			nm->id = (int)c["id"];
			nm->needed_cores = (int)c["needed_cores"];
			nm->model_file_path = (std::string)c["model_file_path"];
			nm->spike_file_path = (std::string)c["spike_file_path"];
			nm->requested_time = (int)c["requested_time"];
			nm->benchmark_model_name = (std::string)c["benchmark_model_name"];
			return nm;
		}

		NemoModel NemoConfig::get_model(int model_id){
			return models[model_id];
		}

		//template<>
		//std::ostream& operator<<(std::ostream& output_stream, const NemoModel& p) {

		std::string NemoModel::to_string() const {
			char fmtp[90] = {'\0'};
			char idf[30] = {'\0'};
			char bdf[30] = {'\0'};
			snprintf(idf, 20, "id: %d nid: %d ", this->id, this->needed_cores);
			snprintf(bdf, 20, "Time: %.1f", this->requested_time);
			//snprintf(fmtp,80,"%-10s%-10s Benchmark: %-10s",idf,bdf,p.benchmark_model_name.c_str());
			snprintf(fmtp, 80, "%-10s%-10s", idf, bdf);
			return std::string(fmtp);
			//return output_stream << "id:" << p.id << "nc:" << p.needed_cores << "\npaths:" << p.model_file_path << "\n" << p.spike_file_path << "\n";
		}
		std::ostream& operator<<(std::ostream& os, const NemoModel& model) {

			os << model.to_string();
			return os;
		}

		std::ostream& operator<<(std::ostream& os, const ScheduledTask& task) {
			os << "*"
			   << "start_time: " << task.start_time << " task_id: " << task.task_id;
			return os;
		}
	}// namespace config
}// namespace nemo

#pragma clang diagnostic pop