#include "../../include/nemo_codes/nemo_codes.h"

#include <iostream>

void hello() {
    std::cout << "Hello, World!" << std::endl;
}

int primary_tests(){
    return 42;
}

int main(int argc, char **argv){


    tw_init(&argc, &argv);
    return 0;

}