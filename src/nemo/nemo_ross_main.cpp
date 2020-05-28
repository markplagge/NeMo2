//
// Created by Mark Plagge on 4/30/20.
//
#include <iostream>
#include <ross.h>

#include "include/nemo.h"
#include "nemo_config/NemoConfig.h"
extern tw_lptype ne_lps[8];



int main (int argc, char *argv[]){

	using namespace  nemo;
	auto main_config = new nemo::NemoConfig();
	auto options = main_config->nemo_tw_options;
	tw_opt_add(options);
	tw_init(&argc, &argv);
	g_tw_lookahead = 0.001;
	auto cw = MPI_COMM_WORLD;
	int w_size;
	MPI_Comm_size(cw,&w_size);

	long nlp_per_pe = main_config->number_of_cores / w_size;
	std::cout << "Total Cores: " << main_config -> number_of_cores << " Cores per PE: " << nlp_per_pe << "\n";
	//tw_define_lps()
	//main_config -> set the options from the option file
	main_config->init_from_tw_opts(options);
	std::cout << NemoConfig::mean << '\n';
	//main_config -> load network defs
	//main_config -> init


}