cmake_minimum_required(VERSION 3.27)
 
 
set(CMAKE_CXX_STANDARD 20)


project(MCGS LANGUAGES CXX )



file(GLOB_RECURSE SRC ${PROJECT_SOURCE_DIR}/src/GSContext.cpp ${PROJECT_SOURCE_DIR}/src/sort.cpp)


add_library(src ${SRC})
message(STATUS ${SRC})
target_include_directories(src PUBLIC ${PROJECT_SOURCE_DIR}/include)
 

set(MCRT_DIR MCRT/cmake)
find_package(MCRT REQUIRED)
target_link_libraries(src PUBLIC MCRT)



   add_subdirectory(${PROJECT_SOURCE_DIR}/app)







