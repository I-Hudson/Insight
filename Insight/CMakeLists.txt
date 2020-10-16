# Get all the files for the lib
file(GLOB_RECURSE _source_list 
    src/*.cpp* src/*.h* src/*.hpp*
    vendor/stb_image/*.cpp vendor/stb_image/*.h
    vendor/glm/glm/*.hpp vendor/glm/glm/*.ini
    vendor/glad/src/*.cpp
    vendor/imgui/*.cpp vendor/imgui/*.h
    vendor/TinyXML2/*.cpp vendor/TinyXML2/*.h
    )
foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

#Add all files from _source_list to the project
add_library(Insight ${_source_list})

add_compile_definitions(IS_PLATFORM_WINDOWS)
add_compile_definitions(IS_BUILD_DLL)
add_compile_definitions(GLM_FORCE_SWIZZLE)
add_compile_definitions(IS_VULKAN)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DIS_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DIS_RELEASE")

# Setup all the include dirs for the lib
target_include_directories(Insight PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(Insight PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/Insight")
target_include_directories(Insight PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/Platform")
target_include_directories(Insight PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/Vulkan")

target_include_directories(Insight PUBLIC "${CMAKE_SOURCE_DIR}/Insight/vendor/spdlog/include")
target_include_directories(Insight PRIVATE "${CMAKE_SOURCE_DIR}/Insight/vendor/GLFW/include")
target_include_directories(Insight PUBLIC "${CMAKE_SOURCE_DIR}/Insight/vendor/assimp/include")
target_include_directories(Insight PUBLIC "${CMAKE_SOURCE_DIR}/Insight/vendor/assimp/BINARIES/x64/Include")
target_include_directories(Insight PUBLIC "${CMAKE_SOURCE_DIR}/Insight/vendor/glm")
target_include_directories(Insight PRIVATE "${CMAKE_SOURCE_DIR}/Insight/vendor/stb_image")
target_include_directories(Insight PRIVATE "${CMAKE_SOURCE_DIR}/Insight/vendor/glad/include")
target_include_directories(Insight PUBLIC "${CMAKE_SOURCE_DIR}/Insight/vendor/imgui")
target_include_directories(Insight PRIVATE "${CMAKE_SOURCE_DIR}/Insight/vendor/TinyXML2")
target_include_directories(Insight PRIVATE "$ENV{VULKAN_SDK}/include")

target_link_directories(Insight PUBLIC "$ENV{VULKAN_SDK}/lib")
target_link_directories(Insight PUBLIC "${CMAKE_SOURCE_DIR}/Insight/vendor/assimp/BINARIES/x64/lib/$(ConfigurationName)")

target_link_libraries(Insight debug assimp-vc142-mtd optimized assimp-vc142-mt)

target_compile_definitions(Insight PUBLIC INSIGHT_LIB_VERSION=1)

add_subdirectory("${CMAKE_SOURCE_DIR}/Insight/vendor/GLFW/")
target_link_libraries(Insight glfw)
