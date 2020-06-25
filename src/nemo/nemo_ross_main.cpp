#include <climits>
/**
* NeMo main file - taken from the main nemo.c program
* Created by Mark Plagge on 4/30/20.
*/

#include "include/nemo.h"
#include "mapping_functions.h"
#include "nemo_config/NemoConfig.h"
#include "nemo_formatted_print.h"
#include "nemo_neuro_system/neurosynaptic_cores/NemoCoreDefs.h"
#include "nemo_neuro_system/neurosynaptic_cores/NemoCoreLPWrapper.h"
#include "nemo_neuro_system/neurosynaptic_cores/NemoNeuroCoreBase.h"
#include <codecvt>
#include <iostream>
#include <ross.h>
#include <utility>
namespace nemo {
	config::NemoConfig* global_config = NULL;
	namespace config {
		extern unsigned int LPS_PER_PE;
	}
	namespace p {
		using namespace config;




		template<>
		void print_vector_limit<>(std::vector<config::ScheduledTask> elms) {
			int chk = 0;
		std:
			stringstream sb;
			for (const auto& elm : elms) {
				if (VERBOSE == 0 || VERBOSE != chk) {
					sb << tbstop << pr_emph("Scheduler Entry:") << endl
					   << tbstop << elm;
					for (const auto& item : global_config->models) {
						if (item.id == elm.model_id) {
							sb << " MF: ";
							if (item.benchmark_model_name.compare("NONE")) {
								sb << item.benchmark_model_name << endl;
							}
							else {
								sb << item.model_file_path << endl;
							}
						}
					}
				}
				chk += 1;
			}
			print_vec_nlines(chk, "Scheduled Tasks: ");
			ss << sb.str();
			ss << star_line << endl;
		}
		template<>
		void print_vector_limit<config::NemoModel>(std::vector<config::NemoModel> elms) {

			int chk = 0;
			std::stringstream sb;
			for (auto& e : elms) {
				if (VERBOSE != 0 && VERBOSE == chk) {
				}
				else {

					//print_config_element(e);
					sb << tbstop << pr_emph("Model: ") << chk << endl
					   << tbstop << e;
					//ss << setw(22) << rowstar  <<endl;
					std::string nm("NONE");
					if (not e.benchmark_model_name.compare("NONE")) {
						sb << endl;
						sb << tbstop << tbstop << setw(4) << "SPK: " << setw(30) << e.spike_file_path << endl;
						sb << tbstop << tbstop << setw(4) << "MDL: " << setw(30) << e.model_file_path << endl;
					}
					else {
						sb << endl;
						sb << tbstop << tbstop << pr_emph(" Benchmark Model: ") << endl;
						sb << tbstop << tbstop << tbstop << e.benchmark_model_name << endl;
					}

					sb << line << endl;
				}

				chk += 1;
			}
			ss << pr_loc_h("Models in Sim", chk);
			ss << sb.str();
			ss << star_line << endl;
		}
		template<>
		void print_config_vector<NemoModel>(std::vector<NemoModel> elms) {
			print_vector_limit(std::move(elms));
		}
		template<>
		void print_config_vector<config::ScheduledTask>(std::vector<config::ScheduledTask> elms) {
			//pr_e("NeuroOS Task Schedule:", " ");
			print_vector_limit(std::move(elms));
		}
		template<>
		void pr_v<config::ScheduledTask>(__unused std::string desc, std::vector<config::ScheduledTask> elms) {
			print_vector_limit(std::move(elms));
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
		 (map_f)nemo_map,
		 sizeof(neuro_system::NemoCoreScheduler)},
		{(init_f)neuro_system::NemoNeuroCoreBase::core_init,
		 (pre_run_f)neuro_system::NemoNeuroCoreBase::pre_run,
		 (event_f)neuro_system::NemoNeuroCoreBase::forward_event,
		 (revent_f)neuro_system::NemoNeuroCoreBase::reverse_event,
		 (commit_f)neuro_system::NemoNeuroCoreBase::core_commit,
		 (final_f)neuro_system::NemoNeuroCoreBase::core_finish,
		 (map_f)nemo_map,
		 sizeof(neuro_system::NemoNeuroCoreBase)},
		{0},
};

void init_nemo(nemo::config::NemoConfig* cfg) {
	using namespace config;
	CORE_SIZE = cfg->ns_cores_per_chip * cfg->neurons_per_core;
	SIM_SIZE = cfg->total_chips * CORE_SIZE;
	LPS_PER_PE  = cfg->lps_per_pe;
	SYNAPSES_IN_CORE = 1;// still one due to super-synapse logic from Original nemo

	// configure ROSS
	auto nlp = cfg->lps_per_pe;
	if (g_tw_mynode == 0) {
		nlp++;
	}
	g_tw_nlp = nlp;
	g_tw_lookahead = cfg->lookahead;
	g_tw_lp_types = ne_lps;
	g_tw_lp_typemap = lp_typemapper;
	g_tw_events_per_pe = cfg->est_events_per_pe;

	/** set up LPs */
	tw_define_lps(nlp,sizeof(nemo_message));
	int i = 0;
	if (g_tw_mynode == 0){
		std::cout <<"DEBUG: creating scheduler core" <<endl;
		tw_lp_settype(i, &ne_lps[0]);
		i ++;
	}

	for(; i < (int) g_tw_nlp; i ++){
		tw_lp_settype(i, &ne_lps[1]);
	}

}

void print_sim_config() {
	using namespace config;
	using namespace p;

	start_sim_hdr();
	ss << std::boolalpha;
	pr_e<uint>(std::string("Total chips: "), global_config->total_chips);
	pr_e<u_int>("Neurons Per Core: ", global_config->neurons_per_core);
	if (global_config->do_neuro_os) {
		pr_e("N.O.S. Scheduler Mode: ", global_config->sched_mode_to_string());
	}
	else{
		pr_e("N.O.S. Scheduler Mode: ", "DISABLED");
	}
	pr_e("Total (SIM) LPs :", global_config->total_lps);
	pr_e("LPs per PE:", global_config->lps_per_pe);
	pr_e("Lookahead set at: ", global_config->lookahead);
	pr_e("Preset events per pe: ",global_config->est_events_per_pe);
	pr_e("Save spikes? ",global_config->save_all_spikes);
	pr_e("Save Memb. Pots?",global_config->save_membrane_pots);
	pr_e("Save N.O.S. Stats?",global_config->save_nos_stats);
	pr_v("Stat File Locs:",global_config->stat_files());
	pr_v("Core Types: ", global_config->core_type_ids);
	pr_e("Debug mode? ", config::NemoConfig::DEBUG_FLAG);
	pr_e("Save all spikes? ", global_config->save_all_spikes);
	pr_e("Save membrane potentials? ", global_config->save_membrane_pots);
	pr_e("Save N.O.S. scheduler stats? ", global_config->save_nos_stats);

	pr_v<NemoModel>("", global_config->models);
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
	// initialize ROSS and NeMo:
	init_nemo(main_config);
	if(g_tw_mynode == 0){
		print_sim_config();
	}
	MPI_Barrier(MPI_COMM_WORLD);
	if(tw_ismaster()) printf("@@@ Calling run...\n");
	MPI_Barrier(MPI_COMM_WORLD);
	g_tw_ts_end = 5;
	tw_run();

	// main_config -> load network defs
	//main_config -> init

	delete (main_config);
}