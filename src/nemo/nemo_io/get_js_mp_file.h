//
// Created by Mark Plagge on 6/25/20.
//

#ifndef NEMOTNG_GET_JS_MP_FILE_H
#define NEMOTNG_GET_JS_MP_FILE_H
#include <fstream>
#include <json.hpp>
namespace nemo{
	namespace file_help {

		std::ifstream get_file(std::string model_filename);
		nlohmann::json load_json_data( std::string filename);
	}
}
#endif//NEMOTNG_GET_JS_MP_FILE_H
