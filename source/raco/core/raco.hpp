#pragma once

#include "raco/core/raco.h"
#include "raco/misc/traits.h"
#include "raco/algorithm/evaluator.h"

namespace raco {
   template<typename tasks_creator>
   template<typename new_tasks_creator>
   auto check<tasks_creator>::tasks(
      new_tasks_creator&& creator) && -> check<decltype(std::function(std::declval<new_tasks_creator>()))> {
      check<decltype(std::function(std::declval<new_tasks_creator>()))> result;
      result.m_tasks_creator = std::function(std::forward<new_tasks_creator>(creator));
      result.m_post_condition_validator = std::move(m_post_condition_validator);
      result.m_invariant_validator = std::move(m_invariant_validator);
      result.m_path_pruning_enabled = m_path_pruning_enabled;
      result.m_deadlock_timeout = m_deadlock_timeout;
      result.m_depth_limit = m_depth_limit;
      result.m_info_stream = m_info_stream;
      result.m_info_level = m_info_level;
      result.m_summary_type = m_summary_type;
      result.m_continue_on_error = m_continue_on_error;
      return result;
   }

   template<typename tasks_creator>
   check<tasks_creator>&& check<tasks_creator>::post_condition(post_condition_validator validator) && {
      m_post_condition_validator = std::move(validator);
      return std::move(*this);
   }

   template<typename tasks_creator>
   check<tasks_creator>&& check<tasks_creator>::invariant(invariant_validator validator) && {
      m_invariant_validator = std::move(validator);
      return std::move(*this);
   }

   template<typename tasks_creator>
   check<tasks_creator>&& check<tasks_creator>::enable_path_pruning(const bool enable) && {
      m_path_pruning_enabled = enable;
      return std::move(*this);
   }

   template<typename tasks_creator>
   check<tasks_creator>&& check<tasks_creator>::deadlock_timeout(std::chrono::nanoseconds s) && {
      m_deadlock_timeout = s;
      return std::move(*this);
   }

   template<typename tasks_creator>
   check<tasks_creator>&& check<tasks_creator>::depth_limit(const size_t limit) && {
      m_depth_limit = limit;
      return std::move(*this);
   }

   template<typename tasks_creator>
   check<tasks_creator>&& check<tasks_creator>::info_stream_to(std::ostream& stream) && {
      m_info_stream = &stream;
      return std::move(*this);
   }

   template<typename tasks_creator>
   check<tasks_creator>&& check<tasks_creator>::error_stream_to(std::ostream& stream) && {
      m_error_stream = &stream;
      return std::move(*this);
   }

   template<typename tasks_creator>
   check<tasks_creator>&& check<tasks_creator>::show_info(info_level level) && {
      m_info_level = level;
      return std::move(*this);
   }

   template<typename tasks_creator>
   check<tasks_creator>&& check<tasks_creator>::show_summary(const summary_type type) && {
      m_summary_type = type;
      return std::move(*this);
   }

   template<typename tasks_creator>
   check<tasks_creator>&& check<tasks_creator>::ignore_depth_limit_warning() && {
      m_ignore_depth_limit_warning = true;
      return std::move(*this);
   }

   template<typename tasks_creator>
   check<tasks_creator>&& check<tasks_creator>::continue_on_error() && {
      m_continue_on_error = true;
      return std::move(*this);
   }

   template<typename tasks_creator>
   bool check<tasks_creator>::run() const && {
      static_assert(raco::is_callable<tasks_creator, raco::state&>,
                    "The tasks creator function must accept only 'state&'!");
      static_assert(!std::is_same_v<tasks_creator, default_tasks_creator_type>,
                    "You have to set the tasks creator function by calling 'tasks(...)'");
      static_assert(raco::returns_tuple_v<tasks_creator, raco::state&>,
                    "The tasks creator function must return a tuple of raco::tasks");
      return raco::evaluator(this).run();
   }
}
