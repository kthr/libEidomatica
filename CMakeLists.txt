cmake_minimum_required(VERSION 2.8.4)
project(libEidomatica)

set(SOURCES 
#	src/alg/connected_components.cpp
	src/alg/graphcut.cpp
	src/maxflow/graph.cpp
	src/maxflow/maxflow.cpp
	src/utilities/parameters.cpp
	src/utilities/utilities.cpp
#	src/c_wrapper.cpp
#	src/library_link.c
    src/library_link.cpp
)

include_directories(
    lib/
    src/
)

add_custom_target(
    Version
    COMMAND /bin/bash version.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
add_library(Eidomatica SHARED ${SOURCES})
add_dependencies(Eidomatica Version)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost COMPONENTS filesystem system) 
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(Eidomatica ${Boost_LIBRARIES})
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}" ${CMAKE_MODULE_PATH})
find_package(Mathematica)
if(${Mathematica_WolframLibrary_FOUND})
    include_directories(${Mathematica_WolframLibrary_INCLUDE_DIR} ${Mathematica_MathLink_INCLUDE_DIR})
    target_link_libraries(Eidomatica ${Mathematica_MathLink_LIBRARIES})
endif()

if(APPLE)
    message("\nEntering configuration for APPLE computers:\n")
#    set(CMAKE_CXX_COMPILER g++-mp-4.7)
#    set(CMAKE_C_COMPILER gcc-mp-4.7)
#    set(CMAKE_CXX_FLAGS "-g -pipe -Wall -I/opt/local/include -std=c++11 -I/opt/local/include/gcc47/c++ -I/opt/local/include/gcc47/c++/x86_64-apple-darwin12/")
    set(CMAKE_CXX_FLAGS "-mtune=native -O4 -pipe -Wall -std=c++11")
#    set(CMAKE_C_FLAGS "-O4 -mtune=native -pipe -Wall -std=c99")
    set_target_properties(Eidomatica
        PROPERTIES
        LINK_FLAGS "-v -L/opt/local/lib/gcc47"
     )
endif()

if(WIN32)
endif()

if(UNIX)
endif()