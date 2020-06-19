/**
* NeMo main file - taken from the main nemo.c program
* Created by Mark Plagge on 4/30/20.
*/
#include <iostream>
#include <ross.h>

#include "include/nemo.h"
#include "nemo_config/NemoConfig.h"
#include "nemo_neuro_system/neurosynaptic_cores/NemoCoreLPWrapper.h"
extern tw_lptype ne_lps[8];
namespace nemo {
	config::NemoConfig* global_config = NULL;
}
using namespace nemo;

tw_lptype ne_lps[8] = {
		{0},
		{(init_f)neuro_system::NemoCoreLPWrapper::core_init,
		 (pre_run_f)neuro_system::NemoCoreLPWrapper::pre_run,
		 (event_f)neuro_system::NemoCoreLPWrapper::forward_event,
		 (revent_f)neuro_system::NemoCoreLPWrapper::reverse_event,
		 (commit_f)neuro_system::NemoCoreLPWrapper::core_commit,
		 (final_f)neuro_system::NemoCoreLPWrapper::core_finish,
		 (map_f)config::nemo_map_linear,
		 sizeof(neuro_system::NemoNeuroCoreBase)},
		{0},
};

int main(int argc, char* argv[])
{
	using namespace nemo;
	auto main_config = new nemo::config::NemoConfig();
	auto options = main_config->nemo_tw_options;
	tw_opt_add(options);
	tw_init(&argc, &argv);
	g_tw_lookahead = 0.001;
	auto cw = MPI_COMM_WORLD;
	int w_size;
	MPI_Comm_size(cw, &w_size);
	main_config->init_from_tw_opts();
	nemo::global_config = main_config;
	// initialize ROSS
	// main_config -> load network defs
	//main_config -> init
}