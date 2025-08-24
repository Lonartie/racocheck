#pragma once

#include "evaluator.h"
#include "path_queue.h"
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>

namespace raco {
   template <typename tasks_creator>
   class multi_threaded_evaluator {
   public:
      explicit multi_threaded_evaluator(const check<tasks_creator>* check);
      
      [[nodiscard]] bool run();
      
   private:
      void generate_paths();
      void worker_thread();
      void aggregate_results(const evaluator<tasks_creator>& worker_eval);
      
      const check<tasks_creator>* m_model = nullptr;
      path_queue m_path_queue;
      std::atomic<bool> m_has_errors{false};
      std::atomic<size_t> m_total_iterations{0};
      std::mutex m_results_mutex;
      std::stringstream m_aggregated_errors;
   };
}