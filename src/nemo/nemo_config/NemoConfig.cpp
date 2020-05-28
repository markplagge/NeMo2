//
// Created by Mark Plagge on 12/6/19.
//

#include "NemoConfig.h"
#include <iostream>
#include <ross.h>
namespace nemo {
	namespace config {
		std::string NemoConfig::get_settings() {
			std::stringstream output;
			output << "Cores per chip: " << ns_cores_per_chip << "\n";
			output << "Total Chips" <<  total_chips << "\n";
			output << "Sim size (total NS CORES)" << total_sim_size << "\n";
			output << "Scheduler in use? " << do_neuro_os << "\n";
			if (do_neuro_os){
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

		tw_lptype ne_lps[8] = {
				{(init_f)neuro_system::NemoCoreLPWrapper::core_init,
				 (pre_run_f)neuro_system::NemoCoreLPWrapper::pre_run,
				 (event_f)neuro_system::NemoCoreLPWrapper::forward_event,
				 (revent_f)neuro_system::NemoCoreLPWrapper::reverse_event,
				 (commit_f)neuro_system::NemoCoreLPWrapper::core_commit,
				 (final_f)neuro_system::NemoCoreLPWrapper::core_finish,
				 (map_f)nemo_map_linear,
				 sizeof(neuro_system::NemoNeuroCoreBase)},
				{0},
		};

		tw_optdef NemoConfig::nemo_tw_options[] = {
				TWOPT_GROUP("NeMo 2 - TNG Runtime Options"),
				TWOPT_FLAG("debug", NemoConfig::DEBUG_FLAG, "Debug mode?"),
				TWOPT_ULONG("mean", NemoConfig::mean, "test_value"),
				TWOPT_CHAR("cfg",NemoConfig::main_config_file, "Main configuration file"),
				TWOPT_END()};

		tw_peid nemo_map_linear(tw_lpid gid)
		{

			return 1;
		}

		void NemoConfig::init_from_tw_opts()
		{


		}
		NemoConfig::NemoConfig()
		{
		}
	}
}// namespace nemo
