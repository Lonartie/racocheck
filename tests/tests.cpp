#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#define RACO_TEST_ENABLED
#include "source/raco.h"

#include <thread>
#include <chrono>

// Test functions for race condition detection
raco::fun<int> add_operation(int& value) {
    RACO_CHECKPOINT;
    int tmp = value;
    RACO_TEST(tmp = tmp + 1);
    RACO_RETURN RACO_TEST_INLINE(value = tmp);
}

raco::fun<int> subtract_operation(int& value) {
    RACO_CHECKPOINT;
    int tmp = value;
    RACO_TEST(tmp = tmp - 1);
    RACO_RETURN RACO_TEST_INLINE(value = tmp);
}

// Simple functions for basic testing
raco::fun<void> increment(int& x) {
    RACO_TEST(x = x + 1);
    RACO_RETURN;
}

raco::fun<void> decrement(int& x) {
    RACO_TEST(x = x - 1);
    RACO_RETURN;
}

TEST_CASE("Single-threaded path exploration") {
    using namespace raco;
    
    SUBCASE("Basic increment/decrement test") {
        bool result = check()
            .tasks([](state& s) {
                auto& counter = s.create<int>("counter", 0);
                return std::tuple{
                    increment(counter),
                    decrement(counter)
                };
            })
            .post_condition([](const state& s) {
                return s.get<int>("counter") == 0;
            })
            .invariant([](const state& s) {
                const auto& val = s.get<int>("counter");
                return val >= -1 && val <= 1;
            })
            .depth_limit(5)
            .ignore_depth_limit_warning()
            .deadlock_timeout(std::chrono::milliseconds(100))
            .continue_on_error()
            .run();
            
        // This should pass because increment+decrement = 0 when done in order
        CHECK(result); // We expect this to work correctly
    }
    
    SUBCASE("Single thread (should work correctly)") {
        bool result = check()
            .tasks([](state& s) {
                auto& counter = s.create<int>("counter", 0);
                return std::tuple{
                    increment(counter)
                };
            })
            .post_condition([](const state& s) {
                return s.get<int>("counter") == 1;
            })
            .depth_limit(5)
            .ignore_depth_limit_warning()
            .deadlock_timeout(std::chrono::milliseconds(100))
            .run();
            
        CHECK(result); // Single task should always satisfy post-condition
    }
}

TEST_CASE("Multi-threaded path exploration") {
    using namespace raco;
    
    SUBCASE("Multi-threaded with 2 threads") {
        bool result = check()
            .tasks([](state& s) {
                auto& counter = s.create<int>("counter", 0);
                return std::tuple{
                    increment(counter),
                    decrement(counter)
                };
            })
            .post_condition([](const state& s) {
                return s.get<int>("counter") == 0;
            })
            .invariant([](const state& s) {
                const auto& val = s.get<int>("counter");
                return val >= -1 && val <= 1;
            })
            .depth_limit(5)
            .ignore_depth_limit_warning()
            .deadlock_timeout(std::chrono::milliseconds(100))
            .continue_on_error()
            .num_threads(2) // Enable multi-threading
            .run();
            
        // This should work correctly like single-threaded
        CHECK(result); // Multi-threaded should get same result
    }
    
    SUBCASE("Multi-threaded with 4 threads") {
        bool result = check()
            .tasks([](state& s) {
                auto& counter = s.create<int>("counter", 0);
                return std::tuple{
                    increment(counter),
                    decrement(counter)
                };
            })
            .post_condition([](const state& s) {
                return s.get<int>("counter") == 0;
            })
            .invariant([](const state& s) {
                const auto& val = s.get<int>("counter");
                return val >= -1 && val <= 1;
            })
            .depth_limit(5)
            .ignore_depth_limit_warning()
            .deadlock_timeout(std::chrono::milliseconds(100))
            .continue_on_error()
            .num_threads(4) // Enable multi-threading with more threads
            .run();
            
        CHECK(result); // Should work correctly
    }
}

