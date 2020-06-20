//
// Created by Mark Plagge on 11/1/19.
//
#include "NemoNeuronGeneric.h"
namespace nemo
{
namespace neuro_system
{

	void NemoNeuronGeneric::integrate(unsigned int source_id) {
		membrane_pot = membrane_pot + weights[source_id];
	}
	void NemoNeuronGeneric::leak() {
		membrane_pot += leak_v;
	}
	void NemoNeuronGeneric::leak_n(int n_leaks) {
		for (int i = 0; i < n_leaks; i++) {
			membrane_pot += leak_v;
		}
	}
	bool NemoNeuronGeneric::fire() {
		if (membrane_pot >= threshold) {
			//membrane_pot = reset_val;
			return true;
		}
		else {
			return false;
		}
	}
	void NemoNeuronGeneric::reset() {
		if (membrane_pot >= threshold){
			membrane_pot = reset_val;
		}
	}
	void NemoNeuronGeneric::set_cur_lp(tw_lp* new_lp) {
		NemoNeuronGeneric::cur_lp = new_lp;
	}
	tw_lp* NemoNeuronGeneric::get_cur_lp() const {
		return cur_lp;
	}
	void NemoNeuronGeneric::set_self_manage_spike_events(bool self_manage_spike_events) {
		NemoNeuronGeneric::self_manage_spike_events = self_manage_spike_events;
	}
	bool NemoNeuronGeneric::is_self_manage_spike_events() const {
		return self_manage_spike_events;
	}




}// namespace neuro_system
}// namespace nemo