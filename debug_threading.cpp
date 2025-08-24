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

int main() {
   using namespace raco;
   std::cout << "=== Testing Multi-threaded Mode (Debug) ===\n";
   
   try {
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
             .depth_limit(5)  // Reduced for debugging
             .ignore_depth_limit_warning()
             .deadlock_timeout(std::chrono::milliseconds(100))
             .info_stream_to(std::cout)
             .error_stream_to(std::cerr)
             .continue_on_error()
             .num_threads(2)  // Enable multi-threading
             .run();
             
      std::cout << "Multi-threaded result: " << (result ? "PASS" : "FAIL") << "\n";
   } catch (const std::exception& e) {
      std::cout << "Exception: " << e.what() << "\n";
      return 1;
   } catch (...) {
      std::cout << "Unknown exception occurred\n";
      return 1;
   }
   
   return 0;
}