//
// Created by Mark Plagge on 5/2/20.
//

#include "ModelFile.h"
#include <fstream>
#include <json.hpp>
#include <iostream>
#include <sstream>
#include "get_js_mp_file.h"
// Global static pointer used to ensure a single instance of the class.
namespace nemo {

	ModelFile::ModelFile(const std::string& model_file_path) : model_file_path(model_file_path) {
		//load up the model:
		load_model();


	}
	void  ModelFile::load_model(){
		using namespace nlohmann;
		// Supports both json and messagepack formats using fancy nhlo


//		std::ifstream i(model_file_path);

//		if (model_file_path.find(".mb")!= std::string::npos){
//			j = json::from_msgpack(i);
//		}else {
//
//			i >> j;
//		}
		auto j =nemo::file_help::load_json_data(model_file_path);



		int itr = 0;
		for (json::iterator it = j.begin(); it != j.end(); ++it) {
			//js_map[it.key()] = it.value().dump();
			auto coreid = it.value()["coreID"].get<unsigned long>();
			auto neuronid = it.value()["localID"].get<unsigned long>();
			if (itr == 0){
				auto core_id = it.value()["model_id"].dump();
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
	bool ModelFile::is_valid_model() const {
		return valid_model;
	}

	std::string ModelFile::get_core_settings(unsigned long core_id) {
		std::stringstream s;
		auto cid_elms = js_map[core_id];
		for (const auto& neuron : cid_elms) {
			s << neuron.second << "\n";
		}

		return s.str();
	}

}
