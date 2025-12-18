file(GLOB_RECURSE hdrs "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB_RECURSE srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# исключаем main.cpp из библиотеки
list(FILTER srcs EXCLUDE REGEX "main\\.cpp$")

# библиотека проекта
add_library(${PROJECT_NAME} STATIC
    ${srcs}
    ${hdrs}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)


add_executable(${PROJECT_NAME}_run
    main.cpp
    logger.cpp
)

target_link_libraries(${PROJECT_NAME}_run
    PRIVATE ${PROJECT_NAME}
)
