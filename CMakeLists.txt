cmake_minimum_required(VERSION 3.16.3)

project(smp)

set(SOURCES
    src/database.c
    src/main.c
    src/helpers.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(smp PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(smp -ljson-c)

# Install target to /usr/local/bin/ so it's executable by name
install(TARGETS smp DESTINATION bin)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")