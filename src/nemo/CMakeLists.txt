find_package(MPI)
project(nemo2_lib LANGUAGES CXX C)

INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR} ${lmdir})
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(USE_MODEL_RDR 1 CACHE BOOL "Use NeMo->TN JSON Model reader lib?")

## Model Compile-Time Options (Will add more later)
set(NeMo_NEURONS_IN_CORE 256 CACHE STRING "The number of neurons in each simulated core")
set(NeMo_NUM_WEIGHTS_IN_NEURON 4 CACHE STRING "How many weights does a neuron have? (4 is TN default)")
set(NeMo_DEBUG 0 CACHE BOOL "Enable debug messages")
set(nemo_lib_hdrs
        nemo_globals.h
        include/nemo.h
        nemo_neuro_system/neuron_models/NemoNeuronGeneric.h
        nemo_io/NemoCoreOutput.h
        nemo_neuro_system/neurosynaptic_cores/NemoNeuroCoreBase.h
        include/NemoConfig.h

        )
set(nemo_lib_srcs

        nemo_neuro_system/neurosynaptic_cores/NemoNeuroCoreBase.cpp
        nemo_neuro_system/neuron_models/NemoNeuronGeneric.cpp
        nemo_io/NemoCoreOutput.cpp
        nemo_globals.cpp
        NemoConfig.cpp
        nemo_ross.h mapping_functions.cpp nemo_neuro_system/neuron_models/NemoNeuronFactory.cpp nemo_neuro_system/neuron_models/NemoNeuronFactory.h nemo_io/NemoNeuroInitState.cpp nemo_io/NemoNeuroInitState.h nemo_io/NemoModelInput.cpp nemo_io/NemoModelInput.h nemo_neuro_system/neurosynaptic_cores/NemoNeuroCoreSimple.cpp nemo_neuro_system/neurosynaptic_cores/NemoNeuroCoreSimple.h)

add_library(nemo2_lib ${nemo_lib_hdrs} ${nemo_lib_srcs})
set_property(TARGET nemo2_lib PROPERTY CXX_STANDARD 14)
message("-------------------------------- ${EIGEN_INCLUDE_FLDR}")
target_include_directories(nemo2_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/nemo.h ${EIGEN_INCLUDE_FLDR})
target_link_libraries(nemo2_lib ROSS m)
#target_link_libraries(nemo2_lib Eigen3::Eigen)