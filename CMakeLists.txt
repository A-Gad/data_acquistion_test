cmake_minimum_required(VERSION 3.5)
project(vibration-monitor LANGUAGES C CXX)

# KISS FFT is included as a vendor source tree in kissfft/ and
# is provided under the BSD-3-Clause license.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra -O2 -Wno-interference-size)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall")

set(SPI_CPP_SRCS
  src/aquistion.cpp
  src/mcp3008spi.cpp
  src/ringbuffer.cpp
  src/processing_thread.cpp
)

set(SPI_C_SRCS
  kissfft/kiss_fft.c
  kissfft/kiss_fftr.c
)

add_executable(vibration-monitor ${SPI_CPP_SRCS} ${SPI_C_SRCS})
target_include_directories(vibration-monitor PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/kissfft
)
target_link_libraries(vibration-monitor PRIVATE pthread rt m)

enable_testing()
option(BUILD_FFT_TEST "Build fft_test target" OFF)
if(BUILD_FFT_TEST)
  add_executable(fft_test tests/fft_test.cpp ${SPI_C_SRCS})
  target_include_directories(fft_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/kissfft
  )
  target_link_libraries(fft_test PRIVATE m)
endif()

install(TARGETS vibration-monitor RUNTIME DESTINATION bin)
