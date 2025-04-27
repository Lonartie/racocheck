#pragma once

#include "raco/core/state.h"
#include "raco/misc/traits.h"
#include "raco/misc/pack.h"
#include <iostream>

namespace raco {
   enum summary_type : uint8_t {
      NO_SUMMARY,
      PATH,
      VISUALIZE
   };

   enum info_level : uint8_t {
      NO_INFO,
      DEBUG,
      TIME_ESTIMATE
   };

   using default_tasks_creator_type = std::function<std::tuple<>(raco::state&)>;

   template<typename tasks_creator = default_tasks_creator_type>
   class check {
      using post_condition_validator = std::function<bool(const raco::state&)>;
      static constexpr auto default_post_condition_validator = [](const auto&) { return true; };

      using invariant_validator = std::function<bool(const raco::state&)>;
      static constexpr auto default_invariant_validator = [](const auto&) { return true; };

      template<typename Any> friend class raco::check;
      template<typename Any> friend class raco::evaluator;

      using tasks_t = return_type_t<tasks_creator, state&>; // tuple<task<A>, task<B>, ...>
      using tasks_pack_t = tuple_to_pack_t<tasks_t>; // pack<task<A>, task<B>, ...>
      using returns_pack_t = typename tasks_pack_t::template transform<task_remover>; // pack<A, B, ...>
      using returns_t = typename returns_pack_t::template to<std::tuple>; // tuple<A, B, ...>

   public:
      template<typename new_tasks_creator>
      auto tasks(new_tasks_creator&& creator) && -> check<decltype(std::function(std::declval<new_tasks_creator>()))>;
      check&& post_condition(post_condition_validator validator) &&;
      check&& invariant(invariant_validator validator) &&;
      check&& enable_path_pruning(bool enable = true) &&;
      check&& deadlock_timeout(std::chrono::nanoseconds s) &&;
      check&& depth_limit(size_t limit) &&;
      check&& info_stream_to(std::ostream& stream) &&;
      check&& error_stream_to(std::ostream& stream) &&;
      check&& show_info(info_level level) &&;
      check&& show_summary(summary_type type) &&;
      check&& ignore_depth_limit_warning() &&;
      check&& continue_on_error() &&;
      [[nodiscard]] bool run() const &&;

   private:
      tasks_creator m_tasks_creator = nullptr;
      post_condition_validator m_post_condition_validator = default_post_condition_validator;
      invariant_validator m_invariant_validator = default_invariant_validator;
      summary_type m_summary_type = NO_SUMMARY;
      info_level m_info_level = NO_INFO;
      size_t m_depth_limit = 1000;
      std::ostream* m_info_stream = &std::cout;
      std::ostream* m_error_stream = &std::cerr;
      std::chrono::nanoseconds m_deadlock_timeout = std::chrono::seconds(1);
      bool m_continue_on_error = false;
      bool m_path_pruning_enabled = false;
      bool m_ignore_depth_limit_warning = false;
   };
}
