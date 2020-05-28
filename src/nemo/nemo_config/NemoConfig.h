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
	struct NemoConfig {


		int number_of_cores = 4096;
		::std::vector<core_types> core_map;
		//core_types GetCoreType(::std::string core_type){}
		::std::map<core_types, neuro_system::NemoNeuroCoreBase*> core_type_map;
		static tw_petype main_pe[4];
		//const tw_petype* get_main_pe() const {};
		static  bool DEBUG_FLAG;
		static tw_optdef nemo_tw_options[];
		static u_long mean;
		static std::string _model_file_path;
		static char *model_file_path;
		static char *spike_file_path;
		static char *os_cfg_file_path;
		static char *os_sched_mode;
		NemoConfig();
		void init_from_tw_opts();

	};
	void NemoPostLpInit(tw_pe* pe);
	tw_peid NemoMapLinear(tw_lpid gid);
	extern tw_lptype ne_lps[8];


	class NemoCFG {
	public:

	};

}// namespace nemo
#endif// NEMOTNG_NEMOCONFIG_H
/*
 *
 */
