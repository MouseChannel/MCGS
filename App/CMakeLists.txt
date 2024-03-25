file(GLOB_RECURSE MCRT_RASTER_SRC ${PROJECT_SOURCE_DIR}/MCRT/example/base/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp )
add_library(mcrt_raster_src ${MCRT_RASTER_SRC})
target_link_libraries(mcrt_raster_src PUBLIC  mcrt_src)

target_include_directories(mcrt_raster_src PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})





target_link_libraries(src PUBLIC mcrt_raster_src imgui)

add_executable(MCGS raster_pbr.cpp)
target_link_libraries(MCGS mcrt_raster_src)

target_include_directories(MCGS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
 






# test only




# set(Torch_DIR "/home/mocheng/software/libtorch-cxx11-abi-shared-with-deps-2.0.1+cu118/libtorch/share/cmake/Torch")
# find_package(Torch REQUIRED)
# target_include_directories(src PUBLIC ${TORCH_INCLUDE_DIRS})
 

# add_executable(ORIGIN_TEST main.cpp)
# target_link_libraries(ORIGIN_TEST src origin_cuda torch_cuda mcrt_raster_src )
# target_include_directories(ORIGIN_TEST PUBLIC ${PROJECT_SOURCE_DIR}/include)
# target_include_directories(ORIGIN_TEST PUBLIC ${TORCH_INCLUDE_DIRS})
# target_include_directories(ORIGIN_TEST PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})