cmake_minimum_required( VERSION 3.5.1)
project(BOOST)

set(CMAKE_PREFIX_PATH D:/boost-1.83.0/boost-1.83.0)

find_package(Boost COMPONENTS thread REQUIRED)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )
set( Boost_USE_STATIC_LIBS        OFF )
set( Boost_USE_MULTITHREADED      ON )
set( Boost_USE_STATIC_RUNTIME     OFF )
set( BOOST_ALL_DYN_LINK           ON ) # force dynamic linking for all libraries

# add_executable( ${PROJECT_NAME} client.cpp )
# add_executable( ${PROJECT_NAME} client_async.cpp )
add_executable( ${PROJECT_NAME} HelloThreader.cpp )
target_link_libraries( ${PROJECT_NAME} ${Boost_LIBRARIES})