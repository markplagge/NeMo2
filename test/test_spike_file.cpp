//
// Created by Mark Plagge on 6/23/20.
//
#include "../src/nemo/nemo_io/SpikeFile.h"
#include "./catch/catch.hpp"
#include "test_utils.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

std::string demo_spike_file("/Users/plaggm/dev/nemo-codes/config/models/mnist_spike.json");
int n_cores = 120;
int n_axons = 10;


TEST_CASE("Spike file init") {
  auto sf = nemo::SpikeFile(demo_spike_file);
  REQUIRE(sf.input_spikes.size() > 0);
}


TEST_CASE("Demo Spike Check"){

  std::vector<unsigned int> times;
  std::vector<unsigned int> cores;
  std::vector<unsigned int> axons;
  std::string spike_check_file = "../config/models/mini_spike.csv";
  std::ifstream test_sp_file(spike_check_file);

  std::string line;
  while (getline(test_sp_file, line)){
    auto spdat = split(line,',');
    times.push_back(spdat[0]);
    cores.push_back(spdat[1]);
    axons.push_back(spdat[2]);
  }

  auto sf = nemo::SpikeFile(demo_spike_file);

  for (auto time : times) {
    for (auto core : cores) {
      for (auto axon : axons){
        auto sfd = sf.get_spikes_at_time(time);
        std::vector<int>::iterator it;
        nemo::SpikeRep sv {time,core,axon};
        auto res = std::find(sfd.begin(),sfd.end(),sv);
        INFO("Time: "<< time << " Core: " << core << " Axon:" << axon);
        REQUIRE(res != sfd.end());
      }
    }
  }
}