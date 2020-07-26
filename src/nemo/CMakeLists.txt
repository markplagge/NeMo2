find_package(MPI)
project(nemo2_lib LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 14)

# Nemo uses ROSS as the backend - so we add it here
INCLUDE_DIRECTORIES(${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR} ${lmdir})

# This allows support for fancy IDEs such as VIM
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
#TrueNorth neuron API can output JSON files. The reader of them is a  library, and this flag turns it on or off
set(USE_MODEL_RDR 1 CACHE BOOL "Use NeMo->TN JSON Model reader lib?")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mtune=native -O3 -flto")
## Model Compile-Time Options have been moved to a config file.

#set(NeMo_NEURONS_IN_CORE 256 CACHE STRING "The number of neurons in each simulated core")
#set(NeMo_NUM_WEIGHTS_IN_NEURON 4 CACHE STRING "How many weights does a neuron have? (4 is TN default)")

## COMPILE-TIME FLAGS: Debugs, etc.
option(NeMo_DEBUG  "Enable debug messages" On)
configure_file("nemo_build_options.h.in"
        "include/nemo_build_options.h")

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
        nemo_io/ModelFile.h
        nemo_neuro_system/neurosynaptic_cores/NemoCoreDefs.h
        nemo_neuro_system/neuron_models/NemoNeuronTrueNorth.h
        mapping_functions.h
        nemo_formatted_print.h
        nemo_io/get_js_mp_file.h
        nemo_neuro_system/VirtualCore.h
        nemo_neuro_system/neuron_models/neuron_factory.h
        nemo_neuro_system/neuron_models/TNNeuronState.h
        nemo_neuro_system/BenchmarkModelDef.h
        BF_Event_Status.h
        nemo_neuro_system/neuron_models/NemoBenchmarkModels.h
        nemo_neuro_system/neurosynaptic_cores/NemoCoreSchedulerLight.h
        nemo_io/NemoCoreDebugOutput.h
        nemo_io/VirtualCoreReports.h)

set(nemo_lib_srcs
        nemo_neuro_system/neurosynaptic_cores/NemoNeuroCoreBase.cpp
        nemo_neuro_system/neuron_models/NemoNeuronGeneric.cpp
        nemo_io/NemoCoreOutput.cpp
        nemo_globals.cpp
        nemo_config/NemoConfig.cpp
        nemo_ross.h mapping_functions.cpp
        nemo_io/ModelFile.cpp
        nemo_neuro_system/neuron_models/NemoNeuronTrueNorth.cpp
        mapping_functions.cpp nemo_io/SpikeFile.cpp
        nemo_io/SpikeFile.h
        nemo_neuro_system/neurosynaptic_cores/TaskProcessMap.cpp
        nemo_io/get_js_mp_file.cpp
        nemo_neuro_system/VirtualCore.cpp
        nemo_neuro_system/neuron_models/TNNeuronState.cpp
        nemo_neuro_system/BenchmarkModelDef.cpp
        BF_Event_Status.cpp
        nemo_io/VirtualCoreReports.cpp
        nemo_neuro_system/neuron_models/NemoBenchmarkModels.cpp
        nemo_neuro_system/neurosynaptic_cores/NemoCoreSchedulerLight.cpp
        nemo_io/NemoCoreDebugOutput.cpp
        nemo_neuro_system/neurosynaptic_cores/TaskProcessMap.h)


add_library(nemo2_lib ${nemo_lib_hdrs} ${nemo_lib_srcs})
#nemo_ross_main.cpp contains main().
# it is named nemo_*ross*_main since it only uses ROSS as the backend.
add_executable(nemo2 nemo_ross_main.cpp) #


set_property(TARGET nemo2_lib PROPERTY CXX_STANDARD 14)

## The EIGEN BLAS library is used for neuromorphic matrix math.
# The SQLITE library is used to store spikes - but eventually will be replaced with JSON
message("-------------------------------- ${EIGEN_INCLUDE_FLDR}")
target_include_directories(nemo2_lib PUBLIC  ${NL_JSON_DIR}  ../../external/configuru ../../external/visit_struct/include ../libs/neuro_os/include ../libs/neuro_os/extern/pybind11/include "${CMAKE_CURRENT_BINARY_DIR}/include")
target_include_directories(nemo2_lib PRIVATE
        "${CMAKE_CURRENT_BINARY_DIR}/include"    )
target_link_libraries(nemo2_lib  neuro_os ROSS m)
target_include_directories(nemo2 PUBLIC nemo2_lib ${SQL_FOLDER})
target_link_libraries(nemo2 nemo2_lib ROSS m)

#Would like to avoid using yet another library for the math. Currently neuromorphic matrix math is implemented in loops.
#target_link_libraries(nemo2_lib Eigen3::Eigen)