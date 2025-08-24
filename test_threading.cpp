#include <iostream>
#include <sstream>
#include <thread>

#define RACO_TEST_ENABLED
#include "source/raco.h"

raco::fun<int> add_5(int& v) {
   RACO_CHECKPOINT;
   int tmp = v;
   RACO_TEST(tmp = tmp + 5);
   RACO_RETURN RACO_TEST_INLINE(v = tmp);
}

raco::fun<int> sub_5(int& v) {
   RACO_TEST(int tmp = v);
   RACO_TEST(tmp = tmp - 5);
   RACO_TEST(v = tmp);
   RACO_RETURN v;
}

int test_single_threaded() {
   using namespace raco;
   std::cout << "=== Testing Single-threaded Mode ===\n";
   
   auto result = check()
          .tasks([](state& s) {
             auto& value = s.create<int>("value", 0);
             return std::tuple{
                add_5(value), sub_5(value)
             };
          })
          .post_condition([](const state& s) {
             return s.get<int>("value") == 0;
          })
          .invariant([](const state& s) {
             const auto& val = s.get<int>("value");
             return val == 0 || val == 5 || val == -5;
          })
          .depth_limit(10)
          .ignore_depth_limit_warning()
          .deadlock_timeout(std::chrono::milliseconds(100))
          .info_stream_to(std::cout)
          .error_stream_to(std::cerr)
          .continue_on_error()
          .run();
          
   std::cout << "Single-threaded result: " << (result ? "PASS" : "FAIL") << "\n\n";
   return result ? 0 : 1;
}

int test_multi_threaded() {
   using namespace raco;
   std::cout << "=== Testing Multi-threaded Mode (2 threads) ===\n";
   
   auto result = check()
          .tasks([](state& s) {
             auto& value = s.create<int>("value", 0);
             return std::tuple{
                add_5(value), sub_5(value)
             };
          })
          .post_condition([](const state& s) {
             return s.get<int>("value") == 0;
          })
          .invariant([](const state& s) {
             const auto& val = s.get<int>("value");
             return val == 0 || val == 5 || val == -5;
          })
          .depth_limit(10)
          .ignore_depth_limit_warning()
          .deadlock_timeout(std::chrono::milliseconds(100))
          .info_stream_to(std::cout)
          .error_stream_to(std::cerr)
          .continue_on_error()
          .num_threads(2)  // Enable multi-threading
          .run();
          
   std::cout << "Multi-threaded result: " << (result ? "PASS" : "FAIL") << "\n\n";
   return result ? 0 : 1;
}

int main(int, char**) {
   int single_result = test_single_threaded();
   int multi_result = test_multi_threaded();
   
   std::cout << "=== Summary ===\n";
   std::cout << "Single-threaded: " << (single_result == 0 ? "PASS" : "FAIL") << "\n";
   std::cout << "Multi-threaded: " << (multi_result == 0 ? "PASS" : "FAIL") << "\n";
   
   return (single_result != 0 || multi_result != 0) ? 1 : 0;
}