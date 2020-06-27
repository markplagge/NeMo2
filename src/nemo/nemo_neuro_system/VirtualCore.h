//
// Created by Mark Plagge on 6/25/20.
//

#ifndef NEMOTNG_VIRTUALCORE_H
#define NEMOTNG_VIRTUALCORE_H
#include <vector>

struct VirtualCore {

	//given a core def (from modelfile), and
	/*
	 * original:
	 * core_x,neuron_n  -> core_y, neuron_y
	 * virtualCore:
	 * core_z = core_y
	 */


	unsigned int core_translation(unsigned int process_dest_core, unsigned int process_id);

};



#endif//NEMOTNG_VIRTUALCORE_H

