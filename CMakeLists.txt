cmake_minimum_required(VERSION 3.16)
project(shooter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Build GLFW for X11 by default")
endif()
FetchContent_Declare(glfw
    URL https://codeload.github.com/glfw/glfw/tar.gz/a74efa0d5628b74adc0426af4c5710e287fa7c2c
    URL_HASH SHA256=7e85e39ade1efdda00734d323a0230b5c938074e9fb2ddbe453bdbb281064d2c)

set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glm
    URL https://codeload.github.com/g-truc/glm/tar.gz/0af55ccecd98d4e5a8d1fad7de25ba429d60e863
    URL_HASH SHA256=e7f187d83523f505eb38dd25d297ea6c0d4ed856d733e808f18253f5a8fa88a0)
FetchContent_MakeAvailable(glfw)
# GLM 1.0.1 khai báo policy cũ. Dùng policy từ CMake 3.10 khi cấu hình GLM bằng CMake 4.
if(CMAKE_VERSION VERSION_GREATER_EQUAL 4 AND NOT DEFINED CMAKE_POLICY_VERSION_MINIMUM)
    set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
    FetchContent_MakeAvailable(glm)
    unset(CMAKE_POLICY_VERSION_MINIMUM)
else()
    FetchContent_MakeAvailable(glm)
endif()

add_library(glad STATIC third_party/glad/src/glad.c)
target_include_directories(glad PUBLIC third_party/glad/include)
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE third_party/stb)

add_library(shooter_physics STATIC src/physics/AABB.cpp src/world/MapCatalog.cpp
    src/world/ImageMapGrid.cpp src/world/ImageMapLoader.cpp
    src/world/Level.cpp src/world/Player.cpp src/world/Weapon.cpp
    src/world/ArenaRoutes.cpp src/world/Enemy.cpp src/world/WaveManager.cpp)
target_include_directories(shooter_physics PUBLIC src)
target_link_libraries(shooter_physics PUBLIC glm::glm PRIVATE stb_image)

add_library(shooter_render STATIC src/core/Window.cpp
    src/gfx/Shader.cpp src/gfx/Mesh.cpp src/gfx/Primitives.cpp
    src/gfx/Camera.cpp src/gfx/LevelRenderer.cpp src/gfx/ShadowMap.cpp src/gfx/Texture.cpp
    src/gfx/WeaponRenderer.cpp src/gfx/ImpactSparks.cpp src/ui/HUD.cpp)
target_link_libraries(shooter_render PUBLIC glfw glad shooter_physics PRIVATE stb_image)
target_compile_definitions(shooter_render PUBLIC GLFW_INCLUDE_NONE ASSET_DIR="assets"
    $<$<CONFIG:Debug>:SHOOTER_DEBUG>)
add_executable(shooter src/main.cpp src/core/Game.cpp src/core/Input.cpp src/core/FileDialog.cpp)
target_link_libraries(shooter PRIVATE shooter_render)
if(WIN32)
    target_link_libraries(shooter PRIVATE comdlg32)
endif()
if(MSVC)
    set(shooter_warnings /W4 /utf-8)
else()
    set(shooter_warnings -Wall -Wextra)
endif()
target_compile_options(shooter PRIVATE ${shooter_warnings})
target_compile_options(shooter_render PRIVATE ${shooter_warnings})
target_compile_options(shooter_physics PRIVATE ${shooter_warnings})

include(CTest)
if(BUILD_TESTING)
    add_executable(physics_tests tests/PhysicsTests.cpp)
    target_link_libraries(physics_tests PRIVATE shooter_physics)
    target_compile_options(physics_tests PRIVATE ${shooter_warnings})
    add_test(NAME physics COMMAND physics_tests)
    add_executable(combat_tests tests/CombatTests.cpp)
    target_link_libraries(combat_tests PRIVATE shooter_physics)
    target_compile_options(combat_tests PRIVATE ${shooter_warnings})
    add_test(NAME combat COMMAND combat_tests)
    add_executable(wave_tests tests/WaveTests.cpp)
    target_link_libraries(wave_tests PRIVATE shooter_physics)
    target_compile_options(wave_tests PRIVATE ${shooter_warnings})
    add_test(NAME waves COMMAND wave_tests)
endif()
set(shooter_output_dir "${CMAKE_CURRENT_BINARY_DIR}")
if(CMAKE_CONFIGURATION_TYPES)
    string(APPEND shooter_output_dir "/$<CONFIG>")
endif()
set_target_properties(shooter PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${shooter_output_dir}")
# Shader có thể đổi riêng, nên đồng bộ asset cả khi mã C++ không cần liên kết lại.
add_custom_target(shooter_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets" "${shooter_output_dir}/assets"
    VERBATIM)
add_dependencies(shooter shooter_assets)
if(BUILD_TESTING)
    add_dependencies(physics_tests shooter_assets)
    add_dependencies(wave_tests shooter_assets)
endif()

# Phép đo dùng đúng renderer của game và cần context đồ họa. Chỉ build khi gọi đích danh.
if(BUILD_TESTING)
    add_executable(render_benchmark EXCLUDE_FROM_ALL tests/RenderBenchmark.cpp)
    target_link_libraries(render_benchmark PRIVATE shooter_render)
    target_compile_options(render_benchmark PRIVATE ${shooter_warnings})
    set_target_properties(render_benchmark PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${shooter_output_dir}")
    add_dependencies(render_benchmark shooter_assets)
endif()
