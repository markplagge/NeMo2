find_package(MPI)
project(nemo2_lib LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 14)

# Nemo uses ROSS as the backend - so we add it here
INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR} ${lmdir})

# This allows support for fancy IDEs such as VIM
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
#TrueNorth neuron API can output JSON files. The reader of them is a  library, and this flag turns it on or off
set(USE_MODEL_RDR 1 CACHE BOOL "Use NeMo->TN JSON Model reader lib?")

## Model Compile-Time Options have been moved to a config file.

#set(NeMo_NEURONS_IN_CORE 256 CACHE STRING "The number of neurons in each simulated core")
#set(NeMo_NUM_WEIGHTS_IN_NEURON 4 CACHE STRING "How many weights does a neuron have? (4 is TN default)")
#set(NeMo_DEBUG 0 CACHE BOOL "Enable debug messages")


#haders for nemo_lib:
set(nemo_lib_hdrs
        ## Configuru is a JSON based config library - including it directly is easier than a library.
        ../../external/configuru/configuru.hpp
        ## From C globals are here
        nemo_globals.h
        # nemo.h is the base header
        include/nemo.h

        nemo_neuro_system/neuron_models/NemoNeuronGeneric.h
        nemo_io/NemoCoreOutput.h
        nemo_neuro_system/neurosynaptic_cores/NemoNeuroCoreBase.h
        nemo_config/NemoConfig.h
        nemo_io/NemoNeuroInitState.h
        nemo_neuro_system/neurosynaptic_cores/NemoNeuroCoreSimple.h
        nemo_neuro_system/neurosynaptic_cores/NemoCoreLPWrapper.h
        nemo_io/ModelFile.h)
set(nemo_lib_srcs

        nemo_neuro_system/neurosynaptic_cores/NemoNeuroCoreBase.cpp
        nemo_neuro_system/neuron_models/NemoNeuronGeneric.cpp
        nemo_io/NemoCoreOutput.cpp
        nemo_globals.cpp
        nemo_config/NemoConfig.cpp
        nemo_ross.h mapping_functions.cpp
        nemo_io/NemoNeuroInitState.cpp
        nemo_neuro_system/neurosynaptic_cores/NemoNeuroCoreSimple.cpp
        nemo_neuro_system/neurosynaptic_cores/NemoCoreLPWrapper.cpp
        nemo_io/ModelFile.cpp nemo_config/NemoModel.cpp nemo_config/NemoModel.h nemo_neuro_system/neurosynaptic_cores/NemoCoreScheduler.cpp nemo_neuro_system/neurosynaptic_cores/NemoCoreScheduler.h)

add_library(nemo2_lib ${nemo_lib_hdrs} ${nemo_lib_srcs})
#nemo_ross_main.cpp contains main().
# it is named nemo_*ross*_main since it only uses ROSS as the backend.
add_executable(nemo2 nemo_ross_main.cpp) #


set_property(TARGET nemo2_lib PROPERTY CXX_STANDARD 14)
## The EIGEN BLAS library is used for neuromorphic matrix math.
# The SQLITE library is used to store spikes - but eventually will be replaced with JSON
message("-------------------------------- ${EIGEN_INCLUDE_FLDR}")
target_include_directories(nemo2_lib PUBLIC ${EIGEN_INCLUDE_FLDR} ${SQL_FOLDER} ../../external/configuru ../../external/visit_struct/include ../libs/neuro_os/include)
target_link_libraries(nemo2_lib ROSS SQLITE3 neuro_os m)
target_include_directories(nemo2 PUBLIC nemo2_lib ${SQL_FOLDER})
target_link_libraries(nemo2 nemo2_lib ROSS m)
#Would like to avoid using yet another library for the math. Currently neuromorphic matrix math is implemented in loops.
#target_link_libraries(nemo2_lib Eigen3::Eigen)