cmake_minimum_required(VERSION 3.29)
project(racocheck_tests)

set(CMAKE_CXX_STANDARD 20)

# Download doctest
include(FetchContent)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.9
)
FetchContent_MakeAvailable(doctest)

add_executable(racocheck_tests
        tests.cpp
        ../source/raco.h
        ../source/raco/coros/task.h
        ../source/raco/coros/task.hpp
        ../source/raco/misc/pack.h
        ../source/raco/misc/macros.h
        ../source/raco/core/raco.h
        ../source/raco/core/raco.hpp
        ../source/raco/core/state.h
        ../source/raco/core/state.hpp
        ../source/raco/misc/traits.h
        ../source/raco/algorithm/evaluator.h
        ../source/raco/algorithm/evaluator.hpp
        ../source/raco/algorithm/path_queue.h
        ../source/raco/algorithm/path_queue.hpp
        ../source/raco/algorithm/multi_threaded_evaluator.h
        ../source/raco/algorithm/multi_threaded_evaluator.hpp
)

target_include_directories(racocheck_tests
PRIVATE
    ../
    ../source/
)

# Add pthread support for multi-threading
find_package(Threads REQUIRED)
target_link_libraries(racocheck_tests doctest::doctest Threads::Threads)