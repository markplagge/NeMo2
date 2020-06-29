//
// Created by Mark Plagge on 11/1/19.
//

#include "NemoCoreOutput.h"
#include <mpi.h>

#include <utility>
void nemo::NemoCoreOutput::save_spike(long source_neuron, long dest_core, long dest_axon, double sched_spike_time, double cur_time, bool is_interchip) const {
	NemoSpikeData s{};
	s.source_neuron = source_neuron;
	s.dest_core = dest_core;
	s.source_core = core_id;
	s.dest_axon = dest_axon;
	s.dest_core = dest_core;
	s.cur_time = cur_time;
	s.sched_spike_time = sched_spike_time;
	s.interchip = is_interchip;
	output_handler->add_data(s);
}
void nemo::NemoCoreOutput::save_spike(long source_neuron, long dest_core, long dest_axon, double sched_spike_time,
									  double cur_time) const {
	save_spike(source_neuron, dest_core, dest_axon, sched_spike_time, cur_time, false);
}

void nemo::NemoCoreOutput::save_membrane_pot(long source_neuron, double membrane_pot, double cur_time) const {
	NemoMembraneData m{};
	m.source_core = core_id;
	m.membrane_pot = membrane_pot;
	m.source_neuron = source_neuron;
	m.cur_time = cur_time;
	output_handler->add_data(m);
}

void nemo::NemoCoreOutput::save_spike_data(nemo::NemoSpikeData d) {
}

void nemo::NemoCoreOutput::save_membrane_data(nemo::NemoMembraneData d) {
}

nemo::NemoCoreOutput::NemoCoreOutput(long core_id, nemo::NemoOutputHandler* output_handler)
	: core_id(core_id),output_handler(output_handler) {}

std::ostream& nemo::operator<<(std::ostream& os, const nemo::NemoCoreOutput& output) {
	os << "core_id: " << output.core_id << " output_handler: " << output.output_handler;
	return os;
}

void nemo::NemoPosixOut::open_comms() {
	sp_f_out.open(spike_out);
	mp_f_out.open(mpot_out);
}

void nemo::NemoPosixOut::close_comms() {
	sp_f_out.close();
	mp_f_out.close();
}

void nemo::NemoPosixOut::write() {
	if (spike_data.size() > prewrite_buffer) {
		if (sp_out_hdr == 0) {
			sp_f_out << spike_data[0].get_header();
			sp_out_hdr = 1;
		}
		for (NemoSpikeData datum : spike_data) {
			sp_f_out << datum.to_csv();
		}
	}
	if (mem_data.size() > prewrite_buffer) {
		if (mp_out_hdr == 0) {
			mp_out_hdr = 1;
			mp_f_out << mem_data[0].get_header();
		}
		for (NemoSpikeData datum : spike_data) {
			mp_f_out << datum.to_csv();
		}
	}
}

nemo::NemoPosixOut::NemoPosixOut(std::string filename, int rank): filename(std::move(filename)),rank(rank) {
	mpot_out = std::string("mpot_") + filename;

}

nemo::NemoPosixOut::NemoPosixOut() {
	int rnk;
	MPI_Comm_rank(MPI_COMM_WORLD, &rnk);
	filename = std::string("output_") + std::to_string(rank) + ".csv";
	spike_out = std::string("spike_") + filename;
	mpot_out = std::string("mpot_") + filename;

	rank = rnk;
}
nemo::NemoPosixOut::NemoPosixOut(std::string spike_fn, std::string mpot_fn, int rank): filename(spike_fn),
													mpot_out(std::move(mpot_fn)),spike_out(spike_fn),rank(rank) {

}
