# Get all the files for the lib
file(GLOB_RECURSE _source_list 
    src/*.cpp* src/*.h* src/*.hpp*
    )
foreach(_source IN ITEMS ${_source_list})
    get_filename_component(_source_path "${_source}" PATH)
    string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")
    string(REPLACE "/" "\\" _group_path "${_group_path}")
    source_group("${_group_path}" FILES "${_source}")
endforeach()

add_compile_definitions(IS_PLATFORM_WINDOWS)

add_executable(Sandbox ${_source_list})

target_link_libraries(Sandbox PRIVATE Insight)

set_target_properties(Sandbox PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY $(OutDir))