#include <iostream>
#include <sstream>
#include <thread>

#define RACO_TEST_ENABLED
#include "source/raco.h"

raco::fun<void> set_5(int& v) {
   RACO_TEST(v = 5);
}

raco::fun<int*> wait_5(int& v) {
   while (!RACO_TEST_INLINE(v == 5)) {
   }
   RACO_CHECKPOINT;
   RACO_TEST_INLINE(v = 20);
   RACO_RETURN 0;
}

int main(int, char**) {
   using namespace raco;
   return check()
          .tasks([](state& s) {
             auto& value = s.create<int>("value", 0);
             return std::tuple{
                set_5(value), wait_5(value)
             };
          })
          .post_condition([](const state& s) {
             return s.get<int>("value") == 20
                    && s.get_return<int*>(1) == &s.get<int>("value");
          })
          .invariant([](const state& s) {
             const auto& val = s.get<int>("value");
             return val == 0 || val == 5 || val == 20;
          })
          .enable_path_pruning()
          .depth_limit(10)
          .ignore_depth_limit_warning()
          .deadlock_timeout(std::chrono::milliseconds(100))
          .info_stream_to(std::cout)
          .error_stream_to(std::cerr)
          .show_info(TIME_ESTIMATE)
          .show_summary(PATH)
          .continue_on_error()
          .run()
             ? EXIT_SUCCESS
             : EXIT_FAILURE;
}
