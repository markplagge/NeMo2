//
// Created by Mark Plagge on 6/22/20.
//
#include "../src/nemo/nemo_io/ModelFile.h"
#include "./catch/catch.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "test_utils.h"


std::vector<int> get_neuron_core(std::string line){
  //"TN_4_172":{
  std::string delimiter = "_";
  std::string endbit = ":";
  size_t pos = 0;
  std::string token;
  pos = line.find(endbit);
  auto core_name = line.substr(0,pos); //"{TN_4_172"
  core_name.erase(0,1);
  pos = core_name.find(delimiter);
  core_name.erase(0,pos + delimiter.length()); //4_172"
  pos = core_name.find("\"");
  core_name.erase(core_name.length() - 1, 1);

  return split(core_name, '_');

}

int get_num_cores_in_file(const std::string& filename){
   std::ifstream infile (filename);
   std::string line;
   int max_core = 0;
   while (getline(infile, line)){
     auto core_neuron = get_neuron_core(line);
     auto ci = core_neuron[0];
     if (ci > max_core){
       max_core = ci;
     }
   }
   return max_core;
}
int get_num_neurons_per_core(const std::string & filename){
  std::ifstream infile (filename);
  std::string line;
  int max_neuron = 0;
  while (getline(infile, line)){
    auto core_neuron = get_neuron_core(line);
    auto ci = core_neuron[1];
    if (ci > max_neuron){
      max_neuron = ci;
    }
  }
  return max_neuron;
}
int get_num_needed_cores(std::string filename){
    return get_num_cores_in_file(filename) + 1;
};


std::string demo_filename("/Users/plaggm/dev/nemo-codes/config/models/nemo_demo.json");


TEST_CASE("Max Neurons/Cores in demo file"){

  auto max_neuron = get_num_neurons_per_core(demo_filename);
  auto max_core = get_num_cores_in_file(demo_filename);
  REQUIRE(max_neuron == 249);
  REQUIRE(max_core == 4);
  REQUIRE(get_num_needed_cores(demo_filename) == 5);
}
TEST_CASE("Test get_neuron_core Test"){
  std::ifstream infile (demo_filename);
  std::string line;
  int max_0 = 0;
  int max_1 = 0;
  while (getline (infile, line)) {
    // Checks go here for the line
    auto neuron_core_id = get_neuron_core(line);
    INFO("Line: " << line);
    REQUIRE(neuron_core_id[0] < 6);
    REQUIRE(neuron_core_id[1] < 250);
    if (neuron_core_id[0] > max_0 ) {
      max_0 = neuron_core_id[0];
    }
    if (neuron_core_id[1] > max_1){
      max_1 = neuron_core_id[1];
    }
  }
  REQUIRE(max_0 < max_1);
}
TEST_CASE("Model Read Test"){
  using namespace nemo;
  auto mdl_file = ModelFile(demo_filename);
  REQUIRE(mdl_file.get_num_needed_cores() == get_num_needed_cores(demo_filename));
  auto js_dict = mdl_file.get_js_map();
  std::ifstream infile (demo_filename);
  std::string line;
  while (getline (infile, line)){
    // Checks go here for the line
    auto neuron_core_id = get_neuron_core(line);
    auto nid = neuron_core_id[1];
    auto cid = neuron_core_id[0];
    //direct dict check:
    auto neuron_str = js_dict[cid][nid];
    INFO("COREID=" << cid);
    INFO("NID=" << nid);
    REQUIRE (neuron_str.length() > 0);
  }

}