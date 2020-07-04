//
// Created by Mark Plagge on 5/2/20.
//

#include "ModelFile.h"
#include "get_js_mp_file.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <ross.h>
#include <sstream>
#include <utility>
#include <ross.h>
// Global static pointer used to ensure a single instance of the class.
namespace nemo {

	namespace util {
		std::string replace_string_regex( std::string const & in, std::string  const & from, std::string  const & to ){
			return std::regex_replace( in, std::regex(from), to );
		}
		void replace_string_in_place(std::string& subject, const std::string& search,
				const std::string& replace) {
			size_t pos = 0;
			while((pos = subject.find(search, pos)) != std::string::npos) {
				subject.replace(pos, search.length(), replace);
				pos += replace.length();
			}
		}
	}
	ModelFile::ModelFile(std::string  model_file_path) : model_file_path(std::move(model_file_path)) {
		//load up the model:
		load_model();


	}
	int ModelFile::read_file(const std::string& model_path){
		std::ifstream is (model_path,std::ifstream::binary);
		if(!is.is_open()) {
			tw_error(TW_LOC, "Model with path %s was not opened.",model_path.c_str());
			exit(-1);
		}
		is.seekg (0, is.end);
		int length = is.tellg();
		is.seekg (0, is.beg);
		char * buffer = new char [length];
		is.read(buffer,length);
		if (is) {
			std::cout << "Read " << length << " bytes \n";
			auto current_pos = buffer;

			auto line = buffer;
			bool did_fix = false;
			for (int i = 0; i < length; i++) {

				if (*current_pos == '\n') {
					*current_pos = '\0';
					if (!did_fix) {
						did_fix = parse_line(line);
					}
					else {
						did_fix = false;
					}
					line = current_pos + 1;
				}
				current_pos++;
			}
		}
		is.close();
		delete[](buffer);
		return length;
	}
	bool ModelFile::fix_extra_line_dat(char* linep) {
		auto has_extra = [](auto line) { return strstr(line, ",\"TN_"); };
		bool fixed = false;
		auto tl = has_extra(linep);
		if (tl != NULL) {
			fixed = true;
			int ipos = 0;
			tl[ipos] = '#';
			//			while(++ipos < 256){
			//				if (tl[ipos] == '{'){
			//					break;
			//				}else{
			//					tl[ipos] = '#';
			//
			//				}
			//			}
			fix_extra_line_dat(linep);
		}
		return fixed;
	}
	std::stringstream buf_str;
	bool ModelFile::parse_line(char* line) {
		long core_id;
		long neuron_id;
		const char tok[2] = "_";
		const char enter_tok[3] = "\"";
		auto data = strchr(line, ':') + 1;

		bool did_fix = fix_extra_line_dat(data);
		char* d1;
		if (did_fix) {
			auto x = 3;
			d1 = strchr(data, '#');
			char dx[strlen(d1) + 5];
			dx[strlen(d1) + 4] = '\0';
			char* d2 = dx;
			strncpy(d2, d1, strlen(d1) + 4);
			d2[0] = '\0';
			d2[strlen(d2) + 1] = ' ';
			d2[strlen(d2) + 2] = ' ';
			d2++;
			d1[0] = ' ';
			d1[1] = '\0';
			parse_line(d2);
		}
		auto datalen = strlen(data);
		if (data[datalen - 1] == '\n') {
			data[datalen - 2] = ' ';
		}
		data[datalen - 1] = '\n';

		std::string dat_s(data);
		char* token = strtok(line, tok) + 1;
		char* stred;

		token = strtok(NULL, tok);
		core_id = strtol(token, &stred, 10);
		token = strtok(NULL, enter_tok);
		neuron_id = strtol(token, &stred, 10);
		if (js_core_map[core_id] == nullptr) {
			js_core_map[core_id] = new std::stringstream();
		}

		*js_core_map[core_id] << data;
		return did_fix;
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
		if (js_core_map.count(core_id))
			return js_core_map[core_id]->str();
		return std::string();
	}

}
