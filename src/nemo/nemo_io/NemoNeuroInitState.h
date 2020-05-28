//
// Created by Mark Plagge on 3/29/20.
//

#ifndef NEMOTNG_NEMONEUROINITSTATE_H
#define NEMOTNG_NEMONEUROINITSTATE_H

namespace nemo
{
namespace neuro_system
{
/**
 * NemoNeuroInitState
 * A class that contains the data required for an individual neuron
 * This is used to help make neurons from data sources - the Nemo input file
 * that defines models is read in - and line by line the string data is fed into the nemo neuro system.
 *
 */
class NemoNeuroInitState {
  double membrane_pot = 0;
  double leak_v = -1;
  double threshold = 1;
  double reset_val = 0;
};

class NemoNeuroInitLIF: NemoNeuroInitState {
};

class NemoNeuroInitTN: NemoNeuroInitState {
};

class NemoNeuroInitData {
};

}// namespace neuro_system
}// namespace nemo

#endif//NEMOTNG_NEMONEUROINITSTATE_H
