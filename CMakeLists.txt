cmake_minimum_required(VERSION 3.27)
# cmake_policy(SET CMP0128 NEW)
# set(CMAKE_C_STANDARD    17)

# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CUDA_STANDARD 20)
# if(NOT DEFINED CMAKE_CUDA20_STANDARD_COMPILE_OPTION)
# set(CMAKE_CUDA20_STANDARD_COMPILE_OPTION "")
# set(CMAKE_CUDA20_EXTENSION_COMPILE_OPTION "")
# endif()
 set(CMAKE_CUDA_ARCHITECTURES 75)
 set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)

# set(CMAKE_CUDA_FLAGS "-std=c++20")
#set(CUDA_NVCC_FLAGS -G;-g;)

project(MCGS LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CUDA_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

file(GLOB_RECURSE SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_library(src ${SRC})
message(STATUS ${SRC})
target_include_directories(src PUBLIC ${PROJECT_SOURCE_DIR}/include)


include_directories(${PROJECT_SOURCE_DIR}/origin_cuda)
file(GLOB_RECURSE ORIGIN_CUDA ${PROJECT_SOURCE_DIR}/origin_cuda/*.cpp ${PROJECT_SOURCE_DIR}/origin_cuda/*.cu)
add_library(origin_cuda ${ORIGIN_CUDA})

# set_target_properties(origin_cuda PROPERTIES CUDA_ARCHITECTURES "70;75;86")

# target_include_directories(origin_cuda PUBLIC ${PROJECT_SOURCE_DIR}/origin_cuda/glm/)





set(Torch_DIR "/home/mocheng/software/libtorch-cxx11-abi-shared-with-deps-2.0.1+cu118/libtorch/share/cmake/Torch")
find_package(Torch REQUIRED)
target_include_directories(src PUBLIC ${TORCH_INCLUDE_DIRS})
 
# find_package( OpenCV REQUIRED )
# include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(MCGS main.cpp)
target_link_libraries(MCGS src origin_cuda torch_cuda  )
target_include_directories(MCGS PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(MCGS PUBLIC ${TORCH_INCLUDE_DIRS})
