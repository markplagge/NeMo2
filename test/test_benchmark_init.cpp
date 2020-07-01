//
// Created by Mark Plagge on 6/30/20.
//
#include "../src/nemo/nemo_io/SpikeFile.h"
#include "../src/nemo/nemo_neuro_system/BenchmarkModelDef.cpp"
#include "./catch/catch.hpp"
#include "test_utils.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
namespace nemo {
namespace neuro_system {
      struct GenericID_BaseTest : BenchmarkModelInit<GenericID_BaseTest>{

      };

}
}
TEST_CASE("BASE INIT TEST"){
  nemo::neuro_system::GenericID_BaseTest base;
  base.init_benchmark_neuron_interface(42, 128);
  for (auto &weight : base.weights) {
    REQUIRE(weight == -1);
  }
  REQUIRE(base.weights.size() > 0);
}

TEST_CASE("IDENTITY BENCHMARK WEIGHTS"){
   nemo::neuro_system::GenericIdentityInit identity_init;
   identity_init.init_benchmark_neuron_interface(42, 128);
   int i = 0;
   for (const auto &weight : identity_init.weights) {
     if (i++ == 42){
       REQUIRE(weight == 1);
     }else{
       REQUIRE(weight == 0);
     }

   }
}