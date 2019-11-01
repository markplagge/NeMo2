cmake_minimum_required(VERSION 3.15)

#### NeMo TNG - The Next Generation
# CODES -> NeMo -> CODES
# For all your neuromorphic hardware simulation needs
# Developed by Mark Plagge

set (PROJECT_NAME "NeMoTNG")
set (PROJECT_DESCRIPTION "A parallel discrete even simulation based model of neuromorphic hardware - as a CODES workload")
set (PROJECT_AUTHOR "Mark Plagge")
project(${PROJECT_NAME} VERSION 0.0.1 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

include(ExternalProject)
include("cmake/HunterGate.cmake")

# ===========================
# CMake Modules and required packages config
# ===========================
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
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

## CODES CONFIG AND BUILD ################3
set(USE_SYSTEM_BISON OFF CACHE BOOL "Use the system path BISON/YACC")
set(USE_CUSTOM_BISON ON CACHE BOOL "Use a custom BISON path")
set(CUSTOM_BISON "/usr/local/opt/bison/bin/bison -y" CACHE STRING "Custom BISON Path")
set(FLAT_NAMESPACE OFF CACHE BOOL "Compile CODES with a flat namespace)")

find_package(BISON)

set(CODES_BISON_ARG " ")

if(USE_SYSTEM_BISON)
    message("Using default BISON path for CODES")
elseif(USE_CUSTOM_BISON)
    message("Using custom BISON at ${CUSTOM_BISON}")
    set(CODES_BISON_ARG "YACC:${CUSTOM_BISON}")
elseif(${BISON_FOUND})
    message("Using BISON found at ${BISON_EXECUTABLE}")
    set(CODES_BISON_ARG "YACC:${BISON_EXECUTABLE}")
else()
    message("Not using system BISON/YACC but no BISON/YACC found")
endif()

set(ROSS_BIN_FOLDER ${CMAKE_BINARY_DIR}/ross_build)
set(CODES_BIN_FOLDER ${CMAKE_BINARY_DIR}/codes_build)
message("PKG_CONFIG_PATH: ${ROSS_BIN_FOLDER}/lib/pkgconfig")
set(CODES_ENV_ARGS "PKG_CONFIG_PATH:${ROSS_BIN_FOLDER}/lib/pkgconfig,${CODES_BISON_ARG},CC:${MPI_C_COMPILER},CXX:${MPI_CXX_COMPILER},CFLAGS:${CMAKE_C_FLAGS},CXXFLAGS:${CMAKE_CXX_FLAGS}")
list(APPEND CODES_CONFIG_OPTS "prefix:${CODES_BIN_FOLDER}")

message(${CODES_ENV_ARGS})
message(${CODES_CONFIG_OPTS})

#### ROSSS EXTERNAL PROJECT
list(APPEND R_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:path=${ROSS_BIN_FOLDER} " )
list(APPEND R_CMAKE_ARGS "-DROSS_BUILD_SHARED_LIBS=ON ")
LIST(APPEND R_CMAKE_ARGS "-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} ")
ExternalProject_Add(ROSS_EXT
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/ROSS
        INSTALL_DIR ${ROSS_BIN_FOLDER}
        BINARY_DIR ${ROSS_BIN_FOLDER}
        CMAKE_ARGS
                -DCMAKE_INSTALL_PREFIX:path=${ROSS_BIN_FOLDER}
                -DROSS_BUILD_SHARED_LIBS=ON
                -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}

        )
ExternalProject_Add(project_CODES
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/CODES
        INSTALL_DIR ${CODES_BIN_FOLDER}
        BINARY_DIR ${CODES_BIN_FOLDER}

        CONFIGURE_COMMAND   autoreconf ${CMAKE_SOURCE_DIR}/external/codes/ -fi -Im4
        COMMAND             ls
        COMMAND   python3   ${CMAKE_SOURCE_DIR}/codes_config.py --env_vars=${CODES_ENV_ARGS} --codes_path=${CMAKE_SOURCE_DIR}/external/codes "-a ${CODES_CONFIG_OPTS}"

        BUILD_COMMAND make
        INSTALL_COMMAND make install

        DEPENDS ROSS_EXT
        )


add_library(CODES STATIC IMPORTED)
set_property(TARGET CODES PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/codes_build/lib)
add_dependencies(CODES project_CODES)

include_directories(${CODES_BIN_FOLDER}/include)
link_directories(${CODES_BIN_FOLDER}/lib)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/ROSS)

set(ENV{PKG_CONFIG_PATH}  "/Users/markplagge/dev/codes-nemo/cmake-build-debug/codes_build/lib/pkgconfig")
find_package(PkgConfig REQUIRED)
pkg_search_module(LIBCODES  codes)
include_directories(${LIBCODES_INCLUDE_DIRS})
message(" !@!!! ${LIBCODES_INCLUDE_DIRS} ")
message(" !@!!!!${LIBCODES_LDFLAGS}" )
# YACC="/usr/local/opt/bison/bin/bison -y"
#./configure --prefix=`pwd` PKG_CONFIG_PATH=/Users/markplagge/dev/ross-build/lib/pkgconfig
#CFLAGS=-g CXXFLAGS="-g -I/usr/local/Cellar/open-mpi/4.0.2/include"
# LDFLAGS="-L/usr/local/opt/libevent/lib -L/usr/local/Cellar/open-mpi/4.0.2/lib" LIBS="-lmpi"
#BUILD CODES and install to external/codes_build
add_subdirectory(src/nemo)
add_subdirectory(src/nemo_codes)


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
