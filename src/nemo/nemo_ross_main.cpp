/**
* NeMo main file - taken from the main nemo.c program
* Created by Mark Plagge on 4/30/20.
*/
#include <iostream>
#include <ross.h>

#include "include/nemo.h"
#include "nemo_config/NemoConfig.h"
#include "nemo_formatted_print.h"
#include "nemo_neuro_system/neurosynaptic_cores/NemoCoreDefs.h"
#include "nemo_neuro_system/neurosynaptic_cores/NemoCoreLPWrapper.h"
#include "nemo_neuro_system/neurosynaptic_cores/NemoNeuroCoreBase.h"
#include <codecvt>
namespace nemo {
	config::NemoConfig* global_config = NULL;
	namespace config {
		extern unsigned int LPS_PER_PE;
	}
	namespace p {

		extern int VERBOSE;
		template<>
		void pr_e(std::string desc, config::ScheduledTask elm) {
			std::stringstream elx;
			elx << elm;

			pr_e(desc, elx.str());
		}
		template<>
		void print_config_element<config::NemoModel>(config::NemoModel const& elm) {
			ss << rowstar << elm << rowstar << endl;
		}
		template<typename T>
		void pr_loc_h(T hdr_msg) {
			char str_buffer[200] = {'\0'};

			snprintf(str_buffer, 99, "%s%20s%s%38s\n", rowstar.c_str(), " ",
					 print_as_color(hdr_msg, ansi_color_code::bright_green).c_str(), rowstar.c_str());
			ss << print_as_color(vx, ansi_color_code::cyan) << endl
			   << str_buffer;
		}

		template<>
		void print_vector_limit<>(std::vector<config::ScheduledTask> elms) {
			int chk = 0;

			chk += 1;
			if (VERBOSE != 0 && VERBOSE == chk) {
				//break;
			}
		}
		template<>
		void print_vector_limit<config::NemoModel>(std::vector<config::NemoModel> elms) {
			pr_loc_h("Models in Sim:");
			//			snprintf(vmh,99,"%s%20s%s%38s\n",rowstar.c_str()," ",
			//					 print_as_color("Models In Sim:",ansi_color_code::bright_green).c_str(),rowstar.c_str());

			//ss << vmh;
			//ss << print_as_color(vx, ansi_color_code::cyan) << endl;
			//			pr_e("Models in Sim:"," ");
			int chk = 0;

			for (auto& e : elms) {

				//print_config_element(e);
				ss << tbstop << pr_emph("Model: ") << endl
				   << tbstop << e;
				//ss << setw(22) << rowstar  <<endl;
				std::string nm("NONE");
				if (e.benchmark_model_name.compare("NONE")) {
					ss << endl;
					ss << tbstop << tbstop << setw(4) << "SPK: " << setw(30) << e.spike_file_path << endl;
					ss << tbstop << tbstop << setw(4) << "MDL: " << setw(30) << e.model_file_path << endl;
				}
				else {
					ss << endl;
					ss << tbstop << tbstop << pr_emph(" Benchmark Model: ") << endl;
					ss << tbstop << tbstop << tbstop << e.benchmark_model_name << endl;
				}

				ss << line << endl;
				chk += 1;
				if (VERBOSE != 0 && VERBOSE == chk) {
					break;
				}
			}
			ss << print_as_color(vx, ansi_color_code::cyan) << endl;
		}
		template<>
		void print_config_vector<config::NemoModel>(std::vector<config::NemoModel> elms) {
			print_vector_limit(elms);
		}
		template<>
		void print_config_vector<config::ScheduledTask>(std::vector<config::ScheduledTask> elms) {
			pr_e("NeuroOS Task Schedule:", " ");
			print_vector_limit(elms);
		}
		template<>
		void pr_v<config::ScheduledTask>(std::string desc, std::vector<config::ScheduledTask> elms) {
			print_vector_limit(elms);
		}

	}// namespace p
}// namespace nemo
using namespace nemo;

/*
 * Initialize globals here
 */
unsigned int ::nemo::config::SYNAPSES_IN_CORE = 1;//(NEURONS_IN_CORE * AXONS_IN_CORE);
unsigned int ::nemo::config::CORE_SIZE = 1;
unsigned int ::nemo::config::SIM_SIZE = 1;
unsigned int ::nemo::config::LPS_PER_PE = 1;
int ::nemo::p::VERBOSE = 1;

tw_lptype ne_lps[8] = {
		{(init_f)neuro_system::sched_core_init,
		 (pre_run_f)neuro_system::sched_pre_run,
		 (event_f)neuro_system::sched_forward_event,
		 (revent_f)neuro_system::sched_reverse_event,
		 (commit_f)neuro_system::sched_core_commit,
		 (final_f)neuro_system::sched_core_finish,
		 (map_f)neuro_system::sched_map_linear,
		 sizeof(neuro_system::NemoCoreScheduler)},
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

void init_nemo(nemo::config::NemoConfig cfg) {
	using namespace config;
	CORE_SIZE = cfg.ns_cores_per_chip;
	SIM_SIZE = cfg.total_chips * CORE_SIZE;
	LPS_PER_PE = cfg.world_size / LPS_PER_PE;
	SYNAPSES_IN_CORE = 1;// still one due to super-synapse logic from Original nemo
}

void print_sim_config() {
	using namespace config;
	using namespace p;

	start_sim_hdr();
	pr_e<uint>(std::string("Number of NS Cores: "), global_config->total_chips);
	pr_e<u_int>("Neurons Per Core: ", global_config->neurons_per_core);

	pr_v<NemoModel>("Models", global_config->models);

	pr_v("Scheduler Inputs", global_config->scheduler_inputs);

	p::start_sim_ftr();
}

int main(int argc, char* argv[]) {
	p::VERBOSE = 5;
	using namespace nemo;
	auto main_config = new nemo::config::NemoConfig();
	auto options = nemo::config::NemoConfig::nemo_tw_options;
	tw_opt_add(options);
	tw_init(&argc, &argv);

	g_tw_lookahead = 0.001;
	auto cw = MPI_COMM_WORLD;
	int w_size;
	MPI_Comm_size(cw, &w_size);
	main_config->world_size = w_size;

	//After parsing options, and after tw_init is called, init the main_config
	main_config->init_from_tw_opts();
	nemo::global_config = main_config;
	print_sim_config();
	// initialize ROSS
	// main_config -> load network defs
	//main_config -> init

	delete (main_config);
}