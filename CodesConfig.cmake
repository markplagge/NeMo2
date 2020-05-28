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
message("Current CPP FLAGS: ${CMAKE_CXX_FLAGS}")
list(APPEND CODES_AUTOCONF_OPTS "D_IO_FILE:FILE")
list(APPEND CODES_CONFIG_OPTS "D_IO_FILE:FILE")

message(${CODES_ENV_ARGS})
message("${CODES_CONFIG_OPTS}")
message(${CODES_CONFIG_OPTS})
string(REPLACE ";" "," CODES_CONFIG_OPT_STR "${CODES_CONFIG_OPTS}")
message(${CODES_CONFIG_OPT_STR})


ExternalProject_Add(project_CODES
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/CODES
        INSTALL_DIR ${CODES_BIN_FOLDER}
        BINARY_DIR ${CODES_BIN_FOLDER}

        CONFIGURE_COMMAND   autoreconf ${CMAKE_SOURCE_DIR}/external/codes/ -fi -Im4
        COMMAND             ls
        COMMAND   python3   ${CMAKE_SOURCE_DIR}/codes_config.py --env_vars=${CODES_ENV_ARGS} --codes_path=${CMAKE_SOURCE_DIR}/external/codes --autoconfig_opts="${CODES_CONFIG_OPT_STR}"

        BUILD_COMMAND make
        INSTALL_COMMAND make install

        DEPENDS ROSS_EXT
        )

#add_library(CODES STATIC IMPORTED src/libs/neuro_os/include/neuro_os.h)
#set_property(TARGET CODES PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/codes_build/lib)
#add_dependencies(CODES project_CODES)

#include_directories(${CODES_BIN_FOLDER}/include)
#link_directories(${CODES_BIN_FOLDER}/lib)