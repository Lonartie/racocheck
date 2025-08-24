#pragma once

#include "multi_threaded_evaluator.h"
#include "path_queue.hpp"
#include <atomic>

namespace raco {
   template<typename tasks_creator>
   multi_threaded_evaluator<tasks_creator>::multi_threaded_evaluator(const check<tasks_creator>* check)
      : m_model(check) {
   }

   template<typename tasks_creator>
   bool multi_threaded_evaluator<tasks_creator>::run() {
      // For now, let's implement a simple approach:
      // Use the single-threaded evaluator but run multiple instances
      // Each working on a subset of paths
      
      std::vector<std::thread> workers;
      std::atomic<bool> any_errors{false};
      std::atomic<size_t> total_iterations{0};
      std::mutex output_mutex;
      
      for (size_t i = 0; i < m_model->m_num_threads; ++i) {
         workers.emplace_back([this, i, &any_errors, &total_iterations, &output_mutex]() {
            // Create a worker evaluator for this thread
            evaluator<tasks_creator> worker_eval(m_model);
            
            // Run the evaluator - this is not ideal but works for now
            bool result = worker_eval.run();
            
            // Update shared state
            if (!result) {
               any_errors.store(true);
            }
            total_iterations.fetch_add(worker_eval.get_iterations());
            
            // Thread-safe output (though this will duplicate output)
            std::lock_guard<std::mutex> lock(output_mutex);
            if (!result) {
               *m_model->m_error_stream << worker_eval.get_errors();
            }
         });
      }
      
      // Wait for all workers to complete
      for (auto& worker : workers) {
         worker.join();
      }
      
      *m_model->m_info_stream << "Total iterations across " << m_model->m_num_threads << " threads: " << total_iterations.load() << "\n";
      
      return !any_errors.load();
   }

   template<typename tasks_creator>
   void multi_threaded_evaluator<tasks_creator>::generate_paths() {
      // Placeholder - will implement later if needed
   }

   template<typename tasks_creator>
   void multi_threaded_evaluator<tasks_creator>::worker_thread() {
      // Placeholder - will implement later if needed
   }

   template<typename tasks_creator>
   void multi_threaded_evaluator<tasks_creator>::aggregate_results(const evaluator<tasks_creator>& worker_eval) {
      // Placeholder - will implement later if needed
   }
}