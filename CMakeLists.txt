cmake_minimum_required(VERSION 3.5)
project(vibration-monitor LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# --- Dependencies Setup ---
include(FetchContent)

# 1. GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# 2. Flatbuffers (Matching TFLite 2.16.2 needs roughly v23.5.26 or v24.3.25)
FetchContent_Declare(
  flatbuffers
  URL https://github.com/google/flatbuffers/archive/refs/tags/v23.5.26.zip
)
# We only need the headers, so disable building flatc binary
set(FLATBUFFERS_BUILD_FLATC OFF CACHE BOOL "" FORCE)
set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest flatbuffers)

add_compile_options(-Wall -Wextra -O2 -Wno-interference-size)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall")

set(SPI_CPP_SRCS
  src/aquistion.cpp
  src/mcp3008spi.cpp
  src/ringbuffer.cpp
  src/processing_thread.cpp
  src/inferenceEngine.cpp
)

set(SPI_C_SRCS
  kissfft/kiss_fft.c
  kissfft/kiss_fftr.c
)

# ============================================================================
# Environment Setup: Cross-Compiling vs Native Testing Environment
# ============================================================================
if(CMAKE_CROSSCOMPILING)
  # Target setup (ARM64 Raspberry Pi via Yocto SDK)
  set(TFLITE_INCLUDE_DIR "/opt/poky/5.0.17/sysroots/cortexa76-poky-linux/usr/include")
  set(TFLITE_LINK_LIBS "tensorflow-lite")
else()
  # -------------------------------------------------------------
  # Native Host setup (Ubuntu VirtualBox VM Architecture)
  # -------------------------------------------------------------
  set(TFLITE_INCLUDE_DIR "/home/vboxuser/tensorflow")
  
  link_directories(/home/vboxuser/playground/spi/lib/native)
  
  # 1. Base Paths to Core Dependencies
  set(FLATBUFFERS_LIB "/home/vboxuser/tflite_build/_deps/flatbuffers-build/libflatbuffers.a")
  set(CPUINFO_LIB "/home/vboxuser/tflite_build/_deps/cpuinfo-build/libcpuinfo.a")
  set(XNNPACK_LIB "/home/vboxuser/tflite_build/_deps/xnnpack-build/libXNNPACK.a")
  
  # 2. Paths to Threading and Hashing Utilities (VERIFIED PATHS)
  set(PTHREADPOOL_LIB "/home/vboxuser/tflite_build/pthreadpool/libpthreadpool.a")
  set(FARMHASH_LIB "/home/vboxuser/tflite_build/_deps/farmhash-build/libfarmhash.a")
  
  # 3. Paths to FFT2D static libraries
  set(FFT2D_LIBS
    "/home/vboxuser/tflite_build/_deps/fft2d-build/libfft2d_fftsg.a"
    "/home/vboxuser/tflite_build/_deps/fft2d-build/libfft2d_fftsg2d.a"
  )
  
  # 4. Automatically collect all individual static ruy components
  file(GLOB RUY_LIBS "/home/vboxuser/tflite_build/_deps/ruy-build/ruy/*.a")
  
  # 5. Combine everything cleanly into the linker stream
  set(TFLITE_LINK_LIBS 
    "tensorflow-lite" 
    ${XNNPACK_LIB}
    ${RUY_LIBS} 
    ${PTHREADPOOL_LIB}
    ${CPUINFO_LIB}
    ${FFT2D_LIBS}
    ${FARMHASH_LIB}
    ${FLATBUFFERS_LIB} 
    "dl"
    "pthread"
  )
endif()
# --- Main Executable ---
add_executable(vibration-monitor ${SPI_CPP_SRCS} ${SPI_C_SRCS})
target_include_directories(vibration-monitor PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/kissfft
  ${TFLITE_INCLUDE_DIR}
  ${flatbuffers_SOURCE_DIR}/include
)
target_link_libraries(vibration-monitor PRIVATE pthread rt m ${TFLITE_LINK_LIBS})

enable_testing()

# --- Unit Tests ---
add_executable(unit_tests 
    tests/ringbuffer_test.cpp
    tests/fft_test_gtest.cpp
    tests/inference_test.cpp  
    src/ringbuffer.cpp
    src/inferenceEngine.cpp
    ${SPI_C_SRCS}
)

target_include_directories(unit_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/kissfft
  ${TFLITE_INCLUDE_DIR}
  ${flatbuffers_SOURCE_DIR}/include
)
target_link_libraries(unit_tests PRIVATE GTest::gtest_main pthread m ${TFLITE_LINK_LIBS})
add_test(NAME AllUnitTests COMMAND unit_tests)

install(TARGETS vibration-monitor RUNTIME DESTINATION bin)