//
// Created by Mark Plagge on 5/2/20.
//

#include "ModelFile.h"
#include <fstream>
#include <json.hpp>
#include <iostream>

// Global static pointer used to ensure a single instance of the class.
namespace nemo {

	ModelFile::ModelFile(const std::string& model_file_path) : model_file_path(model_file_path) {
		//load up the model:
		load_model();


	}
	void  ModelFile::load_model(){
		using namespace nlohmann;
		/* Legacy values include:
		 * cores = 5
		 * neuronsPerCore = 256
		 * neuron_weights = 4
		 * Also some legacy files use no quotes and an '=' for values
		 * So we ignore values until we get to the model def.*/
		std::ifstream i(model_file_path);
		json j;
		i >> j;
		int itr = 0;
		for (json::iterator it = j.begin(); it != j.end(); ++it) {
			//js_map[it.key()] = it.value().dump();
			auto coreid = it.value()["coreID"].get<unsigned long>();
			auto neuronid = it.value()["localID"].get<unsigned long>();
			if (itr == 0){
				auto core_id = it.value()["model_id"].get<std::string>();
				model_name = core_id;
				itr ++;
			}
			js_map[coreid][neuronid] = it.value().dump();

		}
		num_needed_cores = js_map.size();




	}
	int ModelFile::get_num_needed_cores() const {
		return num_needed_cores;
	}
	const std::map<unsigned long, std::map<unsigned long, std::string>>& ModelFile::get_js_map() const {
		return js_map;
	}

}
