cmake_minimum_required(VERSION 3.1)
project(model_view)

add_compile_options(-std=c++11)
add_definitions(-Wall)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

find_package(OpenSceneGraph REQUIRED osg osgViewer osgDB osgGA osgUtil)

include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} model_view.cpp)
target_link_libraries(${PROJECT_NAME} ${OPENSCENEGRAPH_LIBRARIES})

