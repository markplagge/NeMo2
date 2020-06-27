//
// Created by Mark Plagge on 11/1/19.
//

#ifndef NEMOTNG_NEMO_H
#define NEMOTNG_NEMO_H
#include "../nemo_config/NemoConfig.h"
#include "../nemo_globals.h"
#include "../nemo_io/NemoCoreOutput.h"
#include "../nemo_neuro_system/neurosynaptic_cores/NemoCoreDefs.h"
#include "../nemo_neuro_system/neurosynaptic_cores/NemoNeuroCoreBase.h"
#include "../nemo_neuro_system/neurosynaptic_cores/NemoCoreScheduler.h"
#include "../nemo_ross.h"
#include <ross.h>

namespace nemo {
	extern config::NemoConfig* global_config;
	class Nemo {

		config::NemoConfig nemo_config;
	};

}// namespace nemo
#endif//NEMOTNG_NEMO_H
