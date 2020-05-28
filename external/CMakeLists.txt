ExternalProject_Add(ROSS_EXT
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/ROSS
        INSTALL_DIR ${ROSS_BIN_FOLDER}
        BINARY_DIR ${ROSS_BIN_FOLDER}
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX:path=${ROSS_BIN_FOLDER}
        -DROSS_BUILD_SHARED_LIBS=ON
        -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}

        )
add_subdirectory(catch2)

#ExternalProject_Add(project_CODES
#        SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/CODES
#        INSTALL_DIR ${CODES_BIN_FOLDER}
#        BINARY_DIR ${CODES_BIN_FOLDER}
#
#        CONFIGURE_COMMAND   autoreconf ${CMAKE_SOURCE_DIR}/external/codes/ -fi -Im4
#        COMMAND             ls
#        COMMAND   python3   ${CMAKE_SOURCE_DIR}/codes_config.py --env_vars=${CODES_ENV_ARGS} --codes_path=${CMAKE_SOURCE_DIR}/external/codes "-a ${CODES_CONFIG_OPTS}"
#
#        BUILD_COMMAND make
#        INSTALL_COMMAND make install
#
#        DEPENDS ROSS_EXT
#        )