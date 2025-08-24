cmake_minimum_required(VERSION 3.29)
project(racocheck)

set(CMAKE_CXX_STANDARD 20)

add_executable(racocheck
        main.cpp
        source/raco.h
        source/raco/coros/task.h
        source/raco/coros/task.hpp
        source/raco/misc/pack.h
        source/raco/misc/macros.h
        source/raco/core/raco.h
        source/raco/core/raco.hpp
        source/raco/core/state.h
        source/raco/core/state.hpp
        source/raco/misc/traits.h
        source/raco/algorithm/evaluator.h
        source/raco/algorithm/evaluator.hpp
        source/raco/algorithm/path_queue.h
        source/raco/algorithm/path_queue.hpp
        source/raco/algorithm/multi_threaded_evaluator.h
        source/raco/algorithm/multi_threaded_evaluator.hpp
)

add_executable(test_threading
        test_threading.cpp
        source/raco.h
        source/raco/coros/task.h
        source/raco/coros/task.hpp
        source/raco/misc/pack.h
        source/raco/misc/macros.h
        source/raco/core/raco.h
        source/raco/core/raco.hpp
        source/raco/core/state.h
        source/raco/core/state.hpp
        source/raco/misc/traits.h
        source/raco/algorithm/evaluator.h
        source/raco/algorithm/evaluator.hpp
        source/raco/algorithm/path_queue.h
        source/raco/algorithm/path_queue.hpp
        source/raco/algorithm/multi_threaded_evaluator.h
        source/raco/algorithm/multi_threaded_evaluator.hpp
)

target_include_directories(racocheck
PRIVATE
    ./
    ./source/
)

target_include_directories(test_threading
PRIVATE
    ./
    ./source/
)

# Add pthread support for multi-threading
find_package(Threads REQUIRED)
target_link_libraries(racocheck Threads::Threads)
target_link_libraries(test_threading Threads::Threads)
