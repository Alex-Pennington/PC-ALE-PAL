cmake_minimum_required(VERSION 3.15)
project(PC-ALE-PAL VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(PAL_BUILD_TESTS "Build unit tests" ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Common sources (platform-independent utilities)
set(PAL_COMMON_SOURCES
    src/common/resampler.cpp
)

# Radio protocol sources
set(PAL_RADIO_SOURCES
    src/radios/icom_civ.cpp
    src/radios/yaesu_cat.cpp
    src/radios/kenwood.cpp
    src/radios/elecraft.cpp
)

# PAL library (interfaces + utilities + radio protocols)
add_library(pal STATIC 
    ${PAL_COMMON_SOURCES}
    ${PAL_RADIO_SOURCES}
)

target_include_directories(pal PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Tests
if(PAL_BUILD_TESTS)
    enable_testing()
    
    add_executable(test_resampler tests/test_resampler.cpp)
    target_link_libraries(test_resampler pal)
    add_test(NAME test_resampler COMMAND test_resampler)
    
    # Radio protocol tests (when created)
    # add_executable(test_radios tests/test_radios.cpp)
    # target_link_libraries(test_radios pal)
    # add_test(NAME test_radios COMMAND test_radios)
endif()

# Install
install(DIRECTORY include/pal DESTINATION include)
install(TARGETS pal DESTINATION lib)
