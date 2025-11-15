file(GLOB_RECURSE test_srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(${PROJECT_NAME}_tests ${test_srcs})
target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${PROJECT_NAME})
