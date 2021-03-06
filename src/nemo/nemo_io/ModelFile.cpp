//
// Created by Mark Plagge on 5/2/20.
//

#include "ModelFile.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>
#include "get_js_mp_file.h"
// Global static pointer used to ensure a single instance of the class.
namespace nemo {


	ModelFile::ModelFile(std::string  model_file_path) : model_file_path(std::move(model_file_path)) {
		//load up the model:
		load_model();


	}
	int ModelFile::read_file(const std::string& model_path){
		std::ifstream is (model_path,std::ifstream::binary);
		is.seekg (0, is.end);
		int length = is.tellg();
		is.seekg (0, is.beg);
		char * buffer = new char [length];
		is.read(buffer,length);
		if (is){
			std::cout << "Read " << length << " bytes \n";
			auto current_pos = buffer;

			auto line = buffer;
			for(int i = 0; i < length; i ++ ){
				if(*current_pos == '\n'){
					*current_pos = '\0';
					parse_line(line);
					line = current_pos + 1;
				}
				current_pos ++;

			}
		}
		is.close();
		delete[] (buffer);
		return length;
	}
	void ModelFile::parse_line(char line[]){
		long core_id;
		long neuron_id;
		const char tok[2] = "_";
		const char enter_tok[3] = "\"";
		auto data = strchr(line,':') + 1;

		auto datalen = strlen(data);
		if (data[datalen - 1] == '\n') {
			data[datalen - 2] = ' ';
		}
		data[datalen - 1] = '\n';

		std::string dat_s(data);
		char *token = strtok(line, tok) +1;
		char * stred;

		token = strtok(NULL, tok);
		core_id = strtol(token,&stred,10);
		token = strtok(NULL, enter_tok);
		neuron_id = strtol(token, &stred, 10);
		if (js_core_map[core_id] == nullptr){
			js_core_map[core_id] = new std::stringstream ();
		}
		*js_core_map[core_id] << data;

	}
	void  ModelFile::load_model(){
		auto sz_loaded = read_file(model_file_path);
		num_needed_cores = js_core_map.size();

	}
	int ModelFile::get_num_needed_cores() const {
		return num_needed_cores;
	}

	bool ModelFile::is_valid_model() const {
		return valid_model;
	}

	std::string ModelFile::get_core_settings(unsigned long core_id) {
		//return js_core_map[core_id].str();
		return js_core_map[core_id]->str();

	}

}
