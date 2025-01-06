cmake_minimum_required(VERSION 3.10)
project(learn_opengl)
# 设置 CMake 变量
set(CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR})
set(PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR})
set(PROJECT_BINARY_DIR ${PROJECT_BINARY_DIR})

# 生成 config.h 文件
configure_file(config.h.in config.h @ONLY)

set(GLFW_PATH ${CMAKE_SOURCE_DIR}/../glfw)
set(GLAD_PATH ${CMAKE_SOURCE_DIR}/../glad)

add_executable(learn_opengl main.cpp ${GLAD_PATH}/src/glad.c)

target_include_directories(learn_opengl PRIVATE ${GLFW_PATH}/include PRIVATE ${GLAD_PATH}/include PRIVATE ${CMAKE_BINARY_DIR} PRIVATE ${CMAKE_SOURCE_DIR}/includes)

target_link_directories(learn_opengl PRIVATE ${GLFW_PATH}/build/src)

target_link_libraries(learn_opengl PRIVATE glfw3 opengl32)

add_custom_command(TARGET learn_opengl POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${GLFW_PATH}/build/src/glfw3.dll" $<TARGET_FILE_DIR:learn_opengl>)