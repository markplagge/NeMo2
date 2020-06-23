//
// Created by Mark Plagge on 5/2/20.
//

#ifndef NEMOTNG_MODELFILE_H
#define NEMOTNG_MODELFILE_H
#include <string>
#include <json.hpp>

namespace nemo {

	class ModelFile {
	public:
		std::string get_core_neuron_settings(unsigned long core_id, unsigned long neuron_id) {
			return js_map[core_id][neuron_id];
		}
		ModelFile(const std::string& model_file_path);

		int get_num_needed_cores() const;
	protected:
		int num_needed_cores = 0;
		char core_neuron_id[64] = {'\0'};
		void load_model();

		int model_id = 0;
		std::string model_name;

	public:
		const std::map<unsigned long, std::map<unsigned long, std::string>>& get_js_map() const;
	protected:
		std::map<unsigned long, std::map<unsigned long,std::string>> js_map;
		int current_core = 0;
		int current_neuron = 0;
		std::string model_file_path;
		nlohmann::json json_model;


	};

}


#endif//NEMOTNG_MODELFILE_H
