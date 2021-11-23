cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
option(SANITIZE_ADDRESS "Address Sanitizer." OFF)

if (SANITIZE_ADDRESS)
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
endif()

include(FetchContent)

if(NOT InstallVendor)
function(InstallVendor project_name git_repo branch)
        IF (NOT ${project_name})
                FetchContent_Declare(
                        ${project_name}
                        GIT_REPOSITORY ${git_repo}
                        GIT_TAG ${branch}
                        SOURCE_DIR "${PROJECT_SOURCE_DIR}/vendor/${project_name}"
                        BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/${project_name}"
                        SUBBUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/${project_name}-sub"
                )
                FetchContent_MakeAvailable(${project_name})
                set(${project_name} CACHE INTERNAL "")
                message(STATUS "Found ${project_name} @ ${branch}")
        endif()
endfunction()
endif()

set(This oof)
project(${This} LANGUAGES C CXX)

#InstallVendor(doctest https://github.com/onqtam/doctest.git master)

set(LIBRARY_HEADERS
${PROJECT_SOURCE_DIR}/oof.h
)

file(GLOB_RECURSE DEMO_HEADERS ${PROJECT_SOURCE_DIR}/demos/**.h)
file(GLOB_RECURSE DEMO_SOURCES ${PROJECT_SOURCE_DIR}/demos/**.cpp)

file(GLOB_RECURSE TEST_SOURCES ${PROJECT_SOURCE_DIR}/tests/**.cpp)

add_library(${This} INTERFACE)
add_library(${This}::${This} ALIAS ${This})
target_compile_definitions(${This} INTERFACE OOF_IMPL)
target_include_directories(${This} INTERFACE ${PROJECT_SOURCE_DIR})

#add_executable(${This}-demo ${DEMO_HEADERS} ${DEMO_SOURCES})
#add_executable(${This}-test ${TEST_SOURCES})
#target_link_libraries(${This}-test PRIVATE doctest::doctest)