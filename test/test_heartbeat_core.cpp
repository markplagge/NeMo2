//
// Created by Mark Plagge on 3/19/20.
//
#include <iostream>
#include "./catch/catch.hpp"
#include "../src/nemo/include/nemo.h"

#include "../external/eigen/Eigen/Dense"
#include "../external/eigen/Eigen/Eigen"
TEST_CASE("NeMo Heartbeat Cores Tests"){


}
TEST_CASE("Eigen Tests"){
    using namespace Eigen;
    Matrix<int,Dynamic,Dynamic> con_mat(4,4);
    //neuron matrix:
    //Axons are rows, neurons are columns
    //         n n n n
    con_mat << 1,1,1,1, //a
            1,0,1,0, //a
            0,1,0,1,
            0,0,1,1;


    Matrix<int,Dynamic,Dynamic>wt_mat(4,4);
    wt_mat << 1,1,1,1,
            2,2,2,2,
            3,3,3,3,
            0,-1,2,3;

    Matrix<int,1,Dynamic> current_mpots(4);

    auto spike_train = Matrix<int,Dynamic,Dynamic>::Constant(4,1,1);
    current_mpots << 0,0,0,0;

    int test_spike = 2; // spike from axon 2


    Matrix<int,Dynamic,Dynamic> spike_input = MatrixXi::Constant(4,4,0);
    spike_input.row(test_spike).setOnes();
    std::cout << spike_input << " - spike_in \n";

    auto cm = (con_mat.array() * wt_mat.array()).row(test_spike) ;

    std::cout << "\n" << current_mpots << " --- cmpot \n";
    std::cout <<"\n----\n"<< cm << "\n --- tval\n";
    std::cout << "\n" <<  cm.array() + current_mpots.array() << "\n\n";
    current_mpots.array() += cm.array();

    //spike_mat.row(test_spike).setOnes();
    //spike_mat.row(test_spike).setZero();
    //std::cout << spike_mat.row(test_spike).setOnes()<< "-- spike_mat_row \n";
    //auto res2 =  wt_mat * spike_mat;
    //std::cout << res2 << "---- newres \n";



    Matrix<int,Dynamic,Dynamic> vx = Matrix4i::Constant(4,4,0);
    vx.row(0).setOnes();


    auto res = (con_mat(test_spike) * wt_mat(test_spike));
    std::cout << con_mat(test_spike) << "\n";
    std::cout << res << " -- res\n";

    std::cout << wt_mat(test_spike) << "\n";
    std::cout << wt_mat << " - wtmat \n";
}