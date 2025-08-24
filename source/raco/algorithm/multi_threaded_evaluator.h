#pragma once

#include "evaluator.h"
#include "path_queue.h"
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <sstream>
#include <deque>

namespace raco {
   template <typename tasks_creator>
   class multi_threaded_evaluator {
   public:
      explicit multi_threaded_evaluator(const check<tasks_creator>* check);
      
      [[nodiscard]] bool run();
      
   private:
      void generate_paths();
      void worker_thread(std::atomic<bool>& any_errors, 
                        std::atomic<size_t>& total_iterations,
                        std::mutex& output_mutex,
                        std::stringstream& combined_errors);
      void worker_thread_with_paths(const std::vector<std::deque<uint8_t>>& all_paths,
                                   size_t start_idx,
                                   size_t end_idx,
                                   std::atomic<bool>& any_errors, 
                                   std::atomic<size_t>& total_iterations,
                                   std::mutex& output_mutex,
                                   std::stringstream& combined_errors);
      void aggregate_results(const evaluator<tasks_creator>& worker_eval);
      
      const check<tasks_creator>* m_model = nullptr;
      path_queue m_path_queue;
   };
}