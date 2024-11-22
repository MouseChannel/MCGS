#file(GLOB_RECURSE OFFSCREEN_PBR ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
#
#add_library(offscreen_lib ${OFFSCREEN_PBR})
#
#target_link_libraries(offscreen_lib rasterlib)
#
#add_executable(offscreen offscreen.cpp)
#
#target_link_libraries(offscreen PRIVATE offscreen_lib)




# file(GLOB_RECURSE MCRT_RASTER_SRC ${PROJECT_SOURCE_DIR}/MCRT/example/base/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp )
# add_library(mcrt_raster_src ${MCRT_RASTER_SRC})
# target_link_libraries(mcrt_raster_src PUBLIC  mcrt_src)

# target_include_directories(mcrt_raster_src PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


# include_directories(${CMAKE_CURRENT_SOURCE_DIR})


# target_link_libraries(src PUBLIC mcrt_raster_src imgui)

add_executable(MCGS offscreen.cpp offscreen_context.cpp App.cpp )
target_link_libraries(MCGS src)

# target_include_directories(MCGS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
