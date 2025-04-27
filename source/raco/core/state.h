#pragma once

#include <map>
#include <string>
#include <any>

namespace raco {
   class state {
   public:
      template <typename T> T& create(std::string name, T default_value = {});
      template <typename T> const T& get(const std::string& name) const;

      template <typename T> void set_return(size_t index, T&& val);
      template <typename T> const T& get_return(size_t index) const;

      friend std::ostream& operator<<(std::ostream& stream, const state& s);

   private:
      std::map<std::string, std::any> m_map;
      std::map<std::string, std::function<std::string()>> m_printers;
      std::map<size_t, std::any> m_returns;
   };
}
