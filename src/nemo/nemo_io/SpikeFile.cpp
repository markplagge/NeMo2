//
// Created by Mark Plagge on 6/23/20.
//

#include "SpikeFile.h"
#include <json.hpp>
#include <fstream>

namespace nemo {

	SpikeFile::SpikeFile(const std::string& spike_filename) : spike_filename(spike_filename) {
		load_spike_file();
	}
	void SpikeFile::load_spike_file() {
		using namespace nlohmann;
		using namespace std;
		ifstream spikef(spike_filename);
		json j;
		spikef >> j;
		auto spike_arr = j[0];
		//[{"time":2,"core":392,"axon":0}, <- spike file format
		for (json::iterator it = j.begin(); it != j.end(); ++ it){
			auto v = it.value();
			auto time = v["time"].get<unsigned int>();
			SpikeRep spike {
					time,
					v["core"].get<unsigned int>(),
					        v["axon"].get<unsigned int>()
			};
			input_spikes[time].push_back(spike);
		}
	}

	std::vector<SpikeRep> SpikeFile::get_spikes_at_time(unsigned int time) {
		return input_spikes[time];
	}

}