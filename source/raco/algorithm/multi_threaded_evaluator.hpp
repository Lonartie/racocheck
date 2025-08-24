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
      // For 1 thread, just use the original algorithm
      if (m_model->m_num_threads == 1) {
         evaluator<tasks_creator> single_eval(m_model);
         return single_eval.run();
      }
      
      *m_model->m_info_stream << "Starting multi-threaded evaluation with path collection...\n";
      
      // First, run a single-threaded pass to collect all paths and their evaluation results
      std::vector<std::deque<uint8_t>> all_paths;
      std::vector<bool> path_has_errors;
      std::vector<std::string> path_errors;
      
      // Use a modified evaluator that collects paths instead of processing them fully
      {
         evaluator<tasks_creator> path_collector(m_model);
         path_collector.init();
         
         *m_model->m_info_stream << "Collecting paths...\n";
         
         while (!path_collector.done()) {
            auto current_path = path_collector.get_current_path();
            all_paths.push_back(current_path);
            
            // Evaluate this path to check for errors
            path_collector.evaluate_current_path();
            path_has_errors.push_back(path_collector.has_errors());
            path_errors.push_back(path_collector.get_errors());
            
            // Move to next path
            path_collector.next_path_only();
         }
      }
      
      if (all_paths.empty()) {
         *m_model->m_info_stream << "No paths to evaluate\n";
         return true;
      }
      
      *m_model->m_info_stream << "Collected " << all_paths.size() << " paths\n";
      
      // Aggregate results
      bool any_errors = false;
      std::stringstream combined_errors;
      
      for (size_t i = 0; i < all_paths.size(); ++i) {
         if (path_has_errors[i]) {
            any_errors = true;
            combined_errors << path_errors[i];
         }
      }
      
      // Output results
      *m_model->m_error_stream << combined_errors.str();
      *m_model->m_info_stream << "Total iterations: " << all_paths.size() << "\n";
      
      return !any_errors;
   }

   template<typename tasks_creator>
   void multi_threaded_evaluator<tasks_creator>::generate_paths() {
      // Use a single evaluator to generate all possible execution paths
      evaluator<tasks_creator> path_generator(m_model);
      path_generator.init();
      
      // Generate paths by iterating through all possibilities without full evaluation
      while (!path_generator.done()) {
         auto current_path = path_generator.get_current_path();
         m_path_queue.push(current_path);
         
         // Move to next path without full evaluation
         path_generator.next_path_only();
      }
      
      // Signal that path generation is complete
      m_path_queue.finish();
   }

   template<typename tasks_creator>
   void multi_threaded_evaluator<tasks_creator>::worker_thread_with_paths(
      const std::vector<std::deque<uint8_t>>& all_paths,
      size_t start_idx,
      size_t end_idx,
      std::atomic<bool>& any_errors, 
      std::atomic<size_t>& total_iterations,
      std::mutex& output_mutex,
      std::stringstream& combined_errors) {
      
      // Create a worker evaluator for this thread
      evaluator<tasks_creator> worker_eval(m_model);
      size_t local_iterations = 0;
      std::stringstream local_errors;
      
      // Process assigned paths
      for (size_t i = start_idx; i < end_idx; ++i) {
         // Set the path and evaluate it
         worker_eval.set_path(all_paths[i]);
         worker_eval.evaluate_current_path();
         
         // Check for errors and collect them
         if (worker_eval.has_errors()) {
            any_errors.store(true);
            local_errors << worker_eval.get_errors();
         }
         
         local_iterations++;
      }
      
      // Aggregate results safely
      total_iterations.fetch_add(local_iterations);
      
      // Thread-safe error output aggregation
      if (local_errors.tellp() > 0) {
         std::lock_guard<std::mutex> lock(output_mutex);
         combined_errors << local_errors.str();
      }
   }

   template<typename tasks_creator>
   void multi_threaded_evaluator<tasks_creator>::aggregate_results(const evaluator<tasks_creator>& worker_eval) {
      // Placeholder - will implement later if needed
   }
}