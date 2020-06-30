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
//		std::string get_core_neuron_settings(unsigned long core_id, unsigned long neuron_id) {
//			return js_map[core_id][neuron_id];
//		}
		std::string get_core_settings(unsigned long core_id);

		ModelFile(std::string  model_file_path);
		ModelFile(){
			valid_model = false;
		}

		int get_num_needed_cores() const;
		bool is_valid_model() const;
		//const std::map<unsigned long, std::map<unsigned long, std::string>>& get_js_map() const;

	protected:
		int read_file(const std::string& model_path);

		void load_model();
		void parse_line(char line[]);
		int num_needed_cores = 0;
		char core_neuron_id[64] = {'\0'};
		bool valid_model = true;
		int model_id = 0;
		std::string model_name;
		//std::map<unsigned long, std::map<unsigned long,std::string>> js_map;
		std::map<unsigned long , std::stringstream * > js_core_map;

		int current_core = 0;
		int current_neuron = 0;
		std::string model_file_path;

		void fix_extra_line_dat(char* linep);
	};

}


#endif//NEMOTNG_MODELFILE_H
