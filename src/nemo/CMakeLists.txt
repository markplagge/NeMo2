find_package(MPI)

INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR} ${lmdir})
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(USE_MODEL_RDR 1 CACHE BOOL "Use NeMo->TN JSON Model reader lib?")

## Model Compile-Time Options (Will add more later)
set(NeMo_NEURONS_IN_CORE 256 CACHE STRING "The number of neurons in each simulated core")
set(NeMo_NUM_WEIGHTS_IN_NEURON 4 CACHE STRING "How many weights does a neuron have? (4 is TN default)")
set(NeMo_DEBUG 0 CACHE BOOL "Enable debug messages")

