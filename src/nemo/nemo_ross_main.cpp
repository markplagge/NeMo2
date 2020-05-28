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
	auto main_config = new nemo::config::NemoConfig();
	auto options = main_config->nemo_tw_options;
	tw_opt_add(options);
	tw_init(&argc, &argv);
	g_tw_lookahead = 0.001;
	auto cw = MPI_COMM_WORLD;
	int w_size;
	MPI_Comm_size(cw,&w_size);


	main_config->init_from_tw_opts();

	//main_config -> load network defs
	//main_config -> init


}