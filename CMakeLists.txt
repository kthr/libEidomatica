cmake_minimum_required(VERSION 2.8.4)
project(libEidomatica)
set(CMAKE_MACOSX_RPATH 1)

#add_subdirectory(src/cgal)

set(SOURCES 
  lib/gco/maxflow.cpp
  lib/gco/GCoptimization.cpp
  lib/gco/LinkedBlockList.cpp
  lib/maxflow/graph.cpp
  lib/maxflow/maxflow.cpp
#	src/alg/connected_components.cpp
  src/alg/alpha_shapes.cpp
  src/alg/density.cpp
  src/alg/graphcut.cpp
	src/alg/multi_label_graphcut.cpp
	src/io/hdf5_reader.cpp
	src/io/hdf5_wrapper.cpp
  lib/gco/graph.cpp
	src/utilities/parameters.cpp
	src/utilities/utilities.cpp
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
find_package(Boost REQUIRED COMPONENTS filesystem system thread)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}" ${CMAKE_MODULE_PATH})
set(Mathematica_USE_LIBCXX_LIBRARIES OFF)
find_package(Mathematica)
find_package(PNG REQUIRED)
find_package(HDF5 REQUIRED COMPONENTS CXX C)
set(CGAL_LIBRARIES /opt/local/lib/libCGAL.dylib /opt/local/lib/libCGAL_Core.dylib /opt/local/lib/libgmp.dylib)

message("Include directories: ${Boost_INCLUDE_DIRS} ${Mathematica_WolframLibrary_INCLUDE_DIR} ${Mathematica_MathLink_INCLUDE_DIR} ${HDF5_INCLUDE_DIRS} ${PNG_INCLUDE_DIR}")
message("Libraries: ${Boost_LIBRARIES} ${Mathematica_MathLink_LIBRARY} ${PNG_LIBRARY} ${HDF5_CXX_LIBRARIES} ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES}")
if(${Boost_FOUND} AND ${Mathematica_WolframLibrary_FOUND} AND ${HDF5_FOUND})
    include_directories(${Boost_INCLUDE_DIRS} ${Mathematica_WolframLibrary_INCLUDE_DIR} ${Mathematica_MathLink_INCLUDE_DIR} ${HDF5_INCLUDE_DIRS} ${PNG_INCLUDE_DIR})
    target_link_libraries(Eidomatica ${Boost_LIBRARIES} ${Mathematica_MathLink_LIBRARIES} ${PNG_LIBRARY} ${HDF5_CXX_LIBRARIES} ${CGAL_LIBRARIES})
    set_target_properties(Eidomatica PROPERTIES COMPILE_DEFINITIONS "GLM_FORCE_RADIANS")
endif()

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type specified, setting build type to 'Release'!")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

install(TARGETS Eidomatica DESTINATION "${Mathematica_USERBASE_DIR}/Applications/Eidomatica/LibraryResources/${Mathematica_HOST_SYSTEM_ID}")

if(APPLE)
    message("\nEntering configuration for APPLE computers:\n")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -pipe -Wall -std=c++11 -stdlib=libc++")
    set(CMAKE_CXX_FLAGS_DEBUG "-v -g -O0 -pipe -Wall -std=c++11 -stdlib=libc++")
endif()

if(WIN32)
endif()

if(UNIX AND NOT APPLE)
	message("\nEntering configuration for UNIX computers:\n")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3 -pipe -Wall -std=c++11")
	set(CMAKE_CXX_FLAGS_DEBUG "-v -g -O0 -pipe -Wall -std=c++11")
endif()
