cmake_minimum_required(VERSION 3.20)

project(glow CXX)

# use std 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE CXX_SOURCES "src/*.cpp")


# add_executable(${PROJECT_NAME} ${CXX_SOURCES})
add_executable(${PROJECT_NAME} ${CXX_SOURCES})


find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(spdlog CONFIG REQUIRED)

find_path(CGLTF_INCLUDE_DIRS "cgltf.h")
target_include_directories(${PROJECT_NAME} PRIVATE ${CGLTF_INCLUDE_DIRS})

# find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
# target_include_directories(${PROJECT_NAME} PRIVATE ${TINYGLTF_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME} PRIVATE ${Stb_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME}
        PRIVATE 
        glfw
        glad::glad
        glm::glm
        assimp::assimp
        imgui::imgui
        spdlog::spdlog
    )


#copy resource folder to build

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)
