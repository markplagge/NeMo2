//
// Created by Mark Plagge on 6/25/20.
//

#include "get_js_mp_file.h"
#include <ross.h>
#include <sstream>
#include <string>


namespace nemo{
	namespace file_help{


		std::ifstream get_file(std::string filename){
			std::ifstream f(filename);
			if (! f.is_open()){
				std::stringstream s;
				s << "FILE " << filename << " could not be opened";
				tw_error(TW_LOC,s.str().c_str());
				exit(-1);
			}
			return f;
		}


		nlohmann::json load_json_data( std::string filename){
			//valid exts are .mp for messagepack
			nlohmann::json js_dat;
			auto f = get_file(filename);
			if(filename.find(".json") != std::string::npos){
				f >> js_dat;
			}else if (filename.find(".mp") != std::string::npos){
				js_dat = nlohmann::json::from_msgpack(f);
			}else{
				std::stringstream s;
				s << "FILE " << filename << " did not recognize file type (json, mp)";
				tw_error(TW_LOC,s.str().c_str());
			}
			return js_dat;
		}

	}
}