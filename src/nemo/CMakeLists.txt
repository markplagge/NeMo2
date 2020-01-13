find_package(MPI)
project(nemo2_lib LANGUAGES CXX C)

INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR} ${lmdir})
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(USE_MODEL_RDR 1 CACHE BOOL "Use NeMo->TN JSON Model reader lib?")

## Model Compile-Time Options (Will add more later)
set(NeMo_NEURONS_IN_CORE 256 CACHE STRING "The number of neurons in each simulated core")
set(NeMo_NUM_WEIGHTS_IN_NEURON 4 CACHE STRING "How many weights does a neuron have? (4 is TN default)")
set(NeMo_DEBUG 0 CACHE BOOL "Enable debug messages")
set(nemo_lib_hdrs nemo_globals.h include/nemo.h neuron_models/NemoNeuronGeneric.h nemo_io/NemoCoreOutput.h NemoNeuroCoreBase.h neuron_models/LIFCore.h include/NemoConfig.h include/CoreLP.h)
set(nemo_lib_srcs NemoNeuroCoreBase.cpp neuron_models/NemoNeuronGeneric.cpp  nemo_io/NemoCoreOutput.cpp  nemo_globals.cpp neuron_models/LIFCore.cpp include/NemoConfig.cpp CoreLP.cpp NemoCoreMap.cpp NemoCoreMap.h)

add_library(nemo2_lib ${nemo_lib_hdrs} ${nemo_lib_srcs})
target_include_directories(nemo2_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/nemo.h)