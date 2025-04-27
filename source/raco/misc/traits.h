#pragma once

#include <functional>

namespace raco {
   template <typename F, typename ... Args>
   concept is_callable = requires(F f, Args&& ... args) {
      { f(std::forward<Args>(args)...) };
   };

   template <typename F, typename ... Args>
   using return_type_t = std::decay_t<decltype(std::declval<F>()(std::declval<Args>()...))>;

   template <typename T> struct is_tuple : std::false_type {};
   template <typename ... Ts> struct is_tuple<std::tuple<Ts...>> : std::true_type {};
   template <typename T> static constexpr auto is_tuple_v = is_tuple<T>::value;

   template <typename F, typename ... Args>
   static constexpr auto returns_tuple_v = is_tuple_v<return_type_t<F, Args...>>;

   class state;

   template <typename R>
   constexpr size_t group_size_of(std::function<R(state&)>&) {
      return std::tuple_size_v<R>;
   }

   template <size_t Begin, size_t End, size_t Inc = 1, typename F>
   void constexpr_for(F&& f) {
      if constexpr (Begin != End) {
         f.template operator()<Begin>();
         constexpr_for<Begin + 1, End>(std::forward<F>(f));
      }
   }
}