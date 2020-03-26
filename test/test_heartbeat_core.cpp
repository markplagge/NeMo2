//
// Created by Mark Plagge on 3/19/20.
//
#include <iostream>
#include "./catch/catch.hpp"
#include "../src/nemo/include/nemo.h"
#include "../src/nemo/nemo_neuro_system/neurosynaptic_cores/HeartbeatTestCore.h"
#include "../external/eigen/Eigen/Dense"
#include "../external/eigen/Eigen/Eigen"
TEST_CASE("NeMo Heartbeat Cores Tests"){
    using namespace nemo::neuro_system;
    auto hbc = HeartbeatTestCore();
    using namespace Eigen;
    ArrayXXi con_mat(4,4);
    con_mat << 1,1,1,1,
               1,0,1,0,
               0,1,0,1,
               0,0,1,1;


    ArrayXXi  wt_mat(4,4);
    wt_mat << 1,1,1,1,
              2,2,2,2,
              3,3,3,3,
              0,-1,2,3;

    ArrayXi current_mpots(4);
    current_mpots << 0,0,0,0;


    int test_spike = 0;

    ArrayXi spike_input(4);
    Array

    auto res = (con_mat(test_spike) * wt_mat(test_spike));
    std::cout << con_mat(test_spike) << "\n";
    std::cout << res << "\n";

    std::cout << wt_mat(test_spike) << "\n";
    std::cout << wt_mat << " - wtmat \n";
}