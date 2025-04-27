#pragma once
#include "state.h"
#include "raco/misc/macros.h"

namespace raco {
   template<typename T>
   T& state::create(std::string name, T default_value) {
      T& value = *std::any_cast<T>(&m_map.emplace(name, std::move(default_value)).first->second);
      m_printers.emplace(std::move(name), [name, this] () -> std::string {
         std::stringstream stream;
         stream << this->get<T>(name);
         return stream.str();
      });
      return value;
   }

   template<typename T>
   const T& state::get(const std::string& name) const {
      return *std::any_cast<T>(&m_map.at(name));
   }

   template<typename T>
   void state::set_return(const size_t index, T&& val) {
      m_returns.emplace(index, std::forward<T>(val));
      m_printers.emplace("return #" + std::to_string(index), [index, this] () -> std::string {
         if constexpr (!std::is_same_v<void_t, T>) {
            std::stringstream stream;
            stream << this->get_return<T>(index);
            return stream.str();
         }
         return "void";
      });
   }

   template<typename T>
   const T& state::get_return(const size_t index) const {
      return *std::any_cast<T>(&m_returns.at(index));
   }

   inline std::ostream& operator<<(std::ostream& stream, const state& s) {
      stream << "raco::state {\n";
      raco::indent();
      for (auto&& [name, printer] : s.m_printers) {
         if (name.find("return #") != std::string::npos) {
            continue;
         }
         stream << raco::nesting() << name << " = " << printer() << "\n";
      }
      for (auto&& [name, printer] : s.m_printers) {
         if (name.find("return #") == std::string::npos) {
            continue;
         }
         stream << raco::nesting() << name << " = " << printer() << "\n";
      }
      raco::unindent();
      return stream << raco::nesting() << "}";
   }
}
