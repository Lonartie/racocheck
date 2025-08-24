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
      
      // Step 1: Use a modified single-threaded run to collect all execution paths
      // We need to suppress error output during this phase to avoid duplicate output
      std::vector<std::deque<uint8_t>> all_paths;
      
      {
         // Temporarily redirect error stream to suppress output during path collection
         std::ostringstream null_stream;
         std::ostream* original_error_stream = m_model->m_error_stream;
         const_cast<check<tasks_creator>*>(m_model)->m_error_stream = &null_stream;
         
         evaluator<tasks_creator> path_collector(m_model);
         path_collector.init();
         
         // Collect all paths by running the single-threaded algorithm
         // but storing each path instead of evaluating it
         while (!path_collector.done()) {
            all_paths.push_back(path_collector.get_current_path());
            
            // Move to next path (this triggers path evaluation internally, 
            // but we'll re-evaluate later in parallel)
            path_collector.next();
         }
         
         // Restore original error stream
         const_cast<check<tasks_creator>*>(m_model)->m_error_stream = original_error_stream;
      }
      
      if (all_paths.empty()) {
         *m_model->m_info_stream << "No paths to evaluate\n";
         return true;
      }
      
      // Step 2: Distribute paths among worker threads
      std::vector<std::thread> workers;
      std::atomic<bool> any_errors{false};
      std::atomic<size_t> total_iterations{0};
      std::mutex output_mutex;
      std::stringstream combined_errors;
      
      size_t paths_per_thread = (all_paths.size() + m_model->m_num_threads - 1) / m_model->m_num_threads;
      
      for (size_t thread_id = 0; thread_id < m_model->m_num_threads; ++thread_id) {
         size_t start_idx = thread_id * paths_per_thread;
         size_t end_idx = std::min(start_idx + paths_per_thread, all_paths.size());
         
         if (start_idx >= all_paths.size()) break;
         
         workers.emplace_back([this, &all_paths, start_idx, end_idx, &any_errors, &total_iterations, &output_mutex, &combined_errors]() {
            worker_thread_with_paths(all_paths, start_idx, end_idx, any_errors, total_iterations, output_mutex, combined_errors);
         });
      }
      
      // Step 3: Wait for all workers to complete
      for (auto& worker : workers) {
         worker.join();
      }
      
      // Step 4: Output results
      *m_model->m_error_stream << combined_errors.str();
      *m_model->m_info_stream << "Total iterations across " << workers.size() << " threads: " << total_iterations.load() << "\n";
      
      return !any_errors.load();
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