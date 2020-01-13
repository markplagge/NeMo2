#project(nemo_codes)


add_executable(nemo_codes nemo_codes.cpp ${CMAKE_SOURCE_DIR}/include/nemo_codes/nemo_codes.h)
include_directories(${CMAKE_SOURCE_DIR}/external/codes/)
target_include_directories(nemo_codes PUBLIC ${CMAKE_SOURCE_DIR}/external/codes ${CMAKE_SOURCE_DIR}/src/nemo/include)
target_link_libraries(nemo_codes ROSS)
target_include_directories(nemo_codes PUBLIC ${ROSS_SOURCE_DIR} ${ROSS_BINARY_DIR})


#target_link_directories(nemo_codes PUBLIC ${CODES_BIN_FOLDER}/lib)
#target_link_options(nemo_codes PUBLIC "-L${CODES_BIN_FOLDER}/lib")
include_directories(${LIBCODES_INCLUDE_DIRS})
target_link_libraries(nemo_codes ${LIBCODES_LDFLAGS})
target_link_libraries(nemo_codes  ROSS)
#link_directories(${CODES_BIN_FOLDER}/lib/)
ADD_DEPENDENCIES(nemo_codes ROSS CODES nemo2_lib )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lROSS -lm -L${CMAKE_BINARY_DIR}/ross_build/lib")