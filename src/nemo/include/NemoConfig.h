//
// Created by Mark Plagge on 12/6/19.
//

#ifndef NEMOTNG_NEMOCONFIG_H
#define NEMOTNG_NEMOCONFIG_H
#include <map>
#include <string>
#include "nemo.h"
//#include <rapidjson/rapidjson.h>
class NemoConfig {
    int number_of_cores = 4096;
    std::vector<core_types> core_map;
    core_types get_core_type(std::string core_type){
    }
    std::map<core_types, INeuroCoreBase> core_type_map;


};

const tw_optdef nemo_tw_options [] =
        {
                TWOPT_GROUP("NeMo 2 - TNG Runtime Options" ),

                TWOPT_END()
        };
#endif //NEMOTNG_NEMOCONFIG_H
