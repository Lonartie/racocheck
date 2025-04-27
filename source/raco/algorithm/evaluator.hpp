#pragma once

#include "evaluator.h"
#include "raco/core/raco.h"
#include <iomanip>

namespace raco {
   template<typename tasks_creator>
   evaluator<tasks_creator>::evaluator(const check<tasks_creator>* check)
      : m_model(check) {
   }

   template<typename tasks_creator>
   bool evaluator<tasks_creator>::run() {
      // 1. init coros and first state
      init();

      // 2. run algorithm
      while (!done()) {
         next();
         log_time();
         m_iterations++;
      }

      // 3. dump output
      *m_model->m_error_stream << m_errors.str() << "\n";
      *m_model->m_info_stream << "Iterations: " << m_iterations << "\n";

      // 4. cleanup
      m_stack.clear();
      m_state = {};
      m_errors.clear();
      reset_state();
      m_iterations = 0;

      // 5. result
      return result();
   }

   template<typename tasks_creator>
   bool evaluator<tasks_creator>::done() const {
      return m_end_reached || (m_has_errors && !m_model->m_continue_on_error);
   }

   template<typename tasks_creator>
   void evaluator<tasks_creator>::next() {
      // 1. try next path
      log_info("Trying path: ", stack_string());

      // 2. observe exec
      observe_exec();

      // 3. set next state for future run
      next_state();
   }

   template<typename tasks_creator>
   void evaluator<tasks_creator>::init() {
      // 1. create initial tasks
      reset_state();

      // 2. create initial path
      m_stack = {0};
   }

   template<typename tasks_creator>
   void evaluator<tasks_creator>::observe_exec() {
      const auto i = m_stack.back();
      const auto handle = m_coros.at(i);
      handle.resume();
      if (m_coros.at(i).done()) {
         set_return(i);
      }
      check_invariant();
   }

   template<typename tasks_creator>
   void evaluator<tasks_creator>::check_post_condition() {
      check_invariant();
      if (m_model->m_post_condition_validator(m_state)) {
         return;
      }

      m_has_errors = true;
      log_error("Post condition not satisfied!\n=== STATE ===\n", m_state, "\n", "=== PATH ===\n", stack_string());
   }

   template<typename tasks_creator>
   void evaluator<tasks_creator>::check_invariant() {
      if (m_model->m_invariant_validator(m_state)) {
         return;
      }

      m_has_errors = true;
      log_error("Invariant not satisfied!\n=== STATE ===\n", m_state, "\n", "=== PATH ===\n", stack_string());
   }

   template<typename tasks_creator>
   void evaluator<tasks_creator>::next_state() {
      // 1. check if we can go deeper
      bool all_done = true;
      for (uint8_t i = 0; i < m_coros.size(); ++i) {
         if (!m_coros.at(i).done()) {
            all_done = false;
            if (m_stack.size() + 1 > m_model->m_depth_limit) {
               if (!m_model->m_ignore_depth_limit_warning) {
                  log_warning("Depth limit exceeded at path: ", stack_string());
               }
               break;
            }
            m_stack.push_back(i);
            m_max_depth = std::max(m_max_depth, m_stack.size());
            return;
         }
      }

      // 2. all coros done, check post condition
      if (all_done) {
         check_post_condition();
      }

      // 3. mutate stack to next state
      while (!m_stack.empty()) {
         const uint8_t max = m_coros.size() - 1;
         while (!m_stack.empty() && m_stack.back() == max) {
            m_stack.erase(m_stack.end() - 1);
         }

         if (m_stack.empty()) {
            break;
         }

         m_stack.back()++;
         reset_state();
         if (!m_coros.at(m_stack.back()).done()) {
            return;
         }
      }

      // 3. no next state available
      m_end_reached = true;
   }

