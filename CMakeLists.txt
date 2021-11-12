cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

set(PROJECT_NAME SFML_GAME3)

project(${PROJECT_NAME})

add_executable(${PROJECT_NAME} 
src/main.cpp
)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/sfml)
target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-audio sfml-system sfml-window sfml-network)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>/res)