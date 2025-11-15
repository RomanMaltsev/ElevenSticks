file(GLOB_RECURSE hdrs "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB_RECURSE srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# ВАЖНО: исключаем main.cpp из библиотеки
list(FILTER srcs EXCLUDE REGEX "main\\.cpp$")

# Библиотека проекта
add_library(${PROJECT_NAME} STATIC ${srcs} ${hdrs})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROG_INCLUDE})

# Исполняемый файл
add_executable(${PROJECT_NAME}_run main.cpp)
target_link_libraries(${PROJECT_NAME}_run PRIVATE ${PROJECT_NAME})
