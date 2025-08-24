#pragma once

#include <deque>
#include <mutex>
#include <condition_variable>
#include <optional>

namespace raco {
   class path_queue {
   public:
      using path_t = std::deque<uint8_t>;

      path_queue() = default;
      
      void push(const path_t& path);
      void push(path_t&& path);
      std::optional<path_t> pop();
      void finish();
      bool is_finished() const;
      size_t size() const;

   private:
      mutable std::mutex m_mutex;
      std::condition_variable m_cv;
      std::deque<path_t> m_paths;
      bool m_finished = false;
   };
}