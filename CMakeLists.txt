### FCFS PROCESS LIBRARY
cmake_minimum_required(VERSION 3.5)

OPTION(NOS_ENABLE_UNIT_TESTS  "Enable FCFS unit tests (uses Catch2)" ON )
project(NeuroOS LANGUAGES CXX VERSION 0.1.5)


set(NEURO_OS_LIB
        include/neuro_os.h
        lib/json.hpp
        src/SimProcess.cpp
        src/SimProcess.h
        src/process_states.h
        src/SimProcessSerial.cpp
        src/SimProcessSerial.h
        src/SimProcessQueue.cpp
        src/SimProcessQueue.h)

add_library(neuro_os STATIC ${NEURO_OS_LIB} )
add_executable(neuro_os_bin main.cpp)
target_link_libraries(neuro_os_bin neuro_os)
#target_link_libraries(fcfs_logic_system simclist)
set_property(TARGET neuro_os PROPERTY CXX_STANDARD 14)
enable_testing()

set(NEURO_OS_TEST_SRC tests/test_test.cpp tests/test_sim_process_system.cpp tests/test_sim_process.cpp)
if(NOS_ENABLE_UNIT_TESTS)
    set(CATCH_CONFIG_ENABLE_BENCHMARKING 1)
    add_compile_definitions(CATCH_CONFIG_ENABLE_BENCHMARKING)
    #add_subdirectory(lib/catch2)
    set(CATCH_CONFIG_ENABLE_BENCHMARKING 1)
    add_executable(nos_tests ${NEURO_OS_TEST_SRC})
    target_link_libraries(nos_tests
            Catch2::Catch2)
    target_link_libraries(nos_tests neuro_os)
    target_include_directories(nos_tests PUBLIC
            include)

    set_property(TARGET nos_tests PROPERTY CXX_STANDARD 14)
    include(CTest)
    #include(ParseAndAddCatchTests)
    #include(Catch)
    set(CATCH_BUILD_TESTING OFF)
    set(CATCH_INSTALL_DOCS OFF)
    set(CATCH_INSTALL_HELPERS ON)
    message("----------- ${CMAKE_MODULE_PATH}")

    catch_discover_tests(nos_tests )
    ParseAndAddCatchTests(nos_tests)
    message("Enabled FCFS system tests")
endif()



