#pragma once

#include "path_queue.h"

namespace raco {
   inline void path_queue::push(const path_t& path) {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_paths.push_back(path);
      m_cv.notify_one();
   }

   inline void path_queue::push(path_t&& path) {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_paths.push_back(std::move(path));
      m_cv.notify_one();
   }

   inline std::optional<path_queue::path_t> path_queue::pop() {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_cv.wait(lock, [this] { return !m_paths.empty() || m_finished; });
      
      if (m_paths.empty()) {
         return std::nullopt;
      }
      
      auto path = std::move(m_paths.front());
      m_paths.pop_front();
      return path;
   }

   inline void path_queue::finish() {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_finished = true;
      m_cv.notify_all();
   }

   inline bool path_queue::is_finished() const {
      std::lock_guard<std::mutex> lock(m_mutex);
      return m_finished && m_paths.empty();
   }

   inline size_t path_queue::size() const {
      std::lock_guard<std::mutex> lock(m_mutex);
      return m_paths.size();
   }
}