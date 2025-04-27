#pragma once

#include <cstdint>
#include <chrono>
#include <deque>

#include "raco/core/state.h"

namespace raco {

   template <typename tasks_creator> class check;
   using float_t = long double;

   template <typename tasks_creator>
   class evaluator {
   public:
      static constexpr auto UPDATE_INTERVAL = std::chrono::seconds(1);
      explicit evaluator(const check<tasks_creator>* check);

      [[nodiscard]] bool run();
      [[nodiscard]] bool done() const;
      [[nodiscard]] bool result() const;
      void next();
      void init();

   private:
      void observe_exec();
      void check_post_condition();
      void check_invariant();
      void next_state();
      void reset_state();
      void set_return(size_t index);

      template <typename ... Ts> void log_info(Ts&& ... ts) const;
      template <typename ... Ts> void log_warning(Ts&& ... ts);
      template <typename ... Ts> void log_error(Ts&& ... ts);
      void log_time();

      [[nodiscard]] float_t max_combinations() const;
      [[nodiscard]] float_t current_combination() const;
      [[nodiscard]] std::string stack_string() const;

   private:
      const check<tasks_creator>* m_model = nullptr;
      std::deque<uint8_t> m_stack;
      std::deque<uint8_t> m_last_stack;
      std::vector<std::coroutine_handle<>> m_coros;
      state m_state;
      std::chrono::high_resolution_clock::time_point m_start = std::chrono::high_resolution_clock::now();
      std::chrono::high_resolution_clock::time_point m_last_update = std::chrono::high_resolution_clock::now();
      size_t m_max_depth = 2;
      std::stringstream m_errors;
      size_t m_iterations = 0;
      bool m_end_reached = false;
      bool m_has_errors = false;
   };
}
