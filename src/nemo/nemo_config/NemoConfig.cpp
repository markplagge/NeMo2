//
// Created by Mark Plagge on 12/6/19.
//

#include "NemoConfig.h"
#include <iostream>
#include <ross.h>
namespace nemo {
	tw_lptype ne_lps[8] = {
			{(init_f)neuro_system::NemoCoreLPWrapper::core_init,
			 (pre_run_f)neuro_system::NemoCoreLPWrapper::pre_run,
			 (event_f)neuro_system::NemoCoreLPWrapper::forward_event,
			 (revent_f)neuro_system::NemoCoreLPWrapper::reverse_event,
			 (commit_f)neuro_system::NemoCoreLPWrapper::core_commit,
			 (final_f)neuro_system::NemoCoreLPWrapper::core_finish,
			 (map_f)NemoMapLinear,
			 sizeof(neuro_system::NemoNeuroCoreBase)},
			{0},
	};
	bool NemoConfig::DEBUG_FLAG = false;
	u_long NemoConfig::mean;
	int path_char_size = 2048;


	char *NemoConfig::model_file_path =(char *) calloc(sizeof(char),path_char_size);
	char *NemoConfig::spike_file_path =(char *) calloc(sizeof(char),path_char_size);
	char *NemoConfig::os_cfg_file_path = (char*) calloc(sizeof(char),path_char_size);
	char *NemoConfig::os_sched_mode = "FCFS\0";
	tw_optdef NemoConfig::nemo_tw_options[]= {
			TWOPT_GROUP("NeMo 2 - TNG Runtime Options"),
			TWOPT_FLAG("debug", NemoConfig::DEBUG_FLAG, "Debug mode?"),
			TWOPT_ULONG("mean", NemoConfig::mean, "test_value"),
			TWOPT_CHAR("model", NemoConfig::model_file_path,"Path to the model file"),
			TWOPT_CHAR("input", NemoConfig::spike_file_path,"Path to the spike input file"),
			TWOPT_GROUP("NeMo NeuroOS Options"),
			TWOPT_CHAR("os_cfg_file", NemoConfig::os_cfg_file_path, "Path to the NeuroOS config file."
																	"Should be JSON format. See neuro_os.json for examples."),
			TWOPT_CHAR("os_sched", NemoConfig::os_sched_mode, "Scheduler mode. Must be one of 'FCFS','RR','FS' (FS TBD)"),
			TWOPT_END()};

	tw_peid	NemoMapLinear(tw_lpid gid){

		return 1;
	}

	void NemoConfig::init_from_tw_opts(){


	}
	NemoConfig::NemoConfig() {

	}
}// namespace nemo
