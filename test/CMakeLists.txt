###  testing for nemo

set(CATCH_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/test/catch)
message(${CATCH_INCLUDE_DIR})
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})
add_executable(base_tests base_tests.cpp test_heartbeat_core.cpp)
target_include_directories(base_tests PRIVATE Catch)
target_include_directories(base_tests PUBLIC ${CMAKE_SOURCE_DIR}/external/eigen)
target_link_libraries(base_tests nemo2_lib)
