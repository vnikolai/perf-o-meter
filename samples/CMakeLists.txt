# Perf-o-meter library usage samples

file(GLOB files "src/*.cpp")

foreach(file ${files})
    get_filename_component(file_name ${file} NAME_WE)
    add_executable(${file_name} ${file})
    target_link_libraries(${file_name} perfometer)
    set_property(TARGET ${file_name} PROPERTY CXX_STANDARD 11)
endforeach()

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(threads pthread)
endif()