   template<typename tasks_creator>
   void evaluator<tasks_creator>::reset_state() {
      log_info("Resetting state to ", stack_string());

      // 1. cleanup current coros
      for (auto&& coro: m_coros) {
         coro.destroy();
      }
      m_coros.clear();

      // 2. create initial state
      m_state = {};
      auto tasks = m_model->m_tasks_creator(m_state);
      using tasks_t = std::decay_t<decltype(tasks)>;
      auto append_coro = [&, this]<size_t I>() {
         m_coros.push_back(std::get<I>(tasks).handle());
      };
      auto append_coros = [&, this]<size_t ... Is>(std::index_sequence<Is...>) {
         (append_coro.template operator()<Is>(), ...);
      };
      append_coros(std::make_index_sequence<std::tuple_size_v<tasks_t> >());

      // 3. replay stack until before last position
      if (!m_stack.empty()) {
         for (size_t k = 0; k < m_stack.size() - 1; ++k) {
            const auto i = m_stack.at(k);
            m_coros.at(i).resume();
            if (m_coros.at(i).done()) {
               set_return(i);
            }
         }
      }
   }

   template<typename tasks_creator>
   void evaluator<tasks_creator>::set_return(size_t index) {
      auto& handle = m_coros.at(index);
      using ret_tuple = typename check<tasks_creator>::returns_t;
      constexpr auto ret_count = std::tuple_size_v<ret_tuple>;

      constexpr_for<0, ret_count>([&]<size_t i>() {
         if (i == index && !std::is_same_v<void_t, std::tuple_element_t<i, ret_tuple>>) {
            using task_t = task<std::tuple_element_t<i, ret_tuple>>;
            m_state.set_return(i, task_t(handle).result());
         }
      });
   }

   template<typename tasks_creator>
   bool evaluator<tasks_creator>::result() const {
      return !m_has_errors;
   }

   template<typename tasks_creator>
   template<typename... Ts>
   void evaluator<tasks_creator>::log_info(Ts&&... ts) const {
      if (m_model->m_info_level & DEBUG) {
         *m_model->m_info_stream << "[INFO]    : ";
         ((*m_model->m_info_stream << ts), ...) << "\n";
      }
   }

   template<typename tasks_creator>
   template<typename... Ts>
   void evaluator<tasks_creator>::log_warning(Ts&&... ts) {
      m_errors << "[WARNING] : ";
      ((m_errors << ts), ...) << "\n";
   }

   template<typename tasks_creator>
   template<typename... Ts>
   void evaluator<tasks_creator>::log_error(Ts&&... ts) {
      m_errors << "[ERROR]   : ";
      ((m_errors << ts), ...) << "\n";
   }

   template<typename tasks_creator>
   void evaluator<tasks_creator>::log_time() {
      if (done()) {
         *m_model->m_info_stream << "\n" << std::flush;
         return;
      }

      const auto now = std::chrono::high_resolution_clock::now();
      const float_t seconds_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start).count() /
                                      1e3;
      if (!(m_model->m_info_level & TIME_ESTIMATE) ||
          now - m_last_update < UPDATE_INTERVAL) {
         return;
      }

      const auto current = current_combination() + 1;
      const auto max = max_combinations();
      const auto percent_done = current / max;
      const auto estimated_duration_seconds = seconds_elapsed * (1.0 / percent_done);
      const auto estimated_time_left_seconds = estimated_duration_seconds - seconds_elapsed;

      *m_model->m_info_stream
            << "\r"
            << "Percentage: " << std::fixed << std::setprecision(2) << (percent_done * 100) << "% "
            << "Iteration: " << std::fixed << std::setprecision(0) << current << " / " << max << " "
            << "Time left: " << estimated_time_left_seconds << " seconds"
            << std::flush;

      m_last_update = now;
   }

   template<typename tasks_creator>
   float_t evaluator<tasks_creator>::max_combinations() const {
      return std::pow(2.0, m_max_depth);
   }

   template<typename tasks_creator>
   float_t evaluator<tasks_creator>::current_combination() const {
      float_t current = 0;
      size_t depth = m_max_depth - 1;
      for (auto&& i: m_stack) {
         current += i * std::pow<float_t>(m_coros.size(), depth--);
      }
      return current;
   }

   template<typename tasks_creator>
   std::string evaluator<tasks_creator>::stack_string() const {
      std::string r = "{";
      for (size_t i = 0; i < m_stack.size(); ++i) {
         if (i != 0) r += ",";
         r += std::to_string(m_stack.at(i));
      }
      return r + "}";
   }
}
