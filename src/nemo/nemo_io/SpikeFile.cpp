//
// Created by Mark Plagge on 6/23/20.
//

#include "SpikeFile.h"
#include <json.hpp>
#include <fstream>
#include "get_js_mp_file.h"
namespace nemo {

	SpikeFile::SpikeFile(const std::string& spike_filename) : spike_filename(spike_filename) {
		load_spike_file();
	}
	void SpikeFile::load_spike_file() {
		using namespace nlohmann;
		using namespace std;
//		ifstream spikef(spike_filename);

		auto j = nemo::file_help::load_json_data(spike_filename);
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
		// if you don't need to allocate a new vector, don't do it (hence the if/else/while)
		std::vector<SpikeRep> spikes;
		if (time == 0){
			if (input_spikes.count(current_time) >= 1) {
				//sp = std::move(input_spikes[current_time]);
				//current_time ++;
				spikes = input_spikes[current_time];
			}

		}else{

			time -= this->spike_offset;
			while(time > this->current_time){
				if (input_spikes.count(time) >= 1){
					auto spx = input_spikes[time];
					//spikes.vector1.insert( vector1.end(), vector2.begin(), vector2.end() );
					spikes.insert(spikes.end(),spx.begin(),spx.end());
				}

				current_time ++;
			}

		}
		return std::move(spikes);
	}
	void SpikeFile::parent_process_wait() {
		this->spike_offset ++;
	}


}