TEST_CASE("Path queue functionality") {
    using namespace raco;
    
    SUBCASE("Basic path queue operations") {
        path_queue queue;
        
        // Test empty queue
        CHECK(queue.size() == 0);
        CHECK_FALSE(queue.is_finished());
        
        // Add some paths
        std::deque<uint8_t> path1 = {0, 1, 0};
        std::deque<uint8_t> path2 = {1, 0, 1};
        
        queue.push(path1);
        queue.push(std::move(path2));
        
        CHECK(queue.size() == 2);
        
        // Pop paths
        auto popped1 = queue.pop();
        CHECK(popped1.has_value());
        CHECK(popped1.value() == path1);
        CHECK(queue.size() == 1);
        
        auto popped2 = queue.pop();
        CHECK(popped2.has_value());
        CHECK(queue.size() == 0);
        
        // Test finishing
        queue.finish();
        CHECK(queue.is_finished());
        
        auto popped3 = queue.pop();
        CHECK_FALSE(popped3.has_value());
    }
}

TEST_CASE("Complex race condition scenarios") {
    using namespace raco;
    
    SUBCASE("Add/subtract operations") {
        bool result = check()
            .tasks([](state& s) {
                auto& value = s.create<int>("value", 0);
                return std::tuple{
                    add_operation(value),
                    subtract_operation(value)
                };
            })
            .post_condition([](const state& s) {
                return s.get<int>("value") == 0;
            })
            .invariant([](const state& s) {
                const auto& val = s.get<int>("value");
                return val >= -1 && val <= 1;
            })
            .depth_limit(8)
            .ignore_depth_limit_warning()
            .deadlock_timeout(std::chrono::milliseconds(100))
            .continue_on_error()
            .run();
            
        // We expect this to find race conditions
        CHECK_FALSE(result);
    }
    
    SUBCASE("Multi-threaded add/subtract operations") {
        bool result = check()
            .tasks([](state& s) {
                auto& value = s.create<int>("value", 0);
                return std::tuple{
                    add_operation(value),
                    subtract_operation(value)
                };
            })
            .post_condition([](const state& s) {
                return s.get<int>("value") == 0;
            })
            .invariant([](const state& s) {
                const auto& val = s.get<int>("value");
                return val >= -1 && val <= 1;
            })
            .depth_limit(8)
            .ignore_depth_limit_warning()
            .deadlock_timeout(std::chrono::milliseconds(100))
            .continue_on_error()
            .num_threads(3) // Use 3 threads
            .run();
            
        // Should find the same race conditions
        CHECK_FALSE(result);
    }
}

TEST_CASE("Thread count configuration") {
    using namespace raco;
    
    SUBCASE("Default thread count should be 1") {
        // Test that by default we use single-threaded mode
        bool result = check()
            .tasks([](state& s) {
                auto& counter = s.create<int>("counter", 0);
                return std::tuple{
                    increment(counter)
                };
            })
            .post_condition([](const state& s) {
                return s.get<int>("counter") == 1;
            })
            .depth_limit(5)
            .ignore_depth_limit_warning()
            .run();
            
        CHECK(result);
    }
    
    SUBCASE("Can configure different thread counts") {
        for (size_t threads : {1, 2, 3, 4}) {
            bool result = check()
                .tasks([](state& s) {
                    auto& counter = s.create<int>("counter", 0);
                    return std::tuple{
                        increment(counter),
                        decrement(counter)
                    };
                })
                .post_condition([](const state& s) {
                    return s.get<int>("counter") == 0;
                })
                .invariant([](const state& s) {
                    const auto& val = s.get<int>("counter");
                    return val >= -1 && val <= 1;
                })
                .depth_limit(4)
                .ignore_depth_limit_warning()
                .deadlock_timeout(std::chrono::milliseconds(100))
                .continue_on_error()
                .num_threads(threads)
                .run();
                
            // All configurations should find same results
            // For simple increment/decrement, all interleavings should work
            CHECK(result);
        }
    }
}