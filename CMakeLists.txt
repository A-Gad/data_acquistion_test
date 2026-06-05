cmake_minimum_required(VERSION 3.5)
project(vibration-monitor LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags roughly matching the Makefile
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra -O2 -Wno-interference-size)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall")

# Source lists (C and C++ mixed)
set(SPI_CPP_SRCS
  aquistion.cpp
  mcp3008spi.cpp
  ringbuffer.cpp
  processing_thread.cpp
)

set(SPI_C_SRCS
  kiss_fft.c
  kiss_fftr.c
)

add_executable(vibration-monitor ${SPI_CPP_SRCS} ${SPI_C_SRCS})
target_include_directories(vibration-monitor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vibration-monitor PRIVATE pthread rt m)

option(BUILD_FFT_TEST "Build fft_test target" OFF)
if(BUILD_FFT_TEST)
  add_executable(fft_test fft_test.cpp ${SPI_C_SRCS})
  target_link_libraries(fft_test PRIVATE m)
endif()

install(TARGETS vibration-monitor RUNTIME DESTINATION bin)
