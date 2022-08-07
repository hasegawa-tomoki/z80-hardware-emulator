project(Google_tests)
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(Google_Tests_run OpCodeTest.cpp ../src/opcode.cpp ../src/log.cpp ../src/mcycle.cpp ../src/registers.cpp ../src/special_registers.cpp ../src/cpu.cpp)
target_link_libraries(Google_Tests_run mcp23s17)
target_link_libraries(Google_Tests_run pigpiod_if2)
target_link_libraries(Google_Tests_run gtest gtest_main)
