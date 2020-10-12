file(GLOB_RECURSE source_files 
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)

add_library(Insight ${source_files})

target_include_directories(Insight PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_include_directories(Insight PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/Insight")
target_include_directories(Insight PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/Platform")
target_include_directories(Insight PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/Platform/Vulkan")

target_compile_definitions(Insight PUBLIC INSIGHT_LIB_VERSION=1)