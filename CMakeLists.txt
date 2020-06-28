cmake_minimum_required(VERSION 3.15)

#### NeMo TNG - The Next Generation
# CODES -> NeMo -> CODES
# For all your neuromorphic hardware simulation needs
# Developed by Mark Plagge

set(PROJECT_NAME "NeMoTNG")
set(PROJECT_DESCRIPTION "A parallel discrete even simulation based model of neuromorphic hardware - as a CODES workload")
set(PROJECT_AUTHOR "Mark Plagge")
project(${PROJECT_NAME} VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

# ===========================
# Libs / archives in (build)/libs, binaries (except for NEMO itself go in (build)/bin
# ===========================

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ===========================
# External Project Support
# ===========================

include(ExternalProject)
#include("cmake/HunterGate.cmake")

# ===========================
# CMake Modules and required packages config
# ===========================
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/catch2/contrib")
message(${CMAKE_MODULE_PATH})
include(CTest)
include(ExternalProject)

# ==========================
# External Projects and Libraries
# ==========================

## JSON support TBD
#option(USE_EXTERNAL_C++_JSON "Use external JSON C++ libraries" OFF)

find_package(MPI)
message("MPI C: ${MPI_C_FOUND}. MPICXX: ${MPI_CXX_FOUND}")
message("MPI C: ${MPI_C_COMPILER}, flags: ${MPI_C_COMPILE_FLAGS} || ${MPI_CXX_COMPILE_FLAGS} || ${MPI_C_LINK_FLAGS}")


#### ROSSS EXTERNAL PROJECT
#list(APPEND R_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:path=${ROSS_BIN_FOLDER} " )
#list(APPEND R_CMAKE_ARGS "-DROSS_BUILD_SHARED_LIBS=ON ")
#LIST(APPEND R_CMAKE_ARGS "-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} ")
#ExternalProject_Add(ROSS_EXT
#        SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/ROSS
#        INSTALL_DIR ${ROSS_BIN_FOLDER}
#        BINARY_DIR ${ROSS_BIN_FOLDER}
#        CMAKE_ARGS
#                -DCMAKE_INSTALL_PREFIX:path=${ROSS_BIN_FOLDER}
#                -DROSS_BUILD_SHARED_LIBS=ON
#                -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
#
#        )
################# REMOVING CODES ###############


######## CATCH 2 ##### (TEST FRAMEWORK)
add_subdirectory(external/catch2)
message(${CMAKE_MODULE_PATH})
message(${CMAKE_MODULE_PATH})
include(ParseAndAddCatchTests)
include(Catch)

####### CONFIGURU JSON PARSER ############
include_directories(${CMAKE_SOURCE_DIR}/external/configuru)
include_directories(${CMAKE_SOURCE_DIR}/external/visit_struct/include/visit_struct)

####### NLH JSON PARSER
set(NL_JSON_DIR ${CMAKE_SOURCE_DIR}/external/nlohmann)
configure_file(config/example_config.json config/example_config.json COPYONLY)

###### ROSS EXTERNAL PROJECT
add_subdirectory(${CMAKE_SOURCE_DIR}/external/ROSS)

##### NEMO OS EXTERNAL PROJECT

### EIGEN Matrix Vector Library
#add_subdirectory(${CMAKE_SOURCE_DIR}/external/Eigen)
#set(EIGEN_INCLUDE_FLDR ${CMAKE_SOURCE_DIR}/external/eigen/)

set(ENV{PKG_CONFIG_PATH}  "/Users/markplagge/dev/codes-nemo/cmake-build-debug/codes_build/lib/pkgconfig")
#find_package(PkgConfig REQUIRED)
#pkg_search_module(LIBCODES  codes)
#include_directories(${LIBCODES_INCLUDE_DIRS})
#message(" !@!!! ${LIBCODES_INCLUDE_DIRS} ")
#message(" !@!!!!${LIBCODES_LDFLAGS}" )
# YACC="/usr/local/opt/bison/bin/bison -y"
#./configure --prefix=`pwd` PKG_CONFIG_PATH=/Users/markplagge/dev/ross-build/lib/pkgconfig
#CFLAGS=-g CXXFLAGS="-g -I/usr/local/Cellar/open-mpi/4.0.2/include"
# LDFLAGS="-L/usr/local/opt/libevent/lib -L/usr/local/Cellar/open-mpi/4.0.2/lib" LIBS="-lmpi"
#BUILD CODES and install to external/codes_build





## Nemo / NemoCODES projects
add_subdirectory(src)
set_property(TARGET ROSS PROPERTY CXX_STANDARD 14)

set(NEURO_OS_FLDR ${CMAKE_SOURCE_DIR}/src/libs/neuro_os)
include_directories(${NEURO_OS_FLDR}/SRC)
include_directories(${NEURO_OS_FLDR}/include)
add_executable(nemo_os_test ${NEURO_OS_FLDR}/main.cpp ${NEURO_OS_FLDR}/src/process_states.h
        ${NEURO_OS_FLDR}/src/SimProcess.h ${NEURO_OS_FLDR}/src/SimProcess.cpp
        src/libs/neuro_os/src/SimProcessQueue.h ${NEURO_OS_FLDR}/src/SimProcessQueue.cpp)

#add_subdirectory(src/nemo_codes)

### SET NEMO FILE LOCATION
set_target_properties(nemo2 nemo2_lib  
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
#################
# UNIT TESTS
#################
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message("Testing Enabled")
    include(CTest)
endif()
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(test)
    message("Adding test folders")
endif()
