//
// Created by Mark Plagge on 11/1/19.
//

#ifndef NEMOTNG_NEMO_H
#define NEMOTNG_NEMO_H
#include "../nemo_config/NemoConfig.h"
#include "../nemo_globals.h"
#include "../nemo_io/NemoCoreOutput.h"
#include "../nemo_neuro_system/neurosynaptic_cores/NemoNeuroCoreBase.h"
#include "../nemo_ross.h"

namespace nemo
{
class Nemo {

  NemoConfig nemo_config;
  void
  init_nemo ();
};

}// namespace nemo
#endif//NEMOTNG_NEMO_H
