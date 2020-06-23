//
// Created by Mark Plagge on 6/23/20.
//

#ifndef NEMOTNG_SPIKEFILE_H
#define NEMOTNG_SPIKEFILE_H
#include <json.hpp>
#include <map>
#include <string>
#include <vector>

namespace nemo {
	struct SpikeRep {
		unsigned int time;
		unsigned int dest_core;
		unsigned int dest_axon;
		bool operator==(const SpikeRep& rhs) const {
			return time == rhs.time && dest_core == rhs.dest_core && dest_axon == rhs.dest_axon;
		}
		bool operator!=(const SpikeRep& rhs) const {
			return !(rhs == *this);
		}

	};


	//[{"time":2,"core":392,"axon":0}, <- spike file format
	class SpikeFile {
	public:
		SpikeFile(const std::string& spike_filename);
		std::vector<SpikeRep> get_spikes_at_time(unsigned int time);
		/* Public for testing */
		std::map<int, std::vector<SpikeRep>> input_spikes;
	private:
		std::string spike_filename;
		void load_spike_file();

	};

}





#endif//NEMOTNG_SPIKEFILE_H